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

#include "submodels/otherGovernors.h"
#include "generators/gridDynGenerator.h"
#include "objectFactory.h"
#include "gridBus.h"
#include "arrayData.h"
#include "gridCoreTemplates.h"

using namespace gridUnits;

gridDynGovernorIeeeSimple::gridDynGovernorIeeeSimple (const std::string &objName) : gridDynGovernor (objName)
{
  // default values
  K = 16.667;
  T1 = 0.1;
  T2 = 0.15;
  T3 = 0.05;
  offsets.local->local.algSize = 0;
  offsets.local->local.diffSize = 2;
  offsets.local->local.jacSize = 6;
  opFlags.set (ignore_deadband);
  opFlags.set (ignore_filter);
  opFlags.set (ignore_throttle);

}

gridCoreObject *gridDynGovernorIeeeSimple::clone (gridCoreObject *obj) const
{
  gridDynGovernorIeeeSimple *gov = cloneBase<gridDynGovernorIeeeSimple, gridDynGovernor> (this, obj);
  if (gov == nullptr)
    {
      return obj;
    }
  gov->T3 = T3;
  gov->Pup = Pup;
  gov->Pdown = Pdown;
  return gov;
}

// destructor
gridDynGovernorIeeeSimple::~gridDynGovernorIeeeSimple ()
{

}

void gridDynGovernorIeeeSimple::objectInitializeA (double time0, unsigned long flags)
{
  gridDynGovernor::objectInitializeA (time0,flags);
  if ((Pmax < 5000) || (Pmin > -5000))
    {
      offsets.local->local.diffRoots++;
      opFlags.set (uses_plimits);
    }
}

// initial conditions
void gridDynGovernorIeeeSimple::objectInitializeB (const IOdata & /*args*/, const IOdata &outputSet, IOdata &inputSet)
{
  if (Wref < 0)
    {
      Wref = m_baseFreq;
    }
  m_state[1] = 0;
  m_state[0] = outputSet[0];
  inputSet[1] = outputSet[0];

}

// residual
void gridDynGovernorIeeeSimple::residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode)
{
  if (isAlgebraicOnly (sMode))
    {
      return;
    }
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);
  derivative (args, sD, resid, sMode);

  Loc.destDiffLoc[0] -= Loc.dstateLoc[0];
  Loc.destDiffLoc[1] -= Loc.dstateLoc[1];
}

void gridDynGovernorIeeeSimple::derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);

  const double *gs = Loc.diffStateLoc;
  //double omega = getControlFrequency (args);
  double omega = args[govOmegaInLocation];
  if (opFlags.test (p_limited))
    {
      Loc.destDiffLoc[0] = 0;
    }
  else
    {
      Loc.destDiffLoc[0] = (args[govpSetInLocation] - gs[0] - K * gs[1] - K * T2 * (omega - 1.0) / T1) / T3;
    }

  Loc.destDiffLoc[1] = (-gs[1] + (1 - T2 / T1) * (omega - 1.0)) / T1;
}

double gridDynGovernorIeeeSimple::timestep (double ttime,  const IOdata &args, const solverMode &)
{


  gridDynGovernorIeeeSimple::derivative ( args, nullptr, m_dstate_dt.data (), cLocalSolverMode);
  double dt = ttime - prevTime;
  m_state[0] += dt * m_dstate_dt[0];
  m_state[1] += dt * m_dstate_dt[1];
  if (opFlags.test (p_limited))
    {

    }
  else
    {
      if (m_state[0] > Pmax)
        {
          opFlags.set (p_limited);
          opFlags.set (p_limit_high);
          m_state[0] = Pmax;
        }
    }


  prevTime = ttime;
  return m_state[0];
}

