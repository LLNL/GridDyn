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
#include "vectorOps.hpp"

#include <cmath>
#include <complex>



gridDynGenModel5type2::gridDynGenModel5type2 (const std::string &objName) : gridDynGenModel5 (objName)
{

}

gridCoreObject *gridDynGenModel5type2::clone (gridCoreObject *obj) const
{
  gridDynGenModel5type2 *gd = cloneBase<gridDynGenModel5type2, gridDynGenModel5> (this, obj);
  if (gd == nullptr)
    {
      return obj;
    }
  return gd;
}

void gridDynGenModel5type2::objectInitializeA (gridDyn_time /*time0*/, unsigned long /*flags*/)
{
  offsets.local->local.diffSize = 5;
  offsets.local->local.algSize = 2;
  offsets.local->local.jacSize = 40;

}

// initial conditions
void gridDynGenModel5type2::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
  double *gm = m_state.data ();
  computeInitialAngleAndCurrent (args, outputSet, Rs, Xq);

  // Edp and Eqp  and Edpp

  gm[6] = Vq + Rs * gm[1] - (Xdpp) * gm[0];
  gm[4] = Vd + Rs * gm[0] + (Xqpp) * gm[1];

  // exciter - assign Ef
  double Eft = gm[6] - (Xd - Xdpp) * gm[0];

  double drat = Tdopp * (Xdpp + Xl) / (Tdop * (Xqp + Xl));
  gm[5] = gm[6] - (Xdp - Xdpp + drat * (Xd - Xdp)) * gm[0] + Taa / Tdop * Eft;
  // record Pm = Pset
  //this should be close to P from above
  double Pmt = gm[4] * gm[0] + gm[6] * gm[1] + (Xdpp - Xqpp) * gm[0] * gm[1];
  //Pmt = P;
  //preset the inputs that should be initialized
  inputSet[2] = Eft;
  inputSet[3] = Pmt;

}

void gridDynGenModel5type2::derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  double *dv = Loc.destDiffLoc;
  //Get the exciter field
  double Eft = args[genModelEftInLocation];
  double Pmt = args[genModelPmechInLocation];

  double drat = Tdopp * (Xdpp + Xl) / (Tdop * (Xqp + Xl));
  // Id and Iq

  // delta
  dv[0] = m_baseFreq * (gmd[1] - 1.0);
  // Edpp and Eqp
  dv[2] = (-gmd[2] - (Xq - Xqpp) * gm[1]) / Tqopp;
  dv[3] = (-gmd[3] + (Xd - Xdp - drat * (Xd - Xdp)) * gm[0] + (1.0 - Taa / Tdop) * Eft) / Tdop;
  //Eqpp
  dv[4] = (-gmd[4] + gmd[3] + (Xdp - Xdpp + drat * (Xd - Xdp)) * gm[0] + Taa / Tdop * Eft) / Tdopp;
  // omega
  double Pe = gmd[2] * gm[0] + gmd[4] * gm[1] + (Xdpp - Xqpp) * gm[0] * gm[1];

  dv[1] = 0.5  * (Pmt - Pe - D * (gmd[1] - 1.0)) / H;


}


void gridDynGenModel5type2::algebraicUpdate (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double /*alpha*/)
{
  Lp Loc = offsets.getLocations (sD, update, sMode, this);
  updateLocalCache (args, sD, sMode);
  solve2x2 (Rs, (Xqpp), -(Xdpp), Rs, Loc.diffStateLoc[2] - Vd, Loc.diffStateLoc[4] - Vq, Loc.destLoc[0], Loc.destLoc[1]);
  m_output = -(Loc.destLoc[1] * Vq + Loc.destLoc[0] * Vd);
}

