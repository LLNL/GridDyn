/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"

#include "utilities/vectorOps.hpp"
#include <utility>
#include <iostream>
#include <cstdio>
#include <set>

BOOST_FIXTURE_TEST_SUITE (simulation_tests, gridDynSimulationTestFixture)

static const std::string validationTestDirectory(GRIDDYN_TEST_DIRECTORY "/validation_tests/");

BOOST_AUTO_TEST_CASE(simulation_ordering_tests)
{

}


BOOST_AUTO_TEST_SUITE_END()