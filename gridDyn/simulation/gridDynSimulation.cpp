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

#include "gridDyn.h"
#include "gridEvent.h"
#include "eventQueue.h"
#include "loadModels/gridLabDLoad.h"
#include "gridBus.h"
#include "gridRecorder.h"
#include "objectFactoryTemplates.h"
#include "griddyn-tracer.h"
#include "objectInterpreter.h"
#include "solvers/solverInterface.h"
#include "stringOps.h"
#include "arrayDataSparse.h"
#include "gridDynSimulationFileOps.h"
#include "gridCoreTemplates.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <cmath>
#include <map>
#include <queue>
#include <cassert>

static typeFactory<gridDynSimulation> gf ("simulation", stringVec { "griddyn", "gridlab" "gridlabd" }, "griddyn");

gridDynSimulation* gridDynSimulation::s_instance = nullptr;

//local search functions for MPI based objects
count_t searchForGridlabDobject (gridCoreObject *obj);

gridDynSimulation::gridDynSimulation (const std::string &objName) : gridSimulation (objName),controlFlags (0ll)
{
  // defaults
#ifndef KLU_ENABLE
  controlFlags.set (dense_solver);
#endif

}

void gridDynSimulation::setInstance (gridDynSimulation* s)
{
  s_instance = s;
}

gridDynSimulation* gridDynSimulation::getInstance (void)
{
  return s_instance;
}

gridCoreObject *gridDynSimulation::clone (gridCoreObject *obj) const
{
  gridDynSimulation *sim = cloneBase<gridDynSimulation, gridSimulation> (this, obj);
  if (sim == nullptr)
    {
      return obj;
    }
  sim->controlFlags = controlFlags;
  sim->max_Vadjust_iterations = max_Vadjust_iterations;
  sim->max_Padjust_iterations = max_Padjust_iterations;


  sim->probeStepTime = probeStepTime;   
  sim->powerAdjustThreshold = powerAdjustThreshold;  
  sim->powerFlowStartTime = powerFlowStartTime;     
  sim->tols = tols;


  sim->default_ordering = default_ordering; 
  sim->powerFlowFile = powerFlowFile; 
  //std::vector < std::shared_ptr < solverInterface >> solverInterfaces; 
  //std::vector<gridObject *>singleStepObjects;
  //now clone the solverInterfaces
  count_t solverInterfaceCount = static_cast<count_t>(solverInterfaces.size());
  sim->solverInterfaces.resize(solverInterfaceCount);
  for (index_t kk=2;kk< solverInterfaceCount;++kk)
  {
	  if (solverInterfaces[kk])
	  {
		  sim->solverInterfaces[kk] = solverInterfaces[kk]->clone(sim->solverInterfaces[kk], true);
	  }
	  
  }
  if ((defPowerFlowMode->offsetIndex<solverInterfaceCount)&&(solverInterfaces[defPowerFlowMode->offsetIndex]))
  {
	  sim->defPowerFlowMode = &(sim->solverInterfaces[defPowerFlowMode->offsetIndex]->getSolverMode());
  }
  if ((defDAEMode->offsetIndex<solverInterfaceCount) && (solverInterfaces[defDAEMode->offsetIndex]))
  {
	  sim->defDAEMode = &(sim->solverInterfaces[defDAEMode->offsetIndex]->getSolverMode());
  }
  if ((defDynAlgMode->offsetIndex<solverInterfaceCount) && (solverInterfaces[defDynAlgMode->offsetIndex]))
  {
	  sim->defDynAlgMode = &(sim->solverInterfaces[defDynAlgMode->offsetIndex]->getSolverMode());
  }
  if ((defDynDiffMode->offsetIndex<solverInterfaceCount) && (solverInterfaces[defDynDiffMode->offsetIndex]))
  {
	  sim->defDynDiffMode = &(sim->solverInterfaces[defDynDiffMode->offsetIndex]->getSolverMode());
  }
  return sim;
}

int gridDynSimulation::checkNetwork (network_check_type checkType)
{
  //make a full list of all buses
  std::vector<gridBus *> bnetwork;
  bnetwork.reserve (busCount);
  getBusVector (bnetwork);
  if (checkType == network_check_type::full)
    {
      slkBusses.clear ();

      for (auto &bus : bnetwork)
        {
          if (bus->enabled)
            {
              //check to make sure the bus can actually work
              if (bus->checkCapable ())
                {
                  bus->Network = 0;
                  auto bt = bus->getType ();
                  if ((bt == gridBus::busType::SLK) || (bt == gridBus::busType::afix))
                    {
                      slkBusses.push_back (bus);
                    }
                }
              else
                {
                  bus->disable ();
                  bus->Network = -1;
                }

            }
          else
            {
              bus->Network = -1;
            }

        }
    }
  else
    {
      for (auto &bus : bnetwork)
        {
          if (bus->Network > 0)
            {
              bus->Network = 0;
            }
        }
    }
  int networkNum = 0;
  std::queue<gridBus *> bstk;
  for (auto &bn : bnetwork)
    {
      if ((bn->Network == 0) && (bn->isConnected ()))
        {
          networkNum++;
          bn->followNetwork (networkNum, bstk);
          while (!bstk.empty ())
            {
              if (bstk.front ()->Network != networkNum)
                {
                  bstk.front ()->followNetwork (networkNum, bstk);
                }
              bstk.pop ();
            }
        }
    }
  //check to make sure we have a swing bus for each network

  for (int nn = 1; nn <= networkNum; nn++)
    {
      bool slkfnd = false, pvfnd = false,afixfnd = false;
      for (auto &bn : slkBusses)
        {
          if (bn->Network == nn)
            {
              if (bn->getType () == gridBus::busType::SLK)
                {
                  slkfnd = true;
                  break;
                }
              else
                {
                  afixfnd = true;
                }
            }
        }
      if (slkfnd == false)
        {
          for (auto &bn : bnetwork)
            {
              if (bn->Network == nn)
                {
                  if (bn->getType () == gridBus::busType::SLK)
                    {
                      slkfnd = true;
                      break;
                    }
                  else if (bn->getType () == gridBus::busType::PV)
                    {
                      pvfnd = true;
                    }
                  else if (bn->getType () == gridBus::busType::afix)
                    {
                      afixfnd = true;
                    }
                  if (afixfnd&&pvfnd)
                    {
                      slkfnd = true;                                               //a pv bus and a separate afix bus can function as a slk bus
                      break;
                    }
                }
            }
        }
      //now we go into a loop to make a new slack bus
      if (slkfnd == false)
        {
          if (controlFlags[no_auto_slack_bus])
            {
              if (controlFlags[no_auto_disconnect])
                {
                  LOG_ERROR ("no SLK bus in network");
                  for (auto &bn : bnetwork)
                    {
                      if (bn->Network == nn)
                        {
                          printf ("Network %d bus %d:%s\n", nn, bn->getUserID (), bn->getName ().c_str ());
                        }
                    }
                  return NO_SLACK_BUS_FOUND;
                }
              else
                {
                  LOG_WARNING ("no SLK bus found in network disconnecting buses");
                  for (auto &bn : bnetwork)
                    {
                      if (bn->Network == nn)
                        {
                          bn->disconnect ();
                        }
                    }
                }
            }
          else
            {
              gridBus *gb = nullptr;
              double maxcap = 0.0;
              for (auto &bn : bnetwork)
                {
                  if (bn->Network == nn)
                    {
                      if (bn->getType () == gridBus::busType::PV)
                        {
                          double cap = bn->getAdjustableCapacityUp ();
                          if (cap > maxcap)
                            {
                              maxcap = cap;
                              gb = bn;
                            }
                        }
                    }
                }
              if (gb)
                {
                  gb->set ("type", "slk");
                }
              else
                {
                  if (controlFlags[no_auto_disconnect])
                    {
                      LOG_ERROR ("no SLK bus or PV bus found in network");
                      for (auto &bn : bnetwork)
                        {
                          if (bn->Network == nn)
                            {
                              printf ("Network %d bus %d:%s\n", nn, bn->getUserID (), bn->getName ().c_str ());
                            }
                        }
                      return NO_SLACK_BUS_FOUND;
                    }
                  else
                    {
                      LOG_WARNING ("no SLK or PV bus found in network disconnecting buses");
                      for (auto &bn : bnetwork)
                        {
                          if (bn->Network == nn)
                            {
                              bn->disconnect ();
                            }
                        }
                    }


                }

            }

        }
    }

  return 0;
}

