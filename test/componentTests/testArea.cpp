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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include "vectorOps.hpp"
#include <cmath>
//testP case for gridCoreObject object


#define AREA_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/area_tests/"

BOOST_FIXTURE_TEST_SUITE (area_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (area_test1)
{
  std::string fname = std::string (AREA_TEST_DIRECTORY "area_test1.xml");

  gds = (gridDynSimulation *)readSimXMLFile (fname);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);

  int count;
  count= gds->getInt ("totalareacount");
  BOOST_CHECK_EQUAL (count, 1);
  count=gds->getInt ("totalbuscount");
  BOOST_CHECK_EQUAL (count, 9);
  //check the linkcount
  count=gds->getInt ("totallinkcount");
  BOOST_CHECK_EQUAL (count, 9);

  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);


  auto st= gds->getState();



  fname = std::string (AREA_TEST_DIRECTORY "area_test0.xml");


  gds2 = (gridDynSimulation *)readSimXMLFile (fname);


  gds2->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);


  auto st2 = gds2->getState();
  auto diffs = countDiffs(st, st2, 0.00001);
  BOOST_CHECK(diffs == 0);
 
}

BOOST_AUTO_TEST_SUITE_END()
