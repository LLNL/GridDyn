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
#include "utilities/vectorOps.hpp"
#include  "utilities/matrixData.h"
#include "core/coreObjectTemplates.h"

#include <cmath>

gridDynGenModelGENROU::gridDynGenModelGENROU (const std::string &objName) : gridDynGenModel5 (objName)
{
  // default values
  Xdpp = 0.2;
  Xqpp = 0.2;
  Xqp = 0.30;
  D = 0.03;

}

coreObject *gridDynGenModelGENROU::clone (coreObject *obj) const
{
  gridDynGenModelGENROU *gd = cloneBase<gridDynGenModelGENROU, gridDynGenModel5> (this, obj);
  if (gd == nullptr)
    {
      return obj;
    }
  return gd;
}

void gridDynGenModelGENROU::dynObjectInitializeA (coreTime /*time0*/, unsigned long /*flags*/)
{
  offsets.local().local.diffSize = 6;
  offsets.local().local.algSize = 2;
  offsets.local().local.jacSize = 37;

}
// initial conditions
void gridDynGenModelGENROU::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet)
{
  computeInitialAngleAndCurrent (inputs, desiredOutput, Rs, Xq);
  double *gm = m_state.data ();

  double D1 = (Xdpp) / (Xdp);
  double Q1 = (Xqpp) / (Xqp);

  double D2 = (Xdp - Xdpp) / (Xdp);
  double Q2 = (Xqp - Xqpp) / (Xqp);

  gm[4] = -(Xq - Xqp) * gm[1];

  // psiq

  gm[7] = -gm[4] - (Xqp) * gm[1];

  gm[6] = ((Xdpp + Xl) * gm[0] + (Xdpp) * gm[0] - Vq - Rs * gm[1]) / (D2 - D1);

  gm[5] = gm[6] + (Xd) * gm[0];
  // record Pm = Pset
  //this should be close to P from above
  double Pmt = D1 * gm[5] * gm[0] - D2 * gm[6] * gm[0] - Q1 * gm[4] * gm[0] + Q2 * gm[7] * gm[0];

  // exciter - assign Ef
  double Eft = gm[5] - (Xd - Xdp) * gm[0];


  // double g4 = gm[6] + (Xqp - Xqpp + qrat * (Xq - Xqp)) * gm[1];
  //preset the inputs that should be initialized
  inputSet[2] = Eft;
  inputSet[3] = Pmt;

}

void gridDynGenModelGENROU::algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double /*alpha*/)
{
  Lp Loc = offsets.getLocations (sD,update, sMode, this);
  updateLocalCache (inputs, sD, sMode);
  solve2x2 (Rs, (Xqpp + Xl), -(Xdpp + Xl), Rs, Loc.diffStateLoc[4] - Vd, Loc.diffStateLoc[5] - Vq, Loc.destLoc[0], Loc.destLoc[1]);
  m_output = -(Loc.destLoc[1] * Vq + Loc.destLoc[0] * Vd);
}


void gridDynGenModelGENROU::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);

  //double V = inputs[voltageInLocation];
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  const double *gmp = Loc.dstateLoc;

//  double *rva = Loc.destLoc;
  double *rvd = Loc.destDiffLoc;