double gridDynSimulation::getState (index_t offset) const
{
  return getState (offset, cLocalSolverMode);
}

double gridDynSimulation::getState (index_t offset, const solverMode &sMode) const
{
  solverMode nMode = sMode;
  double ret = kNullVal;
  if (sMode.local)
    {
      switch (pState)
        {
        case gridState_t::POWERFLOW_COMPLETE:
        case gridState_t::INITIALIZED:
          nMode = *defPowerFlowMode;
          break;
        case gridState_t::DYNAMIC_INITIALIZED:
        case gridState_t::DYNAMIC_COMPLETE:
        case gridState_t::DYNAMIC_PARTIAL:
          nMode = *defDAEMode;
          break;
        default:
          nMode = cEmptySolverMode;                          //this should actually do nothing since the size should be 0
        }
    }


  auto solData = getSolverInterface (nMode);
  if (solData)
    {
      double *state = solData->state_data ();
      if (solData->size () > offset)
        {
          ret = state[offset];
        }
    }

  return ret;
}

std::vector<double> gridDynSimulation::getState (const solverMode &sMode) const
{
  solverMode nMode = sMode;
  if (sMode.local)
    {
      switch (pState)
        {
        case gridState_t::POWERFLOW_COMPLETE:
        case gridState_t::INITIALIZED:
          nMode = *defPowerFlowMode;
          break;
        case gridState_t::DYNAMIC_INITIALIZED:
        case gridState_t::DYNAMIC_COMPLETE:
        case gridState_t::DYNAMIC_PARTIAL:
          nMode = *defDAEMode;
          break;
        default:
          nMode = cEmptySolverMode;                            //this should actually do nothing since the size should be 0
        }
    }

  std::vector<double> st;
  auto solData = getSolverInterface (nMode);
  if (solData)
    {
      double *state = solData->state_data ();
      if (solData->size () != 0)
        {
          st.assign (state, state + solData->size ());
        }
    }

  return st;
}

/*
mixed = 0,  //!< everything is mixed through eachother
grouped = 1,  //!< all similar variables are grouped together (angles, then voltage, then algebraic, then differential)
algebraic_grouped = 2, //!< all the algebraic variables are grouped, then the differential
voltage_first = 3, //!< grouped with the voltage coming first
angle_first = 4,  //!< grouped with the angle coming first
differential_first = 5, //!< differential and algebraic groupd with differential first
*/

void gridDynSimulation::setupOffsets (const solverMode &sMode, offset_ordering offsetOrdering)
{
  solverOffsets b;
  switch (offsetOrdering)
    {
    case offset_ordering::mixed:
      b.algOffset = 0;
      break;
    case offset_ordering::grouped:
    case offset_ordering::voltage_first:
      b.vOffset = 0;
      b.aOffset = voltageStateCount (sMode);
      b.algOffset = b.aOffset + angleStateCount (sMode);
      if (hasDifferential (sMode))
        {
          b.diffOffset = totalAlgSize (sMode);
        }
      break;
    case offset_ordering::algebraic_grouped:
      b.algOffset = 0;
      if (hasDifferential (sMode))
        {
          b.diffOffset = totalAlgSize (sMode);
        }
      break;
    case offset_ordering::angle_first:
      b.aOffset = 0;
      b.vOffset = angleStateCount (sMode);
      b.algOffset = b.aOffset + voltageStateCount (sMode);
      if (hasDifferential (sMode))
        {
          b.diffOffset = totalAlgSize (sMode);
        }
      break;
    case offset_ordering::differential_first:
      b.diffOffset = 0;
      b.algOffset = diffSize (sMode);
      break;
    }

  //call the area setOffset function to distribute the offsets
  setOffsets (b, sMode);
}

// --------------- run the simulation ---------------

int gridDynSimulation::run (double te)
{
  gridDynAction gda;
  gda.command = gridDynAction::gd_action_t::run;
  gda.val_double = te;
  return execute (gda);
}


int gridDynSimulation::add (std::string actionString)
{
  gridDynAction gda (actionString);
  actionQueue.push (gda);
  return OBJECT_ADD_SUCCESS;
}

int gridDynSimulation::add (gridDynAction &newAction)
{
  actionQueue.push (newAction);
  return OBJECT_ADD_SUCCESS;
}


int gridDynSimulation::execute (const std::string &cmd)
{
  gridDynAction gda (cmd);
  return execute (gda);
}

