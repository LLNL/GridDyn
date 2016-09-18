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
#include "gridRecorder.h"
#include "vectorOps.hpp"
#include "eventQueue.h"
#include "gridBus.h"
#include "solvers/solverInterface.h"
#include "gridDynSimulationFileOps.h"
#include "faultResetRecovery.h"
#include "dynamicInitialConditionRecovery.h"
#include "simulation/diagnostics.h"
#include "arrayData.h"
//system libraries
#include <algorithm>
#include <cassert>

#include <cstdio>
//#include <fstream>
//#include <iostream>
#include <cstdlib>

static IOdata kNullVec;   //!<  this is a purposely created empty vector which gets used for functions that take as an input a vector but don't use it.


// --------------- dynamic program ---------------
// dynamic solver and initial conditions
int gridDynSimulation::dynInitialize (double tStart)
{
  if (opFlags[dyn_initialized])
    {
      offsets.unload (true);
    }
  const solverMode &tempSm = (defaultDynamicSolverMethod == dynamic_solver_methods::partitioned) ? *defDynDiffMode : *defDAEMode;

  int retval = makeReady (gridState_t::POWERFLOW_COMPLETE,tempSm);
  if (retval != FUNCTION_EXECUTION_SUCCESS)
    {
      return retval;
    }
  auto dynData = getSolverInterface (tempSm);
  const solverMode &sm = dynData->getSolverMode();
  if (defaultDynamicSolverMethod == dynamic_solver_methods::partitioned)
  {
	  defDynDiffMode = &sm;
  }
  else
  {
	  defDAEMode = &sm;
  }

  if (tStart < startTime)
    {
      tStart = startTime;
    }
  LOG_NORMAL ("Initializing Dynamics to time " + std::to_string (tStart));
  // run any events before the simulation
  //In most cases this should be none but users can manipulate the times if they choose
  EvQ->executeEvents (tStart - 0.001);
  /*
  if (nextEventTime <= tStart)
  {
          nextEventTime = runEvents(tStart);
  }
  */
  dynInitializeA (tStart, lower_flags (controlFlags));


  count_t ssize = stateSize (sm);
  if (ssize == 0)
    {
      LOG_WARNING ("State size==0 stopping computation\n");
      return 0;  //TODO::  add a positive return code when state size is 0
    }
  updateOffsets (sm);
  //check for objects with roots
  count_t totalRoots = 0;
  // CSW: Need to send in the number of roots to find so that memory
  // can be allocated to for the array indicating indices of roots.
  //do the final Ida initializtion
  if (controlFlags[roots_disabled])
    {
      offsets.local->local.algRoots = 0;
      offsets.local->local.diffRoots = 0;
      opFlags[has_roots] = false;
      opFlags[has_alg_roots] = false;
    }
  else
    {
      totalRoots = rootSize (sm);
      if (totalRoots > 0)
        {
          setRootOffset (0, sm);
        }
    }

  dynData->allocate (ssize, totalRoots);

  // initializeB
  dynData->set ("tolerance", tols.rtol);
  //run the dynamic initialization part B
  dynInitializeB (kNullVec);

  //check if any updates need to take place
  // run any 0 time events
  if (state_record_period > 0)
    {
      if (!(stateRecorder))
        {
          stateRecorder = std::make_shared<functionEventAdapter> ([ = ]()
      {
        saveStateBinary (this,stateFile,sm);
        return change_code::no_change;
      }
                                                                  ,state_record_period,tStart);

          stateRecorder->partB_only = true;
          //have to do casting since due to the templates in the eventQueue so we need to upcast
          EvQ->insert (std::static_pointer_cast<eventAdapter> (stateRecorder));
        }
    }
  //Execute any events at the start time
  EvQ->executeEvents (tStart);

  //	assert(nextStopTime == EvQ->getNextTime());
  nextStopTime = EvQ->getNextTime ();

  //initialize the solver
  dynData->initialize (tStart);

  opFlags &= RESET_CHANGE_FLAG_MASK;
  pState = gridState_t::DYNAMIC_INITIALIZED;
  timeCurr = tStart;
  return FUNCTION_EXECUTION_SUCCESS;
}



int gridDynSimulation::runDynamicSolverStep (std::shared_ptr<solverInterface> &dynData, double nextStop, double &timeAct)
{
  int retval = FUNCTION_EXECUTION_SUCCESS;
  if (controlFlags[single_step_mode])
    {
      while ((timeAct + tols.timeTol < nextStop) && (retval == FUNCTION_EXECUTION_SUCCESS))
        {
          retval = dynData->solve (nextStop, timeAct, solverInterface::step_mode::single_step);
          if (retval == FUNCTION_EXECUTION_SUCCESS)
            {
              for (auto &sso : singleStepObjects)
                {
                  sso->setState (timeAct, dynData->state_data (), dynData->deriv_data (), dynData->getSolverMode ());
                }
            }
        }
    }
  else
    {
      retval = dynData->solve (nextStop, timeAct);
    }
  if (retval != FUNCTION_EXECUTION_SUCCESS)
    {
      //   dynData->printStates(true);
      handleEarlySolverReturn (retval, timeAct,dynData);
    }
  return retval;
}

