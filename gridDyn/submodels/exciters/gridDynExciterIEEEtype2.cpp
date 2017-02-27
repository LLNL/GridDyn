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
#include "core/coreObjectTemplates.h"
#include <cmath>

gridDynExciterIEEEtype2::gridDynExciterIEEEtype2 (const std::string &objName) : gridDynExciterIEEEtype1 (objName)
{


}

//cloning function
coreObject *gridDynExciterIEEEtype2::clone (coreObject *obj) const
{
  gridDynExciterIEEEtype2 *gdE = cloneBase<gridDynExciterIEEEtype2, gridDynExciterIEEEtype1> (this, obj);
  if (gdE == nullptr)
    {
      return obj;
    }

  gdE->Tf2 = Tf2;
  return gdE;
}

void gridDynExciterIEEEtype2::dynObjectInitializeA (coreTime /*time*/, unsigned long /*flags*/)
{
  offsets.local().local.diffSize = 4;
  offsets.local().local.jacSize = 16;
  checkForLimits ();
}

// initial conditions
void gridDynExciterIEEEtype2::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet)
{

  gridDynExciter::dynObjectInitializeB (inputs,desiredOutput,inputSet);      //this will dynInitializeB the field state if need be
  double *gs = m_state.data ();
  gs[1] = (Ke + Aex * exp (Bex * gs[0])) * gs[0]; // Vr
  gs[2] = 0;                                      // X1
  gs[3] = gs[1];                                  // X2

  vBias = inputs[voltageInLocation] + gs[1] / Ka - Vref;
  inputSet[exciterVsetInLocation] = Vref;

}



// residual
void gridDynExciterIEEEtype2::residual (const IOdata &inputs, const stateData &sD, double resid[],  const solverMode &sMode)
{
  if (isAlgebraicOnly (sMode))
    {
      return;
    }
  auto offset = offsets.getDiffOffset (sMode);
  const double *es = sD.state + offset;
  const double *esp = sD.dstate_dt + offset;
  double *rv = resid + offset;
  rv[0] = (-(Ke + Aex * exp (Bex * es[0])) * es[0] + es[1]) / Te - esp[0];
  if (opFlags[outside_vlim])
    {
      if (opFlags[etrigger_high])
        {
          rv[1] = esp[1];
        }
      else
        {
          rv[1] = esp[1];
        }
    }
  else
    {
      rv[1] = (-es[1] + Ka * Kf * es[2] + Ka * (Vref + vBias - inputs[voltageInLocation])) / Ta - esp[1];
    }
  rv[2] = (-es[2] + es[1] / Tf2 - es[3] / Tf2) / Tf - esp[2];
  rv[3] = (-es[3] + es[1]) / Tf2 - esp[3];

}

void gridDynExciterIEEEtype2::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);
  const double *es = Loc.diffStateLoc;
  double *d = Loc.destDiffLoc;
  d[0] = (-(Ke + Aex * exp (Bex * es[0])) * es[0] + es[1]) / Te;
  if (opFlags[outside_vlim])
    {
      d[1] = 0;
    }
  else
    {
      d[1] = (-es[1] + Ka * Kf * es[2] + Ka * (Vref + vBias - inputs[voltageInLocation])) / Ta;
    }
  d[2] = (-es[2] + es[1] / Tf2 - es[3] / Tf2) / Tf;
  d[3] = (-es[3] + es[1]) / Tf2;
}

// compute the bus element contributions

// Jacobian
void gridDynExciterIEEEtype2::jacobianElements (const IOdata & /*inputs*/, const stateData &sD,
                                                matrixData<double> &ad,
                                                const IOlocs &inputLocs, const solverMode &sMode)
{
  if  (isAlgebraicOnly (sMode))
    {
      return;
    }
  auto offset = offsets.getDiffOffset (sMode);
  int refI = offset;
  double temp1;
  auto VLoc = inputLocs[0];
  // use the ad.assign Macro defined in basicDefs
  // ad.assign(arrayIndex, RowIndex, ColIndex, value)

  // Ef
  temp1 = -(Ke + Aex * exp (Bex * sD.state[offset]) * (1.0 + Bex * sD.state[offset])) / Te - sD.cj;
  ad.assign ( refI, refI, temp1);
  ad.assign ( refI, refI + 1, 1.0 / Te);

  if (opFlags[outside_vlim])
    {
      ad.assign (refI + 1, refI + 1, sD.cj);
    }
  else
    {
      // Vr
      if (VLoc != kNullLocation)
        {
          ad.assign (refI + 1, VLoc, -Ka / Ta);
        }
      ad.assign (refI + 1, refI + 1, -1.0 / Ta - sD.cj);
      ad.assign (refI + 1, refI + 2, Ka * Kf / Ta);
    }


  // X1
  ad.assign ( refI + 2, refI + 1,  1.0 / (Tf * Tf2));
  ad.assign ( refI + 2, refI + 2, -1.0 / Tf - sD.cj);
  ad.assign ( refI + 2, refI + 3, -1.0 / (Tf * Tf2));

  // X2
  ad.assign (refI + 3, refI + 1, 1.0 / Tf2);
  ad.assign (refI + 3, refI + 3, -1.0 / Tf2 - sD.cj);


  //printf("%f\n",sD.cj);

}


static const stringVec ieeeType2Fields {
  "ef","vr","x1","x2"
};

stringVec gridDynExciterIEEEtype2::localStateNames () const
{
  return ieeeType2Fields;
}

void gridDynExciterIEEEtype2::rootTest (const IOdata &inputs, const stateData &sD, double roots[],  const solverMode &sMode)
{
  auto offset = offsets.getAlgOffset (sMode);
  int rootOffset = offsets.getRootOffset (sMode);
  const double *es = sD.state + offset;


  if (opFlags[outside_vlim])
    {
      roots[rootOffset] = Ka * Kf * es[2] + Ka * (Vref + vBias - inputs[voltageInLocation]) - es[1];
    }
  else
    {
      roots[rootOffset] = std::min (Vrmax - es[1], es[1] - Vrmin) + 0.0001;
      if (es[1] > Vrmax)
        {
          opFlags.set (etrigger_high);
        }
    }
}

change_code gridDynExciterIEEEtype2::rootCheck ( const IOdata &inputs, const stateData &, const solverMode &, check_level_t /*level*/)
{

  double *es = m_state.data ();
  change_code ret = change_code::no_change;
  if (opFlags[outside_vlim])
    {
      double test = Ka * Kf * es[2] + Ka * (Vref + vBias - inputs[voltageInLocation]) - es[1];
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

      if (es[1] > Vrmax + 0.0001)
        {
          opFlags.set (etrigger_high);
          opFlags.set (outside_vlim);
          es[1] = Vrmax;
          ret = change_code::jacobian_change;
          alert (this, JAC_COUNT_DECREASE);
        }
      else if (es[1] < Vrmin - 0.0001)
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

void gridDynExciterIEEEtype2::set (const std::string &param,  const std::string &val)
{
  return gridDynExciterIEEEtype1::set (param, val);
}

// set parameters
void gridDynExciterIEEEtype2::set (const std::string &param, double val, gridUnits::units_t unitType)
{


  if (param == "tf1")
    {
      Tf = val;
    }
  else if (param == "tf2")
    {
      Tf2 = val;
    }
  else
    {
      gridDynExciterIEEEtype1::set (param, val, unitType);
    }

  
}
