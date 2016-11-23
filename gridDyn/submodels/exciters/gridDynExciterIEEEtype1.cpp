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

#include "submodels/gridDynExciter.h"
#include "generators/gridDynGenerator.h"
#include "gridBus.h"
#include "matrixData.h"
#include "gridCoreTemplates.h"
#include <cmath>

gridDynExciterIEEEtype1::gridDynExciterIEEEtype1 (const std::string &objName) : gridDynExciter (objName)
{
  // default values
  Ka = 20;
  Ta = 0.04;

  limitState = 1;
}

//cloning function
gridCoreObject *gridDynExciterIEEEtype1::clone (gridCoreObject *obj) const
{
  gridDynExciterIEEEtype1 *gdE = cloneBase<gridDynExciterIEEEtype1, gridDynExciter> (this, obj);
  if (gdE == nullptr)
    {
      return obj;
    }
  gdE->Ke = Ke;
  gdE->Te = Te;
  gdE->Kf = Kf;
  gdE->Tf = Tf;
  gdE->Aex = Aex;
  gdE->Bex = Bex;
  return gdE;
}

void gridDynExciterIEEEtype1::objectInitializeA (double /*time0*/, unsigned long /*flags*/)
{
  offsets.local->local.diffSize = 3;
  offsets.local->local.jacSize = 14;
  checkForLimits ();
}

// initial conditions
void gridDynExciterIEEEtype1::objectInitializeB (const IOdata &args, const IOdata & outputSet, IOdata &inputSet)
{
  gridDynExciter::objectInitializeB (args, outputSet, inputSet);           //this will initializeB the field state if need be
  double *gs = m_state.data ();
  gs[1] = (Ke + Aex * exp (Bex * gs[0])) * gs[0]; // Vr
  gs[2] = gs[0] * Kf / Tf;                        // Rf
  vBias = args[voltageInLocation] + gs[1] / Ka - Vref;
  inputSet[1] = Vref;
  m_dstate_dt[0] = 0.0;
  m_dstate_dt[1] = 0.0;
  m_dstate_dt[2] = 0.0;
}




// residual
void gridDynExciterIEEEtype1::residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode)
{
  if (!hasDifferential(sMode))
    {
      return;
    }
  auto offset = offsets.getDiffOffset (sMode);
  const double *es = sD->state + offset;
  const double *esp = sD->dstate_dt + offset;
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

      rv[1] = (-es[1] + Ka * es[2] - es[0] * Ka * Kf / Tf + Ka * (Vref + vBias - args[voltageInLocation])) / Ta - esp[1];
    }
  rv[2] = (-es[2] + es[0] * Kf / Tf) / Tf - esp[2];

}

void gridDynExciterIEEEtype1::timestep (gridDyn_time ttime, const IOdata &args, const solverMode &)
{

  derivative ( args, nullptr, m_dstate_dt.data (), cLocalSolverMode);
  double dt = ttime - prevTime;
  m_state[0] += dt * m_dstate_dt[0];
  m_state[1] += dt * m_dstate_dt[1];
  m_state[2] += dt * m_dstate_dt[2];
  prevTime = ttime;
}

void gridDynExciterIEEEtype1::derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode,  this);
  const double *es = Loc.diffStateLoc;
  double *d = Loc.destDiffLoc;
  d[0] = (-(Ke + Aex * exp (Bex * es[0])) * es[0] + es[1]) / Te;
  if (opFlags[outside_vlim])
    {
      d[1] = 0;
    }
  else
    {
      d[1] = (-es[1] + Ka * es[2] - es[0] * Ka * Kf / Tf + Ka * (Vref + vBias - args[voltageInLocation])) / Ta;
		
    }
  d[2] = (-es[2] + es[0] * Kf / Tf) / Tf;
}

