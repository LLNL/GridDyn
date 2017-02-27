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


#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include "solvers/solverInterface.h"
#include "vectorOps.hpp"
#include "core/coreExceptions.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#ifdef ALLOW_DATA_TEST_CASES
#include <boost/test/data/test_case.hpp>
#endif
#include <cstdio>
#include <iostream>

static std::string pFlow_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/pFlow_tests/");

BOOST_FIXTURE_TEST_SUITE (pFlow_tests, gridDynSimulationTestFixture)

/** test to make sure the basic power flow loads and runs*/
BOOST_AUTO_TEST_CASE (pFlow_test1)
{

  std::string fname = pFlow_test_directory+"test_powerflow3m9b.xml";
  gds = readSimXMLFile(fname);
  BOOST_REQUIRE (gds->currentProcessState ()==gridDynSimulation::gridState_t::STARTUP);

  gds->pFlowInitialize ();
  BOOST_REQUIRE (gds->currentProcessState ()==gridDynSimulation::gridState_t::INITIALIZED);

  int count=gds->getInt("totalbuscount");

  BOOST_CHECK_EQUAL (count,9);
  //check the linkcount
  count=gds->getInt ("totallinkcount");
  BOOST_CHECK_EQUAL (count,9);
  BOOST_CHECK_EQUAL(runJacobianCheck(gds, cPflowSolverMode,false), 0);
  gds->powerflow ();
  BOOST_REQUIRE (gds->currentProcessState () ==gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

}

//testcase for power flow from initial start
BOOST_AUTO_TEST_CASE (pFlow_test2)
{

  std::string fname = pFlow_test_directory + "test_powerflow3m9b2.xml";
  gds = readSimXMLFile(fname);
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
  gds = readSimXMLFile(fname);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (),gridDynSimulation::gridState_t::STARTUP);
  gds->pFlowInitialize ();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (),gridDynSimulation::gridState_t::INITIALIZED);


  std::string fname2 = pFlow_test_directory + "test_powerflow3m9b2.xml";
  gds2 = readSimXMLFile (fname2);
  BOOST_REQUIRE_EQUAL (gds2->currentProcessState (),gridDynSimulation::gridState_t::STARTUP);
  gds2->pFlowInitialize ();
  BOOST_REQUIRE_EQUAL (gds2->currentProcessState (),gridDynSimulation::gridState_t::INITIALIZED);

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
  gds = std::make_unique<gridDynSimulation>();
  std::string fname = ieee_test_directory+"ieee30_no_shunt_cap_tap_limit.cdf";

  loadCDF (gds.get(), fname);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::STARTUP);


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
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::INITIALIZED);

  gds->powerflow ();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage (volts2);
  gds->getAngle (ang2);

  auto vdiff = countDiffs(volts1, volts2, 0.001);
  auto adiff = countDiffs(ang1, ang2, 0.001);

  if ((vdiff > 0) || (adiff > 0))
  {
	  printBusResultDeviations(volts1, volts2, ang1, ang2);
  }
  BOOST_CHECK_EQUAL(vdiff, 0u);
  BOOST_CHECK_EQUAL(adiff, 0u);

  //check that the reset works correctly
  gds->reset(reset_levels::voltage_angle);
  gds->getAngle (ang1);
  for (size_t kk = 0; kk < ang1.size (); ++kk)
    {
      BOOST_CHECK_SMALL (ang1[kk], 0.000001);
    }

  gds->pFlowInitialize ();
  gds->powerflow ();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage (volts1);
  gds->getAngle (ang1);
  auto vdiff2 = countDiffs(volts1,volts2,0.0005);
  auto adiff2 = countDiffs(volts1, volts2, 0.0009);

  BOOST_CHECK_EQUAL(vdiff2, 0u);
  BOOST_CHECK_EQUAL(adiff2, 0u);

}

/** test the IEEE 30 bus case with no reactive limits*/
BOOST_AUTO_TEST_CASE (pflow_test30_no_limit)
{
  gds = std::make_unique<gridDynSimulation> ();
  std::string fname = ieee_test_directory+"ieee30_no_limit.cdf";

  loadCDF (gds.get(), fname);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::STARTUP);


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
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::INITIALIZED);

  gds->powerflow ();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

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

  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage (volts1);
  gds->getAngle (ang1);
  auto vdiff2 = countDiffs(volts1, volts2, 0.0005);
  auto adiff2 = countDiffs(volts1, volts2, 0.0009);

  BOOST_CHECK_EQUAL(vdiff2, 0);
  BOOST_CHECK_EQUAL(adiff2, 0);

}


//test case for power flow automatic adjustment
BOOST_AUTO_TEST_CASE (test_pFlow_padjust)
{

  std::string fname = pFlow_test_directory + "test_powerflow3m9b_Padjust.xml";
  gds = readSimXMLFile (fname);
  BOOST_REQUIRE (gds != nullptr);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::STARTUP);


  std::vector<double> P1;
  std::vector<double> P2;



  gds->pFlowInitialize ();
  gds->getBusGenerationReal (P1);
  gds->powerflow ();
  auto wc = gds->getInt("warncount");
  BOOST_CHECK_EQUAL(wc,0);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
  gds->getBusGenerationReal (P2);
	//there should be 2 generators that had their real power levels adjusted instead of just the swing bus
  auto cnt = countDiffs(P2, P1, 0.0002);
  BOOST_CHECK_EQUAL(cnt, 2);


}

