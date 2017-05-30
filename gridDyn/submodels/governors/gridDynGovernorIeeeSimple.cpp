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

#include "submodels/otherGovernors.h"
#include "generators/gridDynGenerator.h"
#include "core/objectFactory.h"
#include "gridBus.h"
#include  "utilities/matrixData.h"
#include "core/coreObjectTemplates.h"

using namespace gridUnits;

gridDynGovernorIeeeSimple::gridDynGovernorIeeeSimple (const std::string &objName) : gridDynGovernor (objName)
{
  // default values
  K = 16.667;
  T1 = 0.1;
  T2 = 0.15;
  T3 = 0.05;
  offsets.local().local.algSize = 0;
  offsets.local().local.diffSize = 2;
  offsets.local().local.jacSize = 6;
  opFlags.set (ignore_deadband);
  opFlags.set (ignore_filter);
  opFlags.set (ignore_throttle);

}

coreObject *gridDynGovernorIeeeSimple::clone (coreObject *obj) const
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

void gridDynGovernorIeeeSimple::dynObjectInitializeA (coreTime time0, unsigned long flags)
{
  gridDynGovernor::dynObjectInitializeA (time0,flags);
  if ((Pmax < 5000) || (Pmin > -5000))
    {
      offsets.local().local.diffRoots++;
      opFlags.set (uses_plimits);
    }
}

// initial conditions
void gridDynGovernorIeeeSimple::dynObjectInitializeB (const IOdata & /*inputs*/, const IOdata &desiredOutput, IOdata &inputSet)
{
  if (Wref < 0)
    {
      Wref = m_baseFreq;
    }
  m_state[1] = 0;
  m_state[0] = desiredOutput[0];
  inputSet[1] = desiredOutput[0];

}

// residual
void gridDynGovernorIeeeSimple::residual (const IOdata &inputs, const stateData &sD, double resid[],  const solverMode &sMode)
{
  if (isAlgebraicOnly (sMode))
    {
      return;
    }
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);
  derivative (inputs, sD, resid, sMode);

  Loc.destDiffLoc[0] -= Loc.dstateLoc[0];
  Loc.destDiffLoc[1] -= Loc.dstateLoc[1];
}

void gridDynGovernorIeeeSimple::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);

  const double *gs = Loc.diffStateLoc;
  //double omega = getControlFrequency (inputs);
  double omega = inputs[govOmegaInLocation];
  if (opFlags[p_limited])
    {
      Loc.destDiffLoc[0] = 0;
    }
  else
    {
      Loc.destDiffLoc[0] = (inputs[govpSetInLocation] - gs[0] - K * gs[1] - K * T2 * (omega - 1.0) / T1) / T3;
    }

  Loc.destDiffLoc[1] = (-gs[1] + (1 - T2 / T1) * (omega - 1.0)) / T1;
}

void gridDynGovernorIeeeSimple::timestep (coreTime ttime,  const IOdata &inputs, const solverMode &)
{


  gridDynGovernorIeeeSimple::derivative ( inputs, emptyStateData, m_dstate_dt.data (), cLocalSolverMode);
  double dt = ttime - prevTime;
  m_state[0] += dt * m_dstate_dt[0];
  m_state[1] += dt * m_dstate_dt[1];
  if (opFlags[p_limited])
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
}

void gridDynGovernorIeeeSimple::jacobianElements (const IOdata & /*inputs*/, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{
  if  (isAlgebraicOnly (sMode))
    {
      return;
    }
  Lp Loc = offsets.getLocations (sD,nullptr, sMode, this);

  int refI = Loc.diffOffset;
  //use the ad.assign Macro defined in basicDefs
  // ad.assign(arrayIndex, RowIndex, ColIndex, value)
  bool linkOmega = true;
  if (inputLocs[govOmegaInLocation] == kNullLocation)
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
      if (!opFlags[p_limited])
        {
          ad.assign (refI, inputLocs[govOmegaInLocation], -K * T2 / (T1 * T3));
        }
      ad.assign (refI + 1, inputLocs[govOmegaInLocation], (T1 - T2) / (T1 * T1));
    }
  if (opFlags[p_limited])
    {
      ad.assign (refI, refI, sD.cj);
    }
  else
    {
      ad.assign (refI, refI, -1 / T3 - sD.cj);
      ad.assign (refI, refI + 1, -K / T3);

      ad.assignCheck (refI, inputLocs[govpSetInLocation], 1 / T3);
    }
  ad.assign (refI + 1, refI + 1, -1 / T1 - sD.cj);
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

void gridDynGovernorIeeeSimple::rootTest (const IOdata &inputs, const stateData &sD, double root[], const solverMode &sMode)
{

  int rootOffset = offsets.getRootOffset (sMode);
  /*if (opFlags.test (uses_deadband))
    {
      gridDynGovernor::rootTest (inputs, sD, root, sMode);
      ++rootOffset;
    }
        */
  if (opFlags[uses_plimits])
    {
      Lp Loc = offsets.getLocations (sD,nullptr, sMode, this);


      double Pmech = Loc.diffStateLoc[0];

      if (!opFlags[p_limited])
        {
          root[rootOffset] = std::min (Pmax - Pmech, Pmech - Pmin);
          if (Pmech > Pmax)
            {
              opFlags.set (p_limit_high);
            }

        }
      else
        {
          //double omega = getControlFrequency (inputs);
          double omega = inputs[govOmegaInLocation];
          root[rootOffset] = (inputs[govpSetInLocation] - Pmech - K * Loc.diffStateLoc[1] - K * T2 * (omega - 1.0) / T1) / T3;
        }
      ++rootOffset;
    }

}

void gridDynGovernorIeeeSimple::rootTrigger (coreTime /*ttime*/, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode)
{
  int rootOffset = offsets.getRootOffset (sMode);
  /*if (opFlags.test (uses_deadband))
    {
      if (rootMask[rootOffset])
        {
          gridDynGovernor::rootTrigger (ttime, inputs, rootMask, sMode);
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

          derivative (inputs, emptyStateData, m_dstate_dt.data (), cLocalSolverMode);
        }
      ++rootOffset;
    }
}


// set parameters
void gridDynGovernorIeeeSimple::set (const std::string &param,  const std::string &val)
{
  gridDynGovernor::set (param, val);
}

void gridDynGovernorIeeeSimple::set (const std::string &param, double val, units_t unitType)
{

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
      gridDynGovernor::set (param, val, unitType);
    }


}








