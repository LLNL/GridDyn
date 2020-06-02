/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"
#include <cstdio>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>
using namespace griddyn;
// test case for coreObject object

#define CONSTRAINT_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/constraint_tests/"

BOOST_FIXTURE_TEST_SUITE(constraint_tests,
                         gridDynSimulationTestFixture,
                         *boost::unit_test::label("quick"))

BOOST_AUTO_TEST_CASE(constraint_test1)
{
    std::string fileName = std::string(CONSTRAINT_TEST_DIRECTORY "test_constSimple1.xml");
    gds = readSimXMLFile(fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);

    gds->consolePrintLevel = print_level::no_print;

    gds->powerflow();
    printf("completed power flow\n");
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    gds->run(30.0);
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}

BOOST_AUTO_TEST_SUITE_END()
