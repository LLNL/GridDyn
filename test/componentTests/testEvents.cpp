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
#include "fileInput/fileInput.h"
#include "griddyn/events/Event.h"
#include <cmath>
#include <cstdio>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

// test case for coreObject object

static std::string event_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/event_tests/");

BOOST_FIXTURE_TEST_SUITE(event_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE(event_test_loadstring)
{
    std::string fileName = event_test_directory + "event_test_loadstring.xml";
    gds = griddyn::readSimXMLFile(fileName);

    griddyn::EventInfo gdEI;
    gdEI.loadString("@2+3|LINK#1:param(MW)=13", gds->getRoot());

    BOOST_CHECK(gdEI.targetObjs[0] != nullptr);
    BOOST_CHECK_EQUAL(gdEI.fieldList[0], "param");
    BOOST_CHECK_EQUAL(gdEI.value[0], 13);
    BOOST_CHECK(gdEI.units[0] == units::MW);
    BOOST_CHECK_EQUAL(gdEI.time[0], griddyn::coreTime(2));
    BOOST_CHECK_EQUAL(gdEI.period, griddyn::coreTime(3));
}

BOOST_AUTO_TEST_CASE(event_test1) {}
BOOST_AUTO_TEST_CASE(event_test2) {}
BOOST_AUTO_TEST_CASE(event_test3) {}
BOOST_AUTO_TEST_CASE(event_test4) {}
BOOST_AUTO_TEST_SUITE_END()
