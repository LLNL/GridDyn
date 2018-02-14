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

// test cases for the simulation outputs

#include "fileInput.h"
#include "simulation/gridDynSimulationFileOps.h"
#include "testHelper.h"
#include "utilities/vectorOps.hpp"
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <cstdlib>

using namespace griddyn;
static std::string pFlow_test_directory = std::string (GRIDDYN_TEST_DIRECTORY "/pFlow_tests/");

BOOST_FIXTURE_TEST_SUITE (output_tests, gridDynSimulationTestFixture)


BOOST_AUTO_TEST_CASE (output_test1)
{
    std::string fileName = pFlow_test_directory + "test_powerflow3m9b2.xml";

    simpleStageCheck (fileName, gridSimulation::gridState_t::POWERFLOW_COMPLETE);
    savePowerFlowCdf (gds.get (), "testout.cdf");

    BOOST_REQUIRE (boost::filesystem::exists ("testout.cdf"));

    gds2 = std::make_unique<gridDynSimulation> ();
    loadFile (gds2.get (), "testout.cdf");
    gds2->powerflow ();

    std::vector<double> st1 = gds->getState (cPflowSolverMode);
    std::vector<double> st2 = gds2->getState (cPflowSolverMode);

    auto diff = countDiffs (st1, st2, 0.000001);
    BOOST_CHECK_EQUAL (diff, 0u);
    remove ("testout.cdf");
}


BOOST_AUTO_TEST_SUITE_END ()