void gridDynSimulation::setupDynamicDAE ()
{
  if (defDAEMode == nullptr)
    {
      setDefaultMode (solution_modes_t::dae_mode, getSolverMode ("dae"));
      updateSolver (*defDAEMode);
    }
  const solverMode &sMode = *defDAEMode;
  int retval = makeReady (gridState_t::DYNAMIC_INITIALIZED, sMode);
  if (retval != FUNCTION_EXECUTION_SUCCESS)
    {
      LOG_ERROR ("Unable to prepare simulation for dynamic solution");
      setErrorCode (retval);
      return;
    }

  auto dynData = getSolverInterface (sMode);
  if (!dynData->isInitialized ())
    {
      updateSolver (sMode);
    }

}

#define JAC_CHECK_ENABLED 0

int gridDynSimulation::dynamicDAEStartupConditions (std::shared_ptr<solverInterface> &dynData, const solverMode &sMode)
{
  int retval = FUNCTION_EXECUTION_SUCCESS;
  if (pState == gridState_t::DYNAMIC_INITIALIZED)
    {
      // do mode 0 IC calculation
      guess (timeCurr, dynData->state_data (), dynData->deriv_data (), sMode);
	
      retval = dynData->calcIC (timeCurr, probeStepTime, solverInterface::ic_modes::fixed_masked_and_deriv, false);
      if (retval)
        {
          //for (size_t kk = 0; kk < dynData->getSize(); ++kk)
          //  {
          //  printf("%d : deriv=%f\n", kk, dynData->deriv_data()[kk]);
          //  }
          retval = generateDaeDynamicInitialConditions (sMode);
          if (retval)
            {
#if JAC_CHECK_ENABLED > 0
              int errc = JacobianCheck (this, sMode);
              if (errc > 0)
                {
                  printStateNames (this, sMode);
                }
#endif
              LOG_ERROR (dynData->getLastErrorString ());
              LOG_ERROR ("Unable to generate initial dynamic solution modeA");
              return retval;
            }
        }


    }
  else
    {
      retval = generateDaeDynamicInitialConditions (sMode);
      if (retval)
        {
          LOG_ERROR (dynData->getLastErrorString ());
          LOG_ERROR ("Unable to generate dynamic solution conditions modeB");
          return retval;
        }
    }
  return retval;
}

// IDA DAE Solver
int gridDynSimulation::dynamicDAE ( double tStop)
{
  int out = FUNCTION_EXECUTION_SUCCESS;
  double nextStop;
  double lastTimeStop;
  int tstep = 0;
  const solverMode &sMode = *defDAEMode;

  setupDynamicDAE ();
  auto dynData = getSolverInterface (sMode);

  int retval = dynamicDAEStartupConditions (dynData, sMode);
  if (retval != FUNCTION_EXECUTION_SUCCESS)
    {
      return retval;
    }

  nextStopTime = std::min (tStop, EvQ->getNextTime ());
  //go into the main loop
  int smStep = 0;
  while (timeReturn < tStop)
    {
      nextStop = std::min (tStop, nextStopTime);

      nextStopTime = tStop;
      if (nextStop - timeCurr < tols.timeTol)         //if the interval is too small just advance the clock a little
        {          //the most likely cause of this is numerical instabily in recorders and events
          timeReturn = nextStop;
        }
      else
        {
          retval = runDynamicSolverStep (dynData, nextStop, timeReturn);
        }


      while (timeReturn + tols.timeTol < nextStop)       //the timeTol is for stopping just prior to the expected stop time
        {
          lastTimeStop = timeCurr;
          dynamicCheckAndReset (sMode);
          retval = generateDaeDynamicInitialConditions (sMode);
          if (retval != FUNCTION_EXECUTION_SUCCESS)
            {
              pState = gridState_t::DYNAMIC_PARTIAL;
              LOG_ERROR ("simulation halted unable to converge");
              LOG_ERROR (dynData->getLastErrorString ());
              return FUNCTION_EXECUTION_FAILURE;
            }
          nextStop = std::min (tStop, EvQ->getNextTime ());             //update the stopping time just in case the events have changed
          retval = runDynamicSolverStep (dynData, nextStop, timeReturn);

          timeCurr = timeReturn;

          // CSW Changed this from 2e-3 to 1e-7: need to rethink this in light of rootfinding
          if (retval != SOLVER_ROOT_FOUND)
            {

              if (timeReturn < lastTimeStop + tols.timeTol)
                {
                  ++tstep;
                  if (tstep == 1) //there are some circumstances where internal models halt advancement until the clock moves forward
                    {
                      timeCurr = timeCurr + tols.timeTol;
                    }
                  else if (tstep > 1)
                    {
                      pState = gridState_t::DYNAMIC_PARTIAL;
                      LOG_ERROR ("simulation halted unable to converge");
                      LOG_ERROR (dynData->getLastErrorString ());
                      return FUNCTION_EXECUTION_FAILURE;
                    }
                }
              else if (timeReturn < lastTimeStop + 1e-4)
                {
                  ++smStep;
                  if (smStep > 10)
                    {
                      LOG_ERROR ("simulation halted too many small time steps");
                      return FUNCTION_EXECUTION_FAILURE;
                    }
                  tstep = 0;
                }
              else
                {
                  smStep = 0;
                  tstep = 0;
                }
            }

        }
      timeCurr = nextStop;
      //transmit the current state to the various objects for updates and recorders
      setState (timeCurr, dynData->state_data (), dynData->deriv_data (), sMode);
      updateLocalCache ();
      auto ret = EvQ->executeEvents (timeCurr);
      if (ret > change_code::non_state_change)
        {
          dynamicCheckAndReset (sMode);
          retval = generateDaeDynamicInitialConditions (sMode);
          if (retval != FUNCTION_EXECUTION_SUCCESS)
            {
              pState = gridState_t::DYNAMIC_PARTIAL;
              LOG_ERROR ("simulation halted unable to converge");
              LOG_ERROR (dynData->getLastErrorString ());
              return FUNCTION_EXECUTION_FAILURE;
            }
        }
      nextStopTime = EvQ->getNextTime ();
    }
  if ((consolePrintLevel >= GD_TRACE_PRINT)||(logPrintLevel >= GD_TRACE_PRINT))
    {
      dynData->logSolverStats (GD_TRACE_PRINT);
      dynData->logErrorWeights (GD_TRACE_PRINT);
    }


  //store the results to the buses
  pState = gridState_t::DYNAMIC_COMPLETE;
  return out;
}

