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
#include "solvers/solverInterface.h"
#include "simulation/diagnostics.h"
#include "vectorOps.hpp"
#include <cstdio>
#include <iostream>


static std::string pFlow_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/pFlow_tests/");

BOOST_FIXTURE_TEST_SUITE (pFlow_tests, gridDynSimulationTestFixture)

/** test to make sure the basic power flow loads and runs*/
BOOST_AUTO_TEST_CASE (pFlow_test1)
{

  std::string fname = pFlow_test_directory+"test_powerflow3m9b.xml";
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_REQUIRE (gds->currentProcessState ()==gridDynSimulation::gridState_t::STARTUP);

  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState ()==gridDynSimulation::gridState_t::INITIALIZED);

  int count=gds->getInt("totalbuscount");
  
  BOOST_CHECK_EQUAL (count,9);
  //check the linkcount
  count=gds->getInt ("totallinkcount");
  BOOST_CHECK_EQUAL (count,9);
  BOOST_CHECK_EQUAL(runJacobianCheck(gds, cPflowSolverMode), 0);
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

}

//testcase for power flow from initial start
BOOST_AUTO_TEST_CASE (pFlow_test2)
{

  std::string fname = pFlow_test_directory + "test_powerflow3m9b2.xml";
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::STARTUP);
  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::INITIALIZED);

  std::vector<double> volts1;
  std::vector<double> ang1;
  std::vector<double> volts2;
  std::vector<double> ang2;

  gds->getVoltage (volts1);
  gds->getAngle (ang1);
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
  gds->getVoltage (volts2);
  gds->getAngle (ang2);
  //ensure the sizes are equal
  BOOST_REQUIRE_EQUAL (volts1.size (),volts2.size ());
  //check the bus voltages and angles
  auto vdiff = countDiffs(volts1,volts2,0.0001);
  auto adiff = countDiffs(ang1, ang2, 0.0001);
  
  BOOST_CHECK(vdiff == 0);
  BOOST_CHECK(adiff == 0);
}

//testcase for power flow from zeros start
BOOST_AUTO_TEST_CASE (pFlow_test3)
{

  std::string fname = pFlow_test_directory + "test_powerflow3m9b.xml";
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::STARTUP);
  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::INITIALIZED);


  std::string fname2 = pFlow_test_directory + "test_powerflow3m9b2.xml";
  gds2 = dynamic_cast<gridDynSimulation *> (readSimXMLFile (fname2));
  BOOST_REQUIRE (gds2->currentProcessState () ==gridDynSimulation::gridState_t::STARTUP);
  gds2->pFlowInitialize ();
  BOOST_REQUIRE (gds2->currentProcessState () ==gridDynSimulation::gridState_t::INITIALIZED);

  std::vector<double> volts1;
  std::vector<double> ang1;
  std::vector<double> volts2;
  std::vector<double> ang2;

  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
  gds->getVoltage (volts1);
  gds->getAngle (ang1);

  gds2->powerflow ();
  BOOST_REQUIRE (gds2->currentProcessState () ==gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
  gds2->getVoltage (volts2);
  gds2->getAngle (ang2);
  //ensure the sizes are equal
  BOOST_REQUIRE_EQUAL (volts1.size (),volts2.size ());
  //check the bus voltages and angles
  auto vdiff = countDiffs(volts1, volts2, 0.0001);
  auto adiff = countDiffs(ang1, ang2, 0.0001);
  BOOST_CHECK(vdiff == 0);
  BOOST_CHECK(adiff == 0);

}

