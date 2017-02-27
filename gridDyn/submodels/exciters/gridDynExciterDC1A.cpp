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

#include "submodels/gridDynExciter.h"
#include "generators/gridDynGenerator.h"
#include "gridBus.h"
#include "matrixData.h"

#include <cmath>

gridDynExciterDC1A::gridDynExciterDC1A (const std::string &objName) : gridDynExciterIEEEtype1 (objName)
{
  // default values
  Ka = 12;
  Ta = 0.06;
  Ke = 1.0;
  Te = .46;
  Kf = .1;
  Tf = 1.0;
  Aex = 0.0032;       // (3.1,.33) and (2.3,.1)
  Bex = 1.4924;
  Vrmin = -.9;
  Vrmax = 1;
}

//cloning function
coreObject *gridDynExciterDC1A::clone (coreObject *obj) const
{
  gridDynExciterDC1A *gdE;
  if (obj == nullptr)
    {
      gdE = new gridDynExciterDC1A ();
    }
  else
    {
      gdE = dynamic_cast<gridDynExciterDC1A *> (obj);
      if (gdE == nullptr)
        {
          gridDynExciter::clone (obj);
          return obj;
        }
    }
  gridDynExciterIEEEtype1::clone (gdE);
  gdE->Tc = Tc;
  gdE->Tb = Tb;
  return gdE;
}

void gridDynExciterDC1A::dynObjectInitializeA (coreTime /*time0*/, unsigned long /*flags*/)
{
  offsets.local().local.diffSize = 4;
  offsets.local().local.jacSize = 19;
  checkForLimits ();
}

// initial conditions
void gridDynExciterDC1A::dynObjectInitializeB (const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet)
{
  gridDynExciter::dynObjectInitializeB (inputs, desiredOutput, fieldSet);           //this will dynInitializeB the field state if need be
  double *gs = m_state.data ();
  gs[1] = (Ke + Aex * exp (Bex * gs[0])) * gs[0]; // Vr
  gs[2] = gs[1] / Ka;                            // X
  gs[3] = gs[0] * Kf / Tf;                       // Rf

  vBias = inputs[voltageInLocation] + gs[1] / Ka - Vref;
  fieldSet[1] = Vref;
}



// residual
void gridDynExciterDC1A::residual (const IOdata &inputs, const stateData &sD, double resid[],  const solverMode &sMode)
{
  if (isAlgebraicOnly (sMode))
    {
      return;
    }
  derivative (inputs, sD, resid, sMode);

  auto offset = offsets.getDiffOffset (sMode);
  const double *esp = sD.dstate_dt + offset;
  resid[offset] -= esp[0];
  resid[offset + 1] -= esp[1];
  resid[offset + 2] -= esp[2];
  resid[offset + 3] -= esp[3];

}

void gridDynExciterDC1A::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode,  this);
  const double *es = Loc.diffStateLoc;
  double *d = Loc.destDiffLoc;
  double V = inputs[voltageInLocation];
  d[0] = (-(Ke + Aex * exp (Bex * es[0])) * es[0] + es[1]) / Te;
  if (opFlags[outside_vlim])
    {
      d[1] = 0;
    }
  else
    {
      d[1] = (-es[1] + ((Vref + vBias - V) - es[0] * Kf / Tf + es[3]) * Ka * Tc / Tb + es[2] * (Tb - Tc) * Ka / Tb) / Ta;
    }
  d[2] = (-es[2] + (Vref + vBias - V) - es[0] * Kf / Tf + es[3]) / Tb;
  d[3] = (-es[3] + es[0] * Kf / Tf) / Tf;
}