void gridDynSimulation::setupDynamicPartitioned ()
{
  const solverMode &sModeAlg = *defDynAlgMode;
  const solverMode &sModeDiff = *defDynDiffMode;
  int retval = makeReady (gridState_t::DYNAMIC_INITIALIZED, sModeDiff);
  if (retval != FUNCTION_EXECUTION_SUCCESS)
    {
      LOG_ERROR ("Unable to prepare simulation for dynamic solution");
      setErrorCode (retval);
      return;
    }
  auto dynDataAlg = getSolverInterface (sModeAlg);
  auto dynDataDiff = getSolverInterface (sModeDiff);
  if (!dynDataAlg->isInitialized ())
    {
      updateSolver (sModeAlg);
    }

  if (!dynDataDiff->isInitialized ())
    {
      updateSolver (sModeDiff);
    }
  dynDataAlg->set ("pair", static_cast<double> (sModeDiff.offsetIndex));
  dynDataDiff->set ("pair", static_cast<double> (sModeAlg.offsetIndex));
}


int gridDynSimulation::dynamicPartitionedStartupConditions (std::shared_ptr<solverInterface> &dynDataDiff, std::shared_ptr<solverInterface> &dynDataAlg, const solverMode &sModeDiff, const solverMode &sModeAlg)
{
  int retval = FUNCTION_EXECUTION_SUCCESS;
  if (pState == gridState_t::DYNAMIC_INITIALIZED)
    {
      if (controlFlags[dae_initialization_for_partitioned])
        {
          auto daeData = getSolverInterface (*defDAEMode);
          if (!daeData->isInitialized ())
            {
              updateSolver (*defDAEMode);
            }
          guess (timeCurr, daeData->state_data (), daeData->deriv_data (), *defDAEMode);
          retval = daeData->calcIC (timeCurr, probeStepTime, solverInterface::ic_modes::fixed_masked_and_deriv, false);
          if (retval)
            {
              retval = generateDaeDynamicInitialConditions (*defDAEMode);
              if (retval)
                {
                  LOG_ERROR (daeData->getLastErrorString ());
                  LOG_ERROR ("Unable to generate initial dynamic solution modeA");
                  return retval;
                }
            }
        }
      else
        {
          guess (timeCurr, dynDataDiff->state_data (), dynDataDiff->deriv_data (), sModeDiff);
          guess (timeCurr, dynDataAlg->state_data (), nullptr, sModeAlg);
        }
    }
  else
    {
      guess (timeCurr, dynDataDiff->state_data (), dynDataDiff->deriv_data (), sModeDiff);
      guess (timeCurr, dynDataAlg->state_data (), nullptr, sModeAlg);
    }
  return retval;
}

