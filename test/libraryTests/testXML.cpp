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

#include "griddyn/Generator.h"
#include "griddyn/gridBus.h"
#include "griddyn/Link.h"
#include "griddyn/loads/zipLoad.h"
#include "griddyn/Exciter.h"
#include "griddyn/GenModel.h"
#include "griddyn/Governor.h"
#include "gmlc/utilities/vectorOps.hpp"
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

// test case for coreObject object

#include "../testHelper.h"

static const std::string xmlTestDirectory (GRIDDYN_TEST_DIRECTORY "/xml_tests/");
// create a test fixture that makes sure everything gets deleted properly

BOOST_FIXTURE_TEST_SUITE(xml_tests, gridDynSimulationTestFixture, * boost::unit_test::label("quick"))

using namespace griddyn;
BOOST_AUTO_TEST_CASE (xml_test1)
{
    // test the loading of a single bus
    std::string fileName = xmlTestDirectory + "test_xmltest1.xml";
    gds = readSimXMLFile (fileName);
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);
    int count = gds->getInt ("totalbuscount");
    BOOST_CHECK_EQUAL (count, 1);
    BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
    BOOST_CHECK_EQUAL (gds->getName ().compare ("test1"), 0);

    gridBus *bus = gds->getBus (0);
    BOOST_REQUIRE (bus != nullptr);
    BOOST_CHECK (bus->getType () == gridBus::busType::SLK);
    BOOST_CHECK_EQUAL (bus->getAngle (), 0);
    BOOST_CHECK_EQUAL (bus->getVoltage (), 1.04);

    Generator *gen = bus->getGen (0);

    if (!(gen))
    {
        return;
    }
    BOOST_CHECK_EQUAL (gen->getRealPower (), -0.7160);
    BOOST_CHECK_EQUAL (gen->getName ().compare ("gen1"), 0);
}


BOOST_AUTO_TEST_CASE (xml_test2)
{
    // test the loading of 3 buses one of each type

    std::string fileName = xmlTestDirectory + "test_xmltest2.xml";
    gds = readSimXMLFile (fileName);
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);
    int count = gds->getInt ("totalbuscount");
    BOOST_CHECK_EQUAL (count, 4);
    BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);

    gridBus *bus = gds->getBus (1);
    BOOST_REQUIRE (bus != nullptr);
    BOOST_CHECK (bus->getType () == gridBus::busType::PV);

    bus = gds->getBus (3);
    BOOST_REQUIRE (bus != nullptr);
    BOOST_CHECK (bus->getType () == gridBus::busType::PQ);

    Load *ld = bus->getLoad ();
    BOOST_REQUIRE (ld != nullptr);
    if (!(ld))
    {
        return;
    }
    BOOST_CHECK_EQUAL (ld->getRealPower (), 1.25);
    BOOST_CHECK_EQUAL (ld->getReactivePower (), 0.5);
    BOOST_CHECK_EQUAL (ld->getName ().compare ("load5"), 0);
}

BOOST_AUTO_TEST_CASE (xml_test3)
{
    // testt the loading of links
    std::string fileName = xmlTestDirectory + "test_xmltest3.xml";
    gds = readSimXMLFile (fileName);
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);
    int count = gds->getInt ("totalbuscount");
    BOOST_CHECK_EQUAL (count, 9);
    BOOST_CHECK_EQUAL (readerConfig::warnCount, 1);
    count = gds->getInt ("linkcount");
    BOOST_CHECK_EQUAL (count, 2);
    // test out link 1
    Link *lnk = gds->getLink (0);
    if (!(lnk))
    {
        return;
    }


    gridBus *bus = lnk->getBus (1);
    if (!(bus))
    {
        return;
    }
    BOOST_CHECK_EQUAL (bus->getName ().compare ("bus1"), 0);

    bus = lnk->getBus (2);
    if (!(bus))
    {
        return;
    }
    BOOST_CHECK_EQUAL (bus->getName ().compare ("bus4"), 0);
    BOOST_CHECK_EQUAL (lnk->getName ().compare ("bus1_to_bus4"), 0);
    // test out link2
    lnk = gds->getLink (1);
    if (!(lnk))
    {
        return;
    }

    bus = lnk->getBus (1);
    if (!(bus))
    {
        return;
    }
    BOOST_CHECK_EQUAL (bus->getName ().compare ("bus4"), 0);

    bus = lnk->getBus (2);
    if (!(bus))
    {
        return;
    }
    BOOST_CHECK_EQUAL (bus->getName ().compare ("bus5"), 0);
}

