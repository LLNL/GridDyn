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
#include "relays/gridRelay.h"
#include "relays/zonalRelay.h"

#include "relays/controlRelay.h"
#include "comms/gridCommunicator.h"
#include "comms/controlMessage.h"

//#include <crtdbg.h>
//test case for link objects


#define RELAY_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/relay_tests/"

BOOST_FIXTURE_TEST_SUITE (relay_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (relay_test1)
{
  std::string fname = std::string (RELAY_TEST_DIRECTORY "relay_test1.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));

  gds->dynInitialize (0);

  zonalRelay *Yp = dynamic_cast<zonalRelay *> (gds->getRelay (0));
  BOOST_CHECK (Yp != nullptr);
}

BOOST_AUTO_TEST_CASE (relay_test2)
{
  //test a bunch of different link parameters to make sure all the solve properly
  std::string fname = std::string (RELAY_TEST_DIRECTORY "relay_test2.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));

  gds->dynInitialize (0);

  zonalRelay *Yp = dynamic_cast<zonalRelay *> (gds->getRelay (0));
  BOOST_CHECK (Yp != nullptr);
  Yp = dynamic_cast<zonalRelay *> (gds->getRelay (1));
  BOOST_CHECK (Yp != nullptr);

  auto obj = dynamic_cast<gridLink*> (gds->find ("bus2_to_bus3"));
  //BOOST_REQUIRE(obj != nullptr);
  gds->run ();
  BOOST_CHECK (obj->isConnected () == false);
  std::vector<double> v;
  gds->getVoltage (v);

  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}


BOOST_AUTO_TEST_CASE (relay_test_multi)
{
  //test a bunch of different link parameters to make sure all the solve properly
  std::string fname = std::string (RELAY_TEST_DIRECTORY "relay_test_multi.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));

  gds->dynInitialize (0);
  int cnt = gds->getInt("relaycount");
  
  BOOST_CHECK_EQUAL (cnt, 12);

  auto obj = dynamic_cast<gridLink*> (gds->find ("bus2_to_bus3"));
  //BOOST_REQUIRE(obj != nullptr);
  gds->run ();
  BOOST_CHECK (obj->isConnected () == false);
  BOOST_CHECK (obj->switchTest (1) == true);
  BOOST_CHECK (obj->switchTest (2) == true);
  std::vector<double> v;
  gds->getVoltage (v);

  auto ps = gds->currentProcessState();
  BOOST_REQUIRE((ps == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE) || (ps == gridDynSimulation::gridState_t::DYNAMIC_PARTIAL));

}


BOOST_AUTO_TEST_CASE(test_bus_relay)
{
  std::string fname = std::string(RELAY_TEST_DIRECTORY "test_bus_relay.xml");
  simpleRunTestXML(fname);

}

BOOST_AUTO_TEST_CASE(test_differential_relay)
{
  std::string fname = std::string(RELAY_TEST_DIRECTORY "test_differential_relay.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));
  gds->consolePrintLevel = 3;
  gds->run();
  auto obj = gds->find("bus1_to_bus3");
  BOOST_CHECK_EQUAL(static_cast<gridObject *>(obj)->isConnected(),false);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}



BOOST_AUTO_TEST_CASE(test_control_relay)
{
	std::string fname = std::string(RELAY_TEST_DIRECTORY "test_control_relay.xml");
	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));
	//gds->consolePrintLevel = 0;
	auto obj = gds->find("bus4::load4");
	auto cr = dynamic_cast<controlRelay *>(gds->getRelay(0));
	BOOST_REQUIRE((cr != nullptr));
	gds->dynInitialize();

	auto comm = makeCommunicator("","control",0);

	auto cm = std::make_shared<controlMessage>(controlMessage::SET);
	cm->m_field = "P";
	cm->m_value = 1.3;

	int ret=comm->transmit("cld4", cm);
	BOOST_REQUIRE(ret == 0);
	BOOST_CHECK(comm->queuedMessages());
	std::uint64_t src;
	auto rep = std::dynamic_pointer_cast<controlMessage>(comm->getMessage(src));
	BOOST_REQUIRE((rep));
	BOOST_CHECK(rep->getMessageType() == controlMessage::SET_SUCCESS);
	auto ldr = obj->get("p");
	BOOST_CHECK_CLOSE(ldr, 1.3, 0.0001);
	//send a get request
	cm->setMessageType(controlMessage::GET);
	cm->m_field = "q";

	comm->transmit("cld4", cm);
	rep = std::dynamic_pointer_cast<controlMessage>(comm->getMessage(src));
	BOOST_CHECK((rep));
	BOOST_CHECK(rep->getMessageType() == controlMessage::GET_RESULT);
	BOOST_CHECK_CLOSE(rep->m_value, 0.126, 0.001);



	

}

BOOST_AUTO_TEST_CASE(test_relay_comms)
{
	std::string fname = std::string(RELAY_TEST_DIRECTORY "test_relay_comms.xml");
	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));
	//gds->consolePrintLevel = 0;
	gds->dynInitialize();
	auto obj = gds->find("sensor1");
	BOOST_REQUIRE(obj != nullptr);
	double val = obj->get("current1");
	BOOST_CHECK(val != kNullVal);
	val = obj->get("current2");
	BOOST_CHECK(val != kNullVal);
	val = obj->get("voltage");
	BOOST_CHECK(val != kNullVal);
	val = obj->get("angle");
	BOOST_CHECK(val != kNullVal);

}
BOOST_AUTO_TEST_SUITE_END()