int gridDynSimulation::dynamicPartitioned (double tStop, double tStep)
{
  int out = FUNCTION_EXECUTION_SUCCESS;
  double nextStepTime = std::min (tStop, timeCurr + tStep);
  double nextEventTime = EvQ->getNextTime ();
  setupDynamicPartitioned ();
  double lastTimeStop = timeCurr;

  auto dynDataAlg = getSolverInterface (*defDynAlgMode);
  auto dynDataDiff = getSolverInterface (*defDynDiffMode);
  const solverMode &sModeAlg = dynDataAlg->getSolverMode ();
  const solverMode &sModeDiff = dynDataDiff->getSolverMode ();

  int tstep = 0;
  int retval = dynamicPartitionedStartupConditions (dynDataDiff, dynDataAlg, sModeDiff, sModeAlg);

  //go into the main loop
  int smStep = 0;
  while (timeCurr < tStop)
    {
      nextStopTime = std::min (nextStepTime, nextEventTime);


      if (nextStopTime - timeCurr < tols.timeTol)                   //if the interval is too small just advance the clock a little
        {                  //the most likely cause of this is a tiny numerical instabily in recorders and events
          timeReturn = nextStopTime;
        }
      else
        {
          retval = dynDataAlg->solve (timeCurr, timeReturn);
          if (retval < 0)
            {
              int mmatch = JacobianCheck (this, dynDataAlg->getSolverMode ());
              if (mmatch > 0)
                {
                  printStateNames (this, sModeAlg);
                }
              dynDataAlg->printResid = true;
            }
          if (retval >= FUNCTION_EXECUTION_SUCCESS)               //can return a 1
            {
              retval = runDynamicSolverStep (dynDataDiff, nextStopTime, timeReturn);
              timeCurr = timeReturn;
            }
        }


      while (timeReturn + tols.timeTol < nextStopTime)                 //the timeTol is for stopping just prior to the expected stop time
        {
          lastTimeStop = timeReturn;
          dynamicCheckAndReset (sModeDiff);
          retval = generatePartitionedDynamicInitialConditions (sModeAlg,sModeDiff);
          if (retval != FUNCTION_EXECUTION_SUCCESS)
            {
              pState = gridState_t::DYNAMIC_PARTIAL;
              LOG_ERROR ("simulation halted unable to converge");
              LOG_ERROR (dynDataDiff->getLastErrorString ());
              return FUNCTION_EXECUTION_FAILURE;
            }
          retval = runDynamicSolverStep (dynDataDiff, nextStopTime, timeReturn);
          timeCurr = timeReturn;

          // CSW Changed this from 2e-3 to 1e-7: need to rethink this in light of rootfinding
          if (retval != SOLVER_ROOT_FOUND)
            {

              if (timeReturn < lastTimeStop + tols.timeTol)
                {
                  ++tstep;
                  if (tstep == 1)                       //there are some circumstances where internal models halt advancement until the clock moves forward
                    {
                      timeCurr = timeCurr + tols.timeTol;
                    }
                  else if (tstep > 1)
                    {
                      pState = gridState_t::DYNAMIC_PARTIAL;
                      LOG_ERROR ("simulation halted unable to converge");
                      LOG_ERROR (dynDataDiff->getLastErrorString ());
                      return FUNCTION_EXECUTION_FAILURE;
                    }
                }
              else if (timeReturn < lastTimeStop + 1e-4)
                {
                  ++smStep;
                  if (smStep > 10)
                    {
                      LOG_ERROR ("simulation halted too many small time steps");
                      return FUNCTION_EXECUTION_FAILURE;
                    }
                  tstep = 0;
                }
              else
                {
                  smStep = 0;
                  tstep = 0;
                }
            }

        }
      timeCurr = nextStopTime;
      if (nextEventTime - tols.timeTol < timeCurr)
        {
          //transmit the current state to the various objects for updates and recorders
          setState (timeCurr, dynDataDiff->state_data (), dynDataDiff->deriv_data (), sModeDiff);
          setState (timeCurr, dynDataAlg->state_data (), nullptr, sModeAlg);
          updateLocalCache ();
          auto ret = EvQ->executeEvents (timeCurr);
          if (ret > change_code::non_state_change)
            {
              dynamicCheckAndReset (sModeDiff);
              retval = generatePartitionedDynamicInitialConditions (sModeAlg,sModeDiff);
              if (retval != FUNCTION_EXECUTION_SUCCESS)
                {
                  pState = gridState_t::DYNAMIC_PARTIAL;
                  LOG_ERROR ("simulation halted unable to converge");
                  LOG_ERROR (dynDataDiff->getLastErrorString ());
                  return FUNCTION_EXECUTION_FAILURE;
                }
            }
          nextEventTime = EvQ->getNextTime ();
        }
      if (nextStepTime - tols.timeTol < timeCurr)
        {
          nextStepTime = std::min (tStop, nextStepTime + tStep);
        }
    }
  if ((consolePrintLevel >= GD_TRACE_PRINT) || (logPrintLevel >= GD_TRACE_PRINT))
    {
      dynDataDiff->logSolverStats (GD_TRACE_PRINT);
      dynDataDiff->logErrorWeights (GD_TRACE_PRINT);
    }


  //store the results to the buses
  pState = gridState_t::DYNAMIC_COMPLETE;
  return out;
}

int gridDynSimulation::dynamicDecoupled (double /*tStop*/, double /*tStep*/)
{
  return FUNCTION_EXECUTION_FAILURE;
}

int gridDynSimulation::step ()
{
  double tact;
  double nextT = timeCurr + stepTime;
  int ret = step (nextT, tact);
  if (tact != nextT)
    {
      return ret;
    }
  return FUNCTION_EXECUTION_SUCCESS;
}


