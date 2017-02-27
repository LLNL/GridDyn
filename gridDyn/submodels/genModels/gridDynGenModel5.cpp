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
#include "matrixData.h"
#include "core/coreObjectTemplates.h"
#include "vectorOps.hpp"

#include <cmath>
#include <complex>



gridDynGenModel5::gridDynGenModel5 (const std::string &objName) : gridDynGenModel4 (objName)
{

}

coreObject *gridDynGenModel5::clone (coreObject *obj) const
{
  gridDynGenModel5 *gd = cloneBase<gridDynGenModel5, gridDynGenModel4> (this, obj);
  if (gd == nullptr)
    {
      return obj;
    }
  gd->Tqopp = Tqopp;
  gd->Taa = Taa;
  gd->Tdopp = Tdopp;
  gd->Xdpp = Xdpp;
  gd->Xqpp = Xqpp;
  return gd;
}

void gridDynGenModel5::dynObjectInitializeA (coreTime /*time0*/, unsigned long /*flags*/)
{
  offsets.local().local.diffSize = 5;
  offsets.local().local.algSize = 2;
  offsets.local().local.jacSize = 40;

}
// initial conditions
void gridDynGenModel5::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet)
{
  double *gm = m_state.data ();
  computeInitialAngleAndCurrent (inputs, desiredOutput, Rs, Xq);

  // Edp and Eqp  and Edpp

  gm[5] = Vq + Rs * gm[1] - (Xdp) * gm[0];
  gm[6] = Vd + Rs * gm[0] + (Xqp) * gm[1];

  double xrat = Tqopp * (Xdp + Xl) / (Tqop * (Xqp + Xl));
  gm[4] = gm[6] + (Xqp - Xdp + xrat * (Xq - Xqp)) * gm[1];

  // record Pm = Pset
  //this should be close to P from above
  double Pmt = gm[6] * gm[0] + gm[5] * gm[1] + (Xdp - Xqp) * gm[0] * gm[1];
  // exciter - assign Ef
  double Eft = gm[5] - (Xd - Xdp) * gm[0];
  //preset the inputs that should be initialized
  inputSet[2] = Eft;
  inputSet[3] = Pmt;

}

void gridDynGenModel5::algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double /*alpha*/)
{
  Lp Loc = offsets.getLocations (sD, update, sMode, this);
  updateLocalCache (inputs, sD, sMode);
  solve2x2 (Rs, (Xqp), -(Xdp), Rs, Loc.diffStateLoc[4] - Vd, Loc.diffStateLoc[3] - Vq, Loc.destLoc[0], Loc.destLoc[1]);
  m_output = -(Loc.destLoc[1] * Vq + Loc.destLoc[0] * Vd);
}


void gridDynGenModel5::residual (const IOdata &inputs, const stateData &sD, double resid[],  const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  const double *gmp = Loc.dstateLoc;

  double *rva = Loc.destLoc;
  double *rvd = Loc.destDiffLoc;
  updateLocalCache (inputs, sD, sMode);

  // Id and Iq
  if (hasAlgebraic (sMode))
    {
      rva[0] = Vd + Rs * gm[0] + (Xqp) * gm[1] - gmd[4];
      rva[1] = Vq + Rs * gm[1] - (Xdp) * gm[0] - gmd[3];
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

void gridDynGenModel5::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);
  const double *ast = Loc.algStateLoc;
  const double *dst = Loc.diffStateLoc;
  double *dv = Loc.destDiffLoc;
  //Get the exciter field
  double Eft = inputs[genModelEftInLocation];
  double Pmt = inputs[genModelPmechInLocation];

  // Id and Iq

  // delta
  dv[0] = m_baseFreq * (dst[1] - 1.0);
  // Edp and Eqp

  double xrat = Tqopp * (Xdp + Xl) / (Tqop * (Xqp + Xl));
  // Edp and Eqp
  dv[2] = (-dst[2] - (Xq - Xqp - xrat * (Xq - Xqp)) * ast[1]) / Tqop;
  dv[3] = (-dst[3] + (Xd - Xdp) * ast[0] + Eft) / Tdop;
  //Edpp
  dv[4] = (-dst[4] + dst[2] - (Xqp - Xdp + xrat * (Xq - Xqp)) * ast[1]) / Tqopp;
  // omega

  double Pe = dst[4] * ast[0] + dst[3] * ast[1] + (Xdp - Xqp) * ast[0] * ast[1];
  dv[1] = 0.5  * (Pmt - Pe - D * (dst[1] - 1.0)) / H;


}

