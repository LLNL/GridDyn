/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../testHelper.h"
#include "core/objectFactory.hpp"
#include "griddyn/Generator.h"
#include "griddyn/simulation/diagnostics.h"
#include <cmath>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>
// test case for coreObject object

#define GOVERNOR_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/governor_tests/"

BOOST_FIXTURE_TEST_SUITE(governor_tests,
                         gridDynSimulationTestFixture,
                         *boost::unit_test::label("quick"))

using namespace griddyn;

BOOST_AUTO_TEST_CASE(gov_stability_test)
{
    std::string fileName = std::string(GOVERNOR_TEST_DIRECTORY "test_gov_stability.xml");

    gds->resetObjectCounters();
    gds = readSimXMLFile(fileName);
    Generator* gen = static_cast<Generator*>(gds->findByUserID("gen", 2));

    auto cof = coreObjectFactory::instance();
    coreObject* obj = cof->createObject("governor", "basic");

    gen->add(obj);

    int retval = gds->dynInitialize();
    BOOST_CHECK_EQUAL(retval, 0);
    requireState(gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

    BOOST_CHECK_EQUAL(runJacobianCheck(gds, cDaeSolverMode), 0);
    gds->run(0.005);
    BOOST_CHECK_EQUAL(runJacobianCheck(gds, cDaeSolverMode), 0);

    gds->run(400.0);
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    std::vector<double> st = gds->getState();
    gds->run(500.0);
    gds->saveRecorders();
    std::vector<double> st2 = gds->getState();

    // check for stability
    BOOST_REQUIRE_EQUAL(st.size(), st2.size());
    int ncnt = 0;
    double a0 = st2[0];
    for (size_t kk = 0; kk < st.size(); ++kk) {
        if (std::abs(st[kk] - st2[kk]) > 0.0001) {
            if (std::abs(st[kk] - st2[kk] + a0) > 0.005 * ((std::max)(st[kk], st2[kk]))) {
                printf("state[%zd] orig=%f new=%f\n", kk, st[kk], st2[kk]);
                ncnt++;
            }
        }
    }
    BOOST_CHECK_EQUAL(ncnt, 0);
}

BOOST_AUTO_TEST_SUITE_END()