// Jacobian
void gridDynExciterIEEEtype1::jacobianElements (const IOdata & /*args*/, const stateData *sD,
                                                matrixData<double> &ad,
                                                const IOlocs &argLocs, const solverMode &sMode)
{
  if  (!hasDifferential (sMode))
    {
      return;
    }
  auto offset = offsets.getDiffOffset (sMode);


  // use the ad.assign Macro defined in basicDefs
  // ad.assign(arrayIndex, RowIndex, ColIndex, value)

  // Ef
  double temp1 = -(Ke + Aex * exp (Bex * sD->state[offset]) * (1.0 + Bex * sD->state[offset])) / Te - sD->cj;
  ad.assign (offset, offset, temp1);
  ad.assign (offset, offset + 1, 1 / Te);
  if (opFlags[outside_vlim])
    {
      ad.assign (offset + 1, offset + 1, sD->cj);
    }
  else
    {
      // Vr

      ad.assignCheckCol (offset + 1, argLocs[voltageInLocation], -Ka / Ta);
      ad.assign (offset + 1, offset, -Ka * Kf / (Tf * Ta));
      ad.assign (offset + 1, offset + 1, -1.0 / Ta - sD->cj);
      ad.assign (offset + 1, offset + 2, Ka / Ta);
    }


  // Rf
  ad.assign (offset + 2, offset, Kf / (Tf * Tf));
  ad.assign (offset + 2, offset + 2, -1.0 / Tf - sD->cj);

  //printf("%f\n",sD->cj);

}

void gridDynExciterIEEEtype1::rootTest (const IOdata &args, const stateData *sD, double roots[],  const solverMode &sMode)
{
  auto offset = offsets.getDiffOffset (sMode);
  int rootOffset = offsets.getRootOffset (sMode);
  const double *es = sD->state + offset;

  //printf("t=%f V=%f\n", ttime, args[voltageInLocation]);

  if (opFlags[outside_vlim])
    {
      roots[rootOffset] = es[2] - es[0] * Kf / Tf + (Vref + vBias - args[voltageInLocation]) - es[1] / Ka + 0.001 * es[1] / Ka / Ta;

    }
  else
    {
      roots[rootOffset] = std::min (Vrmax - es[1], es[1] - Vrmin) + 0.00001;
      if (es[1] >= Vrmax)
        {
          opFlags.set (etrigger_high);
        }
    }


}

change_code gridDynExciterIEEEtype1::rootCheck ( const IOdata &args, const stateData *, const solverMode & /*sMode*/, check_level_t /*level*/)
{

  const double *es = m_state.data ();
  double test;
  change_code ret = change_code::no_change;
  if (opFlags[outside_vlim])
    {
      test = es[2] - es[0] * Kf / Tf + (Vref + vBias - args[voltageInLocation]) - es[1] / Ka;


      if (opFlags[etrigger_high])
        {
          if (test < -0.001 * es[1] / Ka / Ta)
            {
              ret = change_code::jacobian_change;

              LOG_NORMAL ("root change V=" + std::to_string (args[voltageInLocation]));
              opFlags.reset (outside_vlim);
              opFlags.reset (etrigger_high);
              alert (this, JAC_COUNT_INCREASE);
            }
        }
      else
        {
          if (test > -0.001 * es[1] / Ka / Ta)
            {
              LOG_NORMAL ("root change V=" + std::to_string (args[voltageInLocation]));
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

          LOG_NORMAL ("root toggle V=" + std::to_string (args[voltageInLocation]));
          opFlags.set (etrigger_high);
          opFlags.set (outside_vlim);
          m_state[1] = Vrmax;
          m_dstate_dt[1] = 0.0;
          ret = change_code::jacobian_change;
          alert (this, JAC_COUNT_DECREASE);
        }
      else if (es[1] < Vrmin - 0.00001)
        {
          LOG_NORMAL ("root toggle V=" + std::to_string (args[voltageInLocation]));

          opFlags.reset (etrigger_high);
          opFlags.set (outside_vlim);
          m_state[1] = Vrmin;
          m_dstate_dt[1] = 0.0;
          ret = change_code::jacobian_change;
          alert (this, JAC_COUNT_DECREASE);
        }
    }

  return ret;
}


static const stringVec ieeeType1Fields {
  "ef","vr","rf"
};

stringVec gridDynExciterIEEEtype1::localStateNames () const
{
  return ieeeType1Fields;
}


void gridDynExciterIEEEtype1::set (const std::string &param,  const std::string &val)
{
  return gridDynExciter::set (param, val);
}

// set parameters
void gridDynExciterIEEEtype1::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "ke")
    {
      Ke = val;
    }
  else if (param == "te")
    {
      Te = val;
    }
  else if (param == "kf")
    {
      Kf = val;
    }
  else if (param == "tf")
    {
      Tf = val;
    }
  else if (param == "aex")
    {
      Aex = val;
    }
  else if (param == "bex")
    {
      Bex = val;
    }
  else
    {
      gridDynExciter::set (param, val, unitType);
    }

}
