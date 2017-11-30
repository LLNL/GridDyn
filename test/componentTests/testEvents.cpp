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

#include "events/Event.h"
#include "gridDynSimulation.h"
#include "fileInput.h"
#include "testHelper.h"
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <cstdio>

// test case for coreObject object

static std::string event_test_directory = std::string (GRIDDYN_TEST_DIRECTORY "/event_tests/");

BOOST_FIXTURE_TEST_SUITE (event_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (event_test1) {}
BOOST_AUTO_TEST_CASE (event_test2) {}
BOOST_AUTO_TEST_CASE (event_test3) {}
BOOST_AUTO_TEST_CASE (event_test4) {}
BOOST_AUTO_TEST_SUITE_END ()