// Jacobian
void gridDynExciterDC1A::jacobianElements (const IOdata &inputs, const stateData &sD,
                                           matrixData<double> &ad,
                                           const IOlocs &inputLocs, const solverMode &sMode)
{
  if  (isAlgebraicOnly (sMode))
    {
      return;
    }
  auto offset = offsets.getDiffOffset (sMode);
  auto refI = offset;

  auto VLoc = inputLocs[voltageInLocation];
  // use the ad.assign Macro defined in basicDefs
  // ad.assign(arrayIndex, RowIndex, ColIndex, value)

  // Ef
  double temp1 = -(Ke + Aex * exp (Bex * sD.state[offset]) * (1.0 + Bex * sD.state[offset])) / Te - sD.cj;
  ad.assign (refI, refI, temp1);
  ad.assign (refI, refI + 1, 1.0 / Te);

  if (opFlags[outside_vlim])
    {
      limitJacobian (inputs[voltageInLocation], VLoc, refI + 1, sD.cj, ad);

    }
  else
    {
      // Vr
      if (VLoc != kNullLocation)
        {
          ad.assign (refI + 1, VLoc, -Ka * Tc / (Ta * Tb));
        }
      ad.assign (refI + 1, refI, -Ka * Kf * Tc / (Tf * Ta * Tb));
      ad.assign (refI + 1, refI + 1, -1.0 / Ta - sD.cj);
      ad.assign (refI + 1, refI + 2, Ka * (Tb - Tc) / (Ta * Tb));
      ad.assign (refI + 1, refI + 3, Ka * Tc / (Ta * Tb));
    }


  // X
  if (VLoc != kNullLocation)
    {
      ad.assign (refI + 2, VLoc, -1.0 / Tb);
    }
  ad.assign (refI + 2, refI, -Kf / (Tf * Tb));
  ad.assign (refI + 2, refI + 2, -1.0 / Tb - sD.cj);
  ad.assign (refI + 2, refI + 3, 1.0 / Tb);
  // Rf
  ad.assign (refI + 3, refI, Kf / (Tf * Tf));
  ad.assign (refI + 3, refI + 3, -1.0 / Tf - sD.cj);

  //printf("%f--%f--\n",sD.time,sD.cj);

}

void gridDynExciterDC1A::limitJacobian (double /*V*/, int /*Vloc*/, int refLoc, double cj, matrixData<double> &ad)
{
  ad.assign (refLoc, refLoc, cj);
}


void gridDynExciterDC1A::rootTest (const IOdata &inputs, const stateData &sD, double root[],  const solverMode &sMode)
{
  auto offset = offsets.getAlgOffset (sMode);
  const double *es = sD.state + offset;

  int rootOffset = offsets.getRootOffset (sMode);
  if (opFlags[outside_vlim])
    {
      root[rootOffset] = ((Vref + vBias - inputs[voltageInLocation]) - es[0] * Kf / Tf + es[3]) * Ka * Tc / Tb + es[2] * (Tb - Tc) * Ka / Tb - es[1];
    }
  else
    {
      root[rootOffset] = std::min (Vrmax - es[1], es[1] - Vrmin) + 0.00001;
      if (es[1] > Vrmax)
        {
          opFlags.set (etrigger_high);
        }
    }


}

change_code gridDynExciterDC1A::rootCheck ( const IOdata &inputs, const stateData &, const solverMode &, check_level_t /*level*/)
{
  double *es = m_state.data ();
  double test;
  change_code ret = change_code::no_change;
  if (opFlags[outside_vlim])
    {
      test = ((Vref + vBias - inputs[voltageInLocation]) - es[0] * Kf / Tf + es[3]) * Ka * Tc / Tb + es[2] * (Tb - Tc) * Ka / Tb - es[1];
      if (opFlags[etrigger_high])
        {
          if (test < 0.0)
            {
              ret = change_code::jacobian_change;
              opFlags.reset (outside_vlim);
              opFlags.reset (etrigger_high);
              alert (this, JAC_COUNT_INCREASE);
            }
        }
      else
        {
          if (test > 0.0)
            {
              ret = change_code::jacobian_change;
              opFlags.reset (outside_vlim);
              alert (this, JAC_COUNT_INCREASE);
            }
        }
    }
  else
    {

      if (es[1] > Vrmax + 0.00001)
        {
          opFlags.set (etrigger_high);
          opFlags.set (outside_vlim);
          es[1] = Vrmax;
          ret = change_code::jacobian_change;
          alert (this, JAC_COUNT_DECREASE);
        }
      else if (es[1] < Vrmin - 0.00001)
        {
          opFlags.reset (etrigger_high);
          opFlags.set (outside_vlim);
          es[1] = Vrmin;
          ret = change_code::jacobian_change;
          alert (this, JAC_COUNT_DECREASE);
        }
    }

  return ret;
}


static const stringVec dc1aFields {
  "ef","vr","x","rf"
};

stringVec gridDynExciterDC1A::localStateNames () const
{
  return dc1aFields;
}


void gridDynExciterDC1A::set (const std::string &param,  const std::string &val)
{
  return gridDynExciterIEEEtype1::set (param, val);
}

// set parameters
void gridDynExciterDC1A::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "tb")
    {
      Tb = val;
    }
  else if (param == "tc")
    {
      Tc = val;
    }
  else
    {
      gridDynExciterIEEEtype1::set (param, val, unitType);
    }

 
}