/** test the ieee 30 bus case with no shunts*/
BOOST_AUTO_TEST_CASE (pflow_test30_no_shunt)
{
  gds = new gridDynSimulation ();
  std::string fname = ieee_test_directory+"ieee30_no_shunt_cap_tap_limit.cdf";

  loadCDF (gds, fname);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);


  int count = gds->getInt("totalbuscount");
  BOOST_CHECK_EQUAL (count, 30);
  //check the linkcount
  count = gds->getInt("totallinkcount");
  BOOST_CHECK_EQUAL (count, 41);
  std::vector<double> volts1;
  std::vector<double> ang1;
  std::vector<double> volts2;
  std::vector<double> ang2;

  gds->getVoltage (volts1);
  gds->getAngle (ang1);

  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);

  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage (volts2);
  gds->getAngle (ang2);

  auto vdiff = countDiffs(volts1, volts2, 0.001);
  auto adiff = countDiffs(ang1, ang2, 0.001);
 
  if ((vdiff > 0) || (adiff > 0))
  {
	  printBusResultDeviations(volts1, volts2, ang1, ang2);
  }
  BOOST_CHECK_EQUAL(vdiff, 0);
  BOOST_CHECK_EQUAL(adiff, 0);

  //check that the reset works correctly
  gds->reset(reset_levels::voltage_angle);
  gds->getAngle (ang1);
  for (size_t kk = 0; kk < ang1.size (); ++kk)
    {
      BOOST_CHECK_SMALL (ang1[kk], 0.000001);
    }

  gds->pFlowInitialize ();
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage (volts1);
  gds->getAngle (ang1);
  auto vdiff2 = countDiffs(volts1,volts2,0.0005);
  auto adiff2 = countDiffs(volts1, volts2, 0.0009);
  
  BOOST_CHECK_EQUAL(vdiff2, 0);
  BOOST_CHECK_EQUAL(adiff2, 0);

}

/** test the IEEE 30 bus case with no reactive limits*/
BOOST_AUTO_TEST_CASE (pflow_test30_no_limit)
{
  gds = new gridDynSimulation ();
  std::string fname = ieee_test_directory+"ieee30_no_limit.cdf";

  loadCDF (gds, fname);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);


  int count = gds->getInt("totalbuscount");
  BOOST_CHECK_EQUAL (count, 30);
  //check the linkcount
  count = gds->getInt("totallinkcount");
  BOOST_CHECK_EQUAL (count, 41);
  std::vector<double> volts1;
  std::vector<double> ang1;
  std::vector<double> volts2;
  std::vector<double> ang2;

  gds->getVoltage (volts1);
  gds->getAngle (ang1);

  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);

  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage (volts2);
  gds->getAngle (ang2);

  auto vdiff = countDiffs(volts1, volts2, 0.001);
  auto adiff = countDiffs(ang1, ang2, 0.01*kPI/180.0);  //0.01 degrees
  if ((vdiff > 0) || (adiff > 0))
  {
	  printBusResultDeviations(volts1, volts2, ang1, ang2);
  }
  BOOST_CHECK_EQUAL(vdiff, 0);
  BOOST_CHECK_EQUAL(adiff, 0);

  //check that the reset works correctly
  gds->reset(reset_levels::voltage_angle);
  gds->getAngle (ang1);
  for (size_t kk = 0; kk < ang1.size (); ++kk)
    {
      BOOST_CHECK_SMALL (ang1[kk], 0.000001);
    }
  gds->pFlowInitialize ();
  gds->powerflow ();

  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage (volts1);
  gds->getAngle (ang1);
  auto vdiff2 = countDiffs(volts1, volts2, 0.0005);
  auto adiff2 = countDiffs(volts1, volts2, 0.0009);

  BOOST_CHECK_EQUAL(vdiff2, 0);
  BOOST_CHECK_EQUAL(adiff2, 0);

}


//testcase for power flow automatic adjustsment
BOOST_AUTO_TEST_CASE (test_pFlow_padjust)
{

  std::string fname = pFlow_test_directory + "test_powerflow3m9b_Padjust.xml";
  gds = dynamic_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_REQUIRE (gds != nullptr);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);


  std::vector<double> P1;
  std::vector<double> P2;



  gds->pFlowInitialize ();
  gds->getBusGenerationReal (P1);
  gds->powerflow ();
  auto wc = gds->getInt("warncount");
  BOOST_CHECK(wc == 0);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
  gds->getBusGenerationReal (P2);

  double dk = P2[0] - P1[0];
  double diff;
  for (size_t kk = 1; kk < P2.size (); kk++)
    {
      if (P1[kk] > 0)
        {
          diff = std::abs (P2[kk] - P1[kk] - dk);
          BOOST_CHECK_SMALL (diff, 0.0002);
        }
    }


}

