/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"
#include "fmi/fmi_models/fmiMELoad3phase.h"
#include "gmlc/utilities/vectorOps.hpp"
#include "griddyn/gridBus.h"
#include "griddyn/simulation/diagnostics.h"
#include "griddyn/simulation/gridDynSimulationFileOps.h"
#include "griddyn/solvers/solverInterface.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <map>
#include <set>
#include <utility>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

using namespace griddyn;

BOOST_FIXTURE_TEST_SUITE(extraFMU_tests, gridDynSimulationTestFixture)
BOOST_AUTO_TEST_CASE(load_fmu)
{
    fmi::fmiMELoad3phase ld3;
    ld3.set(
        "fmu",
        "C:\\Users\\top1\\Documents\\codeProjects\\griddyn_test_cases\\fmus\\DUMMY_0CYMDIST.fmu");
    ld3.set(
        "_configurationFileName",
        "C:\\Users\\top1\\Documents\\codeProjects\\griddyn_test_cases\\fmus\\configuration.json");
    ld3.dynInitializeA(0, 0);
    IOdata res;
    ld3.dynInitializeB(noInputs, noInputs, res);
}

BOOST_AUTO_TEST_SUITE_END()
