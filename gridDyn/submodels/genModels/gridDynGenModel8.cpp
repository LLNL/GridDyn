/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
   * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
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
#include "matrixData.h"
#include "gridCoreTemplates.h"

#include <cmath>

gridDynGenModel8::gridDynGenModel8 (const std::string &objName) : gridDynGenModel6 (objName)
{

}

gridCoreObject *gridDynGenModel8::clone (gridCoreObject *obj) const
{
  gridDynGenModel8 *gd = cloneBase<gridDynGenModel8, gridDynGenModel6> (this, obj);
  if (gd == nullptr)
    {
      return obj;
    }
  return gd;
}

void gridDynGenModel8::objectInitializeA (double /*time0*/, unsigned long /*flags*/)
{
  offsets.local->local.diffSize = 8;
  offsets.local->local.algSize = 2;
  offsets.local->local.jacSize = 47;

}
// initial conditions
void gridDynGenModel8::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
  computeInitialAngleAndCurrent (args, outputSet, Rs, Xq - Xl);
  double *gm = m_state.data ();

  // Edp and Eqp  and Edpp

  gm[7] = Vq + Rs * gm[1] - (Xdpp - Xl) * gm[0];
  gm[6] = Vd + Rs * gm[0] + (Xqpp - Xl) * gm[1];

  double qrat = Tqopp * (Xqpp - Xl) / (Tqop * (Xqp - Xl));
  double drat = Tdopp * (Xdpp - Xl) / (Tdop * (Xdp - Xl));
  gm[4] = gm[6] + (Xqp - Xqpp + qrat * (Xq - Xqp)) * gm[1];

  // record Pm = Pset
  //this should be close to P from above
  double Pmt = gm[6] * gm[0] + gm[7] * gm[1] + (Xdpp - Xqpp) * gm[0] * gm[1];

  // exciter - assign Ef
  double Eft = gm[7] - (Xd - Xdpp) * gm[0];
  //preset the inputs that should be initialized
  inputSet[2] = Eft;
  inputSet[3] = Pmt;

  gm[5] = gm[7] - (Xdp - Xdpp + drat * (Xd - Xdp)) * gm[0] + Taa / Tdop * Eft;

  gm[8] = gm[7] + (Xdpp - Xl) * gm[0];
  gm[9] = -gm[6] + (Xqpp - Xl) * gm[1];

}

void gridDynGenModel8::derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
  if (isAlgebraicOnly (sMode))
    {
      return;
    }
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);

  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  // const double *gmp = Loc.dstateLoc;

  double *rvd = Loc.destDiffLoc;
  updateLocalCache (args, sD, sMode);

  //Get the exciter field
  double Eft = args[genModelEftInLocation];
  double Pmt = args[genModelPmechInLocation];


  double qrat = Tqopp * (Xqpp + Xl) / (Tqop * (Xqp + Xl));
  double drat = Tdopp * (Xdpp + Xl) / (Tdop * (Xdp + Xl));



  rvd[0] = m_baseFreq * (gmd[1] - 1.0);
  // Edp and Eqp
  rvd[2] = (-gmd[2] - (Xq - Xqp - qrat * (Xq - Xqp)) * gm[1]) / Tqop;
  rvd[3] = (-gmd[3] + (Xd - Xdp - drat * (Xd - Xdp)) * gm[0] + (1.0 - Taa / Tdop) * Eft) / Tdop;
  //Edpp
  rvd[4] = (-gmd[4] + gmd[2] - (Xqp - Xqpp + qrat * (Xq - Xqp)) * gm[1]) / Tqopp;
  rvd[5] = (-gmd[5] + gmd[3] + (Xdp - Xdpp + drat * (Xd - Xdp)) * gm[0] + Taa / Tdop * Eft) / Tdopp;
  // omega
  //double Pe = gm[6] * gm[0] + gm[7] * gm[1] + (Xdpp - Xqpp)*gm[0] * gm[1];
  double Pe2 = gmd[6] * gm[1] - gmd[7] * gm[0];
  rvd[1] = 0.5  * (Pmt - Pe2 - D * (gmd[1] - 1.0)) / H;
  //psid and psiq
  rvd[6] = m_baseFreq * (Vd + Rs * gm[0] + gmd[1] * gmd[7]);
  rvd[7] = m_baseFreq * (Vq + Rs * gm[1] - gmd[1] * gmd[6]);

}

void gridDynGenModel8::residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);

  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  const double *gmp = Loc.dstateLoc;

  double *rva = Loc.destLoc;
  double *rvd = Loc.destDiffLoc;
  updateLocalCache (args, sD, sMode);

  if (hasAlgebraic (sMode))
    {
      // Id and Iq
      rva[0] = Vd + Rs * gm[0] + (Xqpp - Xl) * gm[1] - gmd[4];
      rva[1] = Vq + Rs * gm[1] - (Xdpp - Xl) * gm[0] - gmd[5];
    }
  if (hasDifferential (sMode))
    {
      derivative (args, sD, resid, sMode);
      /// delta
      rvd[0] -= gmp[0];
      rvd[1] -= gmp[1];
      rvd[2] -= gmp[2];
      rvd[3] -= gmp[3];
      rvd[4] -= gmp[4];
      rvd[5] -= gmp[5];
      rvd[6] -= gmp[6];
      rvd[7] -= gmp[7];
    }


}


