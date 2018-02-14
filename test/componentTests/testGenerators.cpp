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

#include "Generator.h"
#include "gridBus.h"
#include "fileInput.h"
#include "testHelper.h"
#include "utilities/timeSeriesMulti.hpp"
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#define GEN_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/gen_tests/"

BOOST_FIXTURE_TEST_SUITE(gen_tests, gridDynSimulationTestFixture)

using namespace griddyn;

// TODO convert to a BOOST_DATA_TEST
BOOST_AUTO_TEST_CASE (gen_test_remote)
{
    std::string fileName = std::string (GEN_TEST_DIRECTORY "test_gen_remote.xml");
    detailedStageCheck (fileName, gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}

BOOST_AUTO_TEST_CASE (gen_test_remoteb)
{
    std::string fileName = std::string (GEN_TEST_DIRECTORY "test_gen_remote_b.xml");
    detailedStageCheck (fileName, gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
}


BOOST_AUTO_TEST_CASE (gen_test_remote2)
{
    std::string fileName = std::string (GEN_TEST_DIRECTORY "test_gen_dualremote.xml");
    detailedStageCheck (fileName, gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}


BOOST_AUTO_TEST_CASE (gen_test_remote2b)
{
    std::string fileName = std::string (GEN_TEST_DIRECTORY "test_gen_dualremote_b.xml");
    detailedStageCheck (fileName, gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
}

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE (gen_test_isoc)
{
    std::string fileName = std::string (GEN_TEST_DIRECTORY "test_isoc2.xml");

    gds = readSimXMLFile (fileName);

    gds->set ("recorddirectory", GEN_TEST_DIRECTORY);

    gds->run ();

    std::string recname = std::string (GEN_TEST_DIRECTORY "datafile.dat");
    timeSeriesMulti<> ts3 (recname);
    BOOST_REQUIRE (ts3.size () > 30);
    BOOST_CHECK (ts3.data (0, 30) < 0.995);
    BOOST_CHECK (ts3[0].back () > 1.0);

    BOOST_CHECK ((ts3.data (1, 0) - ts3[1].back ()) > 0.199);
    remove (recname.c_str ());
}
#endif
BOOST_AUTO_TEST_SUITE_END ()