/** test case for dc power flow*/
BOOST_AUTO_TEST_CASE (pflow_test_dcflow)
{
  gds = std::make_unique<gridDynSimulation> ();
  std::string fname = ieee_test_directory + "ieee30_no_limit.cdf";

  loadCDF (gds.get(), fname);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::STARTUP);


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
  ns->setName("dcflow");
  ns->set("mode", "dc, algebraic");
  gds->add(std::move(ns));

  auto smode = gds->getSolverMode("dcflow");
  gds->set("defpowerflow", "dcflow");
  gds->pFlowInitialize(0.0);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::INITIALIZED);

  runJacobianCheck(gds, smode);

  gds->powerflow();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);



}



//iterated power flow test case
BOOST_AUTO_TEST_CASE (test_iterated_pflow)
{

  std::string fname = pFlow_test_directory + "iterated_test_case.xml";
  gds = readSimXMLFile(fname);
  BOOST_REQUIRE(gds != nullptr);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::STARTUP);
  gds->consolePrintLevel = print_level::no_print;
  gds->set("recorddirectory", pFlow_test_directory);
  gds->run();
  BOOST_REQUIRE (gds->getCurrentTime() >= 575.0);

}



/** test case for a floating bus ie a bus off a line with no load*/
BOOST_AUTO_TEST_CASE(pFlow_test_floating_bus)
{

  std::string fname = pFlow_test_directory + "test_powerflow3m9b_float.xml";
  gds = readSimXMLFile(fname);

  gds->pFlowInitialize();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::INITIALIZED);

  runJacobianCheck(gds, cPflowSolverMode);

  gds->powerflow();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

}
/*
//TODO figure out how to use the DATASET test cases
BOOST_AUTO_TEST_CASE(pflow_test_line_modes)
{
	gds = new gridDynSimulation();
	std::string fname = ieee_test_directory+"ieee30_no_limit.cdf";

	loadCDF(gds, fname);

	BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::STARTUP);


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
		BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::INITIALIZED);
		int errors = runJacobianCheck(gds, smode,false);
		BOOST_REQUIRE_MESSAGE(errors==0, "Errors in " << am << " mode");

		gds->powerflow();
		BOOST_CHECK(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	}


}
*/


  static stringVec approx_modes{ "normal","simple","decoupled","fast_decoupled","simplified_decoupled","small_angle","small_angle_decoupled","small_angle_simplified","linear", };

//static stringVec approx_modes{ "small_angle_decoupled" };
#ifdef ALLOW_DATA_TEST_CASES
namespace data = boost::unit_test::data;

//this test case should be moved towards requiring fixture support withe _F version but I don't want to mandate boost 1.61 yet
BOOST_DATA_TEST_CASE(pflow_test_line_modes, data::make(approx_modes),approx)
{
	auto gdsb = std::make_unique<gridDynSimulation>();
	std::string fname = ieee_test_directory+"ieee30_no_limit.cdf";

	loadCDF(gdsb.get(), fname);

	BOOST_REQUIRE_EQUAL (gdsb->currentProcessState () ,gridDynSimulation::gridState_t::STARTUP);


	int count = gdsb->getInt("totalbuscount");
	BOOST_CHECK_EQUAL(count, 30);
	//check the linkcount
	count = gdsb->getInt("totallinkcount");
	BOOST_CHECK_EQUAL(count, 41);
	std::vector<double> volts1;
	std::vector<double> ang1;
	std::vector<double> volts2;
	std::vector<double> ang2;

	gdsb->getVoltage(volts1);
	gdsb->getAngle(ang1);

		auto ns = makeSolver("kinsol");
		ns->set("name", approx);
		try
		{
			ns->set("approx", approx);
		}
		catch (const invalidParameterValue &)
		{
			BOOST_CHECK_MESSAGE(false, "unrecognized approx mode " << approx);
		}

		
		gdsb->add(std::move(ns));
		auto smode = gdsb->getSolverMode(approx);
		gdsb->set("defpowerflow", approx);
		gdsb->pFlowInitialize(0.0);
		BOOST_REQUIRE_EQUAL (gdsb->currentProcessState () ,gridDynSimulation::gridState_t::INITIALIZED);
		int errors;
			if (approx == "small_angle_decoupled")
			{
				/*there is an initialization difference in this approximation due to the combination of decoupling
				and the small angle approximation.  In decoupled modes the values from the voltages and angles are fixed
				for the calculation of the decoupled quantities but are saved with the full trig calculation.  in the small angle
				approximation this is not done so there is a small error when evaluating the Jacobian right after the power flow initialization
				since the approximation has not yet been made.  All other modes and times do not have this issue*/
				errors = runJacobianCheck(gdsb, smode,0.05,false);
			}
			else
			{
				errors=runJacobianCheck(gdsb, smode,false);
			}
		BOOST_REQUIRE_MESSAGE(errors==0, "Errors in " << approx << " mode");

		gdsb->powerflow();
		BOOST_REQUIRE_EQUAL(gdsb->currentProcessState(),gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

}
#endif 

BOOST_AUTO_TEST_SUITE_END ()