void gridDynGenModel5type2::residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);

  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  const double *gmp = Loc.dstateLoc;

  double *rva = Loc.destLoc;
  double *rvd = Loc.destDiffLoc;
  updateLocalCache (args, sD, sMode);


  // Id and Iq
  if (hasAlgebraic (sMode))
    {
      rva[0] = Vd + Rs * gm[0] + (Xqpp) * gm[1] - gmd[2];
      rva[1] = Vq + Rs * gm[1] - (Xdpp) * gm[0] - gmd[4];
    }

  if (hasDifferential (sMode))
    {
      derivative (args, sD, resid, sMode);
      //Get the exciter field

      // delta
      rvd[0] -= gmp[0];
      rvd[1] -= gmp[1];
      rvd[2] -= gmp[2];
      rvd[3] -= gmp[3];
      rvd[4] -= gmp[4];

    }

}



void gridDynGenModel5type2::jacobianElements (const IOdata &args, const stateData *sD,
                                              matrixData<double> &ad,
                                              const IOlocs &argLocs, const solverMode &sMode)
{
  // ad.assign (arrayIndex, RowIndex, ColIndex, value) const
  Lp Loc = offsets.getLocations (sD, sMode, this);

  auto refAlg = Loc.algOffset;
  auto refDiff = Loc.diffOffset;
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  double V = args[voltageInLocation];
  auto VLoc = argLocs[voltageInLocation];
  auto TLoc = argLocs[angleInLocation];

  updateLocalCache (args, sD, sMode);


  // P
  if (hasAlgebraic (sMode))
    {
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
      ad.assign (refAlg, refAlg + 1, (Xqpp));

      ad.assign (refAlg + 1, refAlg, -(Xdpp));
      ad.assign (refAlg + 1, refAlg + 1, Rs);

      if (isAlgebraicOnly (sMode))
        {
          return;
        }

      // Id Differential

      ad.assign (refAlg, refDiff, -Vq);
      ad.assign (refAlg, refDiff + 2, -1.0);

      // Iq differential

      ad.assign (refAlg + 1, refDiff, Vd);
      ad.assign (refAlg + 1, refDiff + 4, -1.0);
    }
  // delta
  ad.assign (refDiff, refDiff, -sD->cj);
  ad.assign (refDiff, refDiff + 1, m_baseFreq);

  // omega
  double kVal = -0.5  / H;
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 1, refAlg, -0.5  * (gmd[2] + (Xdpp - Xqpp) * gm[1]) / H);
      ad.assign (refDiff + 1, refAlg + 1, -0.5  * (gmd[4] + (Xdpp - Xqpp) * gm[0]) / H);
    }
  ad.assign (refDiff + 1, refDiff + 1, -0.5 *  D / H - sD->cj);
  ad.assign (refDiff + 1, refDiff + 2, -0.5  * gm[0] / H);
  ad.assign (refDiff + 1, refDiff + 4, -0.5  * gm[1] / H);

  ad.assignCheckCol (refDiff + 1, argLocs[genModelPmechInLocation], -kVal);          // governor: Pm


  double drat = Tdopp * (Xdpp + Xl) / (Tdop * (Xqp + Xl));

  // Edpp
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 2, refAlg + 1, -(Xq - Xqpp) / Tqopp);
    }
  ad.assign (refDiff + 2, refDiff + 2, -1 / Tqopp - sD->cj);

  // Eqp
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 3, refAlg, (Xd - Xdp - drat * (Xd - Xdp)) / Tdop);
    }
  ad.assign (refDiff + 3, refDiff + 3, -1.0 / Tdop - sD->cj);

  if (argLocs[genModelEftInLocation] != kNullLocation)        //check if exciter exists
    {
      ad.assign (refDiff + 3, argLocs[genModelEftInLocation], (1.0 - Taa / Tdop) / Tdop);       // exciter: Ef
      ad.assign (refDiff + 4, argLocs[genModelEftInLocation], Taa / Tdop / Tdopp);
    }


  //Eqpp
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 4, refAlg, (Xdp - Xdpp + drat * (Xd - Xdp)) / Tdopp);
    }
  ad.assign (refDiff + 4, refDiff + 3, 1.0 / Tdopp);
  ad.assign (refDiff + 4, refDiff + 4, -1.0 / Tdopp - sD->cj);

}

static const stringVec genModel5type2Names {
  "id","iq","delta","freq","edpp","eqp","eqpp"
};

stringVec gridDynGenModel5type2::localStateNames () const
{
  return genModel5type2Names;
}
