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

#include <boost/test/floating_point_comparison.hpp>

BOOST_FIXTURE_TEST_SUITE(large_validation_tests, gridDynSimulationTestFixture)

static const std::string validationTestDirectory(GRIDDYN_TEST_DIRECTORY "/validation_tests/");
using namespace griddyn;

#ifdef ENABLE_IN_DEVELOPMENT_CASES
#    ifdef ENABLE_EXPERIMENTAL_TEST_CASES
// test pjm case
BOOST_AUTO_TEST_CASE(test_pjm_pflow)
{
    std::string fileName = std::string(OTHER_TEST_DIRECTORY "pf.output.raw");
    gds = std::make_unique<gridDynSimulation>();
    readerInfo ri;
    addflags(ri, "ignore_step_up_transformers");
    loadFile(gds, fileName, &ri);
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
    std::vector<double> gv1, gv2;
    gds->getVoltage(gv1);
    gds->pFlowInitialize();
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::INITIALIZED);
    int mmatch = residualCheck(gds, cPflowSolverMode, 0.2, true);
    if (mmatch > 0) {
        printf("Mmatch failures=%d\n", mmatch);
    }
    // BOOST_REQUIRE(mmatch == 0);

    gds->powerflow();
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds->getVoltage(gv2);
    BOOST_REQUIRE(gv1.size() == gv2.size());
    int diffc = 0;
    double bdiff = 0;
    count_t bdiffi = 0;
    for (size_t kk = 0; kk < gv1.size(); ++kk) {
        if (std::abs(gv1[kk] - gv2[kk]) > 0.0015) {
            ++diffc;
            // printf("vstate %d, %f to %f\n",kk,gv1[kk],gv2[kk]);
            if (std::abs(gv1[kk] - gv2[kk]) > bdiff) {
                bdiff = std::abs(gv1[kk] - gv2[kk]);
                bdiffi = kk;
            }
        }
    }
    BOOST_CHECK(diffc == 0);
    if (diffc > 0) {
        printf("%d diffs, difference bus %d orig=%f, result=%f\n",
               diffc,
               bdiffi,
               gv1[bdiffi],
               gv2[bdiffi]);
    }
}

#    endif
#endif

BOOST_AUTO_TEST_CASE(test_pge_pflow)
{
    std::string fileName =
        std::string("C:\\Users\\top1\\Documents\\PG&E Basecases (epc)\\a16_2018LSP.epc");
    gds = std::make_unique<gridDynSimulation>();
    readerInfo ri;
    loadFile(gds, fileName, &ri);
    requireState(gridDynSimulation::gridState_t::STARTUP);
    std::vector<double> gv1, gv2;
    gds->getVoltage(gv1);
    gds->pFlowInitialize();
    requireState(gridDynSimulation::gridState_t::INITIALIZED);
    residualCheck(gds.get(), cPflowSolverMode, 0.2, true);

    gds->powerflow();
    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds->getVoltage(gv2);
    BOOST_REQUIRE_EQUAL(gv1.size(), gv2.size());
    int diffc = 0;
    double bdiff = 0;
    count_t bdiffi = 0;
    for (size_t kk = 0; kk < gv1.size(); ++kk) {
        if (std::abs(gv1[kk] - gv2[kk]) > 0.0015) {
            ++diffc;
            // printf("vstate %d, %f to %f\n",kk,gv1[kk],gv2[kk]);
            if (std::abs(gv1[kk] - gv2[kk]) > bdiff) {
                bdiff = std::abs(gv1[kk] - gv2[kk]);
                bdiffi = static_cast<count_t>(kk);
            }
        }
    }
    BOOST_CHECK(diffc == 0);
    if (diffc > 0) {
        printf("%d diffs, difference bus %d orig=%f, result=%f\n",
               diffc,
               bdiffi,
               gv1[bdiffi],
               gv2[bdiffi]);
    }
}

BOOST_AUTO_TEST_SUITE_END()
