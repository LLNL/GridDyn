/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"
#include <chrono>

#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

/** these test cases test out the various generator components ability to handle faults
 */

static const std::string contingency_test_directory(GRIDDYN_TEST_DIRECTORY "/contingency_tests/");

BOOST_FIXTURE_TEST_SUITE(largeContingency_tests, gridDynSimulationTestFixture)

using namespace boost::filesystem;
using namespace griddyn;

// Testing N-2 contingencies
BOOST_AUTO_TEST_CASE(contingency_n2)
{
    std::string fileName = contingency_test_directory + "contingency_test3.xml";
    gds = readSimXMLFile(fileName);
    gds->set("printlevel", 0);
    auto start_t = std::chrono::high_resolution_clock::now();
    gds->run();
    auto stop_t = std::chrono::high_resolution_clock::now();
    BOOST_CHECK(exists("contout_N2.csv"));
    remove("contout_N2.csv");

    std::chrono::duration<double> load_time = (stop_t - start_t);
    printf("contingencies run in %f seconds\n", load_time.count());
}

BOOST_AUTO_TEST_CASE(contingency_bcase)
{
    std::string fileName = contingency_test_directory + "contingency_testbig.xml";
    gds = readSimXMLFile(fileName);
    gds->set("printlevel", 0);
    auto start_t = std::chrono::high_resolution_clock::now();
    int ret = gds->run();
    auto stop_t = std::chrono::high_resolution_clock::now();
    BOOST_CHECK(ret == FUNCTION_EXECUTION_SUCCESS);
    BOOST_CHECK(exists("contout_N2.csv"));
    // remove("contout_N2.csv");

    std::chrono::duration<double> load_time = (stop_t - start_t);
    printf("contingencies run in %f seconds\n", load_time.count());
}

BOOST_AUTO_TEST_SUITE_END()