int gridDynSimulation::step (double nextStep, double &timeActual)
{

  if (timeCurr + tols.timeTol >= nextStep)
    {
      if (EvQ->getNextTime () <= timeCurr + tols.timeTol)
        {
          EvQ->executeEvents (timeCurr);
        }
      else
        {
          timeActual = nextStep;
          return FUNCTION_EXECUTION_SUCCESS;
        }

    }
  const solverMode &sm = *defDAEMode;
  int retval = FUNCTION_EXECUTION_SUCCESS;


  auto dynData = getSolverInterface (sm);
  if (pState == gridState_t::DYNAMIC_COMPLETE)
    {
      if (dynamicCheckAndReset (sm))
        {
          retval = generateDaeDynamicInitialConditions (sm);
          if (retval != FUNCTION_EXECUTION_SUCCESS)
            {
              timeActual = timeCurr;
              return 1;
            }
        }        //this step does a reset of IDA if necessary
    }
  else if (pState == gridState_t::DYNAMIC_INITIALIZED)
    {
      // do mode 0 IC calculation
      guess (timeCurr, dynData->state_data (), dynData->deriv_data (), sm);
      retval = dynData->calcIC (timeCurr, probeStepTime, solverInterface::ic_modes::fixed_masked_and_deriv, false);
      if (retval)
        {
          //for (size_t kk = 0; kk < dynData->getSize(); ++kk)
          //  {
          //  printf("%d : deriv=%f\n", kk, dynData->deriv_data()[kk]);
          //  }
          retval = generateDaeDynamicInitialConditions (sm);
          if (retval)
            {
#if JAC_CHECK_ENABLED > 0
              if (JacobianCheck (this,sm) > 0)
                {
                  printStateNames (this, sm);
                }
#endif
              return retval;
            }
        }
      pState = gridState_t::DYNAMIC_PARTIAL;
    }
  else
    {
      //check to make sure nothing has changed
      dynamicCheckAndReset (sm);
      retval = generateDaeDynamicInitialConditions (sm);
      if (retval)
        {
          LOG_ERROR ("Unable to generate dynamic solution conditions modeB");
          return retval;
        }
    }
  nextStopTime = std::min (nextStep, EvQ->getNextTime ());
  double tStop;
  while (timeReturn < nextStep)
    {
      tStop = std::min (nextStep, nextStopTime);

      nextStopTime = nextStep;
      if (tStop - timeCurr < tols.timeTol)         //if the interval is too small just advance the clock a little
        {          //the most likely cause of this is numerical instabily in recorders and events
          timeReturn = tStop;
        }
      else
        {
          retval = runDynamicSolverStep (dynData, nextStopTime, timeReturn);
          timeCurr = timeReturn;

        }


      while (timeReturn + tols.timeTol < tStop)
        {
          double lastTimeStop = timeCurr;
          if (dynamicCheckAndReset (sm))
            {
              retval = generateDaeDynamicInitialConditions (sm);
              if (retval != FUNCTION_EXECUTION_SUCCESS)
                {
                  timeActual = timeCurr;
                  return 1;
                }
            }      //this step does a reset of IDA if necessary
          tStop = std::min (stopTime, EvQ->getNextTime ());             //update the stopping time just in case the events have changed
          retval = runDynamicSolverStep (dynData, nextStopTime, timeReturn);
          timeCurr = timeReturn;
          // CSW Changed this from 2e-3 to 1e-7: need to rethink this in light of rootfinding
          if (timeReturn < lastTimeStop + tols.timeTol)
            {
              timeActual = timeCurr;
              return (1);  //TODO:: PT replace this with a constant
            }

        }
      timeCurr = tStop;
      //transmit the current state to the various objects for updates and recorders
      setState (timeCurr, dynData->state_data (), dynData->deriv_data (), sm);

      auto ret = EvQ->executeEvents (timeCurr);
      if (ret > change_code::no_change)
        {
          dynamicCheckAndReset (sm);
          break;
        }
      nextStopTime = EvQ->getNextTime ();
      //recorders last to capture any state change



    }
  timeActual = timeCurr;
  pState = gridState_t::DYNAMIC_COMPLETE;
  return retval;
}

void gridDynSimulation::handleEarlySolverReturn (int retval, double time, std::shared_ptr<solverInterface> &dynData)
{
  ++haltCount;

  if (opFlags[has_roots])
    {
      if (retval == SOLVER_ROOT_FOUND)             // a root was found in IDASolve
        {               // Note that if a root is found, integration halts at the root time which is
                        // returned in timeReturn.
          int ret = dynData->getRoots ();
          if (!(ret))
            {
              setState (time, dynData->state_data (),dynData->deriv_data (), dynData->getSolverMode ());

              LOG_DEBUG ("Root detected");
              rootTrigger (time, dynData->rootsfound, dynData->getSolverMode ());
            }
        }
      else if (retval == SOLVER_INVALID_STATE_ERROR)
        {
          //if we get into here the most likely cause is a very low voltage bus
          stateData sD (time, dynData->state_data (), dynData->deriv_data ());

          rootCheck (&sD, dynData->getSolverMode (), check_level_t::low_voltage_check);
          //return dynData->calcIC(getCurrentTime(), probeStepTime, solverInterface::ic_modes::fixed_diff, true);
          opFlags.reset (low_bus_voltage);
#if JAC_CHECK_ENABLED > 0
          int mmatch = JacobianCheck (this, dynData->getSolverMode ());
          if (mmatch > 0)
            {
              printStateNames (this, dynData->getSolverMode ());
            }
#endif
        }
    }
}

bool gridDynSimulation::dynamicCheckAndReset (const solverMode &sMode, change_code change)
{
  auto dynData = getSolverInterface (sMode);
  if (opFlags[connectivity_change_flag])
    {
      checkNetwork (network_check_type::simplified);
    }
  if ((opFlags[state_change_flag]) || (change == change_code::state_count_change))    //we changed object states so we have to do a full reset
    {
      if (checkEventsForDynamicReset (timeCurr + probeStepTime, sMode))
        {
          return true;
        }
	  reInitDyn(sMode);

    }
  else if ((opFlags[object_change_flag]) || (change == change_code::object_change))     //the object count changed
    {
      if (checkEventsForDynamicReset (timeCurr + probeStepTime, sMode))
        {
          return true;
        }

      if (dynData->size () != stateSize (sMode))
        {
          reInitDyn (sMode);
        }
      else
        {
          updateOffsets (sMode);
        }
    }
  else if ((opFlags[jacobian_count_change_flag])||(change == change_code::jacobian_change))
    {
      if (checkEventsForDynamicReset (timeCurr + probeStepTime, sMode))
        {
          return true;
        }
      handleRootChange (sMode, dynData);
      dynData->setMaxNonZeros (jacSize (sMode));
      // Allow for the fact that the new size of Jacobian now also has a different number of nonzeros
      dynData->sparseReInit (solverInterface::sparse_reinit_modes::resize);
		

    }
  else if (opFlags[root_change_flag])
  {
	  handleRootChange(sMode, dynData);
  }
  else        //mode ==0
    {
      opFlags &= RESET_CHANGE_FLAG_MASK;
      return false;
    }

  opFlags &= RESET_CHANGE_FLAG_MASK;
  return true;
}

