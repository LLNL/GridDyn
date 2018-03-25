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

#include "griddyn/gridBus.h"
#include "griddyn/simulation/diagnostics.h"
#include "griddyn/simulation/gridDynSimulationFileOps.h"
#include "griddyn/solvers/solverInterface.h"
#include "../testHelper.h"
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "utilities/vectorOps.hpp"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <fstream>

using namespace griddyn;

BOOST_FIXTURE_TEST_SUITE (performance_tests, gridDynSimulationTestFixture)

static const std::string validationTestDirectory (GRIDDYN_TEST_DIRECTORY "/validation_tests/");

BOOST_AUTO_TEST_CASE (performance_tests1)
{
    
    const stringVec perf_cases{"case1354pegase.m", "case2869pegase.m", "case3012wp.m", "case3375wp.m",
                               "case9241pegase.m"};
    
    std::chrono::duration<double> pflow_time (0);
    std::chrono::duration<double> load_time (0);
    for (const auto &mp : perf_cases)
    {
        std::string fileName;
        if (mp.length () > 25)
        {
            fileName = mp;
        }
        else
        {
            fileName = validationTestDirectory + mp;
        }
        for (int kk = 0; kk < 10; ++kk)  // Do this 10 time
        {
            gds = std::make_unique<gridDynSimulation> ();
            gds->set ("consoleprintlevel", "summary");
            auto start_t = std::chrono::high_resolution_clock::now ();
            loadFile (gds.get (), fileName);
            gds->setFlag ("no_powerflow_adjustments");
            auto stop_t = std::chrono::high_resolution_clock::now ();
            load_time += (stop_t - start_t);

            BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

            start_t = std::chrono::high_resolution_clock::now ();
            gds->powerflow ();
            stop_t = std::chrono::high_resolution_clock::now ();
            pflow_time += (stop_t - start_t);

            if (gds->currentProcessState () != gridDynSimulation::gridState_t::POWERFLOW_COMPLETE)
            {
                std::cout << fileName << " did not complete power flow calculation\n";
                break;
            }
            BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
        }
        printf ("%s load in %f powerflow in %f\n", mp.c_str (), load_time.count () / 10.0,
                pflow_time.count () / 10.0);
    }
}


