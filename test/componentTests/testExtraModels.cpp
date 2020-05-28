/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"
#include "gridDynLoader/libraryLoader.h"
#include "griddyn/measurement/collector.h"

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

#define EXTRAMODEL_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/extraModel_tests/"

BOOST_FIXTURE_TEST_SUITE(extramodel_tests, gridDynSimulationTestFixture)
using namespace griddyn;

BOOST_AUTO_TEST_CASE(test_thermaltx_txage, *boost::unit_test::label("quick"))
{
    std::string fileName = std::string(EXTRAMODEL_TEST_DIRECTORY "test_thermaltx_txage.xml");
    loadLibraries();
    runTestXML(fileName, gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    auto col = gds->findCollector("collector#0");
    BOOST_CHECK_EQUAL(col->numberOfPoints(), 14);

    BOOST_CHECK_EQUAL(col->getWarningCount(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
