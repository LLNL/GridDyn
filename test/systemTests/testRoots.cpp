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

#include "gridDynSimulation.h"
#include "fileInput.h"
#include "testHelper.h"
#include "utilities/timeSeries.hpp"
#include "utilities/vectorOps.hpp"
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <cstdio>
// test case for coreObject object

using namespace griddyn;
#define ROOTS_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/rootFinding_tests/"

BOOST_FIXTURE_TEST_SUITE (root_tests, gridDynSimulationTestFixture)

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE (root_test1)
{
    std::string fileName = std::string (ROOTS_TEST_DIRECTORY "test_roots1.xml");

    gds = readSimXMLFile (fileName);
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

    //	gds->consolePrintLevel=0;

    gds->powerflow ();
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

    gds->run (30);
    int alerts = gds->getInt ("alertcount");

    BOOST_CHECK_EQUAL (alerts, 2);
    BOOST_CHECK (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE (test_RampLoadChange2)
{
    std::string fileName = std::string (ROOTS_TEST_DIRECTORY "test_rampLoadChange2.xml");
    simpleRunTestXML (fileName);
}
#endif
/* TODO :: getting this working again will require finishing the updates to governors
which requires the updates to control system logic
*/
/*
BOOST_AUTO_TEST_CASE(test_governor_roots)
{
  std::string fileName = std::string(ROOTS_TEST_DIRECTORY "test_gov_limit3.xml");
  gds = readSimXMLFile(fileName);
  requireState(gridDynSimulation::gridState_t::STARTUP);
  gds->consolePrintLevel = print_level::no_print;
  gds->set("recorddirectory", ROOTS_TEST_DIRECTORY);
  gds->run();
  requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

  std::string recname = std::string(ROOTS_TEST_DIRECTORY "rootDisplay.dat");
  timeSeriesMulti<> ts3;
  int ret = ts3.loadBinaryFile(recname);
  BOOST_CHECK_EQUAL(ret, 0);



  auto d= diff(ts3.data[8]);
  auto mx2 = absMax(ts3.data[9]);
  BOOST_CHECK(mx2<2.52);

  auto r1=d[1000];
  auto r2=d[3000];
  auto r3=d[5500];
  BOOST_CHECK_CLOSE(r1*100.0*100.0,-2,0.5);
  BOOST_CHECK_CLOSE(r2*10000.0,-1,0.5);
  BOOST_CHECK_CLOSE(r3,r1,1.5);
}
*/

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE (test_bus_disable)
{
    std::string fileName = std::string (ROOTS_TEST_DIRECTORY "test_bus_disable.xml");
    simpleRunTestXML (fileName);
}
#endif
BOOST_AUTO_TEST_SUITE_END ()
