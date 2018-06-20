/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */
#include "../events/Event.h"

#include "../gridDynSimulation.h"

#include "core/coreExceptions.h"
#include "dynamicInitialConditionRecovery.h"
#include "../events/eventQueue.h"
#include "faultResetRecovery.h"
#include "../gridBus.h"
#include "gridDynSimulationFileOps.h"
#include "diagnostics.h"
#include "../solvers/solverInterface.h"
#include "utilities/matrixData.hpp"
#include "utilities/vectorOps.hpp"
// system libraries
#include <algorithm>
#include <cassert>

#include <cstdio>
//#include <fstream>
//#include <iostream>
namespace griddyn
{
static IOdata kNullOutputVec;  //!<  this is a purposely created empty vector which gets used for functions that take as
//! an input a vector but don't use it.

// --------------- dynamic program ---------------
// dynamic solver and initial conditions
int gridDynSimulation::dynInitialize (coreTime tStart)
{
    if (opFlags[dyn_initialized])
    {
        offsets.unload (true);
    }
    const solverMode &tempSm =
      (defaultDynamicSolverMethod == dynamic_solver_methods::partitioned) ? *defDynDiffMode : *defDAEMode;

    int retval = makeReady (gridState_t::POWERFLOW_COMPLETE, tempSm);
    if (retval != FUNCTION_EXECUTION_SUCCESS)
    {
        return retval;
    }
    auto dynData = getSolverInterface (tempSm);
    const solverMode &sm = dynData->getSolverMode ();
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
    // In most cases this should be none but users can manipulate the times if they choose
    EvQ->executeEvents (tStart - 0.001);

    dynInitializeA (tStart, lower_flags (controlFlags));

    count_t ssize = stateSize (sm);
    if (ssize == 0)
    {
        LOG_WARNING ("State size==0 stopping computation\n");
        return 0;  // TODO::  add a positive return code when state size is 0
    }
    updateOffsets (sm);
    // check for objects with roots
    count_t totalRoots = 0;
    // CSW: Need to send in the number of roots to find so that memory
    // can be allocated to for the array indicating indices of roots.
    // do the final Ida initialization
    if (controlFlags[roots_disabled])
    {
        offsets.local ().local.algRoots = 0;
        offsets.local ().local.diffRoots = 0;
        opFlags[has_roots] = false;
        opFlags[has_alg_roots] = false;
    }
    else
    {
        totalRoots = rootSize (sm);
        if (totalRoots > 0)
        {
            setRootOffset (0, sm);
            opFlags[has_roots] = true;
            opFlags[has_alg_roots] = (offsets.getOffsets(sm).total.algRoots > 0);

        }
        else
        {
            opFlags[has_roots] = false;
            opFlags[has_alg_roots] = false;
        }
    }

    dynData->allocate (ssize, totalRoots);

    // dynInitializeB
    dynData->set ("tolerance", tols.rtol);
    // run the dynamic initialization part B there is no actual output from an area currently
    dynInitializeB (noInputs, noInputs, kNullOutputVec);

    // check if any updates need to take place
    // run any 0 time events
    if (state_record_period > timeZero)
    {
        if (!(stateRecorder))
        {
            stateRecorder = std::make_shared<functionEventAdapter> (
              [=]() {
                  saveStateBinary (this, stateFile, sm);
                  return change_code::no_change;
              },
              state_record_period, tStart);

            stateRecorder->partB_only = true;
            add (stateRecorder);
        }
    }
    // Execute any events at the start time
    EvQ->executeEvents (tStart);

    //	assert(nextStopTime == EvQ->getNextTime());
    nextStopTime = EvQ->getNextTime ();

    // initialize the solver
    dynData->initialize (tStart);

    opFlags &= RESET_CHANGE_FLAG_MASK;
    pState = gridState_t::DYNAMIC_INITIALIZED;
    currentTime = tStart;
    return FUNCTION_EXECUTION_SUCCESS;
}

int gridDynSimulation::runDynamicSolverStep (std::shared_ptr<SolverInterface> &dynData,
                                             coreTime nextStop,
                                             coreTime &timeActual)
{
    int retval = FUNCTION_EXECUTION_SUCCESS;
    if (controlFlags[single_step_mode])
    {
        while ((timeActual + tols.timeTol < nextStop) && (retval == FUNCTION_EXECUTION_SUCCESS))
        {
            retval = dynData->solve (nextStop, timeActual, SolverInterface::step_mode::single_step);
            if (retval == FUNCTION_EXECUTION_SUCCESS)
            {
                for (auto &sso : singleStepObjects)
                {
                    sso->setState (timeActual, dynData->state_data (), dynData->deriv_data (),
                                   dynData->getSolverMode ());
                }
            }
        }
    }
    else
    {
        retval = dynData->solve (nextStop, timeActual);
    }
    if (retval != FUNCTION_EXECUTION_SUCCESS)
    {
        //   dynData->printStates(true);
        handleEarlySolverReturn (retval, timeActual, dynData);
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

int gridDynSimulation::dynamicDAEStartupConditions (std::shared_ptr<SolverInterface> &dynData,
                                                    const solverMode &sMode)
{
    int retval = FUNCTION_EXECUTION_SUCCESS;
    if (pState == gridState_t::DYNAMIC_INITIALIZED)
    {
        // do mode 0 IC calculation
        guessState (currentTime, dynData->state_data (), dynData->deriv_data (), sMode);

        retval =
          dynData->calcIC (currentTime, probeStepTime, SolverInterface::ic_modes::fixed_masked_and_deriv, false);
        if (retval != FUNCTION_EXECUTION_SUCCESS)
        {
            // for (size_t kk = 0; kk < dynData->getSize(); ++kk)
            //  {
            //  printf("%d : deriv=%f\n", kk, dynData->deriv_data()[kk]);
            //  }
            retval = generateDaeDynamicInitialConditions (sMode);
            if (retval != FUNCTION_EXECUTION_SUCCESS)
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
        if (retval != FUNCTION_EXECUTION_SUCCESS)
        {
            LOG_ERROR (dynData->getLastErrorString ());
            LOG_ERROR ("Unable to generate dynamic solution conditions modeB");
            return retval;
        }
    }
    return retval;
}

// IDA DAE Solver
int gridDynSimulation::dynamicDAE (coreTime tStop)
{
    int out = FUNCTION_EXECUTION_SUCCESS;

    int tstep = 0;
    const solverMode &sMode = *defDAEMode;

    setupDynamicDAE ();
    auto dynData = getSolverInterface (sMode);

    int retval = dynamicDAEStartupConditions (dynData, sMode);
    if (retval != FUNCTION_EXECUTION_SUCCESS)
    {
        return retval;
    }

    nextStopTime = (std::min) (tStop, EvQ->getNextTime ());


    //If running in block mode, integrate over entire simulation interval
    if (dynData->getFlag("block_mode_only"))
    { //this is primarily for braid
        nextStopTime = tStop;
    }

    // go into the main loop
    int smStep = 0;
    while (timeReturn < tStop)
    {
        coreTime nextStop = (std::min) (tStop, nextStopTime);

        nextStopTime = tStop;
        if (nextStop - currentTime < tols.timeTol)  // if the interval is too small just advance the clock a little
        {  // the most likely cause of this is numerical instability in recorders and events
            timeReturn = nextStop;
        }
        else
        {
            runDynamicSolverStep (dynData, nextStop, timeReturn);
        }

        while (timeReturn + tols.timeTol <
               nextStop)  // the timeTol is for stopping just prior to the expected stop time
        {
            coreTime lastTimeStop = currentTime;
            dynamicCheckAndReset (sMode);
            retval = generateDaeDynamicInitialConditions (sMode);
            if (retval != FUNCTION_EXECUTION_SUCCESS)
            {
                pState = gridState_t::DYNAMIC_PARTIAL;
                LOG_ERROR ("simulation halted unable to converge");
                LOG_ERROR (dynData->getLastErrorString ());
                return FUNCTION_EXECUTION_FAILURE;
            }
            // update the stopping time just in case the events have changed
            nextStop = (std::min) (tStop, EvQ->getNextTime ());
            retval = runDynamicSolverStep (dynData, nextStop, timeReturn);

            currentTime = timeReturn;

            if (retval != SOLVER_ROOT_FOUND)
            {
                if (timeReturn < lastTimeStop + tols.timeTol)
                {
                    ++tstep;
                    if (tstep == 1)  // there are some circumstances where internal models halt advancement until
                    // the clock moves forward
                    {
                        currentTime += tols.timeTol;
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
        currentTime = nextStop;
        // transmit the current state to the various objects for updates and recorders
        setState (currentTime, dynData->state_data (), dynData->deriv_data (), sMode);
        updateLocalCache ();
        auto ret = EvQ->executeEvents (currentTime);
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
    if ((consolePrintLevel >= print_level::trace) || (logPrintLevel >= print_level::trace))
    {
        dynData->logSolverStats (print_level::trace);
        dynData->logErrorWeights (print_level::trace);
    }

    // store the results to the buses
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
        throw (executionFailure (this, "Unable to prepare simulation for dynamic solution"));
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

int gridDynSimulation::dynamicPartitionedStartupConditions (std::shared_ptr<SolverInterface> &dynDataDiff,
                                                            std::shared_ptr<SolverInterface> &dynDataAlg,
                                                            const solverMode &sModeDiff,
                                                            const solverMode &sModeAlg)
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
            guessState (currentTime, daeData->state_data (), daeData->deriv_data (), *defDAEMode);
            retval = daeData->calcIC (currentTime, probeStepTime,
                                      SolverInterface::ic_modes::fixed_masked_and_deriv, false);
            if (retval != FUNCTION_EXECUTION_SUCCESS)
            {
                retval = generateDaeDynamicInitialConditions (*defDAEMode);
                if (retval != FUNCTION_EXECUTION_SUCCESS)
                {
                    LOG_ERROR (daeData->getLastErrorString ());
                    LOG_ERROR ("Unable to generate initial dynamic solution modeA");
                    return retval;
                }
            }
        }
        else
        {
            guessState (currentTime, dynDataDiff->state_data (), dynDataDiff->deriv_data (), sModeDiff);
            guessState (currentTime, dynDataAlg->state_data (), nullptr, sModeAlg);
        }
    }
    else
    {
        guessState (currentTime, dynDataDiff->state_data (), dynDataDiff->deriv_data (), sModeDiff);
        guessState (currentTime, dynDataAlg->state_data (), nullptr, sModeAlg);
    }
    return retval;
}

int gridDynSimulation::dynamicPartitioned (coreTime tStop, coreTime tStep)
{
    int out = FUNCTION_EXECUTION_SUCCESS;

    auto nextEventTime = EvQ->getNextTime ();
    setupDynamicPartitioned ();
    coreTime lastTimeStop = currentTime;

    auto dynDataAlg = getSolverInterface (*defDynAlgMode);
    auto dynDataDiff = getSolverInterface (*defDynDiffMode);

    dynDataDiff->set ("step", tStep);
    auto &sModeAlg = dynDataAlg->getSolverMode ();
    auto &sModeDiff = dynDataDiff->getSolverMode ();

    int tstep = 0;
    int retval = dynamicPartitionedStartupConditions (dynDataDiff, dynDataAlg, sModeDiff, sModeAlg);
    if (retval != FUNCTION_EXECUTION_SUCCESS)
    {
        return retval;
    }
    // go into the main loop
    int smStep = 0;
    while (getSimulationTime() < tStop)
    {
        nextStopTime = std::min (tStop, nextEventTime);

        if (nextStopTime - currentTime <
            tols.timeTol)  // if the interval is too small just advance the clock a little
        {  // the most likely cause of this is a tiny numerical instability in recorders and events
            timeReturn = nextStopTime;
        }
        else
        {
            retval = runDynamicSolverStep (dynDataDiff, nextStopTime, timeReturn);
            if (retval == FUNCTION_EXECUTION_SUCCESS)
            {
                dynAlgebraicSolve (timeReturn, dynDataDiff->state_data (), dynDataDiff->deriv_data (), sModeDiff);
            }
        }

        while (timeReturn + tols.timeTol <
               nextStopTime)  // the timeTol is for stopping just prior to the expected stop time
        {
            lastTimeStop = timeReturn;
            dynamicCheckAndReset (sModeDiff);
            retval = generatePartitionedDynamicInitialConditions (sModeAlg, sModeDiff);
            if (retval != FUNCTION_EXECUTION_SUCCESS)
            {
                pState = gridState_t::DYNAMIC_PARTIAL;
                LOG_ERROR ("simulation halted unable to converge");
                LOG_ERROR (dynDataDiff->getLastErrorString ());
                return FUNCTION_EXECUTION_FAILURE;
            }
            retval = runDynamicSolverStep (dynDataDiff, nextStopTime, timeReturn);
            dynAlgebraicSolve (timeReturn, dynDataDiff->state_data (), dynDataDiff->deriv_data (), sModeDiff);
            currentTime = timeReturn;

            // CSW Changed this from 2e-3 to 1e-7: need to rethink this in light of rootfinding
            if (retval != SOLVER_ROOT_FOUND)
            {
                if (timeReturn < lastTimeStop + tols.timeTol)
                {
                    ++tstep;
                    if (tstep == 1)  // there are some circumstances where internal models halt advancement until
                    // the clock moves forward
                    {
                        currentTime = currentTime + tols.timeTol;
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
        currentTime = nextStopTime;
        if (nextEventTime - tols.timeTol < currentTime)
        {
            // transmit the current state to the various objects for updates and recorders
            setState (currentTime, dynDataDiff->state_data (), dynDataDiff->deriv_data (), sModeDiff);
            setState (currentTime, dynDataAlg->state_data (), nullptr, sModeAlg);
            updateLocalCache ();
            auto ret = EvQ->executeEvents (currentTime);
            if (ret > change_code::non_state_change)
            {
                dynamicCheckAndReset (sModeDiff);
                retval = generatePartitionedDynamicInitialConditions (sModeAlg, sModeDiff);
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
    }
    if ((consolePrintLevel >= print_level::trace) || (logPrintLevel >= print_level::trace))
    {
        dynDataDiff->logSolverStats (print_level::trace);
        dynDataDiff->logErrorWeights (print_level::trace);
    }

    // store the results to the buses
    pState = gridState_t::DYNAMIC_COMPLETE;
    return out;
}

int gridDynSimulation::dynamicDecoupled (coreTime /*tStop*/, coreTime /*tStep*/)
{
    return FUNCTION_EXECUTION_FAILURE;
}

int gridDynSimulation::step ()
{
    coreTime tact;
    coreTime nextT = currentTime + stepTime;
    int ret = step (nextT, tact);
    return (tact == nextT) ? FUNCTION_EXECUTION_SUCCESS : ret;
}

int gridDynSimulation::step (coreTime nextStep, coreTime &timeActual)
{
    if (currentTime >= nextStep)
    {
        if (EvQ->getNextTime () <= currentTime)
        {
            EvQ->executeEvents (currentTime);
        }
        else
        {
            timeActual = nextStep;
            return FUNCTION_EXECUTION_SUCCESS;
        }
    }
    timeActual = currentTime;
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
                return 1;
            }
        }  // this step does a reset of IDA if necessary
    }
    else if (pState == gridState_t::DYNAMIC_INITIALIZED)
    {
        // do mode 0 IC calculation
        guessState (currentTime, dynData->state_data (), dynData->deriv_data (), sm);
        retval =
          dynData->calcIC (currentTime, probeStepTime, SolverInterface::ic_modes::fixed_masked_and_deriv, false);
        if (retval != FUNCTION_EXECUTION_SUCCESS)
        {
            // for (size_t kk = 0; kk < dynData->getSize(); ++kk)
            //  {
            //  printf("%d : deriv=%f\n", kk, dynData->deriv_data()[kk]);
            //  }
            retval = generateDaeDynamicInitialConditions (sm);
            if (retval != FUNCTION_EXECUTION_SUCCESS)
            {
#if JAC_CHECK_ENABLED > 0
                if (JacobianCheck (this, sm) > 0)
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
        // check to make sure nothing has changed
        dynamicCheckAndReset (sm);
        retval = generateDaeDynamicInitialConditions (sm);
        if (retval != FUNCTION_EXECUTION_SUCCESS)
        {
            LOG_ERROR ("Unable to generate dynamic solution conditions modeB");
            return retval;
        }
    }
    nextStopTime = std::min (nextStep, EvQ->getNextTime ());

    while (timeReturn < nextStep)
    {
        coreTime tStop = std::min (nextStep, nextStopTime);

        nextStopTime = nextStep;
        if (tStop - currentTime < tols.timeTol)  // if the interval is too small just advance the clock a little
        {  // the most likely cause of this is numerical instability in recorders and events
            timeReturn = tStop;
        }
        else
        {
            retval = runDynamicSolverStep (dynData, nextStopTime, timeReturn);
            currentTime = timeReturn;
        }

        while (timeReturn + tols.timeTol < tStop)
        {
            coreTime lastTimeStop = currentTime;
            if (dynamicCheckAndReset (sm))
            {
                retval = generateDaeDynamicInitialConditions (sm);
                if (retval != FUNCTION_EXECUTION_SUCCESS)
                {
                    timeActual = currentTime;
                    return 1;
                }
            }  // this step does a reset of IDA if necessary
            tStop =
              std::min (stopTime,
                        EvQ->getNextTime ());  // update the stopping time just in case the events have changed
            retval = runDynamicSolverStep (dynData, nextStopTime, timeReturn);
            currentTime = timeReturn;
            // CSW Changed this from 2e-3 to 1e-7: need to rethink this in light of rootfinding
            if (timeReturn < lastTimeStop + tols.timeTol)
            {
                timeActual = currentTime;
                return (retval);  // TODO:: PT replace this with a constant
            }
        }
        currentTime = tStop;
        // transmit the current state to the various objects for updates and recorders
        setState (currentTime, dynData->state_data (), dynData->deriv_data (), sm);

        auto ret = EvQ->executeEvents (currentTime);
        if (ret > change_code::no_change)
        {
            dynamicCheckAndReset (sm);
            break;
        }
        nextStopTime = EvQ->getNextTime ();
        // recorders last to capture any state change
    }
    timeActual = currentTime;
    pState = gridState_t::DYNAMIC_COMPLETE;
    return retval;
}

void gridDynSimulation::handleEarlySolverReturn (int retval,
                                                 coreTime timeActual,
                                                 std::shared_ptr<SolverInterface> &dynData)
{
    ++haltCount;

    if (opFlags[has_roots])
    {
        if (retval == SOLVER_ROOT_FOUND)  // a root was found in IDASolve
        {  // Note that if a root is found, integration halts at the root time which is
            // returned in timeReturn.
            dynData->getRoots ();
            currentTime = timeActual;
            setState (timeActual, dynData->state_data (), dynData->deriv_data (), dynData->getSolverMode ());
            LOG_DEBUG ("Root detected");
            rootTrigger (timeActual, noInputs, dynData->rootsfound, dynData->getSolverMode ());
        }
        else if (retval == SOLVER_INVALID_STATE_ERROR)
        {
            // if we get into here the most likely cause is a very low voltage bus
            stateData sD (timeActual, dynData->state_data (), dynData->deriv_data ());

            rootCheck (noInputs, sD, dynData->getSolverMode (), check_level_t::low_voltage_check);
            // return dynData->calcIC(getSimulationTime(), probeStepTime, SolverInterface::ic_modes::fixed_diff,
            // true);
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
    if ((opFlags[state_change_flag]) ||
        (change == change_code::state_count_change))  // we changed object states so we have to do a full reset
    {
        if (checkEventsForDynamicReset (currentTime + probeStepTime, sMode))
        {
            return true;
        }
        reInitDyn (sMode);
    }
    else if ((opFlags[object_change_flag]) || (change == change_code::object_change))  // the object count changed
    {
        if (checkEventsForDynamicReset (currentTime + probeStepTime, sMode))
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
    else if ((opFlags[jacobian_count_change_flag]) || (change == change_code::jacobian_change))
    {
        if (checkEventsForDynamicReset (currentTime + probeStepTime, sMode))
        {
            return true;
        }
        handleRootChange (sMode, dynData);
        dynData->setMaxNonZeros (jacSize (sMode));
        // Allow for the fact that the new size of Jacobian now also has a different number of non-zeros
        dynData->sparseReInit (SolverInterface::sparse_reinit_modes::resize);
    }
    else if (opFlags[root_change_flag])
    {
        handleRootChange (sMode, dynData);
    }
    else  // mode ==0
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
    // check and deal with voltage Reset
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
        stateData sD (getSimulationTime(), dynData->state_data (), dynData->deriv_data ());

        rootCheck (noInputs, sD, dynData->getSolverMode (), check_level_t::low_voltage_check);
        // return dynData->calcIC(getSimulationTime(), probeStepTime, SolverInterface::ic_modes::fixed_diff, true);
        opFlags.reset (low_bus_voltage);
    }
    // Do the first cut guessState at the solution
    guessState (currentTime, dynData->state_data (), dynData->deriv_data (), sMode);
    auto maxResid = checkResid (this, dynData);
    // double cr2;
    if (std::abs (maxResid.first) > 0.5)
    {
        if ((logPrintLevel >= print_level::debug) || (consolePrintLevel >= print_level::debug))
        {
            stringVec snames;
            getStateName (snames, sMode);
            LOG_DEBUG ("state " + std::to_string (maxResid.second) + ':' + snames[maxResid.second] +
                       " error= " + std::to_string (maxResid.first));
        }
        // converge (currentTime, dynData->state_data (), dynData->deriv_data (), sMode,
        // converge_mode::high_error_only, 0.05);
        // JacobianCheck(sMode);
        //  printStateNames(this,sMode);
    }

    retval = dynData->calcIC (currentTime, probeStepTime, SolverInterface::ic_modes::fixed_diff, true);

    if (retval ==
        -22)  // this is bad initial conditions TODO:: map this to Solver ERROR codes not Sundials ERROR codes
    {
        converge (currentTime, dynData->state_data (), dynData->deriv_data (), sMode,
                  converge_mode::single_iteration, 0.05);
        retval = dynData->calcIC (currentTime, probeStepTime, SolverInterface::ic_modes::fixed_diff, true);
    }
    if (retval == FUNCTION_EXECUTION_SUCCESS)
    {
        retval = checkAlgebraicRoots (dynData);
    }
    // if we still haven't fixed it call the recovery object and let it try to deal with it
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

    if ((consolePrintLevel >= print_level::trace) || (logPrintLevel >= print_level::trace))
    {
        dynData->logSolverStats (print_level::trace, true);
    }
    return retval;
}

int gridDynSimulation::generatePartitionedDynamicInitialConditions (const solverMode &sModeAlg,
                                                                    const solverMode &sModeDiff)
{
    auto dynDataAlg = getSolverInterface (sModeAlg);
    auto dynDataDiff = getSolverInterface (sModeDiff);
    int retval = FUNCTION_EXECUTION_FAILURE;
    // check and deal with voltage Reset
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
        /*stateData sD(getSimulationTime(), dynData->state_data(), dynData->deriv_data());

        rootCheck(&sD, dynData->getSolverMode(), check_level_t::low_voltage_check);
        //return dynData->calcIC(getSimulationTime(), probeStepTime, SolverInterface::ic_modes::fixed_diff, true);
        opFlags.reset(low_bus_voltage);
        */
    }
    coreTime tRet;
    retval = dynDataAlg->solve (currentTime + probeStepTime, tRet);
    if (retval == FUNCTION_EXECUTION_SUCCESS)
    {
        currentTime += probeStepTime;
    }
    return retval;
}

int gridDynSimulation::checkAlgebraicRoots (std::shared_ptr<SolverInterface> &dynData)
{
    if (opFlags[has_alg_roots])
    {
        const solverMode &sMode = dynData->getSolverMode ();
        dynData->getCurrentData ();
        setState (currentTime + probeStepTime, dynData->state_data (), dynData->deriv_data (), sMode);
        updateLocalCache ();
        change_code ret = rootCheck (noInputs, emptyStateData, cLocalSolverMode, check_level_t::full_check);
        handleRootChange (sMode, dynData);
        if (ret > change_code::non_state_change)
        {
            dynamicCheckAndReset (sMode, ret);
            int retval = dynData->calcIC (currentTime, probeStepTime, SolverInterface::ic_modes::fixed_diff, true);

            if (retval < 0)  // this is bad initial conditions
            {
                converge (currentTime, dynData->state_data (), dynData->deriv_data (), sMode,
                          converge_mode::single_iteration, 0.05);
                retval = dynData->calcIC (currentTime, probeStepTime, SolverInterface::ic_modes::fixed_diff, true);
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
        if (checkEventsForDynamicReset (currentTime + probeStepTime, sMode))
        {
            return generateDaeDynamicInitialConditions (sMode);
        }

        reInitDyn (sMode);
        return generateDaeDynamicInitialConditions (sMode);
    }
    return HANDLER_NO_RETURN;
}

void gridDynSimulation::handleRootChange (const solverMode &sMode, std::shared_ptr<SolverInterface> &dynData)
{
    if (opFlags[root_change_flag])  // something with the roots changed
    {
        auto rs = rootSize (sMode);
        if (rs != static_cast<index_t> (dynData->rootsfound.size ()))
        {
            dynData->setRootFinding (rs);
            if (rs > 0)
            {
                setRootOffset (0, sMode);
            }
        }
        opFlags.reset (root_change_flag);
    }
    else if (rootSize (sMode) > 0)
    {
        if (offsets.getRootOffset (sMode) == kNullLocation)
        {
            setRootOffset (0, sMode);
        }
    }
}

void gridDynSimulation::getConstraints (double consData[], const solverMode &sMode)
{
    // if ((controlFlags[voltage_constraints_flag]) || (opFlags[has_constraints]))
    if (controlFlags[voltage_constraints_flag])
    {
        getVoltageStates (consData, sMode);
    }
    if (opFlags[has_constraints])
    {
        Area::getConstraints (consData, sMode);
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

    // check for objects with roots
    count_t nRoots = 0;
    if (controlFlags[roots_disabled])
    {
        offsets.local ().local.algRoots = 0;
        offsets.local ().local.diffRoots = 0;
        opFlags[has_roots] = false;
        opFlags[has_alg_roots] = false; 
    }
    else
    {
        nRoots = rootSize (sMode);
        if (rootSize (sMode) > 0)
        {
            opFlags[has_roots] = true;
            setRootOffset (0, sMode); 
            opFlags[has_alg_roots] = (offsets.local().total.algRoots > 0);

        }
        else
        {
            opFlags[has_roots] = false;
            opFlags[has_alg_roots] = false;
        }
        
    }
    if (controlFlags[constraints_disabled])
    {
        opFlags[has_constraints] = false;
    }

    // CSW: Need to send in the number of roots to find so that memory
    // can be allocated to for the array indicating indices of roots.
    auto ss = stateSize (sMode);
    dynData->allocate (ss, nRoots);

    // guessState an initial condition
    guessState (currentTime, dynData->state_data (), dynData->deriv_data (), sMode);
    // dynInitializeB ida memory
    if (ss > 0)
    {
        dynData->initialize (currentTime);
    }

    opFlags &= RESET_CHANGE_FLAG_MASK;
    pState = gridState_t::DYNAMIC_INITIALIZED;
    return FUNCTION_EXECUTION_SUCCESS;
}

#define DEBUG_RESID 0
#ifdef NDEBUG
#define CHECK_RESID 0
#define CHECK_STATE 0
#else
#define CHECK_RESID 1
#define CHECK_STATE 1
#endif

#if DEBUG_RESID > 0
const static double resid_print_tol = 1e-6;
#endif
// IDA nonlinear function evaluation
int gridDynSimulation::residualFunction (coreTime time,
                                         const double state[],
                                         const double dstate_dt[],
                                         double resid[],
                                         const solverMode &sMode) noexcept
{
    ++residCount;
    stateData sD (time, state, dstate_dt, residCount);

#if (CHECK_STATE > 0)
    auto dynDataa = getSolverInterface (sMode);
    for (index_t kk = 0; kk < dynDataa->size (); ++kk)
    {
        if (!std::isfinite (state[kk]))
        {
            LOG_ERROR ("state[" + std::to_string (kk) + "] is not finite");
            printStateNames (this, sMode);
            return FUNCTION_EXECUTION_FAILURE;
        }
    }
#endif

    fillExtraStateData (sD, sMode);
    // call the area based function to handle the looping
    preEx (noInputs, sD, sMode);
    residual (noInputs, sD, resid, sMode);
    delayedResidual (noInputs, sD, resid, sMode);

#if (CHECK_RESID > 0)
    auto dynDatab = getSolverInterface (sMode);
    for (index_t kk = 0; kk < dynDatab->size (); ++kk)
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
    static coreTime ptime = negTime;
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
        ptime = time;
#if DEBUG_RESID > 1
        for (index_t kk = 0; kk < ss; kk++)
        {
#if (DEBUG_RESID == 3)
            printf ("%d[%f] %d r=%e state=%f\n", static_cast<int> (residCount), static_cast<double> (time),
                    static_cast<int> (kk), resid[kk], state[kk]);
#else
            if (std::abs (resid[kk]) > resid_print_tol)
            {
                printf ("%d[%f] %d r=%e state=%f\n", static_cast<int> (residCount), static_cast<double> (time),
                        static_cast<int> (kk), resid[kk], state[kk]);
            }
#endif
        }
#endif
        dynData->printStates (true);
    }
    else if (ptime != time)
    {
        std::copy (resid, resid + ss, rvals.data ());
        std::copy (state, state + ss, lstate.data ());
        std::fill (dbigger.begin (), dbigger.end (), 0);

#if DEBUG_RESID > 1
        printf ("time change %e\n", static_cast<double> (time - ptime));
        for (index_t kk = 0; kk < ss; kk++)
        {
#if (DEBUG_RESID == 3)
            printf ("%d[%f] %d r=%e state=%f\n", static_cast<int> (residCount), static_cast<double> (time),
                    static_cast<int> (kk), resid[kk], state[kk]);
#else
            if (std::abs (resid[kk]) > resid_print_tol)
            {
                printf ("%d[%f] %d r=%e state=%f\n", static_cast<int> (residCount), static_cast<double> (time),
                        static_cast<int> (kk), resid[kk], state[kk]);
            }
#endif
        }
#endif
        ptime = time;
    }
    else
    {
        for (index_t kk = 0; kk < ss; kk++)
        {
#if DEBUG_RESID > 1
#if (DEBUG_RESID == 3)
            printf ("%d[%f] %d r=%e state=%f dr=%e ds=%e\n", static_cast<int> (residCount),
                    static_cast<double> (time), static_cast<int> (kk), resid[kk], state[kk], resid[kk] - rvals[kk],
                    state[kk] - lstate[kk]);
#else
            if ((std::abs (resid[kk]) > resid_print_tol) || (std::abs (state[kk] - lstate[kk]) > 1e-5))
            {
                printf ("%d[%f] %d r=%e state=%f dr=%e ds=%e\n", static_cast<int> (residCount),
                        static_cast<double> (time), static_cast<int> (kk), resid[kk], state[kk],
                        resid[kk] - rvals[kk], state[kk] - lstate[kk]);
            }
#endif
#endif
            if (std::abs (resid[kk]) - 1e-7 > std::abs (rvals[kk]))
            {
                dbigger[kk] += 1;
                if (dbigger[kk] > 2)
                {
                    printf ("residual[%d] getting bigger from %e to %e at time=%f\n", static_cast<int> (kk),
                            rvals[kk], resid[kk], static_cast<double> (time));
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
    // compute the maximum resid and location
    auto me = std::max_element (resid, resid + ss);
    printf (" max residual at %d = %f\n", static_cast<int> (me - resid), *me);
#endif
    if (opFlags[invalid_state_flag])
    {
        opFlags.reset (invalid_state_flag);
        return 1;
    }
    return 0;
}

int gridDynSimulation::derivativeFunction (coreTime time,
                                           const double state[],
                                           double dstate_dt[],
                                           const solverMode &sMode) noexcept
{
    ++residCount;
    stateData sD (time, state, dstate_dt, residCount);
    fillExtraStateData (sD, sMode);
#if (CHECK_STATE > 0)
    auto dynDataa = getSolverInterface (sMode);
    for (index_t kk = 0; kk < dynDataa->size (); ++kk)
    {
        if (!std::isfinite (state[kk]))
        {
            LOG_ERROR ("state[" + std::to_string (kk) + "] is not finite");
            return FUNCTION_EXECUTION_FAILURE;
        }
    }
#endif

    // call the area based function to handle the looping
    preEx (noInputs, sD, sMode);
    derivative (noInputs, sD, dstate_dt, sMode);
    delayedDerivative (noInputs, sD, dstate_dt, sMode);
    return FUNCTION_EXECUTION_SUCCESS;
}

// Jacobian computation
int gridDynSimulation::jacobianFunction (coreTime time,
                                         const double state[],
                                         const double dstate_dt[],
                                         matrixData<double> &md,
                                         double cj,
                                         const solverMode &sMode) noexcept
{
    ++JacobianCallCount;
    // assuming it is the same data as the preceding residual call  (it is for IDA but not sure if this assumption
    // will be generally valid)
    stateData sD (time, state, dstate_dt, residCount);
    sD.cj = cj;
    fillExtraStateData (sD, sMode);
    // the area function to evaluate the Jacobian elements
    preEx (noInputs, sD, sMode);
    md.clear ();
    jacobianElements (noInputs, sD, md, noInputLocs, sMode);
    delayedJacobian (noInputs, sD, md, noInputLocs, sMode);

    return FUNCTION_EXECUTION_SUCCESS;
}

int gridDynSimulation::rootFindingFunction (coreTime time,
                                            const double state[],
                                            const double dstate_dt[],
                                            double roots[],
                                            const solverMode &sMode) noexcept
{
    stateData sD (time, state, dstate_dt, residCount);
    fillExtraStateData (sD, sMode);
    rootTest (noInputs, sD, roots, sMode);
    return FUNCTION_EXECUTION_SUCCESS;
}

int gridDynSimulation::dynAlgebraicSolve (coreTime time,
                                          const double diffState[],
                                          const double deriv[],
                                          const solverMode &sMode) noexcept
{
    extraStateInformation[sMode.offsetIndex] = diffState;
    extraDerivInformation[sMode.offsetIndex] = deriv;

    auto sd = getSolverInterface (sMode.pairedOffsetIndex);
    int ret = FUNCTION_EXECUTION_FAILURE;
    if (sd)
    {
        coreTime tret;
        ret = sd->solve (time, tret);
        if (ret < 0)
        {
            if (JacobianCheck(this, sd->getSolverMode()) > 0)
            {
                printStateNames (this, sd->getSolverMode ());
            }
			sd->setFlag("print_resid");
        }
    }
    extraStateInformation[sMode.offsetIndex] = nullptr;
    extraDerivInformation[sMode.offsetIndex] = nullptr;
    return ret;
}

}  // namespace griddyn