void gridDynGenModel8::jacobianElements (const IOdata &args, const stateData *sD,
                                         matrixData<double> &ad,
                                         const IOlocs &argLocs, const solverMode &sMode)
{
  // ad.assign (arrayIndex, RowIndex, ColIndex, value) const
  Lp Loc = offsets.getLocations (sD,nullptr, sMode, this);

  double V = args[voltageInLocation];
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
//  const double *gmp = Loc.dstateLoc;

  updateLocalCache (args, sD, sMode);

  auto refAlg = Loc.algOffset;
  auto refDiff = Loc.diffOffset;


  auto VLoc = argLocs[voltageInLocation];
  auto TLoc = argLocs[angleInLocation];



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
      ad.assign (refAlg, refAlg + 1, (Xqpp - Xl));



      ad.assign (refAlg + 1, refAlg, -(Xdpp - Xl));
      ad.assign (refAlg + 1, refAlg + 1, Rs);

      if (isAlgebraicOnly (sMode))
        {
          return;
        }

      // Id Differential

      ad.assign (refAlg, refDiff, -Vq);
      ad.assign (refAlg, refDiff + 4, -1.0);


      // Iq Differential

      ad.assign (refAlg + 1, refDiff, Vd);
      ad.assign (refAlg + 1, refDiff + 5, -1.0);
    }
  // delta
  ad.assign (refDiff, refDiff, -sD->cj);
  ad.assign (refDiff, refDiff + 1, m_baseFreq);

  // omega
  //Pe2 = gm[8] * gm[1] - gm[9] * gm[0];
  double kVal = -0.5  / H;
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 1, refAlg, 0.5  * (gmd[7]) / H);
      ad.assign (refDiff + 1, refAlg + 1, -0.5  * (gmd[6]) / H);
    }
  ad.assign (refDiff + 1, refDiff + 1, -0.5  * D / H - sD->cj);
  ad.assign (refDiff + 1, refDiff + 6, -0.5  * gm[1] / H);
  ad.assign (refDiff + 1, refDiff + 7, 0.5  * gm[0] / H);

  ad.assignCheckCol (refDiff + 1, argLocs[genModelPmechInLocation], -kVal);           // governor: Pm

  double qrat = Tqopp * (Xqpp - Xl) / (Tqop * (Xqp - Xl));
  double drat = Tdopp * (Xdpp - Xl) / (Tdop * (Xdp - Xl));

  // Edp
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 2, refAlg + 1, -(Xq - Xqp - qrat * (Xq - Xqp)) / Tqop);
    }
  ad.assign (refDiff + 2, refDiff + 2, -1.0 / Tqop - sD->cj);

  // Eqp
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 3, refAlg, (Xd - Xdp - drat * (Xd - Xdp)) / Tdop);
    }
  ad.assign (refDiff + 3, refDiff + 3, -1.0 / Tdop - sD->cj);


  if (argLocs[genModelEftInLocation] != kNullLocation)      //check if exciter exists
    {
      ad.assign (refDiff + 3, argLocs[genModelEftInLocation], (1.0 - Taa / Tdop) / Tdop);       // exciter: Ef
      ad.assign (refDiff + 5, argLocs[genModelEftInLocation], Taa / Tdop / Tdopp);
    }
  //Edpp
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 4, refAlg + 1, -(Xqp - Xqpp + qrat * (Xq - Xqp)) / Tqopp);
    }
  ad.assign (refDiff + 4, refDiff + 2, 1.0 / Tqopp);
  ad.assign (refDiff + 4, refDiff + 4, -1.0 / Tqopp - sD->cj);

  //Eqpp
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 5, refAlg, (Xdp - Xdpp + drat * (Xd - Xdp)) / Tdopp);
    }
  ad.assign (refDiff + 5, refDiff + 3, 1.0 / Tdopp);
  ad.assign (refDiff + 5, refDiff + 5, -1.0 / Tdopp - sD->cj);

  /*
  rv[8] = m_baseFreq*(Vd + Rs*gm[0] + gm[3] / m_baseFreq*gm[9]) - gmp[8];
  rv[9] = m_baseFreq*(Vq + Rs*gm[1] - gm[3] / m_baseFreq*gm[8]) - gmp[9];
  */
  //psib and psiq
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 6, refAlg, Rs * m_baseFreq);
    }
  ad.assign (refDiff + 6, refDiff + 1, gmd[7] * m_baseFreq);
  ad.assign (refDiff + 6, refDiff + 6, -sD->cj);
  ad.assign (refDiff + 6, refDiff + 7, gmd[1] * m_baseFreq);
  ad.assign (refDiff + 6, refDiff, -Vq * m_baseFreq);

  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 7, refAlg + 1, Rs * m_baseFreq);
    }
  ad.assign (refDiff + 7, refDiff + 1, -gmd[6] * m_baseFreq);
  ad.assign (refDiff + 7, refDiff + 6, -gmd[1] * m_baseFreq);
  ad.assign (refDiff + 7, refDiff + 7, -sD->cj);
  ad.assign (refDiff + 7, refDiff, Vd * m_baseFreq);

  if (VLoc != kNullLocation)
    {
      ad.assign (refDiff + 6, VLoc, Vd / V * m_baseFreq);
      ad.assign (refDiff + 7, VLoc, Vq / V * m_baseFreq);
    }
  if (TLoc != kNullLocation)
    {
      ad.assign (refDiff + 6, TLoc, Vq * m_baseFreq);
      ad.assign (refDiff + 7, TLoc, -Vd * m_baseFreq);
    }

}

static const stringVec genModel8Names{ "id","iq","delta","freq","edp","eqp","edpp","eqpp","psid","psiq" };

stringVec gridDynGenModel8::localStateNames() const
{
	return genModel8Names;
}