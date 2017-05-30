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
#include "utilities/vectorOps.hpp"
#include <cmath>

gridDynGenModel3::gridDynGenModel3 (const std::string &objName) : gridDynGenModelClassical (objName)
{
  // default values

  Xd = 1.05;
}

coreObject *gridDynGenModel3::clone (coreObject *obj) const
{
  gridDynGenModel3 *gd = cloneBase<gridDynGenModel3, gridDynGenModelClassical> (this, obj);
  if (gd == nullptr)
    {
      return obj;
    }
  gd->Xl = Xl;
  gd->Xdp = Xdp;
  gd->Xq = Xq;
  gd->Tdop = Tdop;

  return gd;
}

void gridDynGenModel3::dynObjectInitializeA (coreTime /*time0*/, unsigned long /*flags*/)
{
  offsets.local().local.diffSize = 3;
  offsets.local().local.algSize = 2;
  offsets.local().local.jacSize = 21;

}
// initial conditions
void gridDynGenModel3::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet)
{
  computeInitialAngleAndCurrent (inputs, desiredOutput, Rs, Xq);
  double *gm = m_state.data ();
  // Edp and Eqp
  E = Vd + Rs * gm[0] + (Xq) * gm[1];
  gm[4] = Vq + Rs * gm[1] - (Xdp) * gm[0];


  // record Pm = Pset
  //this should be close to P from above
  double Pmt = E * gm[0] + gm[4] * gm[1] + (Xdp - Xq) * gm[0] * gm[1];
  //Pmt = P;

  // exciter - assign Ef
  double Eft = gm[4] - (Xd - Xdp) * gm[0];
  //preset the inputs that should be initialized
  inputSet[2] = Eft;
  inputSet[3] = Pmt;
}


void gridDynGenModel3::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  double *dv = Loc.destDiffLoc;
  //Get the exciter field
  double Eft = inputs[genModelEftInLocation];
  double Pmt = inputs[genModelPmechInLocation];

  // Id and Iq

  // delta
  dv[0] = m_baseFreq * (gmd[1] - 1.0);
  // Eqp
  dv[2] = (-gmd[2] + Eft + (Xd - Xdp) * gm[0]) / Tdop;

  // omega
  double Pe = gmd[2] * gm[1] + E * gm[0] + (Xdp - Xq) * gm[0] * gm[1];

  dv[1] = 0.5  * (Pmt - Pe - D * (gmd[1] - 1.0)) / H;

}

void gridDynGenModel3::algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double /*alpha*/)
{
  Lp Loc = offsets.getLocations (sD, update, sMode, this);
  updateLocalCache (inputs, sD, sMode);
  solve2x2 (Rs, (Xq), -(Xdp), Rs, -Vd + E, Loc.diffStateLoc[2] - Vq, Loc.destLoc[0], Loc.destLoc[1]);
  m_output = -(Loc.destLoc[1] * Vq + Loc.destLoc[0] * Vd);

}

void gridDynGenModel3::residual (const IOdata &inputs, const stateData &sD, double resid[],  const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);

  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  const double *gmp = Loc.dstateLoc;

  double *rva = Loc.destLoc;
  double *rvd = Loc.destDiffLoc;
  updateLocalCache (inputs, sD, sMode);

  if (hasAlgebraic (sMode))
    {
      rva[0] = Vd + Rs * gm[0] + (Xq) * gm[1] - E;
      rva[1] = Vq + Rs * gm[1] - (Xdp) * gm[0] - gmd[2];
    }

  if (hasDifferential (sMode))
    {
      //Get the exciter field
      double Eft = inputs[genModelEftInLocation];
      double Pmt = inputs[genModelPmechInLocation];

      // Id and Iq


      // delta
      rvd[0] = m_baseFreq * (gmd[1] - 1.0) - gmp[0];
      // Eqp
      rvd[2] = (-gmd[2] + Eft + (Xd - Xdp) * gm[0]) / Tdop - gmp[2];

      // omega
      double Pe = gmd[2] * gm[1] + E * gm[0] + (Xdp - Xq) * gm[0] * gm[1];

      rvd[1] = 0.5  * (Pmt - Pe - D * (gmd[1] - 1.0)) / H - gmp[1];
    }