BOOST_AUTO_TEST_CASE (xml_test4)
{
    // test the loading of a dynamic generator
    int count;

    std::string fileName = xmlTestDirectory + "test_xmltest4.xml";

    gds = readSimXMLFile (fileName);
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);
    count = gds->getInt ("totalbuscount");
    BOOST_CHECK_EQUAL (count, 1);
    BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);

    gridBus *bus = gds->getBus (0);
    BOOST_REQUIRE (bus != nullptr);
    BOOST_CHECK (bus->getType () == gridBus::busType::SLK);

    Generator *gen = bus->getGen (0);
    BOOST_REQUIRE (gen != nullptr);

    GenModel *gm = nullptr;
    Governor *gov = nullptr;
    Exciter *ext = nullptr;

    coreObject *obj = nullptr;

    obj = gen->find ("exciter");
    BOOST_REQUIRE (obj != nullptr);
    ext = dynamic_cast<Exciter *> (obj);
    BOOST_REQUIRE (ext != nullptr);
    obj = gen->find ("governor");
    BOOST_REQUIRE (obj != nullptr);
    gov = dynamic_cast<Governor *> (obj);
    BOOST_REQUIRE (gov != nullptr);
    obj = gen->find ("genmodel");
    BOOST_REQUIRE (obj != nullptr);
    gm = dynamic_cast<GenModel *> (obj);
    BOOST_REQUIRE (gm != nullptr);
}

BOOST_AUTO_TEST_CASE (xml_test5)
{
    // test the loading of a library
    int count;

    std::string fileName = xmlTestDirectory + "test_xmltest5.xml";

    gds = std::make_unique<gridDynSimulation> ();

    readerInfo ri;

    loadFile (gds.get (), fileName, &ri);
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);
    count = gds->getInt ("totalbuscount");
    BOOST_CHECK_EQUAL (count, 1);
    BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);


    GenModel *gm = nullptr;
    Governor *gov = nullptr;
    Exciter *ext = nullptr;

    coreObject *obj = nullptr;

    obj = ri.findLibraryObject ("ex1");
    BOOST_REQUIRE (obj != nullptr);
    ext = dynamic_cast<Exciter *> (obj);
    BOOST_REQUIRE (ext != nullptr);

    obj = ri.findLibraryObject ("gov1");
    BOOST_REQUIRE (obj != nullptr);
    gov = dynamic_cast<Governor *> (obj);
    BOOST_REQUIRE (gov != nullptr);

    obj = ri.findLibraryObject ("gm1");
    BOOST_REQUIRE (obj != nullptr);
    gm = dynamic_cast<GenModel *> (obj);
    BOOST_REQUIRE (gm != nullptr);
}

BOOST_AUTO_TEST_CASE (xml_test6)
{
    // test the loading of a reference object
    int count;

    std::string fileName = xmlTestDirectory + "test_xmltest6.xml";

    gds = std::make_unique<gridDynSimulation> ();

    readerInfo ri;

    loadFile (gds.get (), fileName, &ri);
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);
    count = gds->getInt ("totalbuscount");
    BOOST_CHECK_EQUAL (count, 1);
    BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);


    gridBus *bus = gds->getBus (0);
    BOOST_REQUIRE (bus != nullptr);
    BOOST_CHECK (bus->getType () == gridBus::busType::SLK);

    Generator *gen = bus->getGen (0);
    BOOST_REQUIRE (gen != nullptr);
    GenModel *gm = nullptr;

    coreObject *obj = nullptr;

    obj = gen->find ("genmodel");
    BOOST_REQUIRE (obj != nullptr);
    gm = dynamic_cast<GenModel *> (obj);
    BOOST_REQUIRE (gm != nullptr);
}

BOOST_AUTO_TEST_CASE (xml_test7)
{
    // test the loading of generator submodels
    int count;

    std::string fileName = xmlTestDirectory + "test_xmltest7.xml";
    gds = readSimXMLFile (fileName);
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);
    count = gds->getInt ("totalbuscount");
    BOOST_CHECK_EQUAL (count, 1);
    BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);

    gridBus *bus = gds->getBus (0);
    BOOST_REQUIRE (bus != nullptr);
    BOOST_CHECK (bus->getType () == gridBus::busType::SLK);

    Generator *gen = bus->getGen (0);
    BOOST_REQUIRE (gen != nullptr);

    GenModel *gm = nullptr;
    Governor *gov = nullptr;
    Exciter *ext = nullptr;

    coreObject *obj = nullptr;

    obj = gen->find ("exciter");
    BOOST_REQUIRE (obj != nullptr);
    ext = dynamic_cast<Exciter *> (obj);
    BOOST_REQUIRE (ext != nullptr);
    obj = gen->find ("governor");
    BOOST_REQUIRE (obj != nullptr);
    gov = dynamic_cast<Governor *> (obj);
    BOOST_REQUIRE (gov != nullptr);
    obj = gen->find ("genmodel");
    BOOST_REQUIRE (obj != nullptr);
    gm = dynamic_cast<GenModel *> (obj);
    BOOST_REQUIRE (gm != nullptr);
}

