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
#include "linkModels/acLine.h"
#include "simulation/diagnostics.h"
//testP case for gridCoreObject object


#define TADJ_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/adj_tests/"

BOOST_FIXTURE_TEST_SUITE (adj_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (adj_test_simple)
{
  std::string fname = std::string (TADJ_TEST_DIRECTORY "adj_test1.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  std::vector<double> st;
  gds->getVoltage (st);
  BOOST_CHECK_GE (st[2], 0.99);
  BOOST_CHECK_LE (st[2], 1.01);

  //tap changing doesn't do anything in this case we are checking to make sure the tap goes all the way
  fname = std::string (TADJ_TEST_DIRECTORY "adj_test2.xml");
  gds2 = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds2->powerflow ();
  BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  adjustableTransformer *adj = dynamic_cast<adjustableTransformer *> (gds2->getLink (1));
  BOOST_CHECK_SMALL (adj->getTap () - 1.1,0.001);

}


BOOST_AUTO_TEST_CASE (adj_test_simple2)
{
  //test multiple interacting controllers
  std::string fname = std::string (TADJ_TEST_DIRECTORY "adj_test3.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  std::vector<double> st;
  gds->getVoltage (st);
  BOOST_CHECK_GE (st[1], 0.99);
  BOOST_CHECK_LE (st[1], 1.01);
  BOOST_CHECK_GE (st[2], 0.99);
  BOOST_CHECK_LE (st[2], 1.01);

  //test multiple interacting controllers voltage reduction mode
  fname = std::string (TADJ_TEST_DIRECTORY "adj_test4.xml");

  gds2 = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds2->powerflow ();
  BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds2->getVoltage (st);
  BOOST_CHECK_GE (st[1], 0.99);
  BOOST_CHECK_LE (st[1], 1.01);
  BOOST_CHECK_GE (st[2], 0.99);
  BOOST_CHECK_LE (st[2], 1.01);

  delete gds;
  gds = NULL;

  //test a remote control bus adjustable link between 1 and 3 and controlling bus 4
  fname = std::string (TADJ_TEST_DIRECTORY "adj_test5.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage (st);
  BOOST_CHECK_GE (st[2], 0.99);
  BOOST_CHECK_LE (st[2], 1.011);

}

//now test the stepped MW control
BOOST_AUTO_TEST_CASE (adj_test_mw)
{
  std::string fname = std::string (TADJ_TEST_DIRECTORY "adj_test6.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  std::vector<double> st;
  gds->getLinkRealPower (st);

  BOOST_CHECK_LE (st[0], 1.05);
  BOOST_CHECK_GE (st[0], 0.95);

}

//now test the stepped MVar control
BOOST_AUTO_TEST_CASE (adj_test_mvar)
{
  std::string fname = std::string (TADJ_TEST_DIRECTORY "adj_test7.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds->pFlowInitialize();
  int mmatch = JacobianCheck(gds,cPflowSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds, cPflowSolverMode);
  }
  BOOST_REQUIRE_EQUAL(mmatch, 0);
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  std::vector<double> st;
  gds->getLinkReactivePower (st,0,2);

  BOOST_CHECK_LE (-st[0], 0.55);
  BOOST_CHECK_GE (-st[0], 0.50);

  fname = std::string (TADJ_TEST_DIRECTORY "adj_test8.xml");


  gds2 = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds2->powerflow ();
  BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds2->getLinkReactivePower (st,0,2);

  BOOST_CHECK_LE (-st[0], 1.1);
  BOOST_CHECK_GE (-st[0], 1.0);

}

BOOST_AUTO_TEST_CASE (adj_testcont_mvar)
{
  std::string fname = std::string (TADJ_TEST_DIRECTORY "adj_test7c.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds->pFlowInitialize();
  int mmatch = JacobianCheck(gds,cPflowSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds, cPflowSolverMode);
  }
  BOOST_REQUIRE_EQUAL(mmatch, 0);
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  std::vector<double> st;
  gds->getLinkReactivePower (st, 0, 2);

  BOOST_CHECK_CLOSE (st[0], -0.50,0.01);

  fname = std::string (TADJ_TEST_DIRECTORY "adj_test8c.xml");


  gds2 = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds2->powerflow ();
  BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds2->getLinkReactivePower (st, 0, 2);

  BOOST_CHECK_CLOSE (st[0], -1.05,0.01);

}

//now test the continuous Voltage control
BOOST_AUTO_TEST_CASE (adj_test_contV)
{
  std::string fname = std::string (TADJ_TEST_DIRECTORY "adj_test9.xml");

  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  gds->pFlowInitialize();
  int mmatch = JacobianCheck(gds,cPflowSolverMode);
  if (mmatch>0)
  {
    printStateNames(gds, cPflowSolverMode);
  }
  BOOST_REQUIRE_EQUAL(mmatch, 0);
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  std::vector<double> st;
  gds->getVoltage (st);

  BOOST_CHECK_CLOSE (st[2], 1.0, 0.00001);
  //test multiple continuous controllers
  fname = std::string (TADJ_TEST_DIRECTORY "adj_test10.xml");

  gds2 = (gridDynSimulation *)readSimXMLFile (fname);
  gds2->powerflow ();
  BOOST_REQUIRE (gds2->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds2->getVoltage (st);
  BOOST_CHECK_CLOSE (st[1], 1.0, 0.00001);
  BOOST_CHECK_CLOSE (st[2], 1.0, 0.00001);


}

BOOST_AUTO_TEST_SUITE_END()