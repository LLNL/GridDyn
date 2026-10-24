/*
 * Copyright (c) 2014-2026, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "../gtestHelper.h"
#include "core/CoreExceptions.h"
#include "core/ObjectFactory.hpp"
#include "griddyn/exciters/ExciterESAC2A.h"
#include "griddyn/stabilizers/StabilizerPss2b.h"
#include "utilities/MatrixDataSparse.hpp"
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <tuple>
#include <vector>

using namespace griddyn;

namespace {
IOdata exciterInputs()
{
    IOdata inputs(exciterInputCount, 0.0);
    inputs[exciterVoltageInLocation] = 1.0;
    inputs[exciterVsetInLocation] = 1.0;
    inputs[exciterXadIfdInLocation] = 0.5;
    return inputs;
}

void configureExciter(exciters::ExciterESAC2A& model)
{
    for (const auto& [name, value] : std::initializer_list<std::pair<std::string_view, double>>{
             {"tr", 0.1},     {"tb", 0.3},     {"tc", 0.15},    {"ka", 10.0},    {"ta", 0.2},
             {"te", 0.5},     {"tf", 1.0},     {"kf", 0.1},     {"kb", 2.0},     {"kh", 0.25},
             {"kc", 0.2},     {"kd", 0.3},     {"ke", 1.0},     {"e1", 1.0},     {"se1", 0.1},
             {"e2", 2.0},     {"se2", 0.2},    {"vfemax", 6.0}, {"vamax", 20.0}, {"vamin", -20.0},
             {"vrmax", 20.0}, {"vrmin", -20.0}}) {
        model.set(name, value);
    }
}

void configureStabilizer(stabilizers::StabilizerPss2b& model)
{
    for (const auto& [name, value] :
         std::initializer_list<std::pair<std::string_view, double>>{{"tw1", 2.0},
                                                                    {"tw2", 3.0},
                                                                    {"t6", 0.2},
                                                                    {"tw3", 4.0},
                                                                    {"tw4", 5.0},
                                                                    {"t7", 0.3},
                                                                    {"ks1", 2.0},
                                                                    {"ks2", 0.7},
                                                                    {"ks3", 0.8},
                                                                    {"t8", 0.4},
                                                                    {"t9", 0.2},
                                                                    {"t1", 0.15},
                                                                    {"t2", 0.1},
                                                                    {"t3", 0.2},
                                                                    {"t4", 0.1},
                                                                    {"t10", 0.05},
                                                                    {"t11", 0.1},
                                                                    {"vstmax", 10.0},
                                                                    {"vstmin", -10.0}}) {
        model.set(name, value);
    }
}

// Numerical derivatives of the residual, including d(xdot)/dx=cj and every
// input column. This also exercises clipped/limited branches away from kinks.
void checkModelJacobian(GridSubModel& model, const IOdata& inputs, std::vector<double> state)
{
    model.setOffset(0, cDaeSolverMode);
    ASSERT_EQ(model.stateSize(cDaeSolverMode), state.size());
    const auto stateCount = static_cast<index_t>(state.size());
    const auto inputCount = static_cast<index_t>(inputs.size());
    constexpr double step = 1e-6;
    constexpr double jacobianCoefficient = 0.73;
    constexpr index_t inputBase = 100;
    std::vector<double> rate(state.size(), 0.0);
    IOlocs inputLocs(inputs.size());
    for (index_t i = 0; i < inputCount; ++i) {
        inputLocs[i] = inputBase + i;
    }
    StateData data(0.0, state.data(), rate.data());
    data.cj = jacobianCoefficient;
    data.stateSize = static_cast<count_t>(state.size());
    MatrixDataSparse<double> matrix;
    model.jacobianElements(inputs, data, matrix, inputLocs, cDaeSolverMode);
    EXPECT_LE(matrix.size(), model.jacSize(cDaeSolverMode));
    matrix.compact();
    const auto residual =
        [&](const IOdata& inputValues,
            const std::vector<double>& stateValues,
            const std::vector<double>& rateValues) {
            StateData trial(0.0, stateValues.data(), rateValues.data());
            trial.stateSize = static_cast<count_t>(stateValues.size());
            std::vector<double> values(stateValues.size(), 0.0);
            model.residual(inputValues, trial, values.data(), cDaeSolverMode);
            return values;
        };
    for (index_t col = 0; col < stateCount + inputCount; ++col) {
        auto plusState = state;
        auto minusState = state;
        auto plusRate = rate;
        auto minusRate = rate;
        auto plusInput = inputs;
        auto minusInput = inputs;
        if (col < stateCount) {
            plusState[col] += step;
            minusState[col] -= step;
            // Algebraic rows ignore xdot, so including cj there is harmless.
            plusRate[col] += jacobianCoefficient * step;
            minusRate[col] -= jacobianCoefficient * step;
        } else {
            plusInput[col - stateCount] += step;
            minusInput[col - stateCount] -= step;
        }
        const auto plus = residual(plusInput, plusState, plusRate);
        const auto minus = residual(minusInput, minusState, minusRate);
        const auto column =
            col < stateCount ? col : inputBase + col - stateCount;
        for (index_t row = 0; row < stateCount; ++row) {
            const double numerical = (plus[row] - minus[row]) / (2.0 * step);
            EXPECT_NEAR(matrix.at(row, column),
                        numerical,
                        2e-5 * std::max(1.0, std::abs(numerical)))
                << "row " << row << " column " << column;
        }
    }
}
}  // namespace

TEST(Pss2bTests, BypassedRampInputClippingAndThirdLeadLagMatchEquations)
{
    stabilizers::StabilizerPss2b model;
    for (const auto name : {"tw1", "tw2", "t6", "tw3", "tw4", "t7", "t1", "t2", "t3", "t4", "t9"}) {
        model.set(name, 0.0);
    }
    model.set("m", 0.0);
    model.set("n", 0.0);
    model.set("ks1", 2.0);
    model.set("ks2", 0.7);
    model.set("ks3", 0.8);
    model.set("t10", 0.1);
    model.set("t11", 0.2);
    model.set("vs1max", 0.08);
    model.set("vs1min", -0.08);
    model.set("vs2max", 0.5);
    model.set("vs2min", -0.5);
    model.dynInitializeA(0.0, 0);
    IOdata fieldSet;
    model.dynInitializeB({1.0, 1.0, 0.8, 0.0}, {0.0}, fieldSet);
    ASSERT_EQ(model.getStates().size(), 2U);
    // Clipped speed=.08, clipped power=.5; u=2*(.08+.8*.7*.5-.7*.5)=.02.
    IOdata inputs{1.2, 1.0, 0.8, 0.9};
    std::vector<double> state{0.0, 0.01};
    std::vector<double> rates(2, 0.0);
    model.setState(0.0, state.data(), rates.data(), cLocalSolverMode);
    model.derivative(inputs, emptyStateData, rates.data(), cLocalSolverMode);
    EXPECT_NEAR(rates[1], 0.05, 1e-12);
    std::vector<double> residual(2, 0.0);
    model.residual(inputs, emptyStateData, residual.data(), cLocalSolverMode);
    EXPECT_NEAR(residual[0], 0.015, 1e-12);
    checkModelJacobian(model, inputs, state);
    model.set("vstmax", 0.01);
    checkModelJacobian(model, inputs, state);
    model.residual(inputs, emptyStateData, residual.data(), cLocalSolverMode);
    EXPECT_NEAR(residual[0], 0.01, 1e-12);
    model.setRootOffset(0, cLocalSolverMode);
    std::array<double, 6> roots{};
    model.rootTest(inputs, emptyStateData, roots.data(), cLocalSolverMode);
    EXPECT_LT(roots[0], 0.0);
    EXPECT_LT(roots[2], 0.0);
    EXPECT_LT(roots[4], 0.0);
    EXPECT_NO_THROW(model.rootTrigger(0.0, inputs, {1, 0, 1, 0, 1, 0}, cLocalSolverMode));
}

TEST(Pss2bTests, VariableRampOrdersAndPerturbedJacobians)
{
    for (const auto& [rampM, rampN] : {std::pair{5, 1}, {1, 3}, {2, 2}, {1, 8}, {8, 1}, {0, 0}}) {
        SCOPED_TRACE(std::to_string(rampM) + "," + std::to_string(rampN));
        stabilizers::StabilizerPss2b model;
        configureStabilizer(model);
        model.set("m", rampM);
        model.set("n", rampN);
        model.dynInitializeA(0.0, 0);
        IOdata inputs{1.0, 1.0, 0.8, 0.7};
        IOdata fieldSet;
        model.dynInitializeB(inputs, {0.0}, fieldSet);
        const auto initialized = model.getStates();
        const std::size_t rampStates = rampM == 0 || rampN == 0 ? 0 : rampM + rampN;
        EXPECT_EQ(initialized.size(), 10 + rampStates);
        std::vector<double> residual(initialized.size(), 0.0);
        model.residual(inputs, emptyStateData, residual.data(), cLocalSolverMode);
        for (const auto value : residual) {
            EXPECT_NEAR(value, 0.0, 1e-12);
        }
        auto state = initialized;
        for (std::size_t i = 1; i < state.size(); ++i) {
            state[i] += 0.001 * i;
        }
        inputs[0] = 1.02;
        inputs[3] = 0.73;
        checkModelJacobian(model, inputs, state);
    }
}

TEST(Pss2bTests, RampCascadeMatchesOpenipslTransferFunction)
{
    stabilizers::StabilizerPss2b model;
    for (const auto name :
         {"tw1", "tw2", "t6", "tw3", "tw4", "t7", "t1", "t2", "t3", "t4", "t10", "t11"}) {
        model.set(name, 0.0);
    }
    model.set("m", 2.0);
    model.set("n", 2.0);
    model.set("mode2", 0.0);
    model.set("t8", 0.4);
    model.set("t9", 0.2);
    model.set("ks1", 1.0);
    model.dynInitializeA(0.0, 0);
    IOdata fieldSet;
    model.dynInitializeB({1.0, 1.0, 0.8, 0.7}, {0.0}, fieldSet);
    std::vector<double> state{0.0, 0.01, 0.02, 0.03, 0.04};
    std::vector<double> rates(state.size(), 0.0);
    model.setState(0.0, state.data(), rates.data(), cLocalSolverMode);
    model.derivative({1.05, 1.0, 0.8, 0.7}, emptyStateData, rates.data(), cLocalSolverMode);
    // Two (1+.4s)/(1+.2s) stages give .09 and .16, followed by two lags.
    EXPECT_NEAR(rates[1], 0.2, 1e-12);
    EXPECT_NEAR(rates[2], 0.35, 1e-12);
    EXPECT_NEAR(rates[3], 0.65, 1e-12);
    EXPECT_NEAR(rates[4], -0.05, 1e-12);
}

TEST(Pss2bTests, BypassedWashoutsInitializeRemainingLagsAtSteadyState)
{
    stabilizers::StabilizerPss2b model;
    configureStabilizer(model);
    for (const auto name : {"tw1", "tw2", "tw3", "tw4"}) {
        model.set(name, 0.0);
    }
    model.dynInitializeA(0.0, 0);
    IOdata fieldSet;
    const IOdata inputs{1.02, 1.0, 0.8, 0.7};
    model.dynInitializeB(inputs, {0.0}, fieldSet);
    std::vector<double> residual(model.getStates().size(), 0.0);
    model.residual(inputs, emptyStateData, residual.data(), cLocalSolverMode);
    for (const auto value : residual) {
        EXPECT_NEAR(value, 0.0, 1e-12);
    }
    EXPECT_NEAR(model.getStates()[0],
                2.0 * (0.02 + (0.8 * 0.7 * 0.7) - (0.7 * 0.7)),
                1e-12);
    checkModelJacobian(model, inputs, model.getStates());
}

TEST(Esac2aTests, SaturationRectifierAndNondefaultGainsInitializeAtEquilibrium)
{
    exciters::ExciterESAC2A model;
    configureExciter(model);
    model.dynInitializeA(0.0, 0);
    auto inputs = exciterInputs();
    IOdata fieldSet;
    model.dynInitializeB(inputs, {1.0}, fieldSet);
    const auto state = model.getStates();
    ASSERT_EQ(state.size(), 6U);
    // Chosen saturation points yield A=0, B=.1, VE*SE(VE)=.1*VE^2.
    // Low-current rectifier: EFD=VE-.577*.2*.5.
    const double exciterVoltage = 1.0577;
    const double fieldExcitationVoltage =
        exciterVoltage + (0.1 * exciterVoltage * exciterVoltage) + 0.15;
    EXPECT_NEAR(state[4], exciterVoltage, 1e-12);
    EXPECT_NEAR(state[5], fieldExcitationVoltage, 1e-12);
    EXPECT_NEAR(state[3], 0.75 * fieldExcitationVoltage, 1e-12);
    EXPECT_NEAR(state[2], 0.075 * fieldExcitationVoltage, 1e-12);
    std::vector<double> residual(state.size(), 0.0);
    model.residual(inputs, emptyStateData, residual.data(), cLocalSolverMode);
    for (const auto value : residual) {
        EXPECT_NEAR(value, 0.0, 1e-10);
    }
    auto perturbed = state;
    perturbed[1] += 0.01;
    perturbed[2] += 0.03;
    perturbed[3] += 0.04;
    perturbed[4] += 0.02;
    perturbed[5] -= 0.01;
    checkModelJacobian(model, inputs, perturbed);
}

TEST(Esac2aTests, InitializationRejectsNonfiniteUsedSignals)
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    for (const index_t inputIndex : {exciterVoltageInLocation,
                                     exciterVsetInLocation,
                                     exciterVssInLocation,
                                     exciterXadIfdInLocation,
                                     exciterVuelInLocation,
                                     exciterVoelInLocation}) {
        for (const double invalid : {nan, infinity}) {
            exciters::ExciterESAC2A model;
            configureExciter(model);
            model.set("uel_enabled", 1.0);
            model.set("oel_enabled", 1.0);
            model.dynInitializeA(0.0, 0);
            auto inputs = exciterInputs();
            inputs[exciterVoelInLocation] = 20.0;
            inputs[inputIndex] = invalid;
            IOdata fieldSet;
            EXPECT_THROW(model.dynInitializeB(inputs, {1.0}, fieldSet), InvalidParameterValue)
                << "input " << inputIndex;
        }
    }
    exciters::ExciterESAC2A model;
    configureExciter(model);
    model.dynInitializeA(0.0, 0);
    auto inputs = exciterInputs();
    // Disabled selectors do not participate in the equation contract.
    inputs[exciterVuelInLocation] = nan;
    inputs[exciterVoelInLocation] = nan;
    IOdata fieldSet;
    EXPECT_NO_THROW(model.dynInitializeB(inputs, {1.0}, fieldSet));
    EXPECT_THROW(model.dynInitializeB(inputs, {nan}, fieldSet), InvalidParameterValue);
}

TEST(Esac2aTests, VariableFieldCeilingHasRectifiedSaturationAndCurrentJacobian)
{
    exciters::ExciterESAC2A model;
    configureExciter(model);
    model.set("vfemax", 3.0);
    model.dynInitializeA(0.0, 0);
    auto inputs = exciterInputs();
    IOdata fieldSet;
    model.dynInitializeB(inputs, {1.0}, fieldSet);
    auto state = model.getStates();
    state[4] = 4.0;  // VE beyond the moving upper bound.
    state[3] = 10.0;  // Outward regulator drive freezes field integration.
    const double ceiling = (-0.99423 + std::sqrt((0.99423 * 0.99423) + 1.14)) / 0.2;
    const double field = ceiling - 0.0577;
    std::vector<double> zero(state.size(), 0.0);
    model.setState(0.0, state.data(), zero.data(), cLocalSolverMode);
    std::vector<double> residual(state.size(), 0.0);
    model.residual(inputs, emptyStateData, residual.data(), cLocalSolverMode);
    EXPECT_NEAR(residual[0], field - state[0], 1e-11);
    EXPECT_NEAR(residual[4], 0.0, 1e-12);
    checkModelJacobian(model, inputs, state);
    model.setRootOffset(0, cLocalSolverMode);
    std::array<double, 4> roots{};
    model.rootTest(inputs, emptyStateData, roots.data(), cLocalSolverMode);
    EXPECT_LT(roots[0], 0.0);
    model.rootTrigger(0.0, inputs, {1, 0, 0, 0}, cLocalSolverMode);
    EXPECT_NEAR(model.getStates()[4], ceiling, 1e-11);
    state = model.getStates();
    state[3] = -1.0;
    model.setState(0.0, state.data(), zero.data(), cLocalSolverMode);
    model.derivative(inputs, emptyStateData, zero.data(), cLocalSolverMode);
    EXPECT_LT(zero[4], 0.0);  // Release into the permitted range.
}

TEST(Esac2aTests, ZeroTimeBypassesAndRegulatorLimitsHaveAnalyticJacobians)
{
    for (const auto& [transducerTime, leadLagTime, amplifierTime] :
         {std::tuple{0.0, 0.0, 0.0}, {0.0, 0.0, 0.2}, {0.1, 0.3, 0.0}}) {
        exciters::ExciterESAC2A model;
        configureExciter(model);
        model.set("tr", transducerTime);
        model.set("tb", leadLagTime);
        model.set("tc", leadLagTime == 0.0 ? 0.0 : 0.15);
        model.set("ta", amplifierTime);
        model.dynInitializeA(0.0, 0);
        auto inputs = exciterInputs();
        IOdata fieldSet;
        model.dynInitializeB(inputs, {1.0}, fieldSet);
        checkModelJacobian(model, inputs, model.getStates());
        inputs[exciterVsetInLocation] += 10.0;
        auto state = model.getStates();
        if (amplifierTime > 0.0) {
            state[1] = 21.0;
        }
        checkModelJacobian(model, inputs, state);
    }
}

TEST(Esac2aTests, OptionalUnderAndOverSelectorsUseTheirOwnBranches)
{
    exciters::ExciterESAC2A model;
    configureExciter(model);
    model.dynInitializeA(0.0, 0);
    auto inputs = exciterInputs();
    IOdata fieldSet;
    model.dynInitializeB(inputs, {1.0}, fieldSet);
    model.set("uel_enabled", 1.0);
    model.set("oel_enabled", 1.0);
    EXPECT_TRUE(model.supportsLimiterSignal(ExciterLimiterSignal::UNDER));
    EXPECT_TRUE(model.supportsLimiterSignal(ExciterLimiterSignal::OVER));
    inputs[exciterVuelInLocation] = 3.0;
    inputs[exciterVoelInLocation] = 20.0;
    checkModelJacobian(model, inputs, model.getStates());
    inputs[exciterVoelInLocation] = 0.5;
    checkModelJacobian(model, inputs, model.getStates());
}

TEST(Esac2aTests, AllRectifierSegmentsAndLowerFieldLimitHaveJacobians)
{
    for (const double normalizedCurrent : {0.2, 0.6, 0.85, 1.2}) {
        exciters::ExciterESAC2A model;
        configureExciter(model);
        model.set("vfemax", 100.0);
        model.dynInitializeA(0.0, 0);
        auto inputs = exciterInputs();
        IOdata fieldSet;
        model.dynInitializeB(inputs, {1.0}, fieldSet);
        auto state = model.getStates();
        state[4] = 1.2;
        inputs[exciterXadIfdInLocation] = normalizedCurrent * 1.2 / 0.2;
        checkModelJacobian(model, inputs, state);
    }
    exciters::ExciterESAC2A model;
    configureExciter(model);
    model.dynInitializeA(0.0, 0);
    const auto inputs = exciterInputs();
    IOdata fieldSet;
    model.dynInitializeB(inputs, {1.0}, fieldSet);
    auto state = model.getStates();
    state[4] = -0.2;
    state[3] = -1.0;
    checkModelJacobian(model, inputs, state);
}

TEST(Esac2aTests, VoltageStepsEnterAndReleaseRegulatorAndFieldLimits)
{
    exciters::ExciterESAC2A model;
    configureExciter(model);
    for (const auto name : {"tr", "tb", "tc", "kf", "kc", "kd", "kh", "se1"}) {
        model.set(name, 0.0);
    }
    model.set("kb", 1.0);
    model.set("vamax", 3.0);
    model.set("vfemax", 2.0);
    model.dynInitializeA(0.0, 0);
    auto inputs = exciterInputs();
    IOdata fieldSet;
    model.dynInitializeB(inputs, {1.0}, fieldSet);
    inputs[exciterVsetInLocation] = 2.0;
    constexpr double timeStep = 0.001;
    for (int i = 1; i <= 2000; ++i) {
        model.timestep(i * timeStep, inputs, cLocalSolverMode);
    }
    // With TR/TB bypassed, the states are EFD, VA, VE and feedback.
    EXPECT_DOUBLE_EQ(model.getStates()[1], 3.0);
    EXPECT_DOUBLE_EQ(model.getStates()[2], 2.0);
    EXPECT_DOUBLE_EQ(model.getStates()[0], 2.0);
    std::vector<double> rates(model.getStates().size(), 0.0);
    model.derivative(inputs, emptyStateData, rates.data(), cLocalSolverMode);
    EXPECT_DOUBLE_EQ(rates[1], 0.0);
    EXPECT_DOUBLE_EQ(rates[2], 0.0);
    inputs[exciterVsetInLocation] = 1.0;
    for (int i = 2001; i <= 6000; ++i) {
        model.timestep(i * timeStep, inputs, cLocalSolverMode);
    }
    EXPECT_NEAR(model.getStates()[1], 1.0, 1e-7);
    EXPECT_NEAR(model.getStates()[2], 1.0, 0.001);
    checkModelJacobian(model, inputs, model.getStates());
}

TEST(NewControlModelTests, FactoryCloneAndInvalidParameters)
{
    auto factory = CoreObjectFactory::instance();
    std::unique_ptr<CoreObject> pss(factory->createObject("pss", "pss2b"));
    auto* stabilizer = dynamic_cast<stabilizers::StabilizerPss2b*>(pss.get());
    ASSERT_NE(stabilizer, nullptr);
    stabilizer->set("m", 2.0);
    stabilizer->set("n", 3.0);
    stabilizer->set("vs1max", 0.15);
    std::unique_ptr<CoreObject> clonedPss(stabilizer->clone());
    EXPECT_DOUBLE_EQ(clonedPss->get("m"), 2.0);
    EXPECT_DOUBLE_EQ(clonedPss->get("n"), 3.0);
    EXPECT_DOUBLE_EQ(clonedPss->get("vs1max"), 0.15);
    EXPECT_THROW(stabilizer->set("m", 1.5), InvalidParameterValue);
    EXPECT_THROW(stabilizer->set("busr1", 2.0), InvalidParameterValue);
    stabilizer->set("m", 8.0);
    EXPECT_THROW(stabilizer->dynInitializeA(0.0, 0), InvalidParameterValue);
    std::unique_ptr<CoreObject> exc(factory->createObject("exciter", "esac2a"));
    auto* exciter = dynamic_cast<exciters::ExciterESAC2A*>(exc.get());
    ASSERT_NE(exciter, nullptr);
    configureExciter(*exciter);
    std::unique_ptr<CoreObject> clonedExciter(exciter->clone());
    EXPECT_DOUBLE_EQ(clonedExciter->get("kb"), 2.0);
    EXPECT_DOUBLE_EQ(clonedExciter->get("kh"), 0.25);
    exciter->set("te", 0.0);
    EXPECT_THROW(exciter->dynInitializeA(0.0, 0), InvalidParameterValue);
}
