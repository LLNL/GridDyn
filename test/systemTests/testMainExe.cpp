/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// test cases for the main executable

#include "../exeTestHelper.h"
#include <cstdlib>
#include <iostream>

#include <boost/test/unit_test.hpp>

static std::string pFlow_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/pFlow_tests/");

BOOST_AUTO_TEST_SUITE(mainexe_tests, *boost::unit_test::label("quick"))

BOOST_AUTO_TEST_CASE(mainexe_test1)
{
    exeTestRunner mainExeRunner(GRIDDYNMAIN_LOCATION, GRIDDYNINSTALL_LOCATION, "griddynMain");
    if (mainExeRunner.isActive()) {
        // printf("exestring=%s\n",mainExeRunner.getExeString().c_str());
        auto out = mainExeRunner.runCaptureOutput("--version");
        // printf("out=%s\n",out.c_str());
        BOOST_CHECK(out.compare(0, 15, "GridDyn version") == 0);
        if (out.compare(0, 15, "GridDyn version") != 0) {
            printf("mismatch out string\n");
            printf("out=%s||\n", out.c_str());
        }
    } else {
        std::cout << "Unable to locate main executable:: skipping test\n";
    }
}

// test is in development
BOOST_AUTO_TEST_CASE(cdf_readwrite_test)
{
    exeTestRunner mainExeRunner(GRIDDYNMAIN_LOCATION, GRIDDYNINSTALL_LOCATION, "griddynMain");
    if (mainExeRunner.isActive()) {
        std::string fileName = pFlow_test_directory + "test_powerflow3m9b2.xml";
        auto out = mainExeRunner.runCaptureOutput(
            fileName + " --powerflow-only --powerflow-output testout.cdf");
        std::cout << out;
    }
}

BOOST_AUTO_TEST_SUITE_END()
