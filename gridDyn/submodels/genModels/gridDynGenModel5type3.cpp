/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
   * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#include "submodels/otherGenModels.h"
#include "generators/gridDynGenerator.h"
#include "gridBus.h"
#include  "utilities/matrixData.h"
#include "core/coreObjectTemplates.h"

#include <cmath>

gridDynGenModel5type3::gridDynGenModel5type3 (const std::string &objName) : gridDynGenModel3 (objName)
{

}

coreObject *gridDynGenModel5type3::clone (coreObject *obj) const
{
  gridDynGenModel5type3 *gd = cloneBase<gridDynGenModel5type3, gridDynGenModel3> (this, obj);
  if (gd == nullptr)
    {
      return obj;
    }
  return gd;
}


void gridDynGenModel5type3::dynObjectInitializeA (coreTime /*time0*/, unsigned long /*flags*/)
{
  offsets.local().local.diffSize = 5;
  offsets.local().local.algSize = 2;
  offsets.local().local.jacSize = 40;

}
// initial conditions
void gridDynGenModel5type3::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet)
{
  computeInitialAngleAndCurrent (inputs, desiredOutput, Rs, Xq);
  double *gm = m_state.data ();

  gm[5] = Vq + Rs * gm[1];
  gm[6] = -(Vd + Rs * gm[0]);

  // Edp and Eqp

  gm[4] = gm[5] - (Xd) * gm[0];


  // record Pm = Pset
  //this should be close to P from above
  double Pmt = gm[5] * gm[1] - gm[6] * gm[0];

  // exciter - assign Ef
  double Eft = gm[4];
  //preset the inputs that should be initialized
  inputSet[2] = Eft;
  inputSet[3] = Pmt;


}

void gridDynGenModel5type3::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  const double *gmp = Loc.dstateLoc;
  double *dv = Loc.destDiffLoc;
  //Get the exciter field
  double Eft = inputs[genModelEftInLocation];
  double Pmt = inputs[genModelPmechInLocation];
  updateLocalCache (inputs, sD, sMode);
  // Id and Iq

  dv[0] = m_baseFreq * (gmd[1] - 1.0);
  // Eqp
  //rv[4] = (-gm[4] + Eft + (Xd - Xdp)*gm[0]) / Tdop - gmp[4];
  dv[2] = (Xd) / (Xdp) * ((Eft - gmd[2]) / Tdop - (Xd - Xdp) / (Xd) * gmp[3]);
  // omega
  double Pe2 = gmd[3] * gm[1] - gmd[4] * gm[0];
  dv[1] = 0.5 / H * (Pmt - Pe2 - D * (gmd[1] - 1.0));

  //psid and psiq
  dv[3] = m_baseFreq * (Vd + Rs * gm[0] + gmd[1] * gmd[4]);
  dv[4] = m_baseFreq * (Vq + Rs * gm[1] - gmd[1] * gmd[3]);


}


void gridDynGenModel5type3::residual (const IOdata &inputs, const stateData &sD, double resid[],  const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);


  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  const double *gmp = Loc.dstateLoc;

  double *rva = Loc.destLoc;
  double *rvd = Loc.destDiffLoc;


  // Id and Iq
  if (hasAlgebraic (sMode))
    {
      rva[0] = gmd[3] - gmd[2] - (Xd) * gm[0];
      rva[1] = gmd[4] - (Xq) * gm[1];
    }
  if (hasDifferential (sMode))
    {
      derivative (inputs, sD, resid, sMode);
      //Get the exciter field

      // delta
      rvd[0] -= gmp[0];
      rvd[1] -= gmp[1];
      rvd[2] -= gmp[2];
      rvd[3] -= gmp[3];
      rvd[4] -= gmp[4];
    }

}



