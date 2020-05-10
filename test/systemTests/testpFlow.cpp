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

#include "../testHelper.h"
#include "core/coreExceptions.h"
#include "gmlc/utilities/vectorOps.hpp"
#include "griddyn/solvers/solverInterface.h"
#include <cstdio>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

using namespace griddyn;
using gmlc::utilities::countDiffs;

static std::string pFlow_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/pFlow_tests/");

BOOST_FIXTURE_TEST_SUITE(pFlow_tests,
                         gridDynSimulationTestFixture,
                         *boost::unit_test::label("quick"))

/** test to make sure the basic power flow loads and runs*/
BOOST_AUTO_TEST_CASE(pFlow_test1)
{
    std::string fileName = pFlow_test_directory + "test_powerflow3m9b.xml";
    gds = readSimXMLFile(fileName);
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);

    gds->pFlowInitialize();
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::INITIALIZED);

    int count = gds->getInt("totalbuscount");

    BOOST_CHECK_EQUAL(count, 9);
    // check the linkcount
    count = gds->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, 9);

    BOOST_CHECK_EQUAL(runJacobianCheck(gds, cPflowSolverMode, false), 0);
    BOOST_CHECK_EQUAL(gds->getInt("jacsize"), 108);
    gds->powerflow();
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}

// testcase for power flow from initial start
BOOST_AUTO_TEST_CASE(pFlow_test2)
{
    std::string fileName = pFlow_test_directory + "test_powerflow3m9b2.xml";
    gds = readSimXMLFile(fileName);
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
    gds->pFlowInitialize();
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::INITIALIZED);

    std::vector<double> volts1;
    std::vector<double> ang1;
    std::vector<double> volts2;
    std::vector<double> ang2;

    gds->getVoltage(volts1);
    gds->getAngle(ang1);
    gds->powerflow();
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds->getVoltage(volts2);
    gds->getAngle(ang2);
    // ensure the sizes are equal
    BOOST_REQUIRE_EQUAL(volts1.size(), volts2.size());
    // check the bus voltages and angles
    auto vdiff = countDiffs(volts1, volts2, 0.0001);
    auto adiff = countDiffs(ang1, ang2, 0.0001);

    BOOST_CHECK(vdiff == 0);
    BOOST_CHECK(adiff == 0);
}

// testcase for power flow from zeros start
BOOST_AUTO_TEST_CASE(pFlow_test3)
{
    std::string fileName = pFlow_test_directory + "test_powerflow3m9b.xml";
    gds = readSimXMLFile(fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);
    gds->pFlowInitialize();
    requireState(gridDynSimulation::gridState_t::INITIALIZED);

    std::string fname2 = pFlow_test_directory + "test_powerflow3m9b2.xml";
    gds2 = readSimXMLFile(fname2);
    requireState2(gridDynSimulation::gridState_t::STARTUP);
    gds2->pFlowInitialize();
    requireState2(gridDynSimulation::gridState_t::INITIALIZED);

    std::vector<double> volts1;
    std::vector<double> ang1;
    std::vector<double> volts2;
    std::vector<double> ang2;

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds->getVoltage(volts1);
    gds->getAngle(ang1);

    gds2->powerflow();
    requireState2(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds2->getVoltage(volts2);
    gds2->getAngle(ang2);
    // ensure the sizes are equal
    BOOST_REQUIRE_EQUAL(volts1.size(), volts2.size());
    // check the bus voltages and angles
    auto vdiff = countDiffs(volts1, volts2, 0.0001);
    auto adiff = countDiffs(ang1, ang2, 0.0001);
    BOOST_CHECK(vdiff == 0);
    BOOST_CHECK(adiff == 0);
}

/** test the ieee 30 bus case with no shunts*/
BOOST_AUTO_TEST_CASE(pflow_test30_no_shunt)
{
    gds = std::make_unique<gridDynSimulation>();
    std::string fileName = ieee_test_directory + "ieee30_no_shunt_cap_tap_limit.cdf";

    loadCDF(gds.get(), fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);

    int count = gds->getInt("totalbuscount");
    BOOST_CHECK_EQUAL(count, 30);
    // check the linkcount
    count = gds->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, 41);
    std::vector<double> volts1;
    std::vector<double> ang1;
    std::vector<double> volts2;
    std::vector<double> ang2;

    gds->getVoltage(volts1);
    gds->getAngle(ang1);

    gds->pFlowInitialize();
    requireState(gridDynSimulation::gridState_t::INITIALIZED);

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    gds->getVoltage(volts2);
    gds->getAngle(ang2);

    auto vdiff = countDiffs(volts1, volts2, 0.001);
    auto adiff = countDiffs(ang1, ang2, 0.001);

    if ((vdiff > 0) || (adiff > 0)) {
        printBusResultDeviations(volts1, volts2, ang1, ang2);
    }
    BOOST_CHECK_EQUAL(vdiff, 0u);
    BOOST_CHECK_EQUAL(adiff, 0u);

    // check that the reset works correctly
    gds->reset(reset_levels::voltage_angle);
    gds->getAngle(ang1);
    for (size_t kk = 0; kk < ang1.size(); ++kk) {
        BOOST_CHECK_SMALL(ang1[kk], 0.000001);
    }

    gds->pFlowInitialize();
    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    gds->getVoltage(volts1);
    gds->getAngle(ang1);
    auto vdiff2 = countDiffs(volts1, volts2, 0.0005);
    auto adiff2 = countDiffs(volts1, volts2, 0.0009);

    BOOST_CHECK_EQUAL(vdiff2, 0u);
    BOOST_CHECK_EQUAL(adiff2, 0u);
}

