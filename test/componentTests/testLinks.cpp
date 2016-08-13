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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include "gridEvent.h"

#include "linkModels/acLine.h"
#include "gridBus.h"
#include "simulation/diagnostics.h"
#include "vectorOps.hpp"

//#include <crtdbg.h>
//test case for link objects


#define LINK_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/link_tests/"

BOOST_FIXTURE_TEST_SUITE (link_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE(link_test1_simple)
{
	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = std::string(LINK_TEST_DIRECTORY "link_test1.xml");

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	BOOST_CHECK_EQUAL(readerConfig::warnCount, 0);
	gds->powerflow();
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	std::vector<double> v;
	gds->getVoltage(v);
  
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a){return (a > 0.95); }));
 
}

BOOST_AUTO_TEST_CASE(link_test_switches)
{
	auto B1 = new gridBus();
	auto B2 = new gridBus();
	auto L1 = new acLine(0.005, 0.2);
	L1->updateBus(B1, 1);
	L1->updateBus(B2, 2);
	B2->set("angle", -0.2);
	L1->updateLocalCache();
	auto P1 = L1->getRealPower(1);
	auto Q1 = L1->getReactivePower(1);
	auto P2 = L1->getRealPower(2);
	auto Q2 = L1->getReactivePower(2);
	BOOST_CHECK((P1 > P2));
	BOOST_CHECK(std::abs(P1) > std::abs(P2));
	L1->set("fault", 0.5);
	L1->updateLocalCache();
	P1 = L1->getRealPower(1);
	Q1 = L1->getReactivePower(1);
	P2 = L1->getRealPower(2);
	Q2 = L1->getReactivePower(2);
	BOOST_CHECK(P1 > 0);
	BOOST_CHECK(P2 > 0);
	BOOST_CHECK(Q1 > 9.99);
	BOOST_CHECK(Q2 > 9.99);
	L1->set("switch1", 1);
	L1->updateLocalCache();
	P1 = L1->getRealPower(1);
	Q1 = L1->getReactivePower(1);
	P2 = L1->getRealPower(2);
	Q2 = L1->getReactivePower(2);
	BOOST_CHECK(P1 == 0);
	BOOST_CHECK(P2 > 0);
	BOOST_CHECK(Q1 == 0);
	BOOST_CHECK(Q2 > 9.99);
	L1->set("switch2", 1);
	L1->updateLocalCache();
	P1 = L1->getRealPower(1);
	Q1 = L1->getReactivePower(1);
	P2 = L1->getRealPower(2);
	Q2 = L1->getReactivePower(2);
	BOOST_CHECK(P1 == 0);
	BOOST_CHECK(P2 == 0);
	BOOST_CHECK(Q1 == 0);
	BOOST_CHECK(Q2 == 0);
	L1->set("fault", -1);
	L1->updateLocalCache();
	P1 = L1->getRealPower(1);
	Q1 = L1->getReactivePower(1);
	P2 = L1->getRealPower(2);
	Q2 = L1->getReactivePower(2);
	BOOST_CHECK(P1 == 0);
	BOOST_CHECK(P2 == 0);
	BOOST_CHECK(Q1 == 0);
	BOOST_CHECK(Q2 == 0);

}

BOOST_AUTO_TEST_CASE(link_test1_dynamic)
{
	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = std::string(LINK_TEST_DIRECTORY "link_test1.xml");

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel=GD_WARNING_PRINT;
	auto g1 = std::make_shared<gridEvent>();

	//this tests events as much as links here
	auto obj = gds->find("load5");
	g1->setTarget(obj, "p");
	g1->value = 1.35;

	auto g2 = g1->clone(); //fullcopy clone
	g2->value = 1.25;

	g1->setTime(1.0);
	g2->setTime(3.4);

	gds->add(g1);
	gds->add(g2);
  gds->run(0.5);
  int mmatch=runJacobianCheck(gds,cDaeSolverMode);

  BOOST_REQUIRE_EQUAL(mmatch,0);
	gds->run(20);
  mmatch = runJacobianCheck(gds,cDaeSolverMode);
 
  BOOST_REQUIRE_EQUAL(mmatch, 0);
	std::vector<double> v;
	gds->getVoltage(v);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a){return (a > 0.95); }));

	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}

