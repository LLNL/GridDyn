/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "gridDynSimulation.h"
#include "fileInput.h"
#include "testHelper.h"
#include <boost/filesystem.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>


/** these test cases test out the various generator components ability to handle faults
*/

static const std::string contingency_test_directory (GRIDDYN_TEST_DIRECTORY "/contingency_tests/");

BOOST_FIXTURE_TEST_SUITE (largeContingency_tests, gridDynSimulationTestFixture)

using namespace boost::filesystem;
using namespace griddyn;


// Testing N-2 contingencies
BOOST_AUTO_TEST_CASE (contingency_n2)
{
    std::string fileName = contingency_test_directory + "contingency_test3.xml";
    gds = readSimXMLFile (fileName);
    gds->set ("printlevel", 0);
    auto start_t = std::chrono::high_resolution_clock::now ();
    gds->run ();
    auto stop_t = std::chrono::high_resolution_clock::now ();
    BOOST_CHECK (exists ("contout_N2.csv"));
    remove ("contout_N2.csv");

    std::chrono::duration<double> load_time = (stop_t - start_t);
    printf ("contingencies run in %f seconds\n", load_time.count ());
}


BOOST_AUTO_TEST_CASE (contingency_bcase)
{
    std::string fileName = contingency_test_directory + "contingency_testbig.xml";
    gds = readSimXMLFile (fileName);
    gds->set ("printlevel", 0);
    auto start_t = std::chrono::high_resolution_clock::now ();
    int ret = gds->run ();
    auto stop_t = std::chrono::high_resolution_clock::now ();
    BOOST_CHECK (ret == FUNCTION_EXECUTION_SUCCESS);
    BOOST_CHECK (exists ("contout_N2.csv"));
    // remove("contout_N2.csv");

    std::chrono::duration<double> load_time = (stop_t - start_t);
    printf ("contingencies run in %f seconds\n", load_time.count ());
}


BOOST_AUTO_TEST_SUITE_END ()
