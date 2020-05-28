/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"
#include "griddyn/simulation/diagnostics.h"
#include "griddyn/solvers/solverInterface.h"
#include <cstdio>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

using namespace griddyn;
static std::string solverMode_test_directory =
    std::string(GRIDDYN_TEST_DIRECTORY "/solvermode_tests/");

BOOST_FIXTURE_TEST_SUITE(solverMode_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_SUITE_END()
