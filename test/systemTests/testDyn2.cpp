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

#include <iostream>
#include <cmath>
//test case for coreObject object

#define DYN2_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/dyn_tests2/"


BOOST_FIXTURE_TEST_SUITE (dyn_tests2,gridDynSimulationTestFixture)


BOOST_AUTO_TEST_CASE (dyn_test_simpleEvent)
{

  std::string fname = std::string (DYN2_TEST_DIRECTORY "test_2m4bDyn.xml");

  gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
  gds->consolePrintLevel = print_level::warning;
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  int retval = gds->dynInitialize ();
  BOOST_CHECK_EQUAL (retval,0);
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

  std::vector<double> st = gds->getState ();


  BOOST_CHECK_EQUAL (st.size (),30u);

  gds->run ();
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
  std::vector<double> st2 = gds->getState ();

  auto diff = countDiffsIgnoreCommon(st, st2, 0.02);
  //check for stability
  BOOST_CHECK_EQUAL (diff,0);
}

BOOST_AUTO_TEST_CASE (dyn_test_simpleChunked)
{
  std::string fname = std::string (DYN2_TEST_DIRECTORY "test_2m4bDyn.xml");
  simpleRunTestXML(fname);
  std::vector<double> st = gds->getState ();


  fname = std::string (DYN2_TEST_DIRECTORY "test_2m4bDyn.xml");
  gds2 = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
  gds2->consolePrintLevel = print_level::warning;
  gds2->run (1.5);
  gds2->run (3.7);
  gds2->run (7.65896);
  gds2->run ();
  BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
  std::vector<double> st2 = gds2->getState ();

  auto diff = countDiffsIgnoreCommon(st, st2, 0.0001);

  BOOST_CHECK(diff == 0);

}

BOOST_AUTO_TEST_CASE (dyn_test_randomLoadChange)
{
  std::string fname = std::string (DYN2_TEST_DIRECTORY "test_randLoadChange.xml");
  simpleRunTestXML(fname);
}

BOOST_AUTO_TEST_CASE (dyn_test_RampLoadChange)
{
  std::string fname = std::string (DYN2_TEST_DIRECTORY "test_rampLoadChange.xml");
  simpleRunTestXML(fname);
}



BOOST_AUTO_TEST_CASE (dyn_test_pulseLoadChange1)
{
  std::string fname = std::string (DYN2_TEST_DIRECTORY "test_pulseLoadChange1.xml");
  simpleRunTestXML(fname);
}

BOOST_AUTO_TEST_CASE (dyn_test_pulseLoadChange2)
{
  std::string fname = std::string (DYN2_TEST_DIRECTORY "test_pulseLoadChange2.xml");
  simpleRunTestXML(fname);
}

#ifdef LOAD_CVODE
BOOST_AUTO_TEST_CASE(dyn_test_sinLoadChange_part_cvode)
{ //using cvode
	std::string fname = std::string(DYN2_TEST_DIRECTORY "test_sineLoad_partitioned1.xml");
	simpleRunTestXML(fname);
}


#endif 
BOOST_AUTO_TEST_CASE(dyn_test_sinLoadChange_part_basic_ode)
{ //using basicode
	std::string fname = std::string(DYN2_TEST_DIRECTORY "test_sineLoad_partitioned2.xml");
	simpleRunTestXML(fname);
}

#ifdef LOAD_ARKODE
BOOST_AUTO_TEST_CASE(dyn_test_sinLoadChange_part_arkode)
{ //using arkode
	std::string fname = std::string(DYN2_TEST_DIRECTORY "test_sineLoad_partitioned3.xml");
	simpleRunTestXML(fname);
}


#endif 

//now check if all the different solvers all produce the same results
BOOST_AUTO_TEST_CASE(dyn_test_compare_ode)
{
	std::string fname = std::string(DYN2_TEST_DIRECTORY "test_sineLoadChange.xml");

}


#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(dyn_test_pulseLoadChange_part)
{
	std::string fname = std::string(DYN2_TEST_DIRECTORY "test_pulseLoadChange1_partitioned.xml");
	simpleRunTestXML(fname);
}
#endif
BOOST_AUTO_TEST_SUITE_END ()
