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

#include "../test/exeTestHelper.h"
#include "../test/testHelper.h"
#include "core/objectFactory.hpp"
#include "griddyn/Generator.h"
#include "griddyn/gridBus.h"
#include "helics/apps/BrokerApp.hpp"
#include "helics/apps/Player.hpp"
#include "helics/apps/Recorder.hpp"
#include "helics/helicsCoordinator.h"
#include "helics/helicsLibrary.h"
#include "helics/helicsLoad.h"
#include "helics/helicsRunner.h"
#include "helics/helicsSource.h"
#include "helics/helicsSupport.h"
#include "utilities/stringToCmdLine.h"
#include "utilities/string_viewOps.h"
#include <complex>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_FIXTURE_TEST_SUITE (helics_tests, gridDynSimulationTestFixture)
using namespace griddyn;
using namespace griddyn::helicsLib;

static const std::string helics_test_directory = std::string (GRIDDYN_TEST_DIRECTORY "/helics_tests/");

BOOST_AUTO_TEST_CASE (time_conversion_test)
{
    coreTime val = 4.5234235;
    auto helicsTime = gd2helicsTime (val);
    coreTime ret = helics2gdTime (helicsTime);

    BOOST_CHECK_SMALL (std::abs (static_cast<double> (val) - static_cast<double> (ret)), 0.0000001);

    val = 9.0;
    helicsTime = gd2helicsTime (val);
    ret = helics2gdTime (helicsTime);

    BOOST_CHECK_SMALL (std::abs (static_cast<double> (val) - static_cast<double> (ret)), 0.000000001);

    val = -2.0;
    helicsTime = gd2helicsTime (val);
    ret = helics2gdTime (helicsTime);

    BOOST_CHECK_SMALL (std::abs (static_cast<double> (val) - static_cast<double> (ret)), 0.000000001);
}

BOOST_AUTO_TEST_CASE (test_pub_sub_str)
{
    helics::FederateInfo fi;
    fi.coreType = helics::core_type::TEST;
    fi.coreInitString = "--autobroker";

    auto vFed = std::make_shared<helics::ValueFederate> ("string_test", fi);
    // register the publications
    auto &pubid = vFed->registerGlobalPublication<std::string> ("pub1");

    auto &subid = vFed->registerSubscription ("pub1");
    vFed->setProperty (helics::defs::properties::period, 1.0);
    vFed->enterExecutingMode ();
    // publish string1 at time=0.0;
    vFed->publish (pubid, "string1");
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    std::string s = subid.getValue<std::string> ();

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed->publish (pubid, "string2");
    // make sure the value is still what we expect
    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string2");
    vFed->finalize ();
}

BOOST_AUTO_TEST_CASE (test_pub_sub_double)
{
    helics::FederateInfo fi;
    fi.coreType = helics::core_type::TEST;
    fi.coreInitString = "--autobroker";

    auto vFed = std::make_shared<helics::ValueFederate> ("double_test", fi);
    // register the publications
    auto pubid = vFed->registerGlobalPublication<double> ("pub1");

    auto &subid = vFed->registerSubscription ("pub1");
    vFed->setProperty (helics::defs::properties::period, 1.0);
    vFed->enterExecutingMode ();
    // publish string1 at time=0.0;
    vFed->publish (pubid, 27.0);
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    double s = subid.getValue<double> ();

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, 27.0);
    // publish a second string
    vFed->publish (pubid, 23.234234);
    // make sure the value is still what we expect
    s = vFed->getDouble (subid);
    BOOST_CHECK_EQUAL (s, 27.0);

    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    subid.getValue (s);

    BOOST_CHECK_CLOSE (s, 23.234234, 0.00001);
    vFed->finalize ();
}

BOOST_AUTO_TEST_CASE (helics_coordinator_tests1)
{
    helicsCoordinator coord;
    auto ind1 = coord.addPublication ("pub1", helics::data_type::helics_double);
    auto ind2 = coord.addSubscription ("pub1");
    BOOST_CHECK_GE (ind1, 0);
    BOOST_CHECK_GE (ind2, 0);
    coord.set ("coretype", "test");
    coord.set ("init", "1");
    coord.set ("name", "coordtest");

    auto fed = coord.RegisterAsFederate ();

    // BOOST_CHECK (fed->currentState () == helics::Federate::op_states::startup);
    // fed->enterInitializationState ();

    BOOST_CHECK (fed->getCurrentMode () == helics::Federate::modes::initializing);
    fed->enterExecutingMode ();
    BOOST_CHECK (fed->getCurrentMode () == helics::Federate::modes::executing);

    coord.publish (ind1, 23.234);
    fed->requestTime (3.0);
    double val = coord.getValueAs<double> (ind2);

    BOOST_CHECK_CLOSE (val, 23.234, 0.0001);

    std::string v = coord.getValueAs<std::string> (ind2);
    BOOST_CHECK ((v.compare (0, 7, "23.2340") == 0) || (v.compare (0, 7, "23.2339") == 0));
}

BOOST_AUTO_TEST_CASE (load_helics_xml)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreInitString = "-f 2 --autobroker";

    auto vFed = std::make_shared<helics::ValueFederate> ("source_test", fi);
    // register the publications
    auto pubid = vFed->registerGlobalPublication<double> ("sourceValue");

    auto hR = new helicsRunner ();
    hR->InitializeFromString (helics_test_directory + "helics_test1.xml --core_type=test");

    coreObject *obj = coreObjectFactory::instance ()->createObject ("source", "helics", "helicsSource");

    BOOST_REQUIRE (dynamic_cast<helicsSource *> (obj) != nullptr);
    // as a note sources are source with respect to GridDyn not HELICS
    auto src = static_cast<helicsSource *> (obj);
    BOOST_REQUIRE (src != nullptr);
    src->set ("valkey", "sourceValue");
    src->set ("period", 2);
    src->set ("value", 0.4);
    auto sim = hR->getSim ();

    auto genObj = sim->find ("bus2::gen#0");
    auto gen = dynamic_cast<Generator *> (genObj);
    BOOST_REQUIRE (gen != nullptr);

    gen->add (src);

    auto res = std::async (std::launch::async, [&]() { hR->simInitialize (); });

    vFed->enterInitializingMode ();

    vFed->publish (pubid, 0.3);
    vFed->enterExecutingMode ();
    res.get ();

    auto resT = std::async (std::launch::async, [&]() { return hR->Step (3.0); });
    auto tm = vFed->requestTime (3.0);
    BOOST_CHECK_EQUAL (tm, 3.0);

    double tret = resT.get ();
    BOOST_CHECK_EQUAL (tret, 3.0);
    BOOST_CHECK_CLOSE (src->getOutput (), 0.3, 0.00001);
    vFed->publish (pubid, 0.5);

    resT = std::async (std::launch::async, [&]() { return hR->Step (5.0); });
    tm = vFed->requestTime (5.0);
    BOOST_CHECK_EQUAL (tm, 5.0);

    tret = resT.get ();
    BOOST_CHECK_EQUAL (tret, 5.0);
    BOOST_CHECK_CLOSE (src->getOutput (), 0.5, 0.00001);
    hR->Finalize ();
    vFed->finalize ();
    delete hR;
}

BOOST_AUTO_TEST_CASE (helics_xml_with_load)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "test2";
    fi.coreInitString = "-f 2 --autobroker";
    auto vFed = std::make_shared<helics::ValueFederate> ("helics_load_test", fi);
    // register the publications
    auto &subid = vFed->registerSubscription ("voltage3key");
    auto &pubid = vFed->registerGlobalPublication<std::complex<double>> ("load3val");

    auto hR = new helicsRunner ();
    hR->InitializeFromString (helics_test_directory + "helics_test2.xml --core_type=test --core_name=test2");

    auto sim = hR->getSim ();

    auto ldObj = sim->find ("bus2::load#0");
    auto ld = dynamic_cast<helicsLoad *> (ldObj);
    BOOST_REQUIRE (ld != nullptr);

    auto res = std::async (std::launch::async, [&]() { hR->simInitialize (); });

    vFed->enterInitializingMode ();

    vFed->enterExecutingMode ();
    pubid.publish (std::complex<double>{130.0, 40.0});

    res.get ();

    auto resT = std::async (std::launch::async, [&]() { return hR->Step (3.0); });
    auto tm = vFed->requestTime (3.0);
    while (tm < 3.0)
    {
        tm = vFed->requestTime (3.0);
    }
    BOOST_CHECK_EQUAL (tm, 3.0);

    double tret = resT.get ();
    BOOST_CHECK_EQUAL (tret, 3.0);
    auto voltageVal = subid.getValue<std::complex<double>> ();
    pubid.publish (std::complex<double>{150.0, 70.0});

    resT = std::async (std::launch::async, [&]() { return hR->Step (7.0); });
    tm = vFed->requestTime (7.0);
    while (tm < 7.0)
    {
        tm = vFed->requestTime (7.0);
    }
    BOOST_CHECK_EQUAL (tm, 7.0);

    tret = resT.get ();
    BOOST_CHECK_EQUAL (tret, 7.0);
    auto voltageVal2 = subid.getValue<std::complex<double>> ();

    BOOST_CHECK_GT (std::abs (voltageVal), std::abs (voltageVal2));
    hR->Finalize ();
    vFed->finalize ();
    delete hR;
}

BOOST_AUTO_TEST_CASE (test_recorder_player)
{
    auto brk = runBroker ("2");
    auto play = runPlayer (helics_test_directory + "source_player.txt --name=player --stop=25 2> playerout.txt");
    auto rec =
      runRecorder (helics_test_directory +
                   "recorder_capture_list.txt --name=rec --stop=25 --output=rec_capture.txt 2> recout.txt");

    BOOST_CHECK (play.get () == 0);
    BOOST_CHECK (rec.get () == 0);

    BOOST_CHECK (brk.get () == 0);
    BOOST_CHECK (boost::filesystem::exists ("rec_capture.txt"));

    std::ifstream inFile ("rec_capture.txt");
    std::string line;
    std::getline (inFile, line);
    std::getline (inFile, line);
    BOOST_CHECK (!line.empty ());
    using namespace utilities::string_viewOps;
    auto lineEle = split (line, whiteSpaceCharacters, delimiter_compression::on);
    BOOST_REQUIRE_GE (lineEle.size (), 3);
    BOOST_CHECK_EQUAL (lineEle[0], "3");
    BOOST_CHECK_EQUAL (lineEle[1], "gen");
    BOOST_CHECK_EQUAL (lineEle[2], "40");

    std::getline (inFile, line);
    std::getline (inFile, line);
    BOOST_CHECK (!line.empty ());

    lineEle = split (line, whiteSpaceCharacters, delimiter_compression::on);
    BOOST_REQUIRE_GE (lineEle.size (), 3);
    BOOST_CHECK_EQUAL (lineEle[0], "11");
    BOOST_CHECK_EQUAL (lineEle[1], "gen");
    BOOST_CHECK_EQUAL (lineEle[2], "50");
    inFile.close ();
    remove ("rec_capture.txt");
}

BOOST_AUTO_TEST_CASE (test_zmq_core)
{
    auto hR = new helicsRunner ();
    hR->InitializeFromString (helics_test_directory + "helics_test3.xml");

    auto sim = hR->getSim ();

    auto genObj = sim->find ("bus2::gen1");
    BOOST_REQUIRE (genObj != nullptr);
    auto src = static_cast<Source *> (genObj->find ("source"));
    BOOST_REQUIRE (src != nullptr);
    auto brk = runBroker ("2");
    auto play = runPlayer (helics_test_directory + "source_player.txt --name=player --stop=24");

    hR->simInitialize ();

    auto tret = hR->Step (5.0);

    BOOST_CHECK_EQUAL (tret, 5.0);

    BOOST_CHECK_CLOSE (src->getOutput (), 0.40, 0.00001);
    tret = hR->Step (9.0);
    BOOST_CHECK_CLOSE (src->getOutput (), 0.45, 0.000001);
    tret = hR->Step (13.0);
    BOOST_CHECK_CLOSE (src->getOutput (), 0.50, 0.00000001);
    tret = hR->Step (17.0);
    BOOST_CHECK_CLOSE (src->getOutput (), 0.55, 0.00000001);
    tret = hR->Step (24.0);
    BOOST_CHECK_CLOSE (src->getOutput (), 0.65, 0.00000001);

    BOOST_CHECK (play.get () == 0);
    hR->Finalize ();
    delete hR;
    sim = nullptr;
    BOOST_CHECK (brk.get () == 0);
    // BOOST_CHECK_CLOSE(src->getOutput(), 0.5, 0.00001);
}

BOOST_AUTO_TEST_CASE (test_collector)
{
    auto hR = std::make_unique<helicsRunner> ();
    hR->InitializeFromString (helics_test_directory + "simple_bus_test1.xml");

    auto sim = hR->getSim ();

    auto brk = helics::apps::BrokerApp ("2");
    auto cmd = utilities::StringToCmdLine ("--tags=\"mag1,mag2,ang1,ang2\"");
    auto rec = helics::apps::Recorder (cmd.getArgCount (), cmd.getArgV ());

    auto fut = std::async (std::launch::async, [&rec]() { rec.runTo (250); });
    hR->simInitialize ();

    auto tret = hR->Run ();

    BOOST_CHECK_EQUAL (tret, 240.0);

    fut.get ();
    hR->Finalize ();
    auto pts = rec.pointCount ();
    BOOST_CHECK_EQUAL (pts, 41 * 4);
    rec.finalize ();
    // I want this freed first PT
    hR = nullptr;
    sim = nullptr;
}

BOOST_AUTO_TEST_CASE (test_collector_vector)
{
    auto hR = std::make_unique<helicsRunner> ();
    hR->InitializeFromString (helics_test_directory + "simple_bus_test_collector_vec.xml");

    auto sim = hR->getSim ();

    auto brk = helics::apps::BrokerApp ("2");
    auto cmd = utilities::StringToCmdLine ("--tags=vout");
    auto rec = helics::apps::Recorder (cmd.getArgCount (), cmd.getArgV ());

    auto fut = std::async (std::launch::async, [&rec]() { rec.runTo (250); });
    hR->simInitialize ();

    auto tret = hR->Run ();

    BOOST_CHECK_EQUAL (tret, 240.0);

    fut.get ();
    hR->Finalize ();
    auto pts = rec.pointCount ();
    BOOST_CHECK_EQUAL (pts, 41);

    auto pt1 = rec.getValue (0);
    BOOST_CHECK_EQUAL (pt1.first, "vout");
    BOOST_CHECK (pt1.second.compare (0, 3, "v4[") == 0);
    rec.finalize ();
    // I want this freed first PT
    hR = nullptr;
    sim = nullptr;
}

BOOST_AUTO_TEST_CASE (test_event)
{
    auto hR = std::make_unique<helicsRunner> ();
    hR->InitializeFromString (helics_test_directory + "simple_bus_test1_event.xml");

    auto sim = hR->getSim ();

    auto brk = helics::apps::BrokerApp ("-f 3");
    auto cmd = utilities::StringToCmdLine ("--tags=\"mag1,mag2,ang1,ang2\"");
    auto rec = helics::apps::Recorder (cmd.getArgCount (), cmd.getArgV ());

    // add a single point
    auto play = helics::apps::Player (std::string{}, helics::FederateInfo ());
    play.addPublication ("breaker", helics::data_type::helics_double);
    play.addPoint (120.0, "breaker", 1.0);

    auto fut_rec = std::async (std::launch::async, [&rec]() { rec.runTo (250); });
    auto fut_play = std::async (std::launch::async, [&play]() { play.run (); });
    hR->simInitialize ();

    auto tret = hR->Run ();

    BOOST_CHECK_EQUAL (tret, 240.0);

    fut_rec.get ();
    fut_play.get ();
    hR->Finalize ();
    auto pts = rec.pointCount ();
    BOOST_CHECK_EQUAL (pts, 41 * 4);
    auto endpt = rec.getValue (163);
    BOOST_CHECK_EQUAL (numeric_conversion (endpt.second, 45.7), 0.0);
    rec.finalize ();
    // I want this freed first PT
    hR = nullptr;
    sim = nullptr;
}

BOOST_AUTO_TEST_CASE (test_vector_event)
{
    auto hR = std::make_unique<helicsRunner> ();
    hR->InitializeFromString (helics_test_directory + "simple_bus_test2_event.xml");

    auto sim = hR->getSim ();

    auto brk = helics::apps::BrokerApp ("-f 3");
    auto cmd = utilities::StringToCmdLine ("--tags=\"mag1,mag2,ang1,ang2\"");
    auto rec = helics::apps::Recorder (cmd.getArgCount (), cmd.getArgV ());

    // add a single point
    auto play = helics::apps::Player (std::string{}, helics::FederateInfo{});
    play.addPublication ("breakers", helics::data_type::helics_vector);
    play.addPoint (120.0, "breakers", "v[0.0,1.0,0.0,0.0]");

    auto fut_rec = std::async (std::launch::async, [&rec]() { rec.runTo (250); });
    auto fut_play = std::async (std::launch::async, [&play]() { play.run (); });
    hR->simInitialize ();

    auto tret = hR->Run ();

    BOOST_CHECK_EQUAL (tret, 240.0);

    fut_rec.get ();
    fut_play.get ();
    hR->Finalize ();
    auto pts = rec.pointCount ();
    BOOST_CHECK_EQUAL (pts, 41 * 4);
    auto endpt = rec.getValue (163);
    BOOST_CHECK_EQUAL (numeric_conversion (endpt.second, 45.7), 0.0);
    rec.finalize ();
    // I want this freed first PT
    hR = nullptr;
    sim = nullptr;
}

BOOST_AUTO_TEST_CASE (test_main_exe)
{
    exeTestRunner mainExeRunner (GRIDDYNINSTALL_LOCATION, GRIDDYNMAIN_LOCATION, "gridDynMain");
    if (mainExeRunner.isActive ())
    {
        auto brk = runBroker ("2");
        auto play = runPlayer (helics_test_directory + "source_player.txt --name=player --stop=24");
        auto out = mainExeRunner.runCaptureOutput (helics_test_directory + "helics_test3.xml --helics");
        auto res = out.find ("HELICS");
        BOOST_CHECK (res != std::string::npos);
        BOOST_CHECK (play.get () == 0);
        BOOST_CHECK (brk.get () == 0);
    }
    else
    {
        std::cout << "Unable to locate main executable:: skipping test\n";
    }
}
#ifdef ENABLE_EXTRA_HELICS_TEST
bool testHELICSCollector ()
{
    helics::clear ();
    std::unique_ptr<gridDynSimulation> gds = std::make_unique<gridDynSimulation> ();
    std::string tfile1 = std::string (HELICS_TEST_DIRECTORY "/helics_collector_test1.xml");

    loadFile (gds.get (), tfile1);

    int cc = gds->getInt ("collectorcount");
    if (cc != 1)
    {
        std::cout << "incorrect number of collectors\n";
        return false;
    }
    gds->run (2.0);
    auto val = helicsGetVal ("p1");
    if (std::abs (val - 0.35) > 0.000001)
    {
        std::cout << "incorrect bus load\n";
        return false;
    }

    auto keys = helics::get_keys ();
    if (keys.size () != 9)
    {
        std::cout << "incorrect number of keys\n";
        return false;
    }
    helicsSendComplex ("helicsload1", 0.45, 0.2);
    helicsSendComplex ("helicsload2", 0.54, 0.31);

    gds->run (4.0);
    val = helicsGetVal ("p1");
    if (std::abs (val - 0.45) > 0.000001)
    {
        std::cout << "incorrect bus real load\n";
        return false;
    }
    val = helicsGetVal ("q2");
    if (std::abs (val - 0.31) > 0.000001)
    {
        std::cout << "incorrect bus reactive load\n";
        return false;
    }
    auto v1 = helicsGetComplex ("helicsvoltage1");
    auto v2 = helicsGetComplex ("helicsvoltage2");
    if (std::abs (v1 - v2) > 0.000001)
    {
        std::cout << "incorrect voltage\n";
        return false;
    }
    return true;
}

#endif

BOOST_AUTO_TEST_SUITE_END ()