int gridDynSimulation::execute (const gridDynAction &cmd)
{
  int out = FUNCTION_EXECUTION_SUCCESS;
  int out2 = FUNCTION_EXECUTION_SUCCESS;
  objInfo obi;
  double t_start, t_step, t_end;
  switch (cmd.command)
    {
    case gridDynAction::gd_action_t::ignore:
      break;
    case gridDynAction::gd_action_t::set:
      obi.LoadInfo (cmd.string1, this);
      if (cmd.val_double == kNullVal)
        {
          out2 = obi.m_obj->set (obi.m_field, cmd.string2);
        }
      else
        {
          out2 = obi.m_obj->set (obi.m_field, cmd.val_double, obi.m_unitType);
        }
      if (out2 != PARAMETER_FOUND)
        {
          out = FUNCTION_EXECUTION_FAILURE;
        }
      break;
    case gridDynAction::gd_action_t::setall:
      setAll (cmd.string1, cmd.string2, cmd.val_double);
      break;
    case gridDynAction::gd_action_t::settime:
      setTime (cmd.val_double);
      break;
    case gridDynAction::gd_action_t::setsolver:
      solverSet (cmd.string1, cmd.string2, cmd.val_double);
      break;
    case gridDynAction::gd_action_t::print:
      break;
    case gridDynAction::gd_action_t::powerflow:
      out = powerflow ();
      break;
    case gridDynAction::gd_action_t::contingency:
      out = FUNCTION_EXECUTION_FAILURE;
      break;
    case gridDynAction::gd_action_t::continuation:
      out = FUNCTION_EXECUTION_FAILURE;
      break;
    case gridDynAction::gd_action_t::initialize:
      t_start = (cmd.val_double != kNullVal) ? cmd.val_double : startTime;

      if (pState == gridState_t::STARTUP)
        {
          out2 = pFlowInitialize (t_start);
          if (out2 == 1)
            {
              LOG_ERROR ("unable to initialize powerflow");
              out = FUNCTION_EXECUTION_FAILURE;
            }
        }
      else if (pState == gridState_t::POWERFLOW_COMPLETE)
        {
          out2 = dynInitialize (t_start);
          if (out2 != 0)
            {
              LOG_ERROR ("unable to dynamic power initialization");
              return FUNCTION_EXECUTION_FAILURE;
            }
        }
      break;
    case gridDynAction::gd_action_t::iterate:
      t_step = (cmd.val_double != kNullVal) ? cmd.val_double : stepTime;
      t_end = (cmd.val_double2 != kNullVal) ? cmd.val_double2 : stopTime;
      out = eventDrivenPowerflow (t_end, t_step);
      break;
    case gridDynAction::gd_action_t::eventmode:
      t_end = (cmd.val_double != kNullVal) ? cmd.val_double : stopTime;
      out = eventDrivenPowerflow (t_end);
      break;
    case gridDynAction::gd_action_t::dynamicDAE:
      t_end = (cmd.val_double != kNullVal) ? cmd.val_double : stopTime;
      if (pState < gridState_t::DYNAMIC_INITIALIZED)
        {
          out2 = dynInitialize (-kBigNum);
          if (out2 != FUNCTION_EXECUTION_SUCCESS)
            {
              return out2;
            }
        }

      if (hasDynamics ())
        {
          LOG_WARNING ("No Differential states halting computation");
          return out2;
        }
      out2 = dynamicDAE (t_end);
      out = out2;
      break;
    case gridDynAction::gd_action_t::dynamicPart:
      t_end = (cmd.val_double != kNullVal) ? cmd.val_double : stopTime;
      t_step = (cmd.val_double2 != kNullVal) ? cmd.val_double2 : stepTime;
      if (pState < gridState_t::DYNAMIC_INITIALIZED)
        {
          out2 = dynInitialize (-kBigNum);
          if (out2 != FUNCTION_EXECUTION_SUCCESS)
            {
              return out2;
            }
        }

      if (hasDynamics ())
        {
          LOG_WARNING ("No Differential states halting computation");
          return out2;
        }
      out2 = dynamicPartitioned (t_end, t_step);
      out = out2;
      break;
    case gridDynAction::gd_action_t::dynamicDecoupled:
      t_end = (cmd.val_double != kNullVal) ? cmd.val_double : stopTime;
      t_step = (cmd.val_double2 != kNullVal) ? cmd.val_double2 : stepTime;
      if (pState < gridState_t::DYNAMIC_INITIALIZED)
        {
          out2 = dynInitialize (-kBigNum);
          if (out2 != FUNCTION_EXECUTION_SUCCESS)
            {
              return out2;
            }
        }

      out2 = dynamicDecoupled (t_end, t_step);
      out = out2;
      break;
    case gridDynAction::gd_action_t::step:
      t_step = (cmd.val_double != kNullVal) ? cmd.val_double : stopTime;
      if (pState < gridState_t::DYNAMIC_INITIALIZED)
        {
          out2 = dynInitialize (-kBigNum);
        }
      if (out2 != FUNCTION_EXECUTION_SUCCESS)
        {
          return out2;
        }
      if (!hasDynamics ())
        {
          LOG_WARNING ("No Differential states halting computation");
          return out2;
        }
      out = step (t_step, t_end);
      break;
    case gridDynAction::gd_action_t::run:
      if (actionQueue.empty ())
        {
          t_end = (cmd.val_double != kNullVal) ? cmd.val_double : stopTime;
          if (controlFlags[power_flow_only])
            {
              out2 = powerflow ();
            }
          else
            {
              if (pState < gridState_t::DYNAMIC_INITIALIZED)
                {
                  if (out != FUNCTION_EXECUTION_SUCCESS)
                    {
                      return out;
                    }
                  out2 = dynInitialize (timeCurr);
                }

              if (hasDynamics ())
                {
                  switch (defaultDynamicSolverMethod)
                    {
                    case dynamic_solver_methods::dae:
                      out2 = dynamicDAE (t_end);
                      break;
                    case dynamic_solver_methods::partitioned:
                      out2 = dynamicPartitioned (t_end, stepTime);
                      break;
                    case dynamic_solver_methods::decoupled:
                      out2 = dynamicDecoupled (t_end, stepTime);
                      break;
                    }

                }
              else
                {
                  LOG_WARNING ("No Differential states moving to stepped power flow");
                  out = eventDrivenPowerflow (t_end, stepTime);
                }

              if (timeCurr >= stopTime)
                {
                  saveRecorders ();
                }
            }

        }
      else
        {
          while (!actionQueue.empty ())
            {
              out = execute (actionQueue.front ());
              actionQueue.pop ();
              if (out != FUNCTION_EXECUTION_SUCCESS)
                {
                  return out;
                }
            }
        }
      break;
    case gridDynAction::gd_action_t::reset:
      if (cmd.val_int >= 0)
        {
          reset (static_cast<reset_levels> (cmd.val_int));
        }
      else
        {
          reset (reset_levels::minimal);
        }
      break;
    case gridDynAction::gd_action_t::save:
      if (cmd.string1 == "recorder")
        {
          saveRecorders ();
        }
      else if (cmd.string1 == "state")
        {
          saveState (this, (cmd.string2.empty ()) ? stateFile : cmd.string2);
        }
      else if (cmd.string1 == "Jacobian")
        {
          saveJacobian (this, (cmd.string2.empty ()) ? "jacobian_" + name + ".bin" : cmd.string2);
        }
      else if (cmd.string1 == "powerflow")
        {
          savePowerFlow (this, (cmd.string2.empty ()) ? powerFlowFile : cmd.string2);
        }
      else if (cmd.string1 == "voltage")
        {
        }
      else if (cmd.string1 == "jacstate")
        {
        }
      break;
    case gridDynAction::gd_action_t::load:
      if (cmd.string1 == "state")
        {
        }
      else if (cmd.string1 == "powerflow")
        {
        }
      break;
    case gridDynAction::gd_action_t::add:
      break;

    case gridDynAction::gd_action_t::rollback:
      break;
    case gridDynAction::gd_action_t::checkpoint:
      break;
    }
  return out;
}

