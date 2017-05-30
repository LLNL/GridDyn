/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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


#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include "simulation/diagnostics.h"
#include "utilities/vectorOps.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <cstdio>
//test case for coreObject object


static const std::string clone_test_directory= std::string(GRIDDYN_TEST_DIRECTORY "/clone_tests/");

BOOST_FIXTURE_TEST_SUITE(clone_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE(cloning_test1)
{

	std::string fname =clone_test_directory+ "clone_test1.xml";
	gds = readSimXMLFile(fname);
	gds->consolePrintLevel = print_level::no_print;

	gds2 = std::unique_ptr<gridDynSimulation>(static_cast<gridDynSimulation *>(gds->clone()));
	gds->powerflow();
	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	gds2->powerflow();
	BOOST_REQUIRE(gds2->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	std::vector<double> V1;
	std::vector<double> V2;
	gds->getVoltage(V1);
	gds2->getVoltage(V2);
	auto diffc = countDiffs(V1, V2, 0.0000001);
	BOOST_CHECK_EQUAL(diffc, 0u);
}

BOOST_AUTO_TEST_CASE(cloning_test2)
{

	std::string fname = clone_test_directory + "clone_test2.xml";
	gds = readSimXMLFile(fname);
	gds->consolePrintLevel = print_level::no_print;

	gds2 = std::unique_ptr<gridDynSimulation>(static_cast<gridDynSimulation *>(gds->clone()));
	gds->powerflow();
	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	gds2->powerflow();
	BOOST_REQUIRE(gds2->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	std::vector<double> V1;
	std::vector<double> V2;
	gds->getVoltage(V1);
	gds2->getVoltage(V2);
	auto diffc = countDiffs(V1, V2, 0.0000001);
	BOOST_CHECK_EQUAL(diffc,0u);
}

/* clone test 3 has an approximation in the solver that should get cloned over to powerflow 2*/
BOOST_AUTO_TEST_CASE(cloning_test_solver_approx)
{

	std::string fname = clone_test_directory + "clone_test3.xml";
	gds = readSimXMLFile(fname);
	gds->consolePrintLevel = print_level::no_print;

	gds2 = std::unique_ptr<gridDynSimulation>(static_cast<gridDynSimulation *>(gds->clone()));
	gds->powerflow();
	BOOST_REQUIRE_EQUAL(gds->currentProcessState(),gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	BOOST_CHECK_EQUAL(gds2->currentProcessState(),gridDynSimulation::gridState_t::STARTUP);
	gds2->powerflow();
	BOOST_REQUIRE_EQUAL(gds2->currentProcessState(),gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	std::vector<double> V1;
	std::vector<double> V2;
	gds->getVoltage(V1);
	gds2->getVoltage(V2);
	auto diffc = countDiffs(V1, V2, 0.0000001);
	BOOST_CHECK_EQUAL(diffc,0u);

	auto obj = gds2->find("bus3::load3");

	auto obj2 = gds->find("bus3::load3");
	BOOST_CHECK_NE(obj->getID(),obj2->getID());
	obj->set("q+", 0.4);

	gds->powerflow();
	gds2->powerflow();

	gds2->getVoltage(V1);
	diffc = countDiffs(V1, V2, 0.0000001);
	BOOST_CHECK_GT(diffc,0u);

	gds->getVoltage(V2);
	diffc = countDiffs(V1, V2, 0.0000001);
	BOOST_CHECK_GT(diffc,0u);

	obj->set("q+", -0.3);
	//ensure there is no connection with the previous simulation object
	auto ret=gds2->powerflow();
	BOOST_CHECK(ret == FUNCTION_EXECUTION_SUCCESS);
}



BOOST_AUTO_TEST_CASE(cloning_test_events)
{
	std::string fname = clone_test_directory + "test_griddyn39_events.xml";
	gds = readSimXMLFile(fname);

	gds2 = std::unique_ptr<gridDynSimulation>(static_cast<gridDynSimulation *>(gds->clone()));

	BOOST_CHECK_EQUAL(gds->getInt("eventcount"), gds2->getInt("eventcount"));
	BOOST_CHECK_EQUAL(gds->getInt("relaycount"), gds2->getInt("relaycount"));

	gds->pFlowInitialize();
	gds2->pFlowInitialize();

	
	std::vector<double> v1;
	std::vector<double> v2;
	auto cnt1 = gds->getVoltage(v1);
	auto cnt2 = gds2->getVoltage(v2);
	BOOST_CHECK_EQUAL(cnt1, cnt2);

	auto diffc = countDiffs(v1, v2, 0.0000001);
	BOOST_CHECK_EQUAL(diffc, 0u);

	auto res = checkObjectEquivalence(gds.get(), gds2.get(), true);
	BOOST_REQUIRE(res);
	
	/** get the event objects and make sure they are independent*/
	std::vector<coreObject *> obj1;
	std::vector<coreObject *> obj2;
	gds->getEventObjects(obj1);
	gds2->getEventObjects(obj2);
	BOOST_REQUIRE_EQUAL(obj1.size(), obj2.size());
	for (size_t ii = 0; ii < obj1.size(); ++ii)
	{
		BOOST_CHECK_NE(obj1[ii]->getID(), obj2[ii]->getID());
	}
	// check the event timing
	BOOST_CHECK_EQUAL(gds->getEventTime(), gds2->getEventTime());
	//run the simulations
	gds2->run();
	gds->run();
	BOOST_CHECK_EQUAL(gds->currentProcessState(),gds2->currentProcessState());

	cnt1=gds->getVoltage(v1);
	cnt2=gds2->getVoltage(v2);
	BOOST_CHECK_EQUAL(cnt1, cnt2);

	diffc = countDiffs(v1, v2, 0.0000001);
	BOOST_CHECK_EQUAL(diffc, 0u);



}

BOOST_AUTO_TEST_SUITE_END()
