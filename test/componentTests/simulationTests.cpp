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

#include "gridDynSimulation.h"
#include "fileInput.h"
#include "testHelper.h"
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "utilities/vectorOps.hpp"
#include <cstdio>
#include <iostream>
#include <set>
#include <utility>

BOOST_FIXTURE_TEST_SUITE (simulation_tests, gridDynSimulationTestFixture)

static const std::string validationTestDirectory (GRIDDYN_TEST_DIRECTORY "/validation_tests/");

BOOST_AUTO_TEST_CASE (simulation_ordering_tests) {}
BOOST_AUTO_TEST_SUITE_END ()