bool gridDynSimulation::hasDynamics () const
{
  return (diffSize (*defDAEMode) > 0);
}

//need to update probably with a new field in solverInterface
count_t gridDynSimulation::nonZeros (const solverMode &sMode) const
{

  return getSolverInterface (sMode)->nonZeros ();

}

// --------------- set properties ---------------
int gridDynSimulation::set (const std::string &param, const std::string &val)
{
  int out = PARAMETER_FOUND;
  if (param == "powerflowfile")
    {
      powerFlowFile = val;
      controlFlags.set (save_power_flow_data);
    }
  else if (param == "defpowerflow")
    {
      out = setDefaultMode (solution_modes_t::powerflow_mode, getSolverMode (val));
    }
  else if (param == "defdae")
    {
      out = setDefaultMode (solution_modes_t::dae_mode, getSolverMode (val));
    }
  else if (param == "defdynalg")
    {
      out = setDefaultMode (solution_modes_t::algebraic_mode, getSolverMode (val));
    }
  else if (param == "defdyndiff")
    {
      out = setDefaultMode (solution_modes_t::differential_mode, getSolverMode (val));
    }
  else if (param == "action")
    {
      auto v = splitlineTrim (val, ';');
      for (auto &actionString : v)
        {
          int ot = add (actionString);
          if (ot != OBJECT_ADD_SUCCESS)
            {
              out = INVALID_PARAMETER_VALUE;
            }
        }
    }
  else if (param == "ordering")
    {
      auto order = convertToLowerCase (val);
      if (order == "mixed")
        {
          default_ordering = offset_ordering::mixed;
        }
      else if (order == "grouped")
        {
          default_ordering = offset_ordering::grouped;
        }
      else if (order == "algebraic_grouped")
        {
          default_ordering = offset_ordering::algebraic_grouped;
        }
      else if (order == "voltage_first")
        {
          default_ordering = offset_ordering::voltage_first;
        }
      else if (order == "angle_first")
        {
          default_ordering = offset_ordering::angle_first;
        }
      else if (order == "differential_first")
        {
          default_ordering = offset_ordering::differential_first;
        }
    }
  else if (param == "dynamicsolvermethod")
    {
      auto method = convertToLowerCase (val);
      if (method == "dae")
        {
          defaultDynamicSolverMethod = dynamic_solver_methods::dae;
        }
      else if (method == "partitioned")
        {
          defaultDynamicSolverMethod = dynamic_solver_methods::partitioned;
        }
      else if (method == "decoupled")
        {
          defaultDynamicSolverMethod = dynamic_solver_methods::decoupled;
        }
      else
        {
          out = INVALID_PARAMETER_VALUE;
        }
    }
  else
    {
      out = gridSimulation::set (param, val);
    }


  return out;
}

std::string gridDynSimulation::getString (const std::string &param) const
{
  if (param == "powerflowfile")
    {
      return powerFlowFile;
    }
  else
    {
      return gridSimulation::getString (param);
    }
}

int gridDynSimulation::setDefaultMode (solution_modes_t mode, const solverMode &sMode)
{
  int out = PARAMETER_FOUND;
  auto sd = getSolverInterface (sMode);
  if (!sd)
    {
      sd = makeSolver (this, sMode);
      add (sd);
    }
  switch (mode)
    {
    case solution_modes_t::powerflow_mode:
      if (isAlgebraicOnly (sMode))
        {
          if (opFlags[pFlow_initialized])
            {
              if (sd->isInitialized ())
                {
                  reInitpFlow ((sd->getSolverMode ()));
                }
            }
          sd->set ("dynamic", false);
          defPowerFlowMode = &(sd->getSolverMode ());
        }
      else
        {
          out = INVALID_PARAMETER_VALUE;
        }
      break;
    case solution_modes_t::dae_mode:
      if (isDAE (sMode))
        {
          if (opFlags[dyn_initialized])
            {
              if (sd->isInitialized ())
                {
                  reInitDyn ((sd->getSolverMode ()));
                }
            }

          defDAEMode = &(sd->getSolverMode ());
        }
      else
        {
          out = INVALID_PARAMETER_VALUE;
        }
      break;
    case solution_modes_t::algebraic_mode:
      if (isAlgebraicOnly (sMode))
        {
          if (opFlags[dyn_initialized])
            {
              if (sd->isInitialized ())
                {
                  reInitDyn ((sd->getSolverMode ()));
                }
            }
          sd->set ("dynamic", true);
          defDynAlgMode = &(sd->getSolverMode ());
        }
      else
        {
          out = INVALID_PARAMETER_VALUE;
        }
      break;
    case solution_modes_t::differential_mode:
      if (isDifferentialOnly (sMode))
        {
          if (opFlags[dyn_initialized])
            {
              if (sd->isInitialized ())
                {
                  reInitDyn ((sd->getSolverMode ()));
                }
            }
          defDynDiffMode = &(sd->getSolverMode ());
        }
      else
        {
          out = INVALID_PARAMETER_VALUE;
        }
      break;
    }

  return out;
}

