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
#include "core/coreExceptions.h"
#include "gmlc/utilities/vectorOps.hpp"
#include "griddyn/Link.h"
#include "griddyn/gridBus.h"
#include <cmath>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>
// testP case for coreObject object

#define AREA_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/area_tests/"

BOOST_FIXTURE_TEST_SUITE(area_tests,
                         gridDynSimulationTestFixture,
                         *boost::unit_test::label("quick"))

using namespace griddyn;
BOOST_AUTO_TEST_CASE(area_test1)
{
    std::string fileName = std::string(AREA_TEST_DIRECTORY "area_test1.xml");

    gds = readSimXMLFile(fileName);
    requireState(gridDynSimulation::gridState_t::STARTUP);

    gds->pFlowInitialize();
    requireState(gridDynSimulation::gridState_t::INITIALIZED);

    int count;
    count = gds->getInt("totalareacount");
    BOOST_CHECK_EQUAL(count, 1);
    count = gds->getInt("totalbuscount");
    BOOST_CHECK_EQUAL(count, 9);
    // check the linkcount
    count = gds->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, 9);

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    auto st = gds->getState();

    fileName = std::string(AREA_TEST_DIRECTORY "area_test0.xml");

    gds2 = readSimXMLFile(fileName);

    gds2->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    auto st2 = gds2->getState();
    auto diffs = gmlc::utilities::countDiffs(st, st2, 0.00001);
    BOOST_CHECK(diffs == 0);
}

BOOST_AUTO_TEST_CASE(area_test_add)
{
    auto area = std::make_unique<Area>("area1");

    auto bus1 = new gridBus("bus1");
    try {
        area->add(bus1);
        area->add(bus1);
    }
    catch (...) {
        BOOST_CHECK(false);
    }

    auto bus2 = bus1->clone();
    try {
        area->add(bus2);
        // this is testing failure
        BOOST_CHECK(false);
    }
    catch (const objectAddFailure& oaf) {
        BOOST_CHECK(oaf.who() == "area1");
    }
    bus2->setName("bus2");
    try {
        area->add(bus2);
        BOOST_CHECK(isSameObject(bus2->getParent(), area.get()));
    }
    catch (...) {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_SUITE_END()
