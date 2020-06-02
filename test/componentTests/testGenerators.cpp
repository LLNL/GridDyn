/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"
#include "gmlc/utilities/TimeSeriesMulti.hpp"
#include "griddyn/Generator.h"
#include "griddyn/gridBus.h"

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

#define GEN_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/gen_tests/"

BOOST_FIXTURE_TEST_SUITE(gen_tests, gridDynSimulationTestFixture, *boost::unit_test::label("quick"))

using namespace griddyn;

// TODO convert to a BOOST_DATA_TEST
BOOST_AUTO_TEST_CASE(gen_test_remote)
{
    std::string fileName = std::string(GEN_TEST_DIRECTORY "test_gen_remote.xml");
    detailedStageCheck(fileName, gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}

BOOST_AUTO_TEST_CASE(gen_test_remoteb)
{
    std::string fileName = std::string(GEN_TEST_DIRECTORY "test_gen_remote_b.xml");
    detailedStageCheck(fileName, gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
}

BOOST_AUTO_TEST_CASE(gen_test_remote2)
{
    std::string fileName = std::string(GEN_TEST_DIRECTORY "test_gen_dualremote.xml");
    detailedStageCheck(fileName, gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}

BOOST_AUTO_TEST_CASE(gen_test_remote2b)
{
    std::string fileName = std::string(GEN_TEST_DIRECTORY "test_gen_dualremote_b.xml");
    detailedStageCheck(fileName, gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
}

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(gen_test_isoc)
{
    std::string fileName = std::string(GEN_TEST_DIRECTORY "test_isoc2.xml");

    gds = readSimXMLFile(fileName);

    gds->set("recorddirectory", GEN_TEST_DIRECTORY);

    gds->run();

    std::string recname = std::string(GEN_TEST_DIRECTORY "datafile.dat");
    TimeSeriesMulti<> ts3(recname);
    BOOST_REQUIRE(ts3.size() > 30);
    BOOST_CHECK(ts3.data(0, 30) < 0.995);
    BOOST_CHECK(ts3[0].back() > 1.0);

    BOOST_CHECK((ts3.data(1, 0) - ts3[1].back()) > 0.199);
    remove(recname.c_str());
}
#endif
BOOST_AUTO_TEST_SUITE_END()
