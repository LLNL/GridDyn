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
#include "fileReaders.h"
#include "simulation/diagnostics.h"
#include <cstdio>

#define HVDC_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/dcLink_tests/"

BOOST_FIXTURE_TEST_SUITE (hvdc_tests, gridDynSimulationTestFixture)

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE (hvdc_test1)
{
  std::string fname = std::string (HVDC_TEST_DIRECTORY "test_hvdc1.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);

  

  int mmatch=JacobianCheck(gds,cPflowSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds,cPflowSolverMode);
  }
  BOOST_REQUIRE(mmatch==0);



  gds->powerflow ();

  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
  gds->dynInitialize();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
  mmatch = residualCheck(gds,cDaeSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds, cDaeSolverMode);
  }
  BOOST_REQUIRE(mmatch == 0);

  mmatch = JacobianCheck(gds,cDaeSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds, cDaeSolverMode);
  }
  BOOST_REQUIRE(mmatch == 0);

}


BOOST_AUTO_TEST_CASE (hvdc_test2)
{
  std::string fname = std::string (HVDC_TEST_DIRECTORY "test_hvdc2.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  gds->dynInitialize();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
  int mmatch = residualCheck(gds,cDaeSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds, cDaeSolverMode);
  }
  BOOST_REQUIRE(mmatch == 0);

  mmatch = JacobianCheck(gds,cDaeSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds, cDaeSolverMode);
  }
  BOOST_REQUIRE(mmatch == 0);

  gds->run(20);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

  gds->run(40);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}


BOOST_AUTO_TEST_CASE(hvdc_test3)
{
  std::string fname = std::string(HVDC_TEST_DIRECTORY "test_hvdc3_sc.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  gds->dynInitialize();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
  int mmatch = residualCheck(gds,cDaeSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds, cDaeSolverMode);
  }
  BOOST_REQUIRE(mmatch == 0);

  mmatch = JacobianCheck(gds,cDaeSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds, cDaeSolverMode);
  }
  BOOST_REQUIRE(mmatch == 0);
  gds->run(20);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

  gds->run(40);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif
BOOST_AUTO_TEST_SUITE_END()