//  double angle = gmd[0] - inputs[angleInLocation];
// double Vq = V * cos (angle);
// double Vd = -V*sin (angle);

  //Get the exciter field
  double Eft = inputs[genModelEftInLocation];
  double Pmt = inputs[genModelPmechInLocation];


  double D1 = (Xdpp) / (Xdp);
  double Q1 = (Xqpp) / (Xqp);

  double D2 = (Xdp - Xdpp) / (Xdp);
  double Q2 = (Xqp - Xqpp) / (Xqp);



  // delta
  rvd[0] = m_baseFreq * (gmd[1] - 1.0);
  // Edp and Eqp
  rvd[2] = (-gmd[2] - (Xq - Xqp) * (gm[1] - (Q2 / (Xqp)) * (gmd[5] + (Xqp) * gm[1] + gmd[2]))) / Tqop;

  rvd[3] = (-gmd[3] - (Xd - Xdp) * (gm[0] - (D2 / (Xdp)) * (gmd[4] + (Xdp) * gm[0] + gmd[3])) + Eft) / Tdop;
  //psid
  rvd[4] = (-gmd[4] + gmd[3] - (Xdp) * gm[0]) / Tdopp;

  rvd[5] = (-gmd[5] - gmd[2] - (Xqp) * gm[1]) / Tqopp;
  // omega

  double Pe = D1 * gmd[3] * gm[1] + D2 * gmd[4] * gm[1] - Q1 * gmd[2] * gm[0] + Q2 * gmd[5] * gm[0];

  rvd[1] = 0.5  * (Pmt - Pe - D * (gmd[1] - 1.0)) / H;
  // Edp and Eqp
  if (std::abs (gmp[1]) > 0.0001)
    {
      printf ("[%d]t=%f gmp[1]=%f Pdiff=%f A=%f, B=%f, C=%f Xdp=%3.3f, Xqp=%3.3f\n", static_cast<int>(getID ()), static_cast<double>(sD.time), gmp[1], Pmt - Pe, gmd[4] * gm[0], gmd[5] * gm[1], (Xdpp - Xqpp) * gm[0] * gm[1], Xdp, Xqp);
    }
}


void gridDynGenModelGENROU::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);

  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  const double *gmp = Loc.dstateLoc;
  double *rva = Loc.destLoc;
  double *rvd = Loc.destDiffLoc;
  updateLocalCache (inputs, sD, sMode);

  double D1 = (Xdpp) / (Xdp);
  double Q1 = (Xqpp) / (Xqp);

  double D2 = (Xdp - Xdpp) / (Xdp);
  double Q2 = (Xqp - Xqpp) / (Xqp);
  // Id and Iq
  if (hasAlgebraic (sMode))
    {
      rva[0] = Vd + Rs * gm[0] + (Xqpp + Xl) * gm[1] - Q1 * gmd[2] + Q2 * gmd[5];
      rva[1] = Vq + Rs * gm[1] - (Xdpp + Xl) * gm[0] - D1 * gmd[3] + D2 * gmd[4];
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
      rvd[5] -= gmp[5];
    }


}



