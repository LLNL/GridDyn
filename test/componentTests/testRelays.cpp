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

#include "../testHelper.h"
#include "griddyn/comms/Communicator.h"
#include "griddyn/comms/controlMessage.h"
#include "griddyn/gridBus.h"
#include "griddyn/relays/controlRelay.h"
#include "griddyn/relays/pmu.h"
#include "griddyn/relays/zonalRelay.h"

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

//#include <crtdbg.h>
// test case for link objects

#define RELAY_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/relay_tests/"

BOOST_FIXTURE_TEST_SUITE(relay_tests,
                         gridDynSimulationTestFixture,
                         *boost::unit_test::label("quick"))
using namespace griddyn;

BOOST_AUTO_TEST_CASE(relay_test1)
{
    std::string fileName = std::string(RELAY_TEST_DIRECTORY "relay_test1.xml");

    gds = readSimXMLFile(fileName);

    gds->dynInitialize(timeZero);

    auto Yp = dynamic_cast<relays::zonalRelay*>(gds->getRelay(0));
    BOOST_CHECK(Yp != nullptr);
}

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(relay_test2)
{
    // test a bunch of different link parameters to make sure all the solve properly
    std::string fileName = std::string(RELAY_TEST_DIRECTORY "relay_test2.xml");

    gds = readSimXMLFile(fileName);

    gds->dynInitialize(timeZero);

    zonalRelay* Yp = dynamic_cast<zonalRelay*>(gds->getRelay(0));
    BOOST_CHECK(Yp != nullptr);
    Yp = dynamic_cast<zonalRelay*>(gds->getRelay(1));
    BOOST_CHECK(Yp != nullptr);

    auto obj = dynamic_cast<Link*>(gds->find("bus2_to_bus3"));
    // BOOST_REQUIRE(obj != nullptr);
    gds->run();
    BOOST_CHECK(obj->isConnected() == false);
    std::vector<double> v;
    gds->getVoltage(v);

    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(relay_test_multi)
{
    // test a bunch of different link parameters to make sure all the solve properly
    std::string fileName = std::string(RELAY_TEST_DIRECTORY "relay_test_multi.xml");

    gds = readSimXMLFile(fileName);

    gds->dynInitialize(timeZero);
    int cnt = gds->getInt("relaycount");

    BOOST_CHECK_EQUAL(cnt, 12);

    auto obj = dynamic_cast<Link*>(gds->find("bus2_to_bus3"));
    // BOOST_REQUIRE(obj != nullptr);
    gds->run();
    BOOST_CHECK(obj->isConnected() == false);
    BOOST_CHECK(obj->switchTest(1) == true);
    BOOST_CHECK(obj->switchTest(2) == true);
    std::vector<double> v;
    gds->getVoltage(v);

    auto ps = gds->currentProcessState();
    BOOST_REQUIRE((ps == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE) ||
                  (ps == gridDynSimulation::gridState_t::DYNAMIC_PARTIAL));
}
#endif

BOOST_AUTO_TEST_CASE(test_bus_relay)
{
    std::string fileName = std::string(RELAY_TEST_DIRECTORY "test_bus_relay.xml");
    simpleRunTestXML(fileName);
}

BOOST_AUTO_TEST_CASE(test_differential_relay)
{
    std::string fileName = std::string(RELAY_TEST_DIRECTORY "test_differential_relay.xml");
    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::summary;
    gds->run();
    auto obj = gds->find("bus1_to_bus3");
    BOOST_CHECK_EQUAL(static_cast<gridComponent*>(obj)->isConnected(), false);
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}

BOOST_AUTO_TEST_CASE(test_control_relay)
{
    std::string fileName = std::string(RELAY_TEST_DIRECTORY "test_control_relay.xml");
    gds = readSimXMLFile(fileName);
    // gds->consolePrintLevel = print_level::no_print;
    auto obj = gds->find("bus4::load4");
    auto cr = dynamic_cast<relays::controlRelay*>(gds->getRelay(0));
    BOOST_REQUIRE((cr != nullptr));
    gds->dynInitialize();

    auto comm = makeCommunicator("", "control", 0);
    comm->initialize();

    auto cm = std::make_shared<commMessage>(comms::controlMessagePayload::SET);
    auto data = cm->getPayload<comms::controlMessagePayload>();
    BOOST_REQUIRE(data != nullptr);
    data->m_field = "P";
    data->m_value = 1.3;

    comm->transmit("cld4", cm);

    BOOST_CHECK(comm->messagesAvailable());
    std::uint64_t src;
    auto rep = comm->getMessage(src);
    BOOST_REQUIRE((rep));
    BOOST_CHECK(rep->getMessageType() == comms::controlMessagePayload::SET_SUCCESS);
    auto ldr = obj->get("p");
    BOOST_CHECK_CLOSE(ldr, 1.3, 0.0001);
    // send a get request
    cm->setMessageType(comms::controlMessagePayload::GET);
    cm->getPayload<comms::controlMessagePayload>()->m_field = "q";

    comm->transmit("cld4", cm);
    rep = comm->getMessage(src);
    BOOST_CHECK((rep));
    BOOST_CHECK(rep->getMessageType() == comms::controlMessagePayload::GET_RESULT);
    BOOST_CHECK_CLOSE(rep->getPayload<comms::controlMessagePayload>()->m_value, 0.126, 0.001);
}

BOOST_AUTO_TEST_CASE(test_relay_comms)
{
    std::string fileName = std::string(RELAY_TEST_DIRECTORY "test_relay_comms.xml");
    gds = readSimXMLFile(fileName);
    // gds->consolePrintLevel = print_level::no_print;
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

BOOST_AUTO_TEST_CASE(pmu_test1)
{
    std::string fileName = std::string(RELAY_TEST_DIRECTORY "pmu_test1.xml");

    gds = readSimXMLFile(fileName);

    gds->dynInitialize(timeZero);

    auto pmu = dynamic_cast<relays::pmu*>(gds->getRelay(0));
    BOOST_CHECK(pmu != nullptr);

    auto bus3 = gds->getBus(2);
    BOOST_CHECK(isSameObject(bus3, pmu->find("target")));
    BOOST_CHECK_CLOSE(bus3->getVoltage(), pmu->getOutput(0), 0.0001);

    BOOST_CHECK_CLOSE(pmu->get("voltage"), pmu->getOutput(0), 0.0001);

    double val = pmu->get("voltage");
    double ang = pmu->get("angle");
    double freq = pmu->get("freq");
    gds->run(20);
    double val2 = pmu->get("voltage");
    double ang2 = pmu->get("angle");
    double freq2 = pmu->get("freq");
    double rocof2 = pmu->get("rocof");
    gds->run(40);
    double val3 = pmu->get("voltage");
    double ang3 = pmu->get("angle");
    double freq3 = pmu->get("freq");
    double rocof3 = pmu->get("rocof");

    BOOST_CHECK_NE(val, val2);
    BOOST_CHECK_NE(val2, val3);
    BOOST_CHECK_GT(freq2, freq);
    BOOST_CHECK_GT(freq3, freq2);
    BOOST_CHECK_NE(ang, ang3);
    BOOST_CHECK_NE(ang, ang2);

    BOOST_CHECK_GT(rocof2, 0.0);
    BOOST_CHECK_GT(rocof3, 0.0);
}
BOOST_AUTO_TEST_SUITE_END()