//test line fault in powerflow and power flow after line fault in recovery.  
BOOST_AUTO_TEST_CASE(link_test_fault_powerflow)
{
	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = std::string(LINK_TEST_DIRECTORY "link_test1.xml");

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
  gds->consolePrintLevel = GD_WARNING_PRINT;
	gds->powerflow();

	std::vector<double> v;
	gds->getVoltage(v);

	//this tests events as much as links here
	auto obj = gds->find("bus8_to_bus9");
	
  

	obj->set("fault", 0.5);
	gds->powerflow();
  int mmatch = runJacobianCheck(gds,cPflowSolverMode);
  BOOST_CHECK(mmatch == 0);
	std::vector<double> v2;
	gds->getVoltage(v2);

	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	obj->set("fault", -1.0);
	gds->powerflow();

	

	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	std::vector<double> v3;
	gds->getVoltage(v3);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a){return (a > 0.95); }));
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	auto mm = countDiffs(v3, v, 0.0001);
	
	BOOST_CHECK_EQUAL(mm, 0);

}

//test line fault in powerflow and power flow after line fault in recovery.  
BOOST_AUTO_TEST_CASE(link_test_fault_powerflow2)
{
	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = std::string(LINK_TEST_DIRECTORY "link_test1.xml");

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
  gds->consolePrintLevel = GD_WARNING_PRINT;
	gds->powerflow();
  
	std::vector<double> v;
	gds->getVoltage(v);

	//this tests events as much as links here
	auto obj = gds->find("bus2_to_bus3");

	obj->set("fault", 0.5);
	gds->powerflow();

	std::vector<double> v2;
	gds->getVoltage(v2);
	BOOST_CHECK(std::all_of(v2.begin(), v2.end(), [](double a){return (a >-1e-8); }));

	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	obj->set("fault", -1.0);
	gds->powerflow();



	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	std::vector<double> v3;
	gds->getVoltage(v3);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a){return (a > 0.95); }));
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	auto mm = countDiffs(v3, v, 0.0001);
	BOOST_CHECK_EQUAL(mm, 0);

}



BOOST_AUTO_TEST_CASE(link_test_fixPower)
{
  //test a bunch of different link parameters to make sure all the solve properly
 gridLink *a=new acLine();
 a->set("r",0.008);
 a->set("x",0.14);
 double v1=1.0;
 double a1=0;
 double v2=1.02;
 double a2=-0.12;
 gridBus *b1=new gridBus(v1,a1);
 b1->set("type","slk");
 gridBus *b2= new gridBus(v2,a2);
 a->updateBus(b1,1);
 a->updateBus(b2,2);

 
 

 a->updateLocalCache();
 double rP1=a->getRealPower(1);
 double qP1=a->getReactivePower(1);
 double rP2 = a->getRealPower(2);
 double qP2 = a->getReactivePower(2);

 b2->setVoltageAngle(v2,-0.18);
 a->fixPower(rP1,qP1,1,1);
 BOOST_CHECK_SMALL(std::abs(a2-b2->getAngle()),0.0001);

 b2->setVoltageAngle(1.05, a2);
 a->fixPower(rP1, qP1, 1, 1);
 BOOST_CHECK_SMALL(std::abs(v2 - b2->getVoltage()), 0.0001);


 b2->setVoltageAngle(v2, -0.18);
 a->fixPower(rP2, qP2, 2, 1);
 BOOST_CHECK_SMALL(std::abs(a2 - b2->getAngle()), 0.0001);

 b2->setVoltageAngle(1.05, a2);
 a->fixPower(rP2, qP2, 2, 1);
 BOOST_CHECK_SMALL(std::abs(v2 - b2->getVoltage()), 0.0001);

 b1->setVoltageAngle(1.05, a1);
 a->fixPower(rP1, qP1, 1, 2);
 BOOST_CHECK_SMALL(std::abs(v1 - b1->getVoltage()), 0.0001);

 b1->setVoltageAngle(v1, 0.02);
 a->fixPower(rP1, qP1, 1, 2);
 BOOST_CHECK_SMALL(std::abs(a1 - b1->getAngle()), 0.0001);

 b1->setVoltageAngle(1.05, a1);
 a->fixPower(rP2, qP2, 2, 2);
 BOOST_CHECK_SMALL(std::abs(v1 - b1->getVoltage()), 0.0001);

 b1->setVoltageAngle(v1, 0.02);
 a->fixPower(rP2, qP2,2, 2);
 BOOST_CHECK_SMALL(std::abs(a1 - b1->getAngle()), 0.0001);

 b1->setVoltageAngle(1.05, -0.07);
 a->fixPower(rP1, qP1, 1, 2);
 BOOST_CHECK_SMALL(std::abs(v1 - b1->getVoltage()), 0.0001);
 BOOST_CHECK_SMALL(std::abs(a1 - b1->getAngle()), 0.0001);

 delete a;
 delete b1;
 delete b2;

}
BOOST_AUTO_TEST_SUITE_END()
