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

// test cases for the main executable

#include "exeTestHelper.h"
#include <boost/test/unit_test.hpp>
#include <cstdlib>
#include <iostream>


static std::string pFlow_test_directory = std::string (GRIDDYN_TEST_DIRECTORY "/pFlow_tests/");

BOOST_AUTO_TEST_SUITE (mainexe_tests)


BOOST_AUTO_TEST_CASE (mainexe_test1)
{
	exeTestRunner mainExeRunner(GRIDDYNINSTALL_LOCATION, GRIDDYNMAIN_LOCATION, "griddynMain");
    if (mainExeRunner.isActive ())
    {
        // printf("exestring=%s\n",mainExeRunner.getExeString().c_str());
        auto out = mainExeRunner.runCaptureOutput ("--version");
        // printf("out=%s\n",out.c_str());
        BOOST_CHECK (out.compare (0, 15, "GridDyn version") == 0);
    }
    else
    {
        std::cout << "Unable to locate main executable:: skipping test\n";
    }
}

// test is in development
BOOST_AUTO_TEST_CASE (cdf_readwrite_test)
{
	exeTestRunner mainExeRunner(GRIDDYNINSTALL_LOCATION, GRIDDYNMAIN_LOCATION, "griddynMain");
    if (mainExeRunner.isActive ())
    {
        std::string fileName = pFlow_test_directory + "test_powerflow3m9b2.xml";
        auto out = mainExeRunner.runCaptureOutput (fileName + " --powerflow-only --powerflow-output testout.cdf");
        std::cout << out;
    }
}

BOOST_AUTO_TEST_SUITE_END ()
