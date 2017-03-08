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
#include "core/coreObjectTemplates.h"
#include "generators/gridDynGenerator.h"
#include "gridBus.h"
#include  "utilities/matrixData.h"

using namespace gridUnits;

gridDynGovernorTgov1::gridDynGovernorTgov1 (const std::string &objName) : gridDynGovernorIeeeSimple (objName)
{
  // default values
  K = 16.667;
  //K = 0.5;
  T1 = 0.5;
  T2 = 1.0;
  T3 = 1.0;
  offsets.local().local.diffSize = 2;
  offsets.local().local.algSize = 1;
  offsets.local().local.jacSize = 10;
  opFlags.set (ignore_deadband);
  opFlags.set (ignore_filter);
  opFlags.set (ignore_throttle);
}

coreObject *gridDynGovernorTgov1::clone (coreObject *obj) const
{
  gridDynGovernorTgov1 *gov = cloneBase<gridDynGovernorTgov1, gridDynGovernorIeeeSimple> (this, obj);
  if (!gov)
    {
      return obj;
    }
  gov->Dt = Dt;
  return gov;
}

// destructor
gridDynGovernorTgov1::~gridDynGovernorTgov1 ()
{

}

// initial conditions
void gridDynGovernorTgov1::dynObjectInitializeB (const IOdata & /*inputs*/, const IOdata &desiredOutput, IOdata &inputSet)
{

  m_state[2] = desiredOutput[PoutLocation];
  m_state[1] = desiredOutput[PoutLocation];
  m_state[0] = desiredOutput[PoutLocation];
  inputSet[govpSetInLocation] = desiredOutput[PoutLocation];

}


// residual
void gridDynGovernorTgov1::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  //double omega = getControlFrequency (inputs);
  double omega = inputs[govOmegaInLocation];
  Lp Loc = offsets.getLocations (sD,resid, sMode, this);
  Loc.destLoc[0] = Loc.algStateLoc[0] - Loc.diffStateLoc[0] + Dt * (omega - 1.0);

  if (isAlgebraicOnly (sMode))
    {
      return;
    }

  derivative (inputs, sD, resid, sMode);

  Loc.destDiffLoc[0] -= Loc.dstateLoc[0];
  Loc.destDiffLoc[1] -= Loc.dstateLoc[1];

}

void gridDynGovernorTgov1::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);

  const double *gs = Loc.diffStateLoc;
  //double omega = getControlFrequency (inputs);
  double omega = inputs[govOmegaInLocation];

  if (opFlags[p_limited])
    {
      Loc.destDiffLoc[1] = 0.0;
    }
  else
    {
      Loc.destDiffLoc[1] = (-gs[1] + inputs[govpSetInLocation] - K * (omega - 1.0) ) / T1;
    }

  Loc.destDiffLoc[0] = (Loc.diffStateLoc[1] - Loc.diffStateLoc[0] - T2 * Loc.destDiffLoc[1]) / T3;
}

void gridDynGovernorTgov1::timestep (coreTime ttime, const IOdata &inputs, const solverMode &)
{
  gridDynGovernorTgov1::derivative (inputs, emptyStateData, m_dstate_dt.data (), cLocalSolverMode);
  double dt = ttime - prevTime;
  m_state[1] += dt * m_dstate_dt[1];
  m_state[2] += dt * m_dstate_dt[2];
  //double omega = getControlFrequency (inputs);
  double omega = inputs[govOmegaInLocation];
  m_state[0] =  m_state[1] - Dt * (omega - 1.0);

  prevTime = ttime;

}

