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
#include "vectorOps.hpp"
#include "fmiGDinfo.h"
#include "fmiSubModel.h"
#include "fmiLoad.h"
#include "gridBus.h"
#include "diagnostics.h"

//test case for gridCoreObject object

#include "testHelper.h"

#define FMI_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/fmi_tests//"
#define FMU_LOC GRIDDYN_TEST_DIRECTORY "..//..//fmus//"
//create a test fixture that makes sure everything gets deleted properly

BOOST_FIXTURE_TEST_SUITE (fmi_tests,F)

BOOST_AUTO_TEST_CASE (fmi_test1)
{
	loadFmiLibrary();
  //test the loading of a single bus
	auto tsb = new fmiLoad(FMU_LOC "ACMotorFMU.fmu");
	tsb->set("a_in", "#");
	stringVec b;
	tsb->getParameterStrings(b, paramStringType::numeric);
	tsb->set("a_in", 0.7);
	tsb->pFlowInitializeA(0, 0);
	auto outP = tsb->getRealPower();
	auto outQ = tsb->getReactivePower();
	tsb->dynInitializeA(0, 0);
	IOdata oset;
	tsb->dynInitializeB({ 1.0, 0 }, oset);

	auto out=tsb->timestep(6, { 1.0, 0 }, cLocalSolverMode);
	outP = tsb->getRealPower();
	outQ = tsb->getReactivePower();
	BOOST_CHECK_CLOSE(out, 0.7, 0.001);
	tsb->set("a_in", 0.9);
	out = tsb->timestep(14, { 1.0, 0 }, cLocalSolverMode);
	outP = tsb->getRealPower();
	outQ = tsb->getReactivePower();
	BOOST_CHECK_CLOSE(out, 0.9, 0.001);
	tsb->set("a_in", 0.6);
	out = tsb->timestep(22, { 1.0, 0 }, cLocalSolverMode);
	outP = tsb->getRealPower();
	outQ = tsb->getReactivePower();
	BOOST_CHECK_CLOSE(out, 0.6, 0.001);
	delete tsb;
}

BOOST_AUTO_TEST_CASE(fmi_xml1)
{
	std::string fname = std::string(FMI_TEST_DIRECTORY "fmimotorload_test1.xml");

	readerConfig::setPrintMode(0);
	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));

	int retval = gds->pFlowInitialize();
	BOOST_CHECK_EQUAL(retval, 0);

	
		int mmatch = JacobianCheck(gds,cPflowSolverMode);
		if (mmatch > 0)
		{

			printStateNames(gds, cPflowSolverMode);
		}
		BOOST_REQUIRE_EQUAL(mmatch, 0);
	gds->powerflow();

	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	gds->dynInitialize();
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
	std::vector<double> v;
	gds->getVoltage(v);
	mmatch = residualCheck(gds,cDaeSolverMode);
	if (mmatch > 0)
	{

		printStateNames(gds, cDaeSolverMode);
	}
	BOOST_REQUIRE_EQUAL(mmatch, 0);
	mmatch = JacobianCheck(gds,cDaeSolverMode);
	if (mmatch > 0)
	{

		printStateNames(gds, cDaeSolverMode);
	}
	BOOST_REQUIRE_EQUAL(mmatch, 0);

	gds->run();
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}

BOOST_AUTO_TEST_CASE(fmi_xml2)
{
	std::string fname = std::string(FMI_TEST_DIRECTORY "fmiload_test2.xml");

	readerConfig::setPrintMode(0);
	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));

	int retval = gds->pFlowInitialize();
	BOOST_CHECK_EQUAL(retval, 0);


	int mmatch = JacobianCheck(gds,cPflowSolverMode);
	if (mmatch > 0)
	{

		printStateNames(gds, cPflowSolverMode);
	}
	BOOST_REQUIRE_EQUAL(mmatch, 0);
	gds->powerflow();

	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	gds->dynInitialize();
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
	std::vector<double> v;
	gds->getVoltage(v);
	mmatch = residualCheck(gds,cDaeSolverMode);
	if (mmatch > 0)
	{

		printStateNames(gds, cDaeSolverMode);
	}
	BOOST_REQUIRE_EQUAL(mmatch, 0);
	mmatch = JacobianCheck(gds,cDaeSolverMode);
	if (mmatch > 0)
	{

		printStateNames(gds, cDaeSolverMode);
	}
	BOOST_REQUIRE_EQUAL(mmatch, 0);

	gds->run();
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}


BOOST_AUTO_TEST_CASE(fmi_xml3)
{
	std::string fname = std::string(FMI_TEST_DIRECTORY "fmiload_test3.xml");

	readerConfig::setPrintMode(0);
	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));

	int retval = gds->pFlowInitialize();
	BOOST_CHECK_EQUAL(retval, 0);


	int mmatch = JacobianCheck(gds,cPflowSolverMode);
	if (mmatch > 0)
	{

		printStateNames(gds, cPflowSolverMode);
	}
	BOOST_REQUIRE_EQUAL(mmatch, 0);
	gds->powerflow();

	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	gds->dynInitialize();
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
	std::vector<double> v;
	gds->getVoltage(v);
	mmatch = residualCheck(gds,cDaeSolverMode);
	if (mmatch > 0)
	{

		printStateNames(gds, cDaeSolverMode);
	}
	BOOST_REQUIRE_EQUAL(mmatch, 0);
	mmatch = JacobianCheck(gds,cDaeSolverMode);
	if (mmatch > 0)
	{

		printStateNames(gds, cDaeSolverMode);
	}
	BOOST_REQUIRE_EQUAL(mmatch, 0);

	gds->run();
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}

BOOST_AUTO_TEST_CASE(fmi_array)
{
	std::string fname = std::string(FMI_TEST_DIRECTORY "block_grid.xml");

	readerConfig::setPrintMode(0);
	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));

	int retval = gds->pFlowInitialize();
	BOOST_CHECK_EQUAL(retval, 0);

	int cnt = gds->getInt("totalbuscount");
	if (cnt<200)
	{

		int mmatch = JacobianCheck(gds,cPflowSolverMode);
		if (mmatch > 0)
		{

			printStateNames(gds, cPflowSolverMode);
		}
		BOOST_REQUIRE_EQUAL(mmatch, 0);
	}
	gds->powerflow();

	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	gds->dynInitialize();
	BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
	std::vector<double> v;
	gds->getVoltage(v);

	auto bus = static_cast<gridBus *>(gds->find("bus_1_1"));
	printf("slk bus p=%f min v= %f\n", bus->getGenerationReal(), *std::min_element(v.begin(), v.end()));
}




BOOST_AUTO_TEST_SUITE_END ()