/* *INDENT-OFF* */
std::map<std::string, int> flagControlMap
{ {"autoallocate",power_adjust_enabled},
  {"power_adjust",power_adjust_enabled },
  {"sparse",-dense_solver},
  {"dense",dense_solver},
  { "no_auto_autogen",no_auto_autogen},
  {"auto_bus_disconnect",auto_bus_disconnect},
  {"roots_disabled",roots_disabled},
  {"voltageconstraints",voltage_constraints_flag},
  {"record_on_halt",record_on_halt_flag},
  {"constraints_disabled",constraints_disabled},
  {"dc_mode",dc_mode},
  {"dcFlow_initialization",dcFlow_initialization},
  {"no_link_adjustments",no_link_adjustments},
  {"disable_link_adjustments",no_link_adjustments},
  {"ignore_bus_limits",ignore_bus_limits},
  { "powerflow_only",power_flow_only },
  { "no_powerflow_adjustments",no_powerflow_adjustments },
  {"savepowerflow",save_power_flow_data},
  {"low_voltage_check",low_voltage_checking},
  {"no_powerflow_error_recovery",no_powerflow_error_recovery},
  {"dae_initialization_for_partitioned",	dae_initialization_for_partitioned },
};

/* *INDENT-ON* */
int gridDynSimulation::setFlag (const std::string &flag, bool val)
{
  int out = PARAMETER_FOUND;
  auto mpf = flagControlMap.find (flag);
  if (mpf != flagControlMap.end ())
    {
      if (mpf->second < 0)
        {
          controlFlags.set (-mpf->second, !val);
        }
      else
        {
          controlFlags.set (mpf->second, val);
        }
    }
  else if (flag == "threads")
    {
      /*parallel_residual_enabled = 7,
                      parallel_jacobian_enabled = 8,
                      parallel_contingency_enabled = 9,*/
      controlFlags.set (parallel_residual_enabled, val);
      controlFlags.set (parallel_contingency_enabled, val);
      //TODO:: PT add some more options controls here
    }
  else
    {
      out = gridSimulation::setFlag (flag, val);
    }
  return out;
}

int gridDynSimulation::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  using namespace gridUnits;
  int out = PARAMETER_FOUND;

  if ((param == "tolerance") || (param == "rtol"))
    {
      tols.rtol = val;
    }
  else if (param == "voltagetolerance")
    {
      tols.voltageTolerance = unitConversionPower (val, unitType, puV, systemBasePower);
    }
  else if (param == "angletolerance")
    {
      tols.angleTolerance = unitConversionAngle (val, unitType, rad);
    }
  else if (param == "defaulttolerance")
    {
      tols.defaultTolerance = val;
    }
  else if (param == "tolerancerelaxation")
    {
      tols.toleranceRelaxationFactor = val;
    }
  else if (param == "powerflowstarttime")
    {
      powerFlowStartTime = unitConversionTime (val, unitType, sec);
    }
  else if (param == "timetolerance")
    {
      tols.timeTol = unitConversionTime (val, unitType, sec);
    }
  else if (param == "poweradjustthreshold")
    {
      powerAdjustThreshold = unitConversionPower (val, unitType, puMW, systemBasePower);
    }
  else if (param == "maxpoweradjustiterations")
    {
      max_Padjust_iterations = static_cast<count_t> (val);
    }
  else if (param == "defpowerflow")
    {
      out = setDefaultMode (solution_modes_t::powerflow_mode, getSolverMode (static_cast<index_t> (val)));
    }
  else if (param == "defdae")
    {
      out = setDefaultMode (solution_modes_t::dae_mode, getSolverMode (static_cast<index_t> (val)));
    }
  else if (param == "defdynalg")
    {
      out = setDefaultMode (solution_modes_t::algebraic_mode, getSolverMode (static_cast<index_t> (val)));
    }
  else if (param == "defdyndiff")
    {
      out = setDefaultMode (solution_modes_t::differential_mode, getSolverMode (static_cast<index_t> (val)));
    }
  else if (param == "maxvoltageadjustiterations")
    {
      max_Vadjust_iterations = static_cast<count_t> (val);
    }
  else
    {
      //out = setFlags (param, val);
      out = gridSimulation::set (param, val, unitType);
      if (out == PARAMETER_NOT_FOUND)
        {
          out = setFlag (param, (val > 0.1));
        }

    }
  return out;
}


int gridDynSimulation::solverSet (const std::string &solverName, const std::string &field, double val)
{
  auto sd = getSolverInterface (solverName);
  return sd->set (field, val);
}


int gridDynSimulation::solverSet (const std::string &solverName, const std::string &field, const std::string &val)
{
  auto sd = getSolverInterface (solverName);
  return sd->set (field, val);
}


double gridDynSimulation::get (const std::string &param, gridUnits::units_t unitType) const
{
  count_t val = kInvalidCount;
  double fval = kNullVal;

  if (param == "statesize")
    {
      if (pState <= gridState_t::POWERFLOW_COMPLETE)
        {
          val = stateSize (*defPowerFlowMode);
        }
      else
        {
          val = stateSize (*defDAEMode);
        }
    }
  else if (param == "nonzeros")
    {
      if (pState <= gridState_t::POWERFLOW_COMPLETE)
        {
          val = nonZeros (*defPowerFlowMode);
        }
      else
        {
          val = nonZeros (*defDAEMode);
        }
    }
  else if (param == "pflowstatesize")
    {
      val = stateSize (*defPowerFlowMode);
    }
  else if (param == "dynstatesize")
    {
      val = stateSize (*defDAEMode);
    }
  else if (param == "pflownonzeros")
    {
      val = nonZeros (*defPowerFlowMode);
    }
  else if (param == "dynnonzeros")
    {
      val = nonZeros (*defDAEMode);
    }
  else if (param == "residcount")
    {
      val = residCount;
    }
  else if (param == "haltcount")
    {
      val = haltCount;
    }
  else if (param == "iterationcount")
    {

      fval = getSolverInterface (*defPowerFlowMode)->get ("iterationcount");

    }
  else if ((param == "jacobiancount") || (param == "jaccount"))
    {
      val = JacobianCount;
    }
  else if (param == "rootcount")
    {
      val = rootCount;
    }
  else if (param == "stepcount")
    {
      auto sd = getSolverInterface (*defDAEMode);
      if ((sd) && (sd->isInitialized ()))
        {
          fval = sd->get ("resevals");
        }
      else
        {
          fval = 0;
        }

    }
  else if (param == "voltagetolerance")
    {
      fval = tols.voltageTolerance;
    }
  else if (param == "angletolerance")
    {
      fval = tols.angleTolerance;
    }
  else if (param == "timetolerance")
    {
      fval = tols.timeTol;
    }
  else if (param == "poweradjustthreshold")
    {
      fval = powerAdjustThreshold;
    }
  else
    {
      fval = gridSimulation::get (param, unitType);
    }
  return (val != kInvalidCount) ? static_cast<double> (val) : fval;
}