int gridDynSimulation::generateDaeDynamicInitialConditions (const solverMode &sMode)
{
  auto dynData = getSolverInterface (sMode);
  int retval = FUNCTION_EXECUTION_FAILURE;
  //check and deal with voltage Reset
  if (opFlags[reset_voltage_flag])
    {
      opFlags.reset (reset_voltage_flag);
      faultResetRecovery frr (this, dynData);
      while (frr.hasMoreFixes ())
        {
          retval = frr.attemptFix ();
          if (retval == FUNCTION_EXECUTION_SUCCESS)
            {
              retval = checkAlgebraicRoots (dynData);
              if (retval == FUNCTION_EXECUTION_SUCCESS)
                {
                  return retval;
                }
            }
        }
    }
  if (opFlags[low_bus_voltage])
    {
      stateData sD (getCurrentTime (), dynData->state_data (),dynData->deriv_data ());

      rootCheck (&sD, dynData->getSolverMode (), check_level_t::low_voltage_check);
      //return dynData->calcIC(getCurrentTime(), probeStepTime, solverInterface::ic_modes::fixed_diff, true);
      opFlags.reset (low_bus_voltage);
    }
  //Do the first cut guess at the solution
  guess (timeCurr, dynData->state_data (), dynData->deriv_data (), sMode);
  int loc;
  auto maxResid = checkResid (this, dynData, &loc);
  // double cr2;
  if (std::abs (maxResid) > 0.5)
    {
      if ((logPrintLevel >= GD_DEBUG_PRINT) || (consolePrintLevel >= GD_DEBUG_PRINT))
        {
          stringVec snames;
          getStateName (snames, sMode);
          LOG_DEBUG ("state " + std::to_string (loc) + ':' + snames[loc] + " error= " + std::to_string (maxResid));
        }
      converge (timeCurr, dynData->state_data (), dynData->deriv_data (), sMode, converge_mode::high_error_only, 0.05);
      // JacobianCheck(sMode);
      //  printStateNames(this,sMode);
    }

  retval = dynData->calcIC (timeCurr, probeStepTime, solverInterface::ic_modes::fixed_diff, true);

  if (retval == -22)       //this is bad initial conditions TODO:: map this to Solver ERROR codes not Sundials ERROR codes
    {
      converge (timeCurr, dynData->state_data (), dynData->deriv_data (), sMode, converge_mode::single_iteration, 0.05);
      retval = dynData->calcIC (timeCurr, probeStepTime, solverInterface::ic_modes::fixed_diff, true);
    }
  if (retval == FUNCTION_EXECUTION_SUCCESS)
    {
      retval = checkAlgebraicRoots (dynData);
    }
  //if we still haven't fixed it call the recovery object and let it try to deal with it
  if (retval < FUNCTION_EXECUTION_SUCCESS)
    {
      dynamicInitialConditionRecovery dicr (this, dynData);
      while (dicr.hasMoreFixes ())
        {
          retval = dicr.attemptFix ();
          if (retval == FUNCTION_EXECUTION_SUCCESS)
            {
              retval = checkAlgebraicRoots (dynData);
              if (retval == FUNCTION_EXECUTION_SUCCESS)
                {
                  return retval;
                }
            }
        }
    }

  if ((consolePrintLevel >= GD_TRACE_PRINT) || (logPrintLevel >= GD_TRACE_PRINT))
    {
      dynData->logSolverStats (GD_TRACE_PRINT, true);
    }
  return retval;
}

int gridDynSimulation::generatePartitionedDynamicInitialConditions (const solverMode &sModeAlg, const solverMode &sModeDiff)
{
  auto dynDataAlg = getSolverInterface (sModeAlg);
  auto dynDataDiff = getSolverInterface (sModeDiff);
  int retval = FUNCTION_EXECUTION_FAILURE;
  //check and deal with voltage Reset
  if (opFlags[reset_voltage_flag])
    {
      opFlags.reset (reset_voltage_flag);
      /*faultResetRecovery frr(this, dynData);
      while (frr.hasMoreFixes())
      {
              retval = frr.attemptFix();
              if (retval == FUNCTION_EXECUTION_SUCCESS)
              {
                      retval = checkAlgebraicRoots(dynData);
                      if (retval == FUNCTION_EXECUTION_SUCCESS)
                      {
                              return retval;
                      }
              }
      }
      */
    }
  if (opFlags[low_bus_voltage])
    {
      /*stateData sD(getCurrentTime(), dynData->state_data(), dynData->deriv_data());

      rootCheck(&sD, dynData->getSolverMode(), check_level_t::low_voltage_check);
      //return dynData->calcIC(getCurrentTime(), probeStepTime, solverInterface::ic_modes::fixed_diff, true);
      opFlags.reset(low_bus_voltage);
      */
    }
  double tRet;
  retval = dynDataAlg->solve (timeCurr + probeStepTime,tRet);
  if (retval == FUNCTION_EXECUTION_SUCCESS)
    {
      timeCurr += probeStepTime;
    }
  return retval;
}

