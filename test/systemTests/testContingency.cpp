/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"

#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

using namespace griddyn;
/** these test cases test out the contingency capabilities in GridDyn
 */

static const std::string contingency_test_directory(GRIDDYN_TEST_DIRECTORY "/contingency_tests/");

BOOST_FIXTURE_TEST_SUITE(contingency_tests,
                         gridDynSimulationTestFixture,
                         *boost::unit_test::label("quick"))

using namespace boost::filesystem;
BOOST_AUTO_TEST_CASE(contingency_test1)
{
    std::string fileName = contingency_test_directory + "contingency_test1.xml";
    gds = readSimXMLFile(fileName);
    gds->run();
    BOOST_CHECK(exists("contout.csv"));
    remove("contout.csv");
}

BOOST_AUTO_TEST_CASE(contingency_test2)
{
    std::string fileName = contingency_test_directory + "contingency_test2.xml";
    gds = readSimXMLFile(fileName);
    gds->set("printlevel", 0);
    gds->run();

    BOOST_CHECK(exists("contout_load.csv"));
    BOOST_CHECK(exists("contout_bus.csv"));
    BOOST_CHECK(exists("contout_gen.csv"));
    BOOST_CHECK(exists("contout_line.csv"));
    remove("contout_load.csv");
    remove("contout_bus.csv");
    remove("contout_gen.csv");
    remove("contout_line.csv");
}

// Testing N-2 contingencies  TODO:: move to testExtra
/*
BOOST_AUTO_TEST_CASE(contingency_test3)
{
    std::string fileName = contingency_test_directory + "contingency_test3.xml";
    gds = readSimXMLFile(fileName);
    gds->set("printlevel", 0);
    gds->run();

    BOOST_CHECK(exists("contout_N2.csv"));
    remove("contout_N2.csv");
}
*/

BOOST_AUTO_TEST_SUITE_END()