static std::map<int, size_t> alertFlags {
  std::make_pair (STATE_COUNT_CHANGE, state_change_flag),
  std::make_pair (STATE_COUNT_INCREASE, state_change_flag),
  std::make_pair (STATE_COUNT_DECREASE, state_change_flag),
  std::make_pair (ROOT_COUNT_CHANGE, root_change_flag),
  std::make_pair (ROOT_COUNT_INCREASE, root_change_flag),
  std::make_pair (ROOT_COUNT_DECREASE, root_change_flag),
  std::make_pair (JAC_COUNT_CHANGE, jacobian_count_change_flag),
  std::make_pair (JAC_COUNT_DECREASE, jacobian_count_change_flag),
  std::make_pair (JAC_COUNT_INCREASE, jacobian_count_change_flag),
  std::make_pair (OBJECT_COUNT_CHANGE, object_change_flag),
  std::make_pair (OBJECT_COUNT_INCREASE, object_change_flag),
  std::make_pair (OBJECT_COUNT_DECREASE, object_change_flag),
  std::make_pair (CONSTRAINT_COUNT_CHANGE, constraint_change_flag),
  std::make_pair (CONSTRAINT_COUNT_INCREASE, constraint_change_flag),
  std::make_pair (CONSTRAINT_COUNT_DECREASE, constraint_change_flag),
  std::make_pair (SLACK_BUS_CHANGE, slack_bus_change),
  std::make_pair (POTENTIAL_FAULT_CHANGE, reset_voltage_flag),
  std::make_pair (VERY_LOW_VOLTAGE_ALERT, low_bus_voltage),
  std::make_pair (INVALID_STATE_ALERT, invalid_state_flag),
  std::make_pair (CONNECTIVITY_CHANGE,connectivity_change_flag),

};

void gridDynSimulation::alert (gridCoreObject *object, int code)
{
  if ((code >= MIN_CHANGE_ALERT) && (code < MAX_CHANGE_ALERT))
    {

      auto res = alertFlags.find (code);
      if (res != alertFlags.end ())
        {
          auto flagNum = res->second;
          opFlags.set (flagNum);
        }
      else
        {
          gridSimulation::alert (object, code);
        }

      gridArea::alert (object, code);
    }
  else if (code == SINGLE_STEP_REQUIRED)
    {
      controlFlags.set (single_step_mode);
      if (dynamic_cast<gridObject *> (object))
        {
          singleStepObjects.push_back (static_cast<gridObject *> (object));
        }

    }
  else
    {
      gridSimulation::alert (object, code);
    }


}

int gridDynSimulation::makeReady (gridState_t desiredState, const solverMode &sMode)
{
  //check to make sure we at or greater than the desiredState
  int retval = FUNCTION_EXECUTION_SUCCESS;
  //check if we have to do some steps first
  //all these function will call this function so it is a recursive function essentially
  if (pState < desiredState)
    {
      switch (desiredState)
        {
        case gridState_t::INITIALIZED:
          retval = pFlowInitialize (timeCurr);
          if (retval != 0)
            {
              LOG_ERROR ("Unable to initialize power flow solution");
              return retval;
            }
          break;
        case gridState_t::POWERFLOW_COMPLETE:
          retval = powerflow ();
          if (retval != 0)
            {
              LOG_ERROR ("unable to complete power flow");
              return retval;
            }
          break;
        case gridState_t::DYNAMIC_INITIALIZED:
          retval = dynInitialize (timeCurr);
          if (retval != 0)
            {
              LOG_ERROR ("Unable to initialize dynamic solution");
              return retval;
            }
          break;
        default:
          break;
        }
    }
  // Now if we are beyond the desired state we might have to do some additional stuff to make sure everything is ready to go
  if (desiredState != pState)
    {
      switch (desiredState)
        {
        case gridState_t::INITIALIZED:
          if (!controlFlags[no_reset])
            {
              reset (reset_levels::minimal);
            }
          retval = reInitpFlow (sMode);
          if (retval != FUNCTION_EXECUTION_SUCCESS)
            {
              LOG_ERROR ("Unable to re-initialize power flow solution");
              return retval;
            }
          break;
        case gridState_t::DYNAMIC_INITIALIZED:
          if (pState == gridState_t::DYNAMIC_PARTIAL)
            {

            }
          else if (pState == gridState_t::DYNAMIC_COMPLETE)
            {
              //check to make sure nothing has changed
              dynamicCheckAndReset (sMode);

            }
          break;
        default:
          break;
        }

    }
  //now check the solverInterface
  auto sd = getSolverInterface (sMode);
  if (!(sd))             //we need to create the solver
    {
      sd = updateSolver (sMode);
    }
  if (!(sd->isInitialized ()))
    {
      if (desiredState == gridState_t::INITIALIZED)
        {
          retval = reInitpFlow (sMode);
        }
      else if (desiredState == gridState_t::DYNAMIC_INITIALIZED)
        {
          retval = reInitDyn (sMode);
        }
    }
  return retval;
}

int gridDynSimulation::countMpiObjects (bool printInfo) const
{
  int gridlabdObjects = 0;

  for (auto &bus : m_Buses)
    {
      gridlabdObjects += searchForGridlabDobject (bus);
    }
  for (auto &area : m_Areas)
    {
	  gridlabdObjects += searchForGridlabDobject (area);
    }
  //print out the gridlabd objects
  if (printInfo)
    {
      std::cout << "NumberOfGridLABDInstances = " << gridlabdObjects << '\n';
    }
  return gridlabdObjects;

}

void gridDynSimulation::setMaxNonZeros (const solverMode &sMode, count_t ssize)
{
  solverInterfaces[sMode.offsetIndex]->setMaxNonZeros (ssize);
}