int gridDynSimulation::checkAlgebraicRoots (std::shared_ptr<solverInterface> &dynData)
{
  if (opFlags[has_alg_roots])
    {
      const solverMode &sMode = dynData->getSolverMode ();
      dynData->getCurrentData ();
      setState (timeCurr + probeStepTime, dynData->state_data (), dynData->deriv_data (), sMode);
      updateLocalCache ();
      change_code ret = rootCheck (nullptr, cLocalSolverMode, check_level_t::full_check);
      handleRootChange (sMode, dynData);
      if (ret > change_code::non_state_change)
        {
          dynamicCheckAndReset (sMode, ret);
          int retval = dynData->calcIC (timeCurr, probeStepTime, solverInterface::ic_modes::fixed_diff, true);

          if (retval < 0)              //this is bad initial conditions
            {
              converge (timeCurr, dynData->state_data (), dynData->deriv_data (), sMode, converge_mode::single_iteration, 0.05);
              retval = dynData->calcIC (timeCurr, probeStepTime, solverInterface::ic_modes::fixed_diff, true);
            }
          return retval;
        }

    }
  return FUNCTION_EXECUTION_SUCCESS;
}


int gridDynSimulation::handleStateChange (const solverMode &sMode)
{
  if (opFlags[state_change_flag])
    {
      if (checkEventsForDynamicReset (timeCurr + probeStepTime, sMode))
        {
          return generateDaeDynamicInitialConditions (sMode);
        }
      else
        {
          reInitDyn (sMode);
          return generateDaeDynamicInitialConditions (sMode);
        }
    }
  return HANDLER_NO_RETURN;
}

void gridDynSimulation::handleRootChange (const solverMode &sMode, std::shared_ptr<solverInterface> &dynData)
{
  if (opFlags[root_change_flag])               //something with the roots changed
    {
      auto rs = rootSize (sMode);
      if (rs != dynData->rootsfound.size ())
        {
          dynData->setRootFinding (rs);
          if (rs > 0)
            {
              setRootOffset (0, sMode);
            }

        }
      opFlags.reset (root_change_flag);
    }
  else if (rootSize(sMode)>0)
  {
	  if (offsets.getRootOffset(sMode) == kNullLocation)
	  {
		  setRootOffset(0, sMode);
	  }
  }
}


void gridDynSimulation::getConstraints (double consData[], const solverMode &sMode)
{
  //if ((controlFlags[voltage_constraints_flag]) || (opFlags[has_constraints]))
  if (controlFlags[voltage_constraints_flag])
    {
      getVoltageStates (consData, sMode);
    }
  if (opFlags[has_constraints])
    {
      gridArea::getConstraints (consData, sMode);
    }

}


void gridDynSimulation::updateOffsets (const solverMode &sMode)
{
  setupOffsets (sMode, default_ordering);
  setMaxNonZeros (sMode, jacSize (sMode));
}

int gridDynSimulation::reInitDyn (const solverMode &sMode)
{
  auto dynData = getSolverInterface (sMode);
  updateOffsets (sMode);

  //check for objects with roots
  count_t nRoots = 0;
  if (controlFlags[roots_disabled])
    {
      offsets.local->local.algRoots = 0;
      offsets.local->local.diffRoots = 0;
      opFlags[has_roots] = false;
    }
  else
    {
      nRoots = rootSize (sMode);
      if (rootSize (sMode) > 0)
        {
          setRootOffset (0, sMode);
        }
    }
  if (controlFlags[constraints_disabled])
    {
      opFlags[has_constraints] = false;
    }

  // CSW: Need to send in the number of roots to find so that memory
  // can be allocated to for the array indicating indices of roots.
  dynData->allocate (stateSize (sMode), nRoots);

  //guess an initial condition
  guess (timeCurr, dynData->state_data (), dynData->deriv_data (), sMode);
  //initializeB ida memory
  if (stateSize (sMode) > 0)
    {
      dynData->initialize (timeCurr);
    }

  opFlags &= RESET_CHANGE_FLAG_MASK;
  pState = gridState_t::DYNAMIC_INITIALIZED;
  return FUNCTION_EXECUTION_SUCCESS;
}





#define DEBUG_RESID 0
#define CHECK_RESID 1
#define CHECK_STATE 1