BOOST_AUTO_TEST_CASE (performance_tests_scaling_pFlow)
{
    std::string testFile = std::string (GRIDDYN_TEST_DIRECTORY "/performance_tests/block_grid3_motor.xml");

    std::vector<int> elements =
      {
        3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  14,  16,  18,  20,  24,  28,  32,
        36,  40,  45,  50,  56,  60,  66,  70,  74,  80,  84,  86,  88,  90,  92,  100, 110,
        120, 132, 136, 140, 148, 156, 164, 168, 172, 180, 188, 196, 204, 212, 218, 220, 224,
        230, 240, 244, 250, 260, 270, 318, 340, 360, 400, /*450, 500, 550, 600, 720,800*/
      };
    int numLoops = 1;
    for (int rr = 0; rr < numLoops; ++rr)
    {
        // std::vector<int> elements = {212,240,260};
        // std::vector<int> elements{ 800 };
        std::chrono::duration<double> pflow_time (0);
        std::chrono::duration<double> load_time (0);

        std::vector<double> ldtime (elements.size ());
        std::vector<double> pftime (elements.size ());
        std::string outstring = "block3_motor_timing_kin1_" + std::to_string (rr) + ".csv";
        std::ofstream outfile (outstring);
        outfile << "N, buses, states, nnz, resid calls, jac call, load time, powerflow time,  solve time, resid "
                   "time, jac time, jac1 time, kin1time\n";
        int ii = 0;
        for (auto gsize : elements)
        {
            gds = std::make_unique<gridDynSimulation> ();

            readerInfo ri;
            ri.addLockedDefinition ("garraySize", std::to_string (gsize));
            gds->set ("consoleprintlevel", "summary");
            auto start_t = std::chrono::high_resolution_clock::now ();
            loadFile (gds.get (), testFile, &ri);
            gds->setFlag ("no_powerflow_adjustments");
            auto stop_t = std::chrono::high_resolution_clock::now ();
            load_time = (stop_t - start_t);
            ldtime[ii] = load_time.count ();

            BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

            start_t = std::chrono::high_resolution_clock::now ();
            gds->powerflow ();
            stop_t = std::chrono::high_resolution_clock::now ();
            gds->updateLocalCache ();
            pflow_time = (stop_t - start_t);
            pftime[ii] = pflow_time.count ();
            int ss = gds->getInt ("statesize");
            int nnz = gds->getInt ("nonzeros");
            int rcount = gds->getInt ("residcount");
            int jcount = gds->getInt ("jaccount");
            printf ("%d size=%d, nnz=%d,ld time=%f, pflow time=%f\n", gsize, ss, nnz, load_time.count (),
                    pflow_time.count ());
            printf ("%d residual calls, %d Jacobian call\n", rcount, jcount);
            auto bus = static_cast<gridBus *> (gds->findByUserID ("bus", 10000000));
            printf ("slack bus gen p=%f, gen q =%f\n", bus->getGenerationReal (), bus->getGenerationReactive ());
            outfile << gsize << ", " << gsize * gsize << ", " << ss << ", " << nnz << ", ";
            outfile << rcount << ", " << jcount << ", ";
            outfile << load_time.count () << ", " << pflow_time.count ();
            auto sc = gds->getSolverInterface ("powerflow");
            outfile << ", " << sc->get ("kintime") << ", " << sc->get ("residtime") << ", " << sc->get ("jactime");
            outfile << ", " << sc->get ("jac1time") << ", " << sc->get ("kin1time") << "\n";
            std::vector<double> v;
            int cnt = gds->getVoltage (v);
            auto minV = minLoc (v);
            auto maxV = maxLoc (v);
            printf ("cnt=%d vmin=%f at %d, vmax=%f at %d \n", cnt, minV.first, minV.second, maxV.first,
                    maxV.second);
            // savePowerFlowCSV(gds,"bigCSV.csv");
        }
    }
}


BOOST_AUTO_TEST_CASE (dynamic_scalable_test)
{
    std::string testFile = std::string (GRIDDYN_TEST_DIRECTORY "/performance_tests/block_grid2_dynamic.xml");


    // std::vector<int> elements = {212,240,260};
    // std::vector<int> elements{ 800 };
    std::chrono::duration<double> pflow_time (0);
    std::chrono::duration<double> load_time (0);

    int gsize = 50;
    //	int ii = 0;

    gds = std::make_unique<gridDynSimulation> ();

    readerInfo ri;
    ri.addLockedDefinition ("garraySize", std::to_string (gsize));
    gds->set ("consoleprintlevel", "summary");
    auto start_t = std::chrono::high_resolution_clock::now ();
    loadFile (gds.get (), testFile, &ri);
    gds->setFlag ("no_powerflow_adjustments");
    auto stop_t = std::chrono::high_resolution_clock::now ();
    load_time = (stop_t - start_t);


    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

    start_t = std::chrono::high_resolution_clock::now ();
    gds->powerflow ();
    stop_t = std::chrono::high_resolution_clock::now ();
    gds->updateLocalCache ();
    pflow_time = (stop_t - start_t);

    int ss = gds->getInt ("statesize");
    int nnz = gds->getInt ("nonzeros");
    int rcount = gds->getInt ("residcount");
    int jcount = gds->getInt ("jaccount");
    printf ("%d size=%d, nnz=%d,ld time=%f, pflow time=%f\n", gsize, ss, nnz, load_time.count (),
            pflow_time.count ());
    printf ("%d residual calls, %d Jacobian call\n", rcount, jcount);
    auto bus = static_cast<gridBus *> (gds->findByUserID ("bus", 10000000));
    printf ("slack bus gen p=%f, gen q =%f\n", bus->getGenerationReal (), bus->getGenerationReactive ());

    gds->run ();


    // savePowerFlowCSV(gds,"bigCSV.csv");
}


BOOST_AUTO_TEST_SUITE_END ()
