/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// test cases for the simulation outputs

#include "../testHelper.h"
#include "gmlc/utilities/vectorOps.hpp"
#include "griddyn/simulation/gridDynSimulationFileOps.h"
#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>

using namespace griddyn;
using gmlc::utilities::countDiffs;

static std::string pFlow_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/pFlow_tests/");

BOOST_FIXTURE_TEST_SUITE(output_tests,
                         gridDynSimulationTestFixture,
                         *boost::unit_test::label("quick"))

BOOST_AUTO_TEST_CASE(output_test1)
{
    std::string fileName = pFlow_test_directory + "test_powerflow3m9b2.xml";

    simpleStageCheck(fileName, gridSimulation::gridState_t::POWERFLOW_COMPLETE);
    savePowerFlowCdf(gds.get(), "testout.cdf");

    BOOST_REQUIRE(boost::filesystem::exists("testout.cdf"));

    gds2 = std::make_unique<gridDynSimulation>();
    loadFile(gds2.get(), "testout.cdf");
    gds2->powerflow();

    std::vector<double> st1 = gds->getState(cPflowSolverMode);
    std::vector<double> st2 = gds2->getState(cPflowSolverMode);

    auto diff = countDiffs(st1, st2, 0.000001);
    BOOST_CHECK_EQUAL(diff, 0u);
    remove("testout.cdf");
}

BOOST_AUTO_TEST_SUITE_END()