std::shared_ptr<solverInterface> gridDynSimulation::getSolverInterface (const solverMode &sMode)
{
  std::shared_ptr<solverInterface> solveD = nullptr;
  if (sMode.offsetIndex < solverInterfaces.size ())
    {

      if (!(solveD = solverInterfaces[sMode.offsetIndex]))
        {
          solveD = updateSolver (sMode);
        }
    }
  else
    {
      solveD = updateSolver (sMode);
    }
  return solveD;
}

const std::shared_ptr<solverInterface> gridDynSimulation::getSolverInterface (const solverMode &sMode) const
{
  std::shared_ptr<solverInterface> solveD = nullptr;
  if (sMode.offsetIndex < solverInterfaces.size ())
    {

      return solverInterfaces[sMode.offsetIndex];
    }
  else
    {
      return nullptr;
    }
}

int gridDynSimulation::add (std::shared_ptr<solverInterface> nSolver)
{
  auto oI = nSolver->getSolverMode ().offsetIndex;

  auto nextIndex = offsets.size ();
  if (oI != kNullLocation)
    {
      if (oI <= nextIndex)
        {
          if (solverInterfaces[oI] == nSolver)
            {
              return OBJECT_ALREADY_MEMBER;
            }
        }
      nextIndex = oI;
    }
  else
    {
      if (nextIndex < 4)
        {
          nextIndex = 4;                             //new modes start at 4
        }
      nSolver->setIndex (static_cast<index_t> (nextIndex));
      if (nextIndex >= solverInterfaces.size ())
        {
          solverInterfaces.resize (nextIndex + 1);
        }
    }


  solverInterfaces[nextIndex] = nSolver;
  solverInterfaces[nextIndex]->lock ();
  nSolver->setSimulationData (this, nSolver->getSolverMode ());
  updateSolver (nSolver->getSolverMode ());
  return OBJECT_ADD_SUCCESS;
}

solverMode gridDynSimulation::getSolverMode (const std::string &solverType)
{
  solverMode sMode (kInvalidCount);
  if ((solverType == "ac") || (solverType == "acflow") || (solverType == "pflow") || (solverType == "powerflow"))
    {
      return *defPowerFlowMode;
    }
  else if ((solverType == "dae") || (solverType == "dynamic"))
    {
      return *defDAEMode;
    }
  else if ((solverType == "algebraic_only") || (solverType == "algebraic") || (solverType == "dynalg"))
    {
      return *defDynAlgMode;
    }
  else if ((solverType == "differential_only") || (solverType == "differential") || (solverType == "dyndiff"))
    {
      return *defDynDiffMode;
    }
  else
    {
      for (auto &sd : solverInterfaces)
        {
          if (sd)
            {
              if (sd->getName() == solverType)
                {
                  return sd->getSolverMode ();
                }
            }
        }
    }
  if ((solverType == "dc") || (solverType == "dcflow"))
    {
      setDC (sMode);
      sMode.algebraic = true;
      sMode.dynamic = false;
      sMode.differential = false;
    }
  else if (solverType == "dc_dynamic")
    {
      sMode.dynamic = true;
      setDC (sMode);
    }
  else
    {

      if (solverType == "dc_algebraic")
        {
          setDC (sMode);
          sMode.algebraic = true;
          sMode.dynamic = true;
          sMode.differential = false;
        }
      else if (solverType == "dc_differential")
        {
          setDC (sMode);
          sMode.differential = true;
          sMode.dynamic = true;
          sMode.algebraic = false;
        }
      else
        {
          auto sd = makeSolver (solverType);
          if (sd)
            {
              add (sd);
              return sd->getSolverMode ();
            }
          else
            {
              return sMode;
            }

        }
      const solverMode nMode = offsets.find (sMode);
      if (nMode.offsetIndex != sMode.offsetIndex)
        {
          return nMode;
        }
      else
        {
          sMode.offsetIndex = offsets.size ();
          if (sMode.offsetIndex < 4)
            {
              sMode.offsetIndex = 4;                   //new modes start at 4
            }
          updateSolver (sMode);

        }
    }
  return sMode;
}

const solverMode &gridDynSimulation::getCurrentMode (const solverMode &sMode) const
{
  if (sMode.offsetIndex != kNullLocation)
    {
      return sMode;
    }
  switch (pState)
    {
    case gridState_t::GD_ERROR:
    case gridState_t::STARTUP:
      return cLocalSolverMode;
    case gridState_t::INITIALIZED:
    case gridState_t::POWERFLOW_COMPLETE:
      return *defPowerFlowMode;
    case gridState_t::DYNAMIC_INITIALIZED:
    case gridState_t::DYNAMIC_COMPLETE:
    case gridState_t::DYNAMIC_PARTIAL:
      switch (defaultDynamicSolverMethod)
        {
        case dynamic_solver_methods::dae:
          return *defDAEMode;
        case dynamic_solver_methods::partitioned:
          return *defDynAlgMode;
        case dynamic_solver_methods::decoupled:
          return cLocalbSolverMode;
        default:
          return *defDAEMode;
        }
    default:
      //this should never happen
      assert (false);
      return cEmptySolverMode;
    }

}

const solverMode &gridDynSimulation::getSolverMode (index_t index) const
{
  if (index < solverInterfaces.size ())
    {
      return (solverInterfaces[index]) ? solverInterfaces[index]->getSolverMode () : cEmptySolverMode;
    }
  else
    {
      return cEmptySolverMode;
    }
}



const solverMode *gridDynSimulation::getSolverModePtr (const std::string &solverType) const
{
  if ((solverType == "ac") || (solverType == "acflow") || (solverType == "pflow") || (solverType == "powerflow"))
    {
      return defPowerFlowMode;
    }
  else if ((solverType == "dae") || (solverType == "dynamic"))
    {
      return defDAEMode;
    }
  else if ((solverType == "dyndiff") || (solverType == "differential_only"))
    {
      return defDynDiffMode;
    }
  else if ((solverType == "dynalg") || (solverType == "algebraic_only"))
    {
      return defDynAlgMode;
    }
  else
    {
      for (auto &sd : solverInterfaces)
        {
          if ((sd)&& (sd->getName() == solverType))
            {
              return &(sd->getSolverMode ());
            }
        }
    }
  return nullptr;
}

