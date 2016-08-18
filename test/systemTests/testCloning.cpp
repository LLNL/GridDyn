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


#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include <vectorOps.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <cstdio>
//test case for gridCoreObject object


static const std::string clone_test_directory= std::string(GRIDDYN_TEST_DIRECTORY "/clone_tests/");

BOOST_FIXTURE_TEST_SUITE(clone_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE(cloning_test1)
{

	std::string fname =clone_test_directory+ "clone_test1.xml";
	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = 0;

	gds2 = static_cast<gridDynSimulation *>(gds->clone());
	gds->powerflow();
	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	gds2->powerflow();
	BOOST_REQUIRE(gds2->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	std::vector<double> V1;
	std::vector<double> V2;
	gds->getVoltage(V1);
	gds2->getVoltage(V2);
	auto diffc = countDiffs(V1, V2, 0.0000001);
	BOOST_CHECK(diffc == 0);
}

BOOST_AUTO_TEST_CASE(cloning_test2)
{

	std::string fname = clone_test_directory + "clone_test2.xml";
	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = 0;

	gds2 = static_cast<gridDynSimulation *>(gds->clone());
	gds->powerflow();
	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	gds2->powerflow();
	BOOST_REQUIRE(gds2->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	std::vector<double> V1;
	std::vector<double> V2;
	gds->getVoltage(V1);
	gds2->getVoltage(V2);
	auto diffc = countDiffs(V1, V2, 0.0000001);
	BOOST_CHECK(diffc == 0);
}

/* clone test 3 has an approximation in the solver that should get cloned over to powerflow 2*/
BOOST_AUTO_TEST_CASE(cloning_test3)
{

	std::string fname = clone_test_directory + "clone_test3.xml";
	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = 0;

	gds2 = static_cast<gridDynSimulation *>(gds->clone());
	gds->powerflow();
	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	gds2->powerflow();
	BOOST_REQUIRE(gds2->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	std::vector<double> V1;
	std::vector<double> V2;
	gds->getVoltage(V1);
	gds2->getVoltage(V2);
	auto diffc = countDiffs(V1, V2, 0.0000001);
	BOOST_CHECK(diffc == 0);
}

BOOST_AUTO_TEST_SUITE_END()