/**test case for making sure library call work exactly like full specification calls*/
BOOST_AUTO_TEST_CASE (xml_test_dynLib)
{
    std::string fileName = xmlTestDirectory + "test_2m4bDyn.xml";
    gds = readSimXMLFile (fileName);
    gds->consolePrintLevel = print_level::no_print;
    gds->run ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st = gds->getState ();


    fileName = xmlTestDirectory + "test_2m4bDyn_lib.xml";
    gds2 = readSimXMLFile (fileName);
    gds2->consolePrintLevel = print_level::no_print;
    gds2->run ();
    BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st2 = gds2->getState ();

    auto sdiffs = countDiffs (st, st2, 5e-5);

    BOOST_CHECK_EQUAL (sdiffs, 0u);
}

/**test case for generators and loads in the main element*/
BOOST_AUTO_TEST_CASE (xml_test_maingen)
{
    std::string fileName = xmlTestDirectory + "test_2m4bDyn.xml";
    gds = readSimXMLFile (fileName);
    gds->consolePrintLevel = print_level::no_print;
    gds->run ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st = gds->getState ();


    fileName = xmlTestDirectory + "test_2m4bDyn_mgen.xml";
    gds2 = readSimXMLFile (fileName);
    gds2->consolePrintLevel = print_level::no_print;
    gds2->run ();
    BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st2 = gds2->getState ();


    auto sdiffs = countDiffs (st, st2, 5e-5);

    BOOST_CHECK_EQUAL (sdiffs, 0u);
}

/**test case for property override and object reloading*/
BOOST_AUTO_TEST_CASE (xml_test_rload)
{
    std::string fileName = xmlTestDirectory + "test_2m4bDyn.xml";
    gds = readSimXMLFile (fileName);
    gds->consolePrintLevel = print_level::no_print;
    gds->run ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st = gds->getState ();


    fileName = xmlTestDirectory + "test_2m4bDyn_rload.xml";
    gds2 = readSimXMLFile (fileName);
    gds2->consolePrintLevel = print_level::no_print;
    gds2->run ();
    BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st2 = gds2->getState ();

    auto sdiffs = countDiffs (st, st2, 5e-5);

    BOOST_CHECK_EQUAL (sdiffs, 0u);
}

/**test case for seperate source file*/
BOOST_AUTO_TEST_CASE (xml_test_source1)
{
    std::string fileName = xmlTestDirectory + "test_2m4bDyn.xml";
    gds = readSimXMLFile (fileName);
    gds->consolePrintLevel = print_level::no_print;
    gds->run ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st = gds->getState ();


    fileName = xmlTestDirectory + "test_2m4bDyn_sep.xml";
    gds2 = readSimXMLFile (fileName);
    gds2->consolePrintLevel = print_level::no_print;
    gds2->run ();
    BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st2 = gds2->getState ();

    auto sdiffs = countDiffs (st, st2, 5e-5);

    BOOST_CHECK_EQUAL (sdiffs, 0u);
}

BOOST_AUTO_TEST_CASE (xml_test8)
{
    // test the loading of a Recorder

    std::string fileName = xmlTestDirectory + "test_xmltest8.xml";

    gds = std::make_unique<gridDynSimulation> ();

    readerInfo ri;

    loadFile (gds.get (), fileName, &ri);
    requireState(gridDynSimulation::gridState_t::STARTUP);
    BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
    BOOST_CHECK_EQUAL (ri.collectors.size (), 1u);
}