const solverMode *gridDynSimulation::getSolverModePtr (index_t index) const
{
  if (index < solverInterfaces.size ())
    {
      return (solverInterfaces[index]) ? &(solverInterfaces[index]->getSolverMode ()) : nullptr;
    }
  else
    {
      return nullptr;
    }
}

const std::shared_ptr<solverInterface> gridDynSimulation::getSolverInterface (index_t index) const
{
  return (index < solverInterfaces.size ()) ? solverInterfaces[index] : nullptr;
}



std::shared_ptr<solverInterface> gridDynSimulation::getSolverInterface (const std::string &solverName)
{
  //just run through the list of solverInterface objects and find the first one that matches the name
  if (solverName == "powerflow")
    {
      return getSolverInterface (*defPowerFlowMode);
    }
  else if (solverName == "dynamic")
    {
      return getSolverInterface (*defDAEMode);
    }
  else if (solverName == "algebraic")
    {
      if (defDynAlgMode)
        {
          return getSolverInterface (*defDynAlgMode);
        }
      else
        {
          return nullptr;
        }
    }
  else if (solverName == "differential")
    {
      if (defDynDiffMode)
        {
          return getSolverInterface (*defDynDiffMode);
        }
      else
        {
          return nullptr;
        }
    }
  for (auto &sd : solverInterfaces)
    {
      if (sd)
        {
          if (sd->getName() == solverName)
            {
              return sd;
            }
        }
    }
  return nullptr;
}


std::shared_ptr<solverInterface> gridDynSimulation::updateSolver (const solverMode &sMode)
{
  auto kIndex = sMode.offsetIndex;
  solverMode sm = sMode;
  if (kIndex >= solverInterfaces.size ())
    {
      if (kIndex > 10000)
        {
          kIndex = static_cast<index_t> (solverInterfaces.size ());
          sm.offsetIndex = kIndex;
        }
      solverInterfaces.resize (kIndex + 1);
    }
  if (!solverInterfaces[kIndex])
    {
      solverInterfaces[kIndex] = makeSolver (this, sMode);
    }

  auto sd = solverInterfaces[kIndex];
  if (!sd)
    {
      printf ("index=%d\n",sMode.offsetIndex);
    }
  if (controlFlags[dense_solver])
    {
      sd->set ("dense",1.0);
    }
  sd->set ("tolerance", tols.rtol);
  if ((pState >= gridState_t::INITIALIZED) && (!isDynamic (sm)))
    {
      auto ss = stateSize (sm);          //statesize calculates rootsize along the way so we do this first
      //so the rootSize(sm) call is much quicker
      sd->allocate (ss, rootSize (sm));
      checkOffsets (sm);
      guess (timeCurr, sd->state_data (), nullptr, sm);
      sd->initialize (timeCurr);
    }
  else if (pState >= gridState_t::DYNAMIC_INITIALIZED)
    {
      auto ss = stateSize (sm);          //statesize calculates rootsize along the way so we do this first
      //so the rootSize(sm) call is much quicker
      sd->allocate (ss, rootSize (sm));
      checkOffsets (sm);

      guess (timeCurr, sd->state_data (), (hasDifferential (sMode)) ? sd->deriv_data () : nullptr, sm);

      sd->initialize (timeCurr);
    }
  return sd;
}


void gridDynSimulation::checkOffsets (const solverMode &sMode)
{
  if (offsets.getAlgOffset (sMode) == kNullLocation)
    {
      updateOffsets (sMode);
    }
}

void gridDynSimulation::getSolverReady (std::shared_ptr<solverInterface> &solver)
{
  auto ss = stateSize (solver->getSolverMode ());
  if (ss != solver->size ())
    {
      reInitDyn (solver->getSolverMode ());
    }
  else
    {
      checkOffsets (solver->getSolverMode ());
    }


}

void gridDynSimulation::fillExtraStateData (stateData *sD, const solverMode &sMode) const
{
  if ((!isDAE (sMode)) && (isDynamic (sMode)))
    {
      if (sMode.pairedOffsetIndex != kNullLocation)
        {
          const solverMode &pSMode = getSolverMode (sMode.pairedOffsetIndex);
          if (isDifferentialOnly (pSMode))
            {
              sD->diffState = solverInterfaces[pSMode.offsetIndex]->state_data ();
              sD->dstate_dt = solverInterfaces[pSMode.offsetIndex]->deriv_data ();
              sD->pairIndex = pSMode.offsetIndex;
            }
          else if (isAlgebraicOnly (pSMode))
            {
              sD->algState = solverInterfaces[pSMode.offsetIndex]->state_data ();
              sD->pairIndex = pSMode.offsetIndex;
            }
          else if (isDAE (pSMode))
            {
              sD->fullState = solverInterfaces[pSMode.offsetIndex]->state_data ();
              sD->pairIndex = pSMode.offsetIndex;
            }
        }

    }
}

bool gridDynSimulation::checkEventsForDynamicReset (double cTime, const solverMode &sMode)
{
  if (EvQ->getNextTime () < cTime)
    {
      auto eventReturn = EvQ->executeEvents (cTime);
      if (eventReturn > change_code::non_state_change)
        {
          return dynamicCheckAndReset (sMode);
        }
    }
  return false;
}


count_t searchForGridlabDobject (gridCoreObject *obj)
{
  count_t cnt = 0;
  gridBus *bus = dynamic_cast<gridBus *> (obj);
  if (bus)
    {
      index_t kk = 0;
      gridCoreObject *obj2 = bus->getLoad (kk);
      while (obj2)
        {
          gridLabDLoad *gLd = dynamic_cast<gridLabDLoad *> (obj2);
          if (gLd)
            {
              cnt += gLd->mpiCount ();
            }
          ++kk;
          obj2 = bus->getLoad (kk);
        }
      return cnt;
    }
  gridArea *area = dynamic_cast<gridArea *> (obj);
  if (area)
    {
      index_t kk = 0;
      bus = area->getBus (kk);
      while (bus)
        {
          cnt += searchForGridlabDobject (bus);
          ++kk;
          bus = area->getBus (kk);
        }
      kk = 0;
      gridArea *a2 = area->getArea (kk);
      while (a2)
        {
          cnt += searchForGridlabDobject (a2);
          ++kk;
          a2 = area->getArea (kk);
        }
      return cnt;
    }
  gridLabDLoad *gLd = dynamic_cast<gridLabDLoad *> (obj);
  if (gLd)
    {
      cnt += gLd->mpiCount ();
    }
  return cnt;

}