#if DEBUG_RESID > 0
const static double resid_print_tol = 1e-7;
#endif
// IDA nonlinear function evaluation
int gridDynSimulation::residualFunction (double ttime, const double state[], const double dstate_dt[], double resid[], const solverMode &sMode)
{
  ++residCount;
  stateData sD (ttime, state,dstate_dt,residCount);

#if (CHECK_STATE > 0)
  auto dynDataa = getSolverInterface (sMode);
  for (size_t kk = 0; kk < dynDataa->size (); ++kk)
    {
      if (!std::isfinite (state[kk]))
        {
          LOG_ERROR ("state[" + std::to_string (kk) + "] is not finite");
		  printStateNames(this,sMode);
          return FUNCTION_EXECUTION_FAILURE;
        }
    }
#endif

  fillExtraStateData (&sD, sMode);
  //call the area based function to handle the looping
  preEx (&sD, sMode);
  residual (&sD, resid, sMode);
  delayedResidual (&sD, resid, sMode);
 // if (sourceFile == "case2383wp.m") //active debugging
  //{
	//  setState(ttime, state, dstate_dt, sMode);
	 // updateLocalCache();
	 // saveBusData(this, "BusData"+std::to_string(residCount)+".csv");
 // }
#if (CHECK_RESID > 0)
  auto dynDatab = getSolverInterface (sMode);
  for (size_t kk = 0; kk < dynDatab->size (); ++kk)
    {
      if (!std::isfinite (resid[kk]))
        {
          LOG_ERROR ("resid[" + std::to_string (kk) + "] is not finite");
          printStateNames (this, sMode);
          assert (std::isfinite (resid[kk]));
        }
    }
#endif

#if (DEBUG_RESID >= 1)
  static std::vector<double> rvals;
  static std::vector<double> lstate;
  static std::vector<int> dbigger;
  static double ptime = -kBigNum;
  auto dynData = getSolverInterface (sMode);
  auto ss = dynData->size ();
  if (rvals.size () != ss)
    {
      rvals.resize (ss);
      dbigger.resize (ss);
      lstate.resize (ss);
      std::copy (resid, resid + ss, rvals.data ());
      std::copy (state, state + ss, lstate.data ());
      std::fill (dbigger.begin (), dbigger.end (), 0);
      ptime = ttime;
#if DEBUG_RESID > 1
      for (index_t kk = 0; kk < ss; kk++)
        {
#if (DEBUG_RESID == 3)
          printf ("%d[%f] %d r=%e state=%f\n", static_cast<int> (residCount), ttime, static_cast<int> (kk), resid[kk], state[kk]);
#else
          if (std::abs (resid[kk]) > resid_print_tol)
            {
              printf ("%d[%f] %d r=%e state=%f\n", static_cast<int> (residCount), ttime, static_cast<int> (kk), resid[kk], state[kk]);
            }
            #endif
        }
#endif
      dynData->printStates (true);

    }
  else if (ptime != ttime)
    {
      std::copy (resid, resid + ss, rvals.data ());
      std::copy (state, state + ss, lstate.data ());
      std::fill (dbigger.begin (), dbigger.end (), 0);

#if DEBUG_RESID > 1
      printf ("time change %e\n", ttime - ptime);
      for (index_t kk = 0; kk < ss; kk++)
        {
#if (DEBUG_RESID == 3)
          printf ("%d[%f] %d r=%e state=%f\n", residCount, ttime, kk, resid[kk], state[kk]);
#else
          if (std::abs (resid[kk]) > resid_print_tol)
            {
              printf ("%d[%f] %d r=%e state=%f\n", static_cast<int> (residCount), ttime, static_cast<int> (kk), resid[kk], state[kk]);
            }
            #endif
        }
#endif
      ptime = ttime;
    }
  else
    {

      for (index_t kk = 0; kk < ss; kk++)
        {
#if DEBUG_RESID > 1
        #if (DEBUG_RESID == 3)
          printf ("%d[%f] %d r=%e state=%f dr=%e ds=%e\n", static_cast<int> (residCount), ttime, static_cast<int> (kk), resid[kk], state[kk], resid[kk] - rvals[kk], state[kk] - lstate[kk]);
        #else
          if ((std::abs (resid[kk]) > resid_print_tol) || (std::abs (state[kk] - lstate[kk]) > 1e-7))
            {
              printf ("%d[%f] %d r=%e state=%f dr=%e ds=%e\n", static_cast<int> (residCount), ttime, static_cast<int> (kk), resid[kk],state[kk],resid[kk] - rvals[kk],state[kk] - lstate[kk]);
            }
        #endif
#endif
          if (std::abs (resid[kk]) - 1e-12 > std::abs (rvals[kk]))
            {
              dbigger[kk] += 1;
              if (dbigger[kk] > 2)
                {
                  printf ("residual[%d] getting bigger from %e to %e at time=%f\n", static_cast<int> (kk), rvals[kk], resid[kk], ttime);
                }

            }
          else
            {
              dbigger[kk] = 0;
            }
        }
      std::copy (resid, resid + ss, rvals.data ());
      std::copy (state, state + ss, lstate.data ());
    }
#endif
  if (opFlags[invalid_state_flag])
    {
      opFlags.reset (invalid_state_flag);
      return 1;
    }
  return 0;
}


int gridDynSimulation::derivativeFunction (double ttime, const double state[], double dstate_dt[], const solverMode &sMode)
{
  ++residCount;
  stateData sD (ttime,state,dstate_dt,residCount);
  fillExtraStateData (&sD, sMode);
#if (CHECK_STATE > 0)
  auto dynDataa = getSolverInterface (sMode);
  for (size_t kk = 0; kk < dynDataa->size (); ++kk)
    {
      if (!std::isfinite (state[kk]))
        {
          LOG_ERROR ("state[" + std::to_string (kk) + "] is not finite");
          return FUNCTION_EXECUTION_FAILURE;
        }
    }
#endif

  //call the area based function to handle the looping
  preEx (&sD, sMode);
  derivative (&sD, dstate_dt, sMode);
  delayedDerivative (&sD, dstate_dt, sMode);
  return FUNCTION_EXECUTION_SUCCESS;
}

// Jacobian computation
int gridDynSimulation::jacobianFunction (double ttime, const double state[], const double dstate_dt[], arrayData<double> *ad, double cj, const solverMode &sMode)
{
  ++JacobianCount;
  //assuming it is the same data as the preceeding residual call  (it is for IDA but not sure if this assumption will be generally valid)
  stateData sD (ttime,state,dstate_dt,residCount);
  sD.cj = cj;
  fillExtraStateData (&sD, sMode);
  //the area function to evaluate the Jacobian elements
  preEx (&sD, sMode);
  ad->clear ();
  jacobianElements (&sD, ad, sMode);
  delayedJacobian (&sD, ad, sMode);

  return FUNCTION_EXECUTION_SUCCESS;
}


int gridDynSimulation::rootFindingFunction (double ttime, const double state[], const double dstate_dt[], double roots[], const solverMode &sMode)
{
  stateData sD (ttime,state,dstate_dt,residCount);
  fillExtraStateData (&sD, sMode);
  rootTest (&sD, roots, sMode);
  return FUNCTION_EXECUTION_SUCCESS;

}