void gridDynGenModel5::jacobianElements (const IOdata &inputs, const stateData &sD,
                                         matrixData<double> &ad,
                                         const IOlocs &inputLocs, const solverMode &sMode)
{
  // ad.assign (arrayIndex, RowIndex, ColIndex, value) const
  Lp Loc = offsets.getLocations (sD,nullptr, sMode, this);

  auto refAlg = Loc.algOffset;
  auto refDiff = Loc.diffOffset;
  const double *gm = Loc.algStateLoc;

  auto VLoc = inputLocs[voltageInLocation];
  auto TLoc = inputLocs[angleInLocation];

  updateLocalCache (inputs, sD, sMode);

  bool hasAlg = hasAlgebraic (sMode);
  // P
  if (hasAlg)
    {
      if (TLoc != kNullLocation)
        {
          ad.assign (refAlg, TLoc, Vq);
          ad.assign (refAlg + 1, TLoc, -Vd);
        }

      // Q
      if (VLoc != kNullLocation)
        {
          double V = inputs[voltageInLocation];
          ad.assign (refAlg, VLoc, Vd / V);
          ad.assign (refAlg + 1, VLoc, Vq / V);
        }

      ad.assign (refAlg, refAlg, Rs);
      ad.assign (refAlg, refAlg + 1, (Xqp));

      ad.assign (refAlg + 1, refAlg, -(Xdp));
      ad.assign (refAlg + 1, refAlg + 1, Rs);

      if (isAlgebraicOnly (sMode))
        {
          return;
        }

      // Id Additional

      ad.assign (refAlg, refDiff, -Vq);
      ad.assign (refAlg, refDiff + 4, -1);

      // Iq Additional
      ad.assign (refAlg + 1, refDiff, Vd);
      ad.assign (refAlg + 1, refDiff + 3, -1);
    }


  if (hasDifferential (sMode))
    {
      // delta
      ad.assign (refDiff, refDiff, -sD.cj);
      ad.assign (refDiff, refDiff + 1, m_baseFreq);

      // omega
      double kVal = -0.5 / H;
      if (hasAlg)
        {
          ad.assign (refDiff + 1, refAlg, -0.5  * (gm[6] + (Xdp - Xqp) * gm[1]) / H);
          ad.assign (refDiff + 1, refAlg + 1, -0.5  * (gm[5] + (Xdp - Xqp) * gm[0]) / H);
        }

      ad.assign (refDiff + 1, refDiff + 1, -0.5  * D / H - sD.cj);
      ad.assign (refDiff + 1, refDiff + 4, -0.5  * gm[0] / H);
      ad.assign (refDiff + 1, refDiff + 3, -0.5  * gm[1] / H);


      ad.assignCheckCol (refDiff + 1, inputLocs[genModelPmechInLocation], -kVal);             // governor: Pm


      double xrat = Tqopp * (Xdp + Xl) / (Tqop * (Xqp + Xl));
      // Edp
      if (hasAlg)
        {
          ad.assign (refDiff + 2, refAlg + 1, -(Xq - Xqp - xrat * (Xq - Xqp)) / Tqop);
        }
      ad.assign (refDiff + 2, refDiff + 2, -1 / Tqop - sD.cj);

      // Eqp
      if (hasAlg)
        {
          ad.assign (refDiff + 3, refAlg, (Xd - Xdp) / Tdop);
        }
      ad.assign (refDiff + 3, refDiff + 3, -1 / Tdop - sD.cj);


      ad.assignCheckCol (refDiff + 3, inputLocs[genModelEftInLocation], 1 / Tdop);             // exciter: Ef

      //Edpp
      if (hasAlg)
        {
          ad.assign (refDiff + 4, refAlg + 1, -(Xqp - Xdp + xrat * (Xq - Xqp)) / Tqopp);
        }
      ad.assign (refDiff + 4, refDiff + 2, 1 / Tqopp);
      ad.assign (refDiff + 4, refDiff + 4, -1 / Tqopp - sD.cj);
    }


}

static const stringVec genModel5Names {
  "id","iq","delta","freq","edp","eqp","edpp"
};

stringVec gridDynGenModel5::localStateNames () const
{
  return genModel5Names;
}

// set parameters
void gridDynGenModel5::set (const std::string &param,  const std::string &val)
{
  return gridDynGenModel4::set (param, val);
}

void gridDynGenModel5::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if ((param == "tqopp") || (param == "tq0pp"))
    {
      Tqopp = val;
    }
  else if (param == "taa")
    {
      Taa = val;
    }
  else if ((param == "tdopp") || (param == "td0pp"))
    {
      Tdopp = val;
    }
  else if (param == "xdpp")
    {
      Xdpp = val;
    }
  else if (param == "xqpp")
    {
      Xqpp = val;
    }
  else if (param == "xpp")
    {
      Xdpp = val;
      Xdpp = val;
    }
  else
    {
      gridDynGenModel4::set (param, val, unitType);
    }

}
