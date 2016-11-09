/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

//test cases for the simulation outputs

#include "testHelper.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include "gridDynFileInput.h"
#include "simulation/gridDynSimulationFileOps.h"
#include "vectorOps.hpp"

static std::string pFlow_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/pFlow_tests/");

BOOST_FIXTURE_TEST_SUITE(output_tests, gridDynSimulationTestFixture)


BOOST_AUTO_TEST_CASE(output_test1)
{
	std::string fname = pFlow_test_directory + "test_powerflow3m9b2.xml";

	simpleStageCheck(fname, gridSimulation::gridState_t::POWERFLOW_COMPLETE);
	savePowerFlowCdf(gds, "testout.cdf");

	BOOST_REQUIRE(boost::filesystem::exists("testout.cdf"));

	gds2 = new gridDynSimulation();
	loadFile(gds2, "testout.cdf");
	gds2->powerflow();

	std::vector<double> st1 = gds->getState(cPflowSolverMode);
	std::vector<double> st2 = gds2->getState(cPflowSolverMode);
	
	auto diff = countDiffs(st1, st2, 0.000001);
	BOOST_CHECK_EQUAL(diff,0);
	remove("testout.cdf");
}


BOOST_AUTO_TEST_SUITE_END()