void gridDynGenModelGENROU::jacobianElements (const IOdata &inputs, const stateData &sD,
                                              matrixData<double> &ad,
                                              const IOlocs &inputLocs, const solverMode &sMode)
{
  Lp Loc = offsets.getLocations  (sD, sMode, this);

  double V = inputs[voltageInLocation];
//  double theta = inputs[angleInLocation];
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
//  const double *gmp = Loc.dstateLoc;

  updateLocalCache (inputs, sD, sMode);


  auto refAlg = Loc.algOffset;
  auto refDiff = Loc.diffOffset;


  auto VLoc = inputLocs[voltageInLocation];
  auto TLoc = inputLocs[angleInLocation];

  // Id Differential
  double D1 = (Xdpp) / (Xdp);
  double Q1 = (Xqpp) / (Xqp);

  double D2 = (Xdp - Xdpp) / (Xdp);
  double Q2 = (Xqp - Xqpp) / (Xqp);

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
      ad.assign (refAlg, refAlg + 1, (Xqpp + Xl));



      ad.assign (refAlg + 1, refAlg, -(Xdpp + Xl));
      ad.assign (refAlg + 1, refAlg + 1, Rs);
      if (isAlgebraicOnly (sMode))
        {
          return;
        }

      //rva[0] = Vd + Rs * gm[0] + (Xqpp)* gm[1] - qrat*gmd[2] + (Xqp - Xqpp) / (Xqp)*gmd[5];
      //rva[1] = Vq + Rs * gm[1] - (Xdpp)* gm[0] - drat*gmd[3] + (Xdp - Xdpp) / (Xdp)*gmd[4];



      ad.assign (refAlg, refDiff, -Vq);
      ad.assign (refAlg, refDiff + 2, -Q1);
      ad.assign (refAlg, refDiff + 5, Q2);
      // Iq Differential

      ad.assign (refAlg + 1, refDiff, Vd);
      ad.assign (refAlg + 1, refDiff + 3, -D1);
      ad.assign (refAlg + 1, refDiff + 4, D2);
    }
  // delta
  ad.assign (refDiff, refDiff, -sD.cj);
  ad.assign (refDiff, refDiff + 1, m_baseFreq);

  //double Pe =D1*gmd[3] * gm[1] + D2*gmd[4] * gm[1] - Q1*gmd[2] * gm[0] + Q2*gmd[5] * gm[0];

  //rvd[1] = 0.5  * (Pmt - Pe - D * (gmd[1] - 1.0)) / H;
  // omega
  double kVal = -0.5 / H;
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 1, refAlg, -0.5  * (-Q1 * gmd[2] + Q2 * gmd[5]) / H);
      ad.assign (refDiff + 1, refAlg + 1, -0.5  * (D1 * gmd[3] + D2 * gmd[4]) / H);
    }

  ad.assign (refDiff + 1, refDiff + 1, -0.5  * D / H - sD.cj);
  ad.assign (refDiff + 1, refDiff + 2, -0.5 * (-Q1 * gm[0]) / H);
  ad.assign (refDiff + 1, refDiff + 3, -0.5 * (D1 * gm[1]) / H);
  ad.assign (refDiff + 1, refDiff + 4, -0.5  * D2 * gm[1] / H);
  ad.assign (refDiff + 1, refDiff + 5, -0.5  * Q2 * gm[0] / H);


  ad.assignCheckCol (refDiff + 1, inputLocs[genModelPmechInLocation], -kVal); // governor: Pm

  // Edp and Eqp
  //rvd[2] =  (-gmd[2] - (Xq - Xqp) *(gm[1]-(Q2/(Xqp))*(gmd[5]+(Xqp)*gm[1]+gmd[2]))) / Tqop;

  //rvd[3] = (-gmd[3] - (Xd - Xdp) *(gm[0] - (D2/(Xdp))*(gmd[4] + (Xdp)*gm[0] + gmd[3]))+Eft) / Tdop;

  // Edp
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 2, refAlg + 1, (-(Xq - Xqp) * (1.0 - Q2)) / Tqop);
    }
  ad.assign (refDiff + 2, refDiff + 5, Q2 * (Xq - Xqp) / (Xqp) / Tqop);
  ad.assign (refDiff + 2, refDiff + 2, (-1.0 + Q2 * (Xq - Xqp) / (Xqp)) / Tqop - sD.cj);

  // Eqp
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 3, refAlg, (-(Xd - Xdp) * (1.0 - D2)) / Tdop);
    }
  ad.assign (refDiff + 3, refDiff + 4, D2 * (Xd - Xdp) / (Xdp) / Tdop);
  ad.assign (refDiff + 3, refDiff + 3, (-1.0 + D2 * (Xd - Xdp) / (Xdp)) / Tdop - sD.cj);

  ad.assignCheckCol (refDiff + 3, inputLocs[genModelEftInLocation], 1.0 / Tdop); // exciter: Ef

  //psid
  //rvd[4] = (-gmd[4] + gmd[3] - (Xdp)*gm[0]) / Tdopp;
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 4, refAlg, -(Xdp) / Tdopp);
    }
  ad.assign (refDiff + 4, refDiff + 3, 1.0 / Tdopp);
  ad.assign (refDiff + 4, refDiff + 4, -1.0 / Tdopp - sD.cj);


  //psiq rvd[5] = (-gmd[5] - gmd[2] - (Xqp)*gm[1]) / Tqopp;
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 5, refAlg + 1, -(Xqp) / Tqopp);
    }
  ad.assign (refDiff + 5, refDiff + 2, -1.0 / Tqopp);
  ad.assign (refDiff + 5, refDiff + 5, -1.0 / Tqopp - sD.cj);

}

static const stringVec genModelGenRouNames {
  "id","iq","delta","freq","edp","eqp","psid","psiq"
};

stringVec gridDynGenModelGENROU::localStateNames () const
{
  return genModelGenRouNames;
}


