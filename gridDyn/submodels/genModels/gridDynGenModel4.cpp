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
#include "gridBus.h"
#include "vectorOps.hpp"
#include "matrixData.h"
#include "gridCoreTemplates.h"

#include <cmath>

gridDynGenModel4::gridDynGenModel4 (const std::string &objName) : gridDynGenModel3 (objName)
{

}

gridCoreObject *gridDynGenModel4::clone (gridCoreObject *obj) const
{
  gridDynGenModel4 *gd = cloneBase<gridDynGenModel4, gridDynGenModel3> (this, obj);
  if (gd == nullptr)
    {
      return obj;
    }
  gd->Xqp = Xqp;
  gd->Tqop = Tqop;
  gd->S10 = S10;
  gd->S12 = S12;
  return gd;
}

void gridDynGenModel4::objectInitializeA (double /*time0*/, unsigned long /*flags*/)
{
  offsets.local->local.diffSize = 4;
  offsets.local->local.algSize = 2;
  offsets.local->local.jacSize = 25;

}

// initial conditions
void gridDynGenModel4::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
  computeInitialAngleAndCurrent (args, outputSet, Rs, Xq);
  double *gm = m_state.data ();

  // Edp and Eqp
  gm[4] = Vd + Rs * gm[0] + (Xqp) * gm[1];
  gm[5] = Vq + Rs * gm[1] - (Xdp) * gm[0];

  // record Pm = Pset
  //this should be close to P from above
  double Pmt = gm[4] * gm[0] + gm[5] * gm[1] + (Xdp - Xqp) * gm[0] * gm[1];

  // exciter - assign Ef
  double Eft = gm[5] - (Xd - Xdp) * gm[0];
  //preset the inputs that should be initialized
  inputSet[2] = Eft;
  inputSet[3] = Pmt;

}


void gridDynGenModel4::residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);
  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;

  updateLocalCache (args, sD, sMode);

  // Id and Iq
  if (hasAlgebraic (sMode))
    {
      double *rva = Loc.destLoc;
      rva[0] = Vd + Rs * gm[0] + (Xqp) * gm[1] - gmd[2];
      rva[1] = Vq + Rs * gm[1] - (Xdp) * gm[0] - gmd[3];
    }

  if (hasDifferential (sMode))
    {
      double *rvd = Loc.destDiffLoc;
      //Get the exciter field
      double Eft = args[genModelEftInLocation];
      double Pmt = args[genModelPmechInLocation];
      const double *gmp = Loc.dstateLoc;
      // delta
      rvd[0] = m_baseFreq * (gmd[1] - 1.0) - gmp[0];
      // Edp and Eqp
      rvd[2] = (-gmd[2] - (Xq - Xqp) * gm[1]) / Tqop - gmp[2];
      rvd[3] = (-gmd[3] + (Xd - Xdp) * gm[0] + Eft) / Tdop - gmp[3];

      // omega
      double Pe = gmd[2] * gm[0] + gmd[3] * gm[1] + (Xdp - Xqp) * gm[0] * gm[1];
      rvd[1] = 0.5  * (Pmt - Pe - D * (gmd[1] - 1.0)) / H - gmp[1];
    }
  // if (parent->parent->name == "BUS_31")
  //   {
  //   printf("[%d]t=%f gmp[1]=%f Vq=%f, Vd=%f,Pdiff=%f A=%f, B=%f, C=%f Id=%f, Iq=%f, Eft=%f\n", getID(), ttime, gmp[1], Vq,Vd, Pmt - Pe, gmd[2] * gm[0], gmd[3] * gm[1], (Xdp - Xqp) * gm[0] * gm[1],gm[0],gm[1],Eft);
  //   }
}

void gridDynGenModel4::timestep (gridDyn_time ttime, const IOdata &args, const solverMode &)
{
  stateData sD (ttime,m_state.data ());
  derivative (args, &sD, m_dstate_dt.data (), cLocalSolverMode);
  double dt = ttime - prevTime;
  m_state[2] += dt * m_dstate_dt[2];
  m_state[3] += dt * m_dstate_dt[3];
  m_state[4] += dt * m_dstate_dt[4];
  m_state[5] += dt * m_dstate_dt[5];
  prevTime = ttime;
  algebraicUpdate (args, &sD, m_state.data (), cLocalSolverMode,1.0);
}

void gridDynGenModel4::algebraicUpdate (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double /*alpha*/)
{
  Lp Loc = offsets.getLocations (sD, update, sMode, this);
  updateLocalCache (args, sD, sMode);
  solve2x2 (Rs, (Xqp), -(Xdp),Rs, Loc.diffStateLoc[2] - Vd, Loc.diffStateLoc[3] - Vq, Loc.destLoc[0], Loc.destLoc[1]);
  m_output = -(Loc.destLoc[1] * Vq + Loc.destLoc[0] * Vd);
}