/** test the IEEE 30 bus case with no reactive limits*/
BOOST_AUTO_TEST_CASE(pflow_test30_no_limit)
{
    gds = std::make_unique<gridDynSimulation>();
    std::string fileName = ieee_test_directory + "ieee30_no_limit.cdf";

    loadCDF(gds.get(), fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);

    int count = gds->getInt("totalbuscount");
    BOOST_CHECK_EQUAL(count, 30);
    // check the linkcount
    count = gds->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, 41);
    std::vector<double> volts1;
    std::vector<double> ang1;
    std::vector<double> volts2;
    std::vector<double> ang2;

    gds->getVoltage(volts1);
    gds->getAngle(ang1);

    gds->pFlowInitialize();
    requireState(gridDynSimulation::gridState_t::INITIALIZED);

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    gds->getVoltage(volts2);
    gds->getAngle(ang2);

    auto vdiff = countDiffs(volts1, volts2, 0.001);
    auto adiff = countDiffs(ang1, ang2, 0.01 * kPI / 180.0);  // 0.01 degrees
    if ((vdiff > 0) || (adiff > 0)) {
        printBusResultDeviations(volts1, volts2, ang1, ang2);
    }
    BOOST_CHECK_EQUAL(vdiff, 0u);
    BOOST_CHECK_EQUAL(adiff, 0u);

    // check that the reset works correctly
    gds->reset(reset_levels::voltage_angle);
    gds->getAngle(ang1);
    for (size_t kk = 0; kk < ang1.size(); ++kk) {
        BOOST_CHECK_SMALL(ang1[kk], 0.000001);
    }
    gds->pFlowInitialize();
    gds->powerflow();

    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    gds->getVoltage(volts1);
    gds->getAngle(ang1);
    auto vdiff2 = countDiffs(volts1, volts2, 0.0005);
    auto adiff2 = countDiffs(volts1, volts2, 0.0009);

    BOOST_CHECK_EQUAL(vdiff2, 0u);
    BOOST_CHECK_EQUAL(adiff2, 0u);
}

// test case for power flow automatic adjustment
BOOST_AUTO_TEST_CASE(test_pFlow_padjust)
{
    std::string fileName = pFlow_test_directory + "test_powerflow3m9b_Padjust.xml";
    gds = readSimXMLFile(fileName);
    BOOST_REQUIRE(gds != nullptr);
    requireState(gridDynSimulation::gridState_t::STARTUP);

    std::vector<double> P1;
    std::vector<double> P2;

    gds->pFlowInitialize();
    gds->updateLocalCache();
    gds->getBusGenerationReal(P1);
    gds->powerflow();
    auto wc = gds->getInt("warncount");
    BOOST_CHECK_EQUAL(wc, 0);
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds->getBusGenerationReal(P2);
    // there should be 2 generators+ the swing bus that had their real power levels adjusted instead of just the swing bus
    auto cnt = countDiffs(P2, P1, 0.0002);
    BOOST_CHECK_EQUAL(cnt, 3u);
}

