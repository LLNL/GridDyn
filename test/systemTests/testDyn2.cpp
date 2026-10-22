/*
 * Copyright (c) 2014-2026, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../gtestHelper.h"
#include "gmlc/utilities/vectorOps.hpp"
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
// test case for CoreObject object

#define DYN2_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/dyn_tests2/"

using namespace griddyn;

class DynamicSystemTests2: public GridDynSimulationTestFixture, public ::testing::Test {};

TEST_F(DynamicSystemTests2, DynTestSimpleEvent)
{
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_2m4bDyn.xml");

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = PrintLevel::WARNING;
    gds->powerflow();
    ASSERT_EQ(gds->currentProcessState(), GridDynSimulation::GridState::POWERFLOW_COMPLETE);

    int retval = gds->dynInitialize();
    EXPECT_EQ(retval, 0);
    ASSERT_EQ(gds->currentProcessState(), GridDynSimulation::GridState::DYNAMIC_INITIALIZED);

    std::vector<double> initialState = gds->getState();

    EXPECT_EQ(initialState.size(), 30U);

    gds->run();
    ASSERT_EQ(gds->currentProcessState(), GridDynSimulation::GridState::DYNAMIC_COMPLETE);
    std::vector<double> finalState = gds->getState();

    auto diff = gmlc::utilities::countDiffsIgnoreCommon(initialState, finalState, 0.02);
    // check for stability
    EXPECT_EQ(diff, 0U);
}

TEST_F(DynamicSystemTests2, DynTestSimpleChunked)
{
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_2m4bDyn.xml");
    simpleRunTestXML(fileName);
    std::vector<double> initialState = gds->getState();

    fileName = std::string(DYN2_TEST_DIRECTORY "test_2m4bDyn.xml");
    gds2 = readSimXMLFile(fileName);
    gds2->consolePrintLevel = PrintLevel::WARNING;
    gds2->run(1.5);
    gds2->run(3.7);
    gds2->run(7.65896);
    gds2->run();
    ASSERT_EQ(gds2->currentProcessState(), GridDynSimulation::GridState::DYNAMIC_COMPLETE);
    std::vector<double> chunkedState = gds2->getState();

    auto diff = gmlc::utilities::countDiffsIgnoreCommon(initialState, chunkedState, 0.0001);

    EXPECT_EQ(diff, 0);
}

TEST_F(DynamicSystemTests2, DynTestRandomLoadChange)
{
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_randLoadChange.xml");
    simpleRunTestXML(fileName);
}

TEST_F(DynamicSystemTests2, DynTestRampLoadChange)
{
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_rampLoadChange.xml");
    simpleRunTestXML(fileName);
}

TEST_F(DynamicSystemTests2, DynTestPulseLoadChange1)
{
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_pulseLoadChange1.xml");
    simpleRunTestXML(fileName);
}

TEST_F(DynamicSystemTests2, DynTestPulseLoadChange2)
{
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_pulseLoadChange2.xml");
    simpleRunTestXML(fileName);
}

#ifdef GRIDDYN_ENABLE_CVODE
TEST_F(DynamicSystemTests2, DynTestSinLoadChangePartCvode)
{  // using cvode
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_sineLoad_partitioned1.xml");
    simpleRunTestXML(fileName);
}

#endif
TEST_F(DynamicSystemTests2, DynTestSinLoadChangePartBasicOde)
{  // using basicode
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_sineLoad_partitioned2.xml");
    simpleRunTestXML(fileName);
}

#ifdef GRIDDYN_ENABLE_ARKODE
TEST_F(DynamicSystemTests2, DynTestSinLoadChangePartArkode)
{  // using arkode
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_sineLoad_partitioned3.xml");
    simpleRunTestXML(fileName);
}

#endif

TEST_F(DynamicSystemTests2, DynTestCompareOde)
{
    std::vector<std::string_view> solvers{"basicode"};
#ifdef GRIDDYN_ENABLE_CVODE
    solvers.emplace_back("cvode");
#endif
#ifdef GRIDDYN_ENABLE_ARKODE
    solvers.emplace_back("arkode");
#endif
    if (solvers.size() < 2) {
        GTEST_SKIP() << "Solver comparison requires at least two available ODE solvers";
    }

    std::vector<double> referenceState;
    for (const auto solver : solvers) {
        SCOPED_TRACE(solver);
        gds = readSimXMLFile(std::string(DYN2_TEST_DIRECTORY "test_2m4bDyn.xml"));
        gds->consolePrintLevel = PrintLevel::NO_PRINT;
        gds->set("dynamicsolvermethod", "partitioned");
        gds->set("defdyndiff", solver);
        gds->set("timestep", 0.001);
        gds->set("stoptime", 0.9);

        ASSERT_EQ(gds->run(), 0);
        ASSERT_EQ(gds->currentProcessState(), GridDynSimulation::GridState::DYNAMIC_COMPLETE);
        EXPECT_NEAR(static_cast<double>(gds->getSimulationTime()), 0.9, 1e-8);

        const auto state = gds->getState(gds->getSolverMode("differential"));
        ASSERT_FALSE(state.empty());
        ASSERT_TRUE(std::all_of(state.begin(), state.end(), [](double value) {
            return std::isfinite(value);
        }));
        if (referenceState.empty()) {
            referenceState = state;
            continue;
        }

        ASSERT_EQ(state.size(), referenceState.size());
        for (std::size_t index = 0; index < state.size(); ++index) {
            EXPECT_NEAR(state[index], referenceState[index], 5e-3)
                << "state index " << index << " differed from the basicode result";
        }
    }
}

#ifdef GRIDDYN_ENABLE_EXPERIMENTAL_TEST_CASES
TEST_F(DynamicSystemTests2, DynTestPulseLoadChangePart)
{
    std::string fileName = std::string(DYN2_TEST_DIRECTORY "test_pulseLoadChange1_partitioned.xml");
    simpleRunTestXML(fileName);
}
#endif