void gridDynGovernorIeeeSimple::jacobianElements (const IOdata & /*args*/, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
  if  (isAlgebraicOnly (sMode))
    {
      return;
    }
  Lp Loc = offsets.getLocations (sD,nullptr, sMode, this);

  int refI = Loc.diffOffset;
  //use the ad->assign Macro defined in basicDefs
  // ad->assign(arrayIndex, RowIndex, ColIndex, value)
  bool linkOmega = true;
  if (argLocs[govOmegaInLocation] == kNullLocation)
    {
      linkOmega = false;
    }
  /*
  if (opFlags.test (uses_deadband))
    {
      if (!opFlags.test (outside_deadband))
        {
          linkOmega = false;
        }
    }
        */
  // Pm
  if (linkOmega)
    {
      if (!opFlags.test (p_limited))
        {
          ad->assign (refI, argLocs[govOmegaInLocation], -K * T2 / (T1 * T3));
        }
      ad->assign (refI + 1, argLocs[govOmegaInLocation], (T1 - T2) / (T1 * T1));
    }
  if (opFlags.test (p_limited))
    {
      ad->assign (refI, refI, sD->cj);
    }
  else
    {
      ad->assign (refI, refI, -1 / T3 - sD->cj);
      ad->assign (refI, refI + 1, -K / T3);

      ad->assignCheck (refI, argLocs[govpSetInLocation], 1 / T3);
    }
  ad->assign (refI + 1, refI + 1, -1 / T1 - sD->cj);
}


index_t gridDynGovernorIeeeSimple::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  if (field == "pm")
    {
      ret = offsets.getDiffOffset (sMode);
    }
  else if (field == "x")
    {
      ret = offsets.getDiffOffset (sMode);
      ret = (ret != kNullLocation) ? ret + 1 : ret;
    }
  return ret;
}

void gridDynGovernorIeeeSimple::rootTest (const IOdata &args, const stateData *sD, double root[], const solverMode &sMode)
{

  int rootOffset = offsets.getRootOffset (sMode);
  /*if (opFlags.test (uses_deadband))
    {
      gridDynGovernor::rootTest (args, sD, root, sMode);
      ++rootOffset;
    }
        */
  if (opFlags.test (uses_plimits))
    {
      Lp Loc = offsets.getLocations (sD,nullptr, sMode, this);


      double Pmech = Loc.diffStateLoc[0];

      if (!opFlags.test (p_limited))
        {
          root[rootOffset] = std::min (Pmax - Pmech, Pmech - Pmin);
          if (Pmech > Pmax)
            {
              opFlags.set (p_limit_high);
            }

        }
      else
        {
          //double omega = getControlFrequency (args);
          double omega = args[govOmegaInLocation];
          root[rootOffset] = (args[govpSetInLocation] - Pmech - K * Loc.diffStateLoc[1] - K * T2 * (omega - 1.0) / T1) / T3;
        }
      ++rootOffset;
    }

}

void gridDynGovernorIeeeSimple::rootTrigger (double /*ttime*/, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
  int rootOffset = offsets.getRootOffset (sMode);
  /*if (opFlags.test (uses_deadband))
    {
      if (rootMask[rootOffset])
        {
          gridDynGovernor::rootTrigger (ttime, args, rootMask, sMode);
        }
      ++rootOffset;
    }
        */
  if (opFlags.test (uses_plimits))
    {
      if (rootMask[rootOffset])
        {
          if (opFlags.test (p_limited))
            {
              opFlags.reset (p_limited);
              opFlags.reset (p_limit_high);
              alert (this, JAC_COUNT_INCREASE);
            }
          else
            {
              opFlags.set (p_limited);
              alert (this, JAC_COUNT_DECREASE);
            }

          derivative (args, nullptr, m_dstate_dt.data (), cLocalSolverMode);
        }
      ++rootOffset;
    }
}


// set parameters
int gridDynGovernorIeeeSimple::set (const std::string &param,  const std::string &val)
{
  return gridDynGovernor::set (param, val);
}

int gridDynGovernorIeeeSimple::set (const std::string &param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;

  //param   = gridDynSimulation::toLower(param);
  if (param == "t3")
    {
      T3 = val;
    }
  else if (param == "pup")
    {
      Pup = unitConversion (val, unitType, puMW, systemBasePower);
    }
  else if (param == "pdown")
    {
      Pdown = unitConversion (val, unitType, puMW, systemBasePower);
    }
  else if (param == "ramplimit")
    {
      Pup = unitConversion (val, unitType, puMW, systemBasePower);
      Pdown = unitConversion (val, unitType, puMW, systemBasePower);
    }
  else
    {
      out = gridDynGovernor::set (param, val, unitType);
    }

  return out;
}