/** test case for dc power flow*/
BOOST_AUTO_TEST_CASE(pflow_test_dcflow)
{
    gds = std::make_unique<gridDynSimulation>();
    std::string fileName = ieee_test_directory + "ieee30_no_limit.cdf";

    loadCDF(gds.get(), fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);

    int count = gds->getInt("totalbuscount");
    BOOST_CHECK_EQUAL(count, 30);
    // check the linkcount
    count = gds->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, 41);
    std::vector<double> volts1;
    std::vector<double> ang1;
    std::vector<double> volts2;
    std::vector<double> ang2;

    gds->getVoltage(volts1);
    gds->getAngle(ang1);

    auto ns = makeSolver("kinsol");
    ns->setName("dcflow");
    ns->set("mode", "dc, algebraic");
    gds->add(std::shared_ptr<SolverInterface>(std::move(ns)));

    auto smode = gds->getSolverMode("dcflow");
    gds->set("defpowerflow", "dcflow");
    gds->pFlowInitialize(0.0);
    requireState(gridDynSimulation::gridState_t::INITIALIZED);

    runJacobianCheck(gds, smode);

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}

// iterated power flow test case
BOOST_AUTO_TEST_CASE(test_iterated_pflow)
{
    std::string fileName = pFlow_test_directory + "iterated_test_case.xml";
    gds = readSimXMLFile(fileName);
    BOOST_REQUIRE(gds != nullptr);
    requireState(gridDynSimulation::gridState_t::STARTUP);
    gds->consolePrintLevel = print_level::no_print;
    gds->set("recorddirectory", pFlow_test_directory);
    gds->run();
    BOOST_REQUIRE_GT(gds->getSimulationTime(), 575.0);
}

/** test case for a floating bus ie a bus off a line with no load*/
BOOST_AUTO_TEST_CASE(pFlow_test_floating_bus)
{
    std::string fileName = pFlow_test_directory + "test_powerflow3m9b_float.xml";
    gds = readSimXMLFile(fileName);

    gds->pFlowInitialize();
    requireState(gridDynSimulation::gridState_t::INITIALIZED);

    runJacobianCheck(gds, cPflowSolverMode);

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}

/** test case for a single line breaker trip*/
BOOST_AUTO_TEST_CASE(pflow_test_single_breaker)
{
    std::string fileName = pFlow_test_directory + "line_single_breaker_trip.xml";
    gds = readSimXMLFile(fileName);

    gds->run(5.0);
    BOOST_CHECK_GE(static_cast<double>(gds->getSimulationTime()), 5.0);
    //gds->timestep(2.05,noInputs,cPflowSolverMode);
    //runJacobianCheck(gds, cPflowSolverMode);
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}
static stringVec approx_modes{
    "normal",
    "simple",
    "decoupled",
    "fast_decoupled",
    "simplified_decoupled",
    "small_angle",
    "small_angle_decoupled",
    "small_angle_simplified",
    "linear",
};

namespace data = boost::unit_test::data;

BOOST_DATA_TEST_CASE_F(gridDynSimulationTestFixture,
                       pflow_test_line_modes,
                       data::make(approx_modes),
                       approx)
{
    gds = std::make_unique<gridDynSimulation>();
    std::string fileName = ieee_test_directory + "ieee30_no_limit.cdf";

    loadCDF(gds.get(), fileName);

    requireState(gridDynSimulation::gridState_t::STARTUP);

    int count = gds->getInt("totalbuscount");
    BOOST_CHECK_EQUAL(count, 30);
    // check the linkcount
    count = gds->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, 41);
    std::vector<double> volts1;
    std::vector<double> ang1;
    std::vector<double> volts2;
    std::vector<double> ang2;

    gds->getVoltage(volts1);
    gds->getAngle(ang1);

    auto ns = makeSolver("kinsol");
    ns->setName(approx);
    try {
        ns->set("approx", approx);
    }
    catch (const invalidParameterValue&) {
        BOOST_CHECK_MESSAGE(false, "unrecognized approx mode " << approx);
    }

    gds->add(std::shared_ptr<SolverInterface>(std::move(ns)));
    auto smode = gds->getSolverMode(approx);
    gds->set("defpowerflow", approx);
    gds->pFlowInitialize(0.0);
    requireState(gridDynSimulation::gridState_t::INITIALIZED);
    int errors;
    if (approx == "small_angle_decoupled") {
        /*there is an initialization difference in this approximation due to the combination of decoupling
        and the small angle approximation.  In decoupled modes the values from the voltages and angles are fixed
        for the calculation of the decoupled quantities but are saved with the full trig calculation.  in the small
        angle
        approximation this is not done so there is a small error when evaluating the Jacobian right after the power
        flow initialization
        since the approximation has not yet been made.  All other modes and times do not have this issue*/
        errors = runJacobianCheck(gds, smode, 0.05, false);
    } else {
        errors = runJacobianCheck(gds, smode, false);
    }
    BOOST_REQUIRE_MESSAGE(errors == 0, "Errors in " << approx << " mode");

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}

BOOST_AUTO_TEST_SUITE_END()
