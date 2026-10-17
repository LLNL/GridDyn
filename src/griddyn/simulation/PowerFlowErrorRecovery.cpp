/*
 * Copyright (c) 2014-2026, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PowerFlowErrorRecovery.h"

#include "../GridDynSimulation.h"
#include "../solvers/SolverInterface.h"
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

namespace griddyn {
powerFlowErrorRecovery::powerFlowErrorRecovery(GridDynSimulation* gds,
                                               std::shared_ptr<SolverInterface> solverData):
    sim(gds), solver(std::move(solverData))
{
}
powerFlowErrorRecovery::~powerFlowErrorRecovery() = default;

powerFlowErrorRecovery::RecoveryReturnCodes powerFlowErrorRecovery::attemptFix(int errorCode)
{
    if ((errorCode == SOLVER_STEP_LIMIT_REACHED) &&
        (sim->currentProcessState() !=
         GridDynSimulation::GridState::INITIALIZED)) {  // something possibly went wrong in the
                                                        // initial setup try a full
                                                        // reinitialization
        if (++attempt_number > 3) {
            return RecoveryReturnCodes::OUT_OF_OPTIONS;
        }
        sim->reInitpFlow(solver->getSolverMode(), ChangeCode::STATE_SIZE_CHANGE);
        return RecoveryReturnCodes::MORE_OPTIONS;
    }
    if ((errorCode == SOLVER_INVALID_STATE_ERROR) && (attempt_number < 5) && lowVoltageFix()) {
        ++attempt_number;
        return RecoveryReturnCodes::MORE_OPTIONS;
    }
    while (attempt_number < 5) {
        ++attempt_number;
        switch (attempt_number) {
            case 1:
                if (powerFlowFix1()) {
                    return RecoveryReturnCodes::MORE_OPTIONS;
                }
                break;
            case 2:
                if (powerFlowFix2()) {
                    return RecoveryReturnCodes::MORE_OPTIONS;
                }
                break;
            case 3:
                if (powerFlowFix3()) {
                    return RecoveryReturnCodes::MORE_OPTIONS;
                }
                break;
            case 4:
                if (powerFlowFix4()) {
                    return RecoveryReturnCodes::MORE_OPTIONS;
                }
                break;
            case 5:
                return powerFlowFix5() ? RecoveryReturnCodes::MORE_OPTIONS :
                                         RecoveryReturnCodes::OUT_OF_OPTIONS;
            default:
                break;
        }
    }
    return RecoveryReturnCodes::OUT_OF_OPTIONS;
}

void powerFlowErrorRecovery::reset()
{
    attempt_number = 0;
}

void powerFlowErrorRecovery::updateInfo(std::shared_ptr<SolverInterface> solverData)
{
    solver = std::move(solverData);
}
int powerFlowErrorRecovery::attempts() const
{
    return attempt_number;
}

std::uint32_t powerFlowErrorRecovery::adjustmentFlags() const
{
    auto flags = lower_flags(sim->controlFlags);
    // Recovery may inspect limits and protection on a failed solve, but
    // discretionary stepped controls require a converged voltage profile.
    SET_CONTROLFLAG(flags, DISABLE_SHUNT_ADJUSTMENTS);
    SET_CONTROLFLAG(flags, DISABLE_TAP_CHANGER_ADJUSTMENTS);
    return flags;
}

// Try any non-reversible adjustments which might be out there
bool powerFlowErrorRecovery::powerFlowFix1()
{
    if (sim->controlFlags[NO_POWERFLOW_ADJUSTMENTS] || !sim->opFlags[HAS_POWERFLOW_ADJUSTMENTS]) {
        return false;
    }
    sim->updateLocalCache();
    const ChangeCode eval =
        sim->powerFlowAdjust(noInputs, adjustmentFlags(), CheckLevel::FULL_CHECK);
    if (eval > ChangeCode::NON_STATE_CHANGE) {
        sim->checkNetwork(GridDynSimulation::NetworkCheckType::SIMPLIFIED);
        sim->reInitpFlow(solver->getSolverMode(), eval);
        return true;
    }
    return false;
}

// try a few rounds of Gauss Seidel like convergence
bool powerFlowErrorRecovery::powerFlowFix2()
{
    sim->guessState(sim->getSimulationTime(),
                    solver->stateData(),
                    nullptr,
                    solver->getSolverMode());
    sim->converge(sim->getSimulationTime(),
                  solver->stateData(),
                  nullptr,
                  solver->getSolverMode(),
                  ConvergeMode::BLOCK_ITERATION,
                  0.1);
    sim->setState(sim->getSimulationTime(), solver->stateData(), nullptr, solver->getSolverMode());
    if (sim->opFlags[HAS_POWERFLOW_ADJUSTMENTS] && !sim->controlFlags[NO_POWERFLOW_ADJUSTMENTS]) {
        sim->updateLocalCache();
        const ChangeCode eval =
            sim->powerFlowAdjust(noInputs, adjustmentFlags(), CheckLevel::REVERSABLE_ONLY);
        sim->reInitpFlow(solver->getSolverMode(), eval);
    }
    return true;
}

// check for some low voltage conditions and change the low voltage load conditions
bool powerFlowErrorRecovery::powerFlowFix3()
{
    if (sim->controlFlags[NO_POWERFLOW_ADJUSTMENTS]) {
        return false;
    }
    std::vector<double> voltages;
    sim->getVoltage(voltages);
    if (std::any_of(voltages.begin(), voltages.end(), [](double voltageValue) {
            return (voltageValue < 0.7);
        })) {
        sim->guessState(sim->getSimulationTime(),
                        solver->stateData(),
                        nullptr,
                        solver->getSolverMode());
        sim->converge(sim->getSimulationTime(),
                      solver->stateData(),
                      nullptr,
                      solver->getSolverMode(),
                      ConvergeMode::SINGLE_ITERATION,
                      0);
        sim->setState(sim->getSimulationTime(),
                      solver->stateData(),
                      nullptr,
                      solver->getSolverMode());
        if (!sim->opFlags[PREV_SETALL_PQVLIMIT]) {
            sim->setAll("load", "pqlowvlimit", 1.0);
            sim->controlFlags.set(VOLTAGE_CONSTRAINTS_FLAG);
            sim->opFlags.set(PREV_SETALL_PQVLIMIT);
            sim->log(sim, PrintLevel::DEBUG, "pq adjust on load");
        }
        sim->updateLocalCache();
        sim->powerFlowAdjust(noInputs, adjustmentFlags(), CheckLevel::REVERSABLE_ONLY);
        sim->reInitpFlow(solver->getSolverMode(), ChangeCode::STATE_SIZE_CHANGE);
        sim->guessState(sim->getSimulationTime(),
                        solver->stateData(),
                        nullptr,
                        solver->getSolverMode());
        sim->converge(sim->getSimulationTime(),
                      solver->stateData(),
                      nullptr,
                      solver->getSolverMode(),
                      ConvergeMode::BLOCK_ITERATION,
                      0.1);
        sim->setState(sim->getSimulationTime(),
                      solver->stateData(),
                      nullptr,
                      solver->getSolverMode());
        sim->updateLocalCache();
        ChangeCode adjustmentEval =
            sim->powerFlowAdjust(noInputs, adjustmentFlags(), CheckLevel::REVERSABLE_ONLY);
        count_t adjustmentCount = 0;
        while (adjustmentEval > ChangeCode::NO_CHANGE) {
            if (adjustmentCount >= sim->max_Vadjust_iterations) {
                sim->reInitpFlow(solver->getSolverMode(), adjustmentEval);
                return true;
            }
            ++adjustmentCount;
            sim->reInitpFlow(solver->getSolverMode(), adjustmentEval);
            sim->guessState(sim->getSimulationTime(),
                            solver->stateData(),
                            nullptr,
                            solver->getSolverMode());
            sim->converge(sim->getSimulationTime(),
                          solver->stateData(),
                          nullptr,
                          solver->getSolverMode(),
                          ConvergeMode::SINGLE_ITERATION,
                          0);
            sim->setState(sim->getSimulationTime(),
                          solver->stateData(),
                          nullptr,
                          solver->getSolverMode());
            sim->updateLocalCache();
            adjustmentEval =
                sim->powerFlowAdjust(noInputs, adjustmentFlags(), CheckLevel::REVERSABLE_ONLY);
        }
        return true;
    }
    return false;
}

// Try to disconnect very low voltage buses
bool powerFlowErrorRecovery::powerFlowFix4()
{
    std::vector<double> voltages;
    sim->getVoltage(voltages);
    if (std::any_of(voltages.begin(), voltages.end(), [](double voltageValue) {
            return (voltageValue < 0.1);
        })) {
        sim->setAll("bus", "lowvdisconnect", 0.03);
        sim->reInitpFlow(solver->getSolverMode());
        return true;
    }
    return false;
}

bool powerFlowErrorRecovery::lowVoltageFix()
{
    if (sim->controlFlags[NO_POWERFLOW_ADJUSTMENTS]) {
        return false;
    }
    const ChangeCode eval =
        sim->powerFlowAdjust(noInputs, adjustmentFlags(), CheckLevel::LOW_VOLTAGE_CHECK);
    if (eval > ChangeCode::NO_CHANGE) {
        sim->checkNetwork(GridDynSimulation::NetworkCheckType::SIMPLIFIED);
        sim->reInitpFlow(solver->getSolverMode(), eval);
        return true;
    }
    return false;
}

// Don't know what to do here yet
bool powerFlowErrorRecovery::powerFlowFix5()
{
    if (sim->controlFlags[NO_POWERFLOW_ADJUSTMENTS]) {
        return false;
    }
    const ChangeCode eval =
        sim->powerFlowAdjust(noInputs, adjustmentFlags(), CheckLevel::HIGH_ANGLE_TRIP);
    if (eval > ChangeCode::NO_CHANGE) {
        sim->checkNetwork(GridDynSimulation::NetworkCheckType::SIMPLIFIED);
        sim->reInitpFlow(solver->getSolverMode(), eval);
        return true;
    }
    return false;
}
}  // namespace griddyn
