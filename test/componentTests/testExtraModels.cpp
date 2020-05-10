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