void gridDynGenModel4::derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);
  const double *ast = Loc.algStateLoc;
  const double *dst = Loc.diffStateLoc;
  double *dv = Loc.destDiffLoc;
  //Get the exciter field
  double Eft = args[genModelEftInLocation];
  double Pmt = args[genModelPmechInLocation];

  // Id and Iq

  // delta
  dv[0] = m_baseFreq * (dst[1] - 1.0);
  // Edp and Eqp
  dv[2] = (-dst[2] - (Xq - Xqp) * ast[1]) / Tqop;
  dv[3] = (-dst[3] + (Xd - Xdp) * ast[0] + Eft) / Tdop;

  // omega
  double Pe = dst[2] * ast[0] + dst[3] * ast[1] + (Xdp - Xqp) * ast[0] * ast[1];
  dv[1] = 0.5  * (Pmt - Pe - D * (dst[1] - 1.0)) / H;

}

void gridDynGenModel4::jacobianElements (const IOdata &args, const stateData *sD,
                                         matrixData<double> &ad,
                                         const IOlocs &argLocs, const solverMode &sMode)
{
  Lp Loc = offsets.getLocations  (sD, sMode, this);

  auto refAlg = Loc.algOffset;
  auto refDiff = Loc.diffOffset;
  const double *gm = Loc.algStateLoc;
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
          if (V == 0)
            {
              printf ("voltage0\n");
            }
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
      // Id Differential

      ad.assign (refAlg, refDiff, -Vq);
      ad.assign (refAlg, refDiff + 2, -1.0);

      // Iq Differential

      ad.assign (refAlg + 1, refDiff, Vd);
      ad.assign (refAlg + 1, refDiff + 3, -1.0);
    }

  if (hasDifferential (sMode))
    {

      // delta
      ad.assign (refDiff, refDiff, -sD->cj);
      ad.assign (refDiff, refDiff + 1, m_baseFreq);

      // omega
      double kVal = -0.5 / H;
      if (hasAlgebraic (sMode))
        {
          ad.assign (refDiff + 1, refAlg, -0.5  * (gm[4] + (Xdp - Xqp) * gm[1]) / H);
          ad.assign (refDiff + 1, refAlg + 1, -0.5  * (gm[5] + (Xdp - Xqp) * gm[0]) / H);

          ad.assign (refDiff + 2, refAlg + 1, -(Xq - Xqp) / Tqop);         //Edp

          ad.assign (refDiff + 3, refAlg, (Xd - Xdp) / Tdop);        //Eqp
        }

      ad.assign (refDiff + 1, refDiff + 1, -0.5 *  D / H - sD->cj);
      ad.assign (refDiff + 1, refDiff + 2, -0.5  * gm[0] / H);
      ad.assign (refDiff + 1, refDiff + 3, -0.5  * gm[1] / H);


      ad.assignCheckCol (refDiff + 1, argLocs[genModelPmechInLocation], -kVal);    // governor: Pm

      ad.assign (refDiff + 2, refDiff + 2, -1.0 / Tqop - sD->cj);

      // Eqp

      ad.assign (refDiff + 3, refDiff + 3, -1.0 / Tdop - sD->cj);


      ad.assignCheckCol (refDiff + 3, argLocs[genModelEftInLocation], 1.0 / Tdop);    // exciter: Ef
    }



}

static const stringVec genModel4Names {
  "id","iq","delta","freq","edp","eqp"
};

stringVec gridDynGenModel4::localStateNames () const
{
  return genModel4Names;
}

// set parameters
void gridDynGenModel4::set (const std::string &param,  const std::string &val)
{

  if (param == "saturation_type")
    {
      sat.setType (val);
    }
  else
    {
      gridDynGenModel3::set (param, val);
    }

}

void gridDynGenModel4::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "xd")
    {
      Xd = val;
    }
  else if (param == "xqp")
    {
      Xqp = val;
    }
  else if ((param == "tqop")||(param == "tq0p"))
    {
      Tqop = val;
    }
  else if ((param == "top") || (param == "t0p"))
    {
      Tqop = val;
      Tdop = val;
    }
  else if ((param == "s1") || (param == "s10"))
    {
      S10 = val;
      sat.setParam (S10, S12);

    }
  else if (param == "s12")
    {
      S12 = val;
      sat.setParam (S10, S12);

    }
  else
    {
      gridDynGenModel3::set (param, val, unitType);
    }

}

