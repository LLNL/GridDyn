/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
   * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department 
 * of Energy by Lawrence Livermore National Laboratory in part under 
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/


#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <cstdio>
//test case for gridCoreObject object

#define CONSTRAINT_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/constraint_tests/"

BOOST_FIXTURE_TEST_SUITE (constraint_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (constraint_test1)
{

  std::string fname = std::string (CONSTRAINT_TEST_DIRECTORY "test_constSimple1.xml");
  gds = (gridDynSimulation *)readSimXMLFile (fname);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  gds->consolePrintLevel = 0;

  gds->powerflow ();
  printf ("completed power flow\n");
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->run (30);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}

BOOST_AUTO_TEST_SUITE_END ()