//

}



void gridDynGenModel3::jacobianElements (const IOdata &inputs, const stateData &sD,
                                         matrixData<double> &ad,
                                         const IOlocs &inputLocs, const solverMode &sMode)
{

  Lp Loc = offsets.getLocations (sD, sMode, this);

  double V = inputs[voltageInLocation];
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
//  const double *gmp = Loc.dstateLoc;

  updateLocalCache (inputs, sD, sMode);

  auto refAlg = Loc.algOffset;
  auto refDiff = Loc.diffOffset;


  auto VLoc = inputLocs[voltageInLocation];
  auto TLoc = inputLocs[angleInLocation];


  if (hasAlgebraic (sMode))
    {
      // P
      if (TLoc != kNullLocation)
        {
          ad.assign (refAlg, TLoc, Vq);
          ad.assign (refAlg + 1, TLoc, -Vd);
        }

      // Q
      if (VLoc != kNullLocation)
        {
          ad.assign (refAlg, VLoc, Vd / V);
          ad.assign (refAlg + 1, VLoc, Vq / V);
        }

      ad.assign (refAlg, refAlg, Rs);
      ad.assign (refAlg, refAlg + 1, (Xq - Xl));



      ad.assign (refAlg + 1, refAlg, -(Xdp - Xl));
      ad.assign (refAlg + 1, refAlg + 1, Rs);
      if (hasDifferential (sMode))
        {
          // Id Differential

          ad.assign (refAlg, refDiff, -Vq);


          ad.assign (refAlg + 1, refDiff, Vd);
          ad.assign (refAlg + 1, refDiff + 2, -1.0);
        }
    }


  if (hasDifferential (sMode))
    {
      // delta
      ad.assign (refDiff, refDiff, -sD.cj);
      ad.assign (refDiff, refDiff + 1, m_baseFreq);

      // omega
      double kVal = -0.5 / H;
      if (hasAlgebraic (sMode))
        {
          //Pe = gm[4] * gm[1] + E*gm[0] + (Xdp - Xq)*gm[0] * gm[1];
          ad.assign (refDiff + 1, refAlg, -0.5  * (E + (Xdp - Xq) * gm[1]) / H);
          ad.assign (refDiff + 1, refAlg + 1, -0.5  * (gmd[2] + (Xdp - Xq) * gm[0]) / H);

          ad.assign (refDiff + 2, refAlg, (Xd - Xdp) / Tdop);
        }

      ad.assign (refDiff + 1, refDiff + 1, -0.5  * D / H - sD.cj);
      ad.assign (refDiff + 1, refDiff + 2, -0.5  * gm[1] / H);

      ad.assignCheckCol (refDiff + 1, inputLocs[genModelPmechInLocation], -kVal);              // governor: Pm

      ad.assign (refDiff + 2, refDiff + 2, -1.0 / Tdop - sD.cj);

      ad.assignCheckCol (refDiff + 2, inputLocs[genModelEftInLocation], 1.0 / Tdop);              // exciter: Ef
    }



}

static const stringVec genModel3Names {
  "id","iq","delta","freq","eqp"
};

stringVec gridDynGenModel3::localStateNames () const
{
  return genModel3Names;
}

// set parameters
void gridDynGenModel3::set (const std::string &param,  const std::string &val)
{
  return gridDynGenModelClassical::set (param, val);
}

void gridDynGenModel3::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "x")
    {
      Xq = val;
      Xd = val;
    }
  else if (param == "xq")
    {
      Xq = val;
    }
  else if (param == "xl")
    {
      Xl = val;
    }
  else if ((param == "xp") || (param == "xdp"))
    {
      Xdp = val;
    }
  else if ((param == "tdop") || (param == "td0p"))
    {
      Tdop = val;
    }
  else if ((param == "top") || (param == "t0p"))
    {
      Tdop = val;
    }
  else
    {
      gridDynGenModelClassical::set (param, val, unitType);
    }

}

