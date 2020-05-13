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

// test case for element readers

#include "../exeTestHelper.h"
#include "../testHelper.h"
#include "runner/gridDynRunner.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(runner_tests)

#ifdef ENABLE_THIS_CASE
BOOST_AUTO_TEST_CASE(runner_test1)
{
    std::string fileName = std::string(GRIDDYN_TEST_DIRECTORY "/runnerTests/test_180_trip.xml");
    // std::string fileName = std::string(GRIDDYN_TEST_DIRECTORY
    // "/../../examples/test_180bus_coupled_relay.xml");

    auto gdr = std::make_shared<griddyn::GriddynRunner>();
    int argc = 4;
    char* argv[] = {"griddyn", "-v", "0", (char*)fileName.data()};

    gdr->Initialize(argc, argv);

    gdr->simInitialize();

    long simTime = 0;
    double m_currentGDTime;
    long prevTime = -1;

    std::string line;
    std::ifstream testFile(
        std::string(GRIDDYN_TEST_DIRECTORY "/runner_tests/coupled-time-solvererror.txt"));
    if (!testFile.is_open()) {
        std::cout << "failed to open file";
    } else {
        while (std::getline(testFile, line) && simTime < 18000000000) {
            std::istringstream ss(line);
            std::string time;
            long playbackTime;
            std::getline(ss, time, ',');
            playbackTime = stol(time, nullptr, 10);
            /*    if (playbackTime < 4000000000)
            {
                simTime += 10000000;
            }
            else if (playbackTime > 5000000000 && playbackTime < 15000000000)
            {
                simTime += 10000000;
            }
            else*/
            {
                simTime = playbackTime;
                std::cout << line << "\t\t" << simTime << std::endl;
            }
            if (prevTime < simTime) {
                // Try explicit (double) casting of simTime and see if crashes
                m_currentGDTime = gdr->Step((double)simTime * 1.0E-9);
                prevTime = simTime;
            } else {
                std::cout << "suppressing duplicate" << std::endl;
			}
        }
        testFile.close();
    }

    for (; simTime < 61000000000; simTime += 10000000) {
        std::cout << simTime << std::endl;
        m_currentGDTime = gdr->Step(simTime * 1.0E-9);
        m_currentGDTime = gdr->Step(simTime * 1.0E-9);
    }

    gdr->Finalize();
}

#endif
BOOST_AUTO_TEST_SUITE_END()
