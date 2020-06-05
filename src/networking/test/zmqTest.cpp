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

#include "../test/testHelper.h"
#include "networking/dimeCollector.h"
#include "networking/zmqInterface.h"
#include "networking/zmqLibrary/zmqContextManager.h"
#include "networking/zmqLibrary/zmqHelper.h"
#include "networking/zmqLibrary/zmqReactor.h"
#include "networking/zmqLibrary/zmqSocketDescriptor.h"
#include <chrono>
#include <thread>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

static const std::string zmq_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/zmq_tests/");

BOOST_FIXTURE_TEST_SUITE(zmq_tests, gridDynSimulationTestFixture)

using namespace zmq;
using namespace zmqlib;
using namespace griddyn;

BOOST_AUTO_TEST_CASE(load_zmq_context_manager)
{
    auto testContextManager = zmqContextManager::getContextPointer("context1");
    BOOST_CHECK(testContextManager->getName() == "context1");
    auto defaultContext = zmqContextManager::getContextPointer();
    BOOST_CHECK(defaultContext->getName().empty());

    auto& alternativeContext = zmqContextManager::getContext("context1");
    // make sure these two methods point at the same thing
    BOOST_CHECK(&alternativeContext == &(testContextManager->getBaseContext()));
}

BOOST_AUTO_TEST_CASE(test_socket_descriptor)
{
    zmqSocketDescriptor zDescriptor("test_socket");
    zDescriptor.addOperation(socket_ops::bind, "inproc://1");

    zDescriptor.type = socket_type::pub;

    zmqSocketDescriptor zDescriptor2("test_socketr");
    zDescriptor2.addOperation(socket_ops::connect, "inproc://1");
    zDescriptor2.addOperation(socket_ops::subscribe, "test1");
    zDescriptor2.type = socketTypeFromString("sub");

    auto& defContext = zmqContextManager::getContext();
    auto sock1 = zDescriptor.makeSocket(defContext);

    auto sock2 = zDescriptor2.makeSocket(defContext);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string mess1 = "test1:hello";
    sock1.send(mess1);

    message_t rxMessage;
    sock2.recv(rxMessage);

    BOOST_CHECK(rxMessage.size() == mess1.size());

    std::string mess2(static_cast<const char*>(rxMessage.data()), rxMessage.size());

    BOOST_CHECK(mess2 == mess1);
}

BOOST_AUTO_TEST_CASE(test_reactorA)
{
    int count = 0;  // this variable should be incremented by each message

    auto reactor = zmqReactor::getReactorInstance("reactor1");

    // build the transmission socket
    zmqSocketDescriptor zDescriptor("test_socket");
    zDescriptor.addOperation(socket_ops::bind, "inproc://1");

    zDescriptor.type = socket_type::pub;
    auto& defContext = zmqContextManager::getContext();
    auto sock1 = zDescriptor.makeSocket(defContext);

    // make the rx socket descriptor
    zmqSocketDescriptor zDescriptor2("test_socketr");
    zDescriptor2.addOperation(socket_ops::connect, "inproc://1");
    zDescriptor2.addOperation(socket_ops::subscribe, "test1");
    zDescriptor2.type = socketTypeFromString("sub");
    zDescriptor2.callback = [&count](const zmq::multipart_t&) { ++count; };
    // add the socket and pause a little while
    reactor->addSocketBlocking(zDescriptor2);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::string mess1 = "test1:hello";
    sock1.send(mess1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    BOOST_CHECK(count == 1);
    // send a message it shouldn't receive
    std::string mess2 = "test2:hello";
    sock1.send(mess2);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    BOOST_CHECK(count == 1);

    // make a modification so it can receive it
    zmqSocketDescriptor zDescriptorMod("test_socketr");
    zDescriptorMod.addOperation(socket_ops::subscribe, "test2");

    reactor->modifySocketBlocking(zDescriptorMod);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sock1.send(mess2);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    BOOST_CHECK(count == 2);
    // close the socket  and make sure it doesn't receive the message
    reactor->closeSocket("test_socketr");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sock1.send(mess2);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    BOOST_CHECK(count == 2);
}

BOOST_AUTO_TEST_CASE(test_reactorB)
{
    int count1 = 0;  // this variable should be incremented by each message
    int count2 = 0;
    auto reactor = zmqReactor::getReactorInstance("reactor1");

    // build the transmission socket
    zmqSocketDescriptor zDescriptor("test_socket");
    zDescriptor.addOperation(socket_ops::bind, "inproc://2");

    zDescriptor.type = socket_type::pub;
    auto& defContext = zmqContextManager::getContext();
    auto sock1 = zDescriptor.makeSocket(defContext);

    // make the rx socket descriptor
    zmqSocketDescriptor zDescriptor2("test_socketr1");
    zDescriptor2.addOperation(socket_ops::connect, "inproc://2");
    zDescriptor2.addOperation(socket_ops::subscribe, "test1");
    zDescriptor2.type = socketTypeFromString("sub");
    zDescriptor2.callback = [&count1](const zmq::multipart_t&) { ++count1; };
    reactor->addSocket(zDescriptor2);
    zmqSocketDescriptor zDescriptor3("test_socketr2");
    zDescriptor3.addOperation(socket_ops::connect, "inproc://2");
    zDescriptor3.addOperation(socket_ops::subscribe, "test2");
    zDescriptor3.type = socketTypeFromString("sub");
    zDescriptor3.callback = [&count2](const zmq::multipart_t&) { ++count2; };
    // add the socket and pause a little while
    reactor->addSocketBlocking(zDescriptor3);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::string mess1 = "test1:hello";
    sock1.send(mess1);
    // send a message it shouldn't receive
    std::string mess2 = "test2:hello";
    sock1.send(mess2);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    BOOST_CHECK(count1 == 1);
    BOOST_CHECK(count2 == 1);
    // make a modification so it can receive it
    zmqSocketDescriptor zDescriptorMod("test_socketr1");
    zDescriptorMod.addOperation(socket_ops::subscribe, "test3");

    reactor->modifySocket(zDescriptorMod);
    zDescriptorMod.name = "test_socketr2";
    reactor->modifySocketBlocking(zDescriptorMod);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::string mess3 = "test3:hello";
    sock1.send(mess3);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    BOOST_CHECK(count1 == 2);
    BOOST_CHECK(count2 == 2);
    // close the socket  and make sure it doesn't receive the message
    reactor->closeSocket("test_socketr1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sock1.send(mess3);
    sock1.send(mess2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    BOOST_CHECK_EQUAL(count1, 2);
    BOOST_CHECK_EQUAL(count2, 4);
}

/*
BOOST_AUTO_TEST_CASE(dime_collector_test)
{
    loadDimeLibrary();
    auto fileName = zmq_test_directory + "dime_test39bus.xml";
    gds = readSimXMLFile(fileName);

    auto v = gds->findCollector("dime1");
    auto dimec = std::dynamic_pointer_cast<dimeLib::dimeCollector>(v);
    BOOST_CHECK(dimec);
}
*/
BOOST_AUTO_TEST_SUITE_END()
