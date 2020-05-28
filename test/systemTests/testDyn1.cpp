/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"
#include "gmlc/utilities/vectorOps.hpp"
#include "griddyn/gridBus.h"
#include "griddyn/primary/infiniteBus.h"
#include "griddyn/simulation/diagnostics.h"

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>
// test case for coreObject object

using namespace griddyn;
using namespace gmlc::utilities;

#define DYN1_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/dyn_tests1/"

BOOST_FIXTURE_TEST_SUITE(dyn_tests1,
                         gridDynSimulationTestFixture,
                         *boost::unit_test::label("quick"))

BOOST_AUTO_TEST_CASE(dyn_test_genModel)
{
    std::string fileName = std::string(DYN1_TEST_DIRECTORY "test_dynSimple1.xml");

    gds = readSimXMLFile(fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);

    gds->pFlowInitialize();
    requireState(gridDynSimulation::gridState_t::INITIALIZED);

    int count = gds->getInt("totalbuscount");
    BOOST_CHECK_EQUAL(count, 1);
    // check the linkcount
    count = gds->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, 0);

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    int retval = gds->dynInitialize();
    BOOST_CHECK_EQUAL(retval, 0);
    requireState(gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

    std::vector<double> st = gds->getState(cDaeSolverMode);

    BOOST_CHECK_EQUAL(st.size(), 8u);
    BOOST_CHECK_CLOSE(st[1], 1, 0.001);  // check the voltage
    BOOST_CHECK_CLOSE(st[0], 0, 0.001);  // check the angle

    BOOST_CHECK_CLOSE(st[5], 1.0, 0.001);  // check the rotational speed
    gds->run();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st2 = gds->getState(cDaeSolverMode);

    // check for stability
    BOOST_REQUIRE_EQUAL(st.size(), st2.size());
    auto diffs = countDiffs(st, st2, 0.0001);
    BOOST_CHECK_EQUAL(diffs, 0u);
}

BOOST_AUTO_TEST_CASE(dyn_test_Exciter)
{
    std::string fileName = std::string(DYN1_TEST_DIRECTORY "test_2m4bDyn_ss_ext_only.xml");
    gds = readSimXMLFile(fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);

    gds->pFlowInitialize();
    requireState(gridDynSimulation::gridState_t::INITIALIZED);

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    int retval = gds->dynInitialize();
    BOOST_CHECK_EQUAL(retval, 0);
    requireState(gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

    auto st = gds->getState(cDaeSolverMode);

    BOOST_CHECK_EQUAL(st.size(), 22u);
    if (st.size() != 22) {
        printStateNames(gds.get(), cDaeSolverMode);
    }

    runResidualCheck(gds, cDaeSolverMode);

    runJacobianCheck(gds, cDaeSolverMode);

    gds->run(0.25);
    runJacobianCheck(gds, cDaeSolverMode);

    gds->run();

    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    auto st2 = gds->getState(cDaeSolverMode);

    // check for stability
    auto diff = countDiffsIgnoreCommon(st, st2, 0.0001);
    BOOST_CHECK(diff == 0);
}

BOOST_AUTO_TEST_CASE(dyn_test_simpleCase)
{
    std::string fileName = std::string(DYN1_TEST_DIRECTORY "test_2m4bDyn_ss.xml");
    gds = readSimXMLFile(fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);

    gds->pFlowInitialize();
    requireState(gridDynSimulation::gridState_t::INITIALIZED);

    int count = gds->getInt("totalbuscount");

    BOOST_CHECK_EQUAL(count, 4);
    // check the linkcount
    count = gds->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, 5);

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    int retval = gds->dynInitialize();
    BOOST_CHECK_EQUAL(retval, 0);
    requireState(gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

    std::vector<double> st = gds->getState(cDaeSolverMode);

    BOOST_CHECK_EQUAL(st.size(), 30u);

    gds->run();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st2 = gds->getState(cDaeSolverMode);

    auto diff = countDiffsIgnoreCommon(st, st2, 0.0001);
    BOOST_CHECK(diff == 0);
}

BOOST_AUTO_TEST_CASE(dyn_test_infinite_bus)
{
    std::string fileName = std::string(DYN1_TEST_DIRECTORY "test_inf_bus.xml");
    gds = readSimXMLFile(fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);
    infiniteBus* bus = dynamic_cast<infiniteBus*>(gds->getBus(0));
    BOOST_CHECK(bus != nullptr);
    gds->pFlowInitialize();
    runJacobianCheck(gds, cPflowSolverMode);

    gds->powerflow();
    std::vector<double> st = gds->getState();

    gds->run();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st2 = gds->getState(cDaeSolverMode);

    BOOST_CHECK_CLOSE(st2[0], st[0], 0.001);
    BOOST_CHECK_CLOSE(st2[1], st[1], 0.001);
}

BOOST_AUTO_TEST_CASE(dyn_test_mbase)
{
    std::string fileName = std::string(DYN1_TEST_DIRECTORY "test_dynSimple1_mod.xml");
    detailedStageCheck(fileName, gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}

/*
BOOST_AUTO_TEST_CASE(dyn_test_griddyn39)
{
    std::string fileName = std::string(DYN1_TEST_DIRECTORY "test_griddyn39.xml");
    detailedStageCheck(fileName, gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
*/

BOOST_AUTO_TEST_SUITE_END()
