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
#include "GhostSwingBusManager.h"
#include "loadModels/gridLabDLoad.h"
#include <cstdio>

#define GRIDLAB_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/gridlabD_tests/"

BOOST_FIXTURE_TEST_SUITE (gridlabd_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (gridlab_test1)
{
  int argc = 0;
  GhostSwingBusManager::Initialize (&argc, nullptr);
  GhostSwingBusManager::SetDebug (false);
  std::string fname = std::string (GRIDLAB_TEST_DIRECTORY "Simple_3Bus_mod.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  int glb = gds->countMpiObjects ();
  BOOST_CHECK_EQUAL (glb, 1);

  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);


  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
  gridLoad *ld = static_cast<gridLoad *> (gds->find ("bus2::gload1"));

  BOOST_REQUIRE (ld != nullptr);
  double val = ld->get ("p");
  BOOST_CHECK_CLOSE (val, 0.9, 0.01);
  val = ld->get ("yp");
  BOOST_CHECK_SMALL (val,  0.000001);
  val = ld->get ("yq");
  BOOST_CHECK_SMALL (val,  0.000001);
  val = ld->get ("ip");
  BOOST_CHECK_SMALL (val, 0.000001);
  val = ld->get ("iq");
  BOOST_CHECK_SMALL (val,  0.000001);
  val = ld->get ("q");
  BOOST_CHECK_CLOSE (val, 0.27, 0.01);



  GhostSwingBusManager::Instance ()->endSimulation ();
}

BOOST_AUTO_TEST_CASE (gridlab_test2)
{
  int argc = 0;
  GhostSwingBusManager::Initialize (&argc, nullptr);
  GhostSwingBusManager::SetDebug (false);
  std::string fname = std::string (GRIDLAB_TEST_DIRECTORY "Simple_3Bus_mod3x.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  int glb = gds->countMpiObjects ();
  BOOST_CHECK_EQUAL (glb, 3);

  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);


  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
  gridLoad *ld = static_cast<gridLoad *> (gds->find ("bus2::gload2"));

  //P = 0.27 Q = -0.1 Ir = 0.34 Iq = -0.13
  BOOST_REQUIRE (ld != nullptr);
  double val = ld->get ("p");
  BOOST_CHECK_CLOSE (val, 0.3, 0.01);
  val = ld->get ("yp");
  BOOST_CHECK_SMALL (val, 0.000001);
  val = ld->get ("yq");
  BOOST_CHECK_SMALL (val, 0.000001);
  val = ld->get ("ip");
  BOOST_CHECK_SMALL (val, 0.000001);
  val = ld->get ("iq");
  BOOST_CHECK_SMALL (val, 0.000001);
  val = ld->get ("q");
  BOOST_CHECK_CLOSE (val, 0.09, 0.01);

  gds->run (30.0);
  GhostSwingBusManager::Instance ()->endSimulation ();
}


BOOST_AUTO_TEST_CASE (gridlab_test3)
{
  int argc = 0;
  GhostSwingBusManager::Initialize (&argc, nullptr);
  GhostSwingBusManager::SetDebug (false);
  std::string fname = std::string (GRIDLAB_TEST_DIRECTORY "Simple_3Bus_mod3x_current.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  int glb = gds->countMpiObjects ();
  BOOST_CHECK_EQUAL (glb, 3);

  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);


  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
  gridLoad *ld = static_cast<gridLoad *> (gds->find ("bus2::gload2"));

  //P = 0.27 Q = -0.1 Ir = 0.34 Iq = -0.13
  BOOST_REQUIRE (ld != nullptr);
  double val = ld->get ("p");
  BOOST_CHECK_SMALL (val, 0.000001);
  val = ld->get ("yp");
  BOOST_CHECK_SMALL (val, 0.000001);
  val = ld->get ("yq");
  BOOST_CHECK_SMALL (val, 0.000001);
  val = ld->get ("ip");
  BOOST_CHECK_CLOSE (val, 0.3, 0.01);
  val = ld->get ("iq");
  BOOST_CHECK_CLOSE (val, 0.09, 0.01);
  val = ld->get ("q");
  BOOST_CHECK_SMALL (val, 0.000001);

  gds->run (30.0);
  GhostSwingBusManager::Instance ()->endSimulation ();
}

BOOST_AUTO_TEST_CASE (test_gridlabArray)
{
  //test the define functionality
  int argc = 0;
  std::string fname = std::string (GRIDLAB_TEST_DIRECTORY "Simple_3Bus_mod3x_mix_scale.xml");
  GhostSwingBusManager::Initialize (&argc, nullptr);
  GhostSwingBusManager::SetDebug (false);
  readerInfo ri;
  ri.keepdefines = true;
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname,&ri));
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  int glb = gds->countMpiObjects ();
  int cnt = 60;
  std::string b = ri.checkDefines ("garraySize");
  if (b != "garraySize")
    {
      cnt = 3 * std::stoi (b);
    }
  b = ri.checkDefines ("outerSize");
  if (b != "outerSize")
    {
      cnt = cnt * std::stoi (b);
    }
  BOOST_CHECK_EQUAL (glb, cnt);
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->run ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}
BOOST_AUTO_TEST_SUITE_END ()