/** test case for dc power flow*/
BOOST_AUTO_TEST_CASE (pflow_test_dcflow)
{
  gds = new gridDynSimulation ();
  std::string fname = ieee_test_directory + "ieee30_no_limit.cdf";

  loadCDF (gds, fname);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);


  int count = gds->getInt("totalbuscount");
  BOOST_CHECK_EQUAL (count, 30);
  //check the linkcount
  count=gds->getInt ("totallinkcount");
  BOOST_CHECK_EQUAL (count, 41);
  std::vector<double> volts1;
  std::vector<double> ang1;
  std::vector<double> volts2;
  std::vector<double> ang2;

  gds->getVoltage (volts1);
  gds->getAngle (ang1);

  auto ns= makeSolver("kinsol");
  ns->set("name", "dcflow");
  ns->set("mode", "dc, algebraic");
  gds->add(ns);

  auto smode = gds->getSolverMode("dcflow");
  gds->set("defpowerflow", "dcflow");
  gds->pFlowInitialize(0);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);

  BOOST_CHECK_EQUAL(runJacobianCheck(gds, smode), 0);
 
  gds->powerflow();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);



}



//iterated power flow test case
BOOST_AUTO_TEST_CASE (test_iterated_pflow)
{

  std::string fname = pFlow_test_directory + "iterated_test_case.xml";
  gds = dynamic_cast<gridDynSimulation *> (readSimXMLFile(fname));
  BOOST_REQUIRE(gds != nullptr);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  gds->set("recorddirectory", pFlow_test_directory);
  gds->run();

  BOOST_REQUIRE (gds->getCurrentTime() >= 23);

}



/** test case for a floating bus ie a bus off a line with no load*/
BOOST_AUTO_TEST_CASE(pFlow_test_floating_bus)
{

  std::string fname = pFlow_test_directory + "test_powerflow3m9b_float.xml";
  gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));
  
  gds->pFlowInitialize();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);

  BOOST_CHECK_EQUAL(runJacobianCheck(gds, cPflowSolverMode), 0);
  
  gds->powerflow();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

}


BOOST_AUTO_TEST_CASE(pflow_test_line_modes)
{
	gds = new gridDynSimulation();
	std::string fname = ieee_test_directory+"ieee30_no_limit.cdf";

	loadCDF(gds, fname);
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);


	int count = gds->getInt("totalbuscount");
	BOOST_CHECK_EQUAL(count, 30);
	//check the linkcount
	count = gds->getInt("totallinkcount");
	BOOST_CHECK_EQUAL(count, 41);
	std::vector<double> volts1;
	std::vector<double> ang1;
	std::vector<double> volts2;
	std::vector<double> ang2;

	gds->getVoltage(volts1);
	gds->getAngle(ang1);

	stringVec approx_modes { "normal","simple","decoupled","fast_decoupled","simplified_decoupled","small_angle","small_angle_decoupled","small_angle_simplified","linear", };

	for (auto &am : approx_modes)
	{
		auto ns = makeSolver("kinsol");
		ns->set("name", am);
		ns->set("mode", "algebraic");
		int ret=ns->set("approx", am);
		if (ret != PARAMETER_FOUND)
		{
			std::cout << "unrecognized approx mode " << am << '\n';
		}
		BOOST_CHECK(ret == PARAMETER_FOUND);
		gds->add(ns);
		auto smode = gds->getSolverMode(am);
		gds->set("defpowerflow", am);
		gds->pFlowInitialize(0);
		BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);
		int errors = runJacobianCheck(gds, smode);
		BOOST_CHECK_EQUAL(errors, 0);
		if (errors > 0)
		{
			std::cout << "Errors in " << am << " mode" << '\n';
		}
		else
		{
			gds->powerflow();
			BOOST_CHECK(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
		}
		
	}


}

BOOST_AUTO_TEST_SUITE_END ()