void gridDynGovernorTgov1::jacobianElements (const IOdata & /*inputs*/, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{

  Lp Loc = offsets.getLocations  (sD,nullptr, sMode,  this);

  int refI = Loc.diffOffset;
  //use the ad.assign Macro defined in basicDefs
  // ad.assign(arrayIndex, RowIndex, ColIndex, value)



  bool linkOmega = (inputLocs[govOmegaInLocation] != kNullLocation);

  /*
  if (opFlags.test (uses_deadband))
    {
      if (!opFlags.test (outside_deadband))
        {
          linkOmega = false;
        }
    }
        */
  //Loc.destLoc[0] = Loc.algStateLoc[0] - Loc.diffStateLoc[0] + Dt*(omega - Wref) / m_baseFreq;
  // Pm
  if (linkOmega)
    {
      ad.assign (Loc.algOffset, inputLocs[govOmegaInLocation], Dt );
    }

  ad.assign (Loc.algOffset, Loc.algOffset, 1);
  if (isAlgebraicOnly (sMode))
    {
      return;
    }
  ad.assign (Loc.algOffset, refI, -1);

  if (opFlags[p_limited])
    {
      ad.assign (refI + 1, refI + 1, sD.cj);
      ad.assign (refI + 1, refI, 1 / T3);
      ad.assign (refI + 1, refI + 1, -1 / T3 - sD.cj);
    }
  else
    {
      ad.assignCheckCol (refI + 1, inputLocs[govpSetInLocation], 1 / T1);
      ad.assign (refI + 1, refI + 1, -1 / T1 - sD.cj);
      if (linkOmega)
        {
          ad.assign (refI + 1, inputLocs[govOmegaInLocation], -K / (T1));
        }

      ad.assign (refI, refI + 1, (1 + T2 / T1) / T3);
      ad.assignCheckCol (refI, inputLocs[govpSetInLocation], -T2 / T1 / T3);
      if (linkOmega)
        {
          ad.assign (refI, inputLocs[govOmegaInLocation], K * T2 / (T1) / T3);
        }
      ad.assign (refI, refI, -1 / T3 - sD.cj);

    }

  // Loc.destDiffLoc[0] = (Loc.diffStateLoc[1] - Loc.diffStateLoc[0] - T2 * (-gs[1] + inputs[govpSetInLocation] - K * (omega - Wref) / m_baseFreq) / T1) / T3;


}

void gridDynGovernorTgov1::rootTest (const IOdata &inputs, const stateData &sD, double root[], const solverMode &sMode)
{

  int rootOffset = offsets.getRootOffset (sMode);
  /* if (opFlags.test (uses_deadband))
     {
       gridDynGovernor::rootTest (inputs, sD, root, sMode);
       ++rootOffset;
     }*/
  if (opFlags[uses_plimits])
    {
      Lp Loc = offsets.getLocations (sD,nullptr, sMode, this);


      double Pmech = Loc.diffStateLoc[1];

      if (opFlags[p_limited])
        {
          //double omega = getControlFrequency (inputs);
          double omega = inputs[govOmegaInLocation];
          root[rootOffset] = (-Pmech + inputs[govpSetInLocation] - K * (omega - 1.0)) / T1;
        }
      else
        {
          root[rootOffset] = std::min (Pmax - Pmech, Pmech - Pmin);
          if (Pmech > Pmax)
            {
              opFlags.set (p_limit_high);
            }
        }
      ++rootOffset;
    }

}

void gridDynGovernorTgov1::rootTrigger (coreTime /*ttime*/, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode)
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
  if (opFlags[uses_plimits])
    {
      if (rootMask[rootOffset])
        {
          if (opFlags[p_limited])
            {
              opFlags.reset (p_limited);
              opFlags.reset (p_limit_high);
              alert (this, JAC_COUNT_INCREASE);
              LOG_DEBUG ("at max power limit");
            }
          else
            {
              if (opFlags[p_limit_high])
                {
                  LOG_DEBUG ("at max power limit");
                }
              else
                {
                  LOG_DEBUG ("at min power limit");
                }
              opFlags.set (p_limited);
              alert (this, JAC_COUNT_DECREASE);
            }
          derivative (inputs, emptyStateData, m_dstate_dt.data (), cLocalSolverMode);
        }
      ++rootOffset;
    }
}

index_t gridDynGovernorTgov1::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  if (field == "pm")
    {
      ret = offsets.getAlgOffset (sMode);
    }
  else if (field == "v1")
    {
      ret = offsets.getDiffOffset (sMode);
    }
  else if (field == "v2")
    {
      ret = offsets.getAlgOffset (sMode);
      ret = (ret != kNullLocation) ? ret + 1 : ret;
    }
  return ret;
}

// set parameters
void gridDynGovernorTgov1::set (const std::string &param,  const std::string &val)
{
  gridDynGovernorIeeeSimple::set (param, val);
}

void gridDynGovernorTgov1::set (const std::string &param, double val, units_t unitType)
{
 
  //param   = gridDynSimulation::toLower(param);
  if (param == "dt")
    {
      Dt = val;
    }
  else
    {
      gridDynGovernorIeeeSimple::set (param, val, unitType);
    }

 
}








