/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"
#include "gmlc/utilities/vectorOps.hpp"
#include <cstdio>
#include <iostream>
#include <set>
#include <utility>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

BOOST_FIXTURE_TEST_SUITE(simulation_tests, gridDynSimulationTestFixture)

static const std::string validationTestDirectory(GRIDDYN_TEST_DIRECTORY "/validation_tests/");

BOOST_AUTO_TEST_CASE(simulation_ordering_tests) {}
BOOST_AUTO_TEST_SUITE_END()