void gridDynGenModel5type3::jacobianElements (const IOdata &inputs, const stateData &sD,
                                              matrixData<double> &ad,
                                              const IOlocs &inputLocs, const solverMode &sMode)
{
  //use the ad.assign Macro defined in basicDefs
  // ad.assign (arrayIndex, RowIndex, ColIndex, value) const
  Lp Loc = offsets.getLocations (sD, sMode, this);

  double V = inputs[voltageInLocation];
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  // const double *gmp = Loc.dstateLoc;

  updateLocalCache (inputs, sD, sMode);

  auto refAlg = Loc.algOffset;
  auto refDiff = Loc.diffOffset;


  auto VLoc = inputLocs[voltageInLocation];
  auto TLoc = inputLocs[angleInLocation];

  // P
  if (hasAlgebraic (sMode))
    {
      ad.assign (refAlg, refAlg, -(Xd));
      ad.assign (refAlg + 1, refAlg + 1, -(Xq));

      if (!hasDifferential (sMode))
        {
          return;
        }
      //Id Differential
      ad.assign (refAlg, refDiff + 2, -1.0);
      ad.assign (refAlg, refDiff + 3, 1.0);
      // Iq Differential

      ad.assign (refAlg + 1, refDiff + 4, 1.0);

    }




  // delta
  ad.assign (refDiff, refDiff, -sD.cj);
  ad.assign (refDiff, refDiff + 1, m_baseFreq);

  // omega
  double kVal = -0.5  / H;
  //rv[3] = 0.5*m_baseFreq / H*(Pmt - Pe2 - D*(gm[3] / m_baseFreq - 1.0)) - gmp[3];
  //Pe = gm[5] * gm[1] - gm[6] * gm[0];
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 1, refAlg, 0.5  * (gm[6]) / H);
      ad.assign (refDiff + 1, refAlg + 1, -0.5  * (gm[5]) / H);
    }
  ad.assign (refDiff + 1, refDiff + 1, -0.5  * D / H - sD.cj);
  ad.assign (refDiff + 1, refDiff + 3, -0.5  * gm[1] / H);
  ad.assign (refDiff + 1, refDiff + 4, 0.5  * gm[0] / H);



  ad.assignCheckCol (refDiff + 1 + 3, inputLocs[genModelPmechInLocation], -kVal);           // governor: Pm


  //  rvd[2] = drat * ((Eft - gmd[2]) / Tdop - drat2 * gmp[3]) - gmp[2];

  double drat = (Xd) / (Xdp);
  double drat2 = (Xd - Xdp) / (Xd);
  ad.assign (refDiff + 2, refDiff + 2, -drat / Tdop - sD.cj);
  ad.assign (refDiff + 2, refDiff + 3, -drat * drat2 * sD.cj);

  ad.assignCheckCol (refDiff + 2, inputLocs[genModelEftInLocation], drat / Tdop);           // exciter: Ef

  // rvd[3] = m_baseFreq * (Vd + Rs * gm[0] + gmd[1] / m_baseFreq * gmd[4]) - gmp[3];
  //rvd[4] = m_baseFreq * (Vq + Rs * gm[1] - gmd[2] / m_baseFreq * gmd[3]) - gmp[4];

  //psib and psiq
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 3, refAlg, Rs * m_baseFreq);
    }
  ad.assign (refDiff + 3, refDiff + 1, gmd[4] * m_baseFreq);
  ad.assign (refDiff + 3, refDiff + 3, -sD.cj);
  ad.assign (refDiff + 3, refDiff + 4, gmd[1] * m_baseFreq);
  ad.assign (refDiff + 3, refDiff, -Vq * m_baseFreq);

  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 4, refAlg + 1, Rs * m_baseFreq);
    }
  ad.assign (refDiff + 4, refDiff + 1, -gmd[3] * m_baseFreq);
  ad.assign (refDiff + 4, refDiff + 3, -gmd[1] * m_baseFreq);
  ad.assign (refDiff + 4, refDiff + 4, -sD.cj);
  ad.assign (refDiff + 4, refDiff, Vd * m_baseFreq);

  if (VLoc != kNullLocation)
    {
      ad.assign (refDiff + 3, VLoc, Vd / V * m_baseFreq);
      ad.assign (refDiff + 4, VLoc, Vq / V * m_baseFreq);
    }
  if (TLoc != kNullLocation)
    {
      ad.assign (refDiff + 3, TLoc, Vq * m_baseFreq);
      ad.assign (refDiff + 4, TLoc, -Vd * m_baseFreq);
    }

}

static const stringVec genModel5type3Names{ "id","iq","delta","freq","eqp","psid","psiq" };

stringVec gridDynGenModel5type3::localStateNames() const
{
	return genModel5type3Names;
}