BOOST_AUTO_TEST_CASE (xml_test9)
{
    // test the define functionality
    std::string fileName = xmlTestDirectory + "test_2m4bDyn.xml";
    gds = readSimXMLFile (fileName);
    gds->consolePrintLevel = print_level::no_print;
    gds->run ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st = gds->getState ();


    fileName = xmlTestDirectory + "test_xmltest9.xml";

    gds2 = readSimXMLFile (fileName);
    gds2->consolePrintLevel = print_level::no_print;
    gds2->run ();
    requireState2(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st2 = gds2->getState ();


    // check for equality
    BOOST_REQUIRE_EQUAL (st.size (), st2.size ());
    // BOOST_REQUIRE_SMALL(compare(st, st2), 0.001);

    double diff = compareVec (st, st2);
    BOOST_CHECK (diff < 0.01);
}

BOOST_AUTO_TEST_CASE (test_bad_xml)
{
    // test the define functionality
    std::string fileName = xmlTestDirectory + "test_bad_xml.xml";
    gds = readSimXMLFile (fileName);
    BOOST_REQUIRE (gds == nullptr);
    std::cout << "NOTE: this was supposed to have a failed file load to check error recovery\n";
}

BOOST_AUTO_TEST_CASE (test_function_constants)
{
    // test the define functionality
    std::string fileName = xmlTestDirectory + "test_function_constant.xml";
    gds = readSimXMLFile (fileName);

    gridBus *bus = gds->getBus (0);
    BOOST_CHECK_CLOSE (bus->getVoltage (), 1.0, 0.00001);

    bus = gds->getBus (1);
    BOOST_CHECK_CLOSE (bus->getAngle (), acos (0.1734), 0.00001);
    bus = gds->getBus (2);
    BOOST_CHECK_CLOSE (bus->getVoltage (), 0.9576, 0.00001);
    bus = gds->getBus (3);
    BOOST_CHECK_CLOSE (bus->getVoltage (), 1.1, 0.00001);
}

// Test case for testing the various means of setting parameters
BOOST_AUTO_TEST_CASE (test_param_specs)
{
    // test the define functionality
    std::string fileName = xmlTestDirectory + "test_param_setting.xml";
    gds = readSimXMLFile (fileName);


    auto bus = gds->getBus (0);
    BOOST_REQUIRE (bus != nullptr);
    auto ld1 = bus->getLoad (0);
    BOOST_REQUIRE (ld1 != nullptr);
    auto ld2 = bus->getLoad (1);
    BOOST_REQUIRE (ld2 != nullptr);

    BOOST_CHECK_CLOSE (ld1->get ("p"), 0.4, 0.0001);
    BOOST_CHECK_CLOSE (ld1->get ("q"), 0.3, 0.0001);
    BOOST_CHECK_CLOSE (ld1->get ("ip"), 0.55, 0.0001);
    BOOST_CHECK_CLOSE (ld1->get ("iq"), 0.32, 0.0001);
    BOOST_CHECK_CLOSE (ld1->get ("yp"), 0.5, 0.0001);
    BOOST_CHECK_CLOSE (ld1->get ("yq"), 0.11, 0.0001);

    BOOST_CHECK_CLOSE (ld2->get ("p"), 0.31, 0.0001);
    BOOST_CHECK_CLOSE (ld2->get ("q", gridUnits::MVAR), 14.8, 0.0001);
    BOOST_CHECK_CLOSE (ld2->get ("yp"), 1.27, 0.0001);
    BOOST_CHECK_CLOSE (ld2->get ("yq"), 0.74, 0.0001);
    // TODO:: PT this capability is not enabled yet
    // BOOST_CHECK_CLOSE(ld2->get("ip"), 0.145, 0.0001);
    // BOOST_CHECK_CLOSE(ld2->get("iq"), 0.064, 0.0001);
}


// Test case for testing the various means of setting parameters
BOOST_AUTO_TEST_CASE (test_custom_element1)
{
    // test the define functionality
    std::string fileName = xmlTestDirectory + "test_custom_element1.xml";
    gds = readSimXMLFile (fileName);

    int bc = gds->getInt ("buscount");
    BOOST_CHECK_EQUAL (bc, 10);
    std::vector<double> V;
    gds->getVoltage (V);
    double mxv = *std::max_element (V.begin (), V.end ());
    double mnv = *std::min_element (V.begin (), V.end ());
    BOOST_CHECK_LT (mnv, mxv);
}

// Test case for testing the various means of setting parameters
BOOST_AUTO_TEST_CASE (test_custom_element2)
{
    // test the define functionality
    std::string fileName = xmlTestDirectory + "test_custom_element2.xml";
    gds = readSimXMLFile (fileName);

    int bc = gds->getInt ("buscount");
    BOOST_CHECK_EQUAL (bc, 7);
    std::vector<double> V;
    gds->getVoltage (V);
    double mxv = *std::max_element (V.begin (), V.end ());
    double mnv = *std::min_element (V.begin (), V.end ());
    BOOST_CHECK_LT (mnv, mxv);
}


// Test case for query conditions in an if element
BOOST_AUTO_TEST_CASE(test_query_if)
{
	// test the define functionality
	std::string fileName = xmlTestDirectory + "test_query_if.xml";
	gds = readSimXMLFile(fileName);

	auto bus = gds->getBus(0);
	//This will show up as 2 or 0 if the conditions are not working properly
	BOOST_CHECK_EQUAL(bus->get("gencount"), 1);
}

BOOST_AUTO_TEST_SUITE_END ()
