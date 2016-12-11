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
#include <boost/filesystem.hpp>
#include "gridDyn.h"
#include "fmi_import/fmiImport.h"
#include "fmi_import/fmiObjects.h"
#include "vectorOps.hpp"
#include "fmiGDinfo.h"
#include "gridBus.h"
#include "simulation/diagnostics.h"

//test case for coreObject object

#include "testHelper.h"
#include "fmi_models/fmiLoad.h"


static const std::string fmi_test_directory(GRIDDYN_TEST_DIRECTORY "/fmi_tests/");
static const std::string fmu_directory(GRIDDYN_TEST_DIRECTORY "/fmi_tests/test_fmus/");

//create a test fixture that makes sure everything gets deleted properly

using namespace boost::filesystem;
BOOST_FIXTURE_TEST_SUITE (fmi_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE(test_fmi_xml)
{
	std::string fmu = fmu_directory + "rectifier.fmu";
	path rectDir(fmu_directory + "rectifier");
	fmiLibrary rectFmu(fmu);
	BOOST_CHECK(rectFmu.isXmlLoaded());
	BOOST_CHECK(rectFmu.getCounts("states") == 4);
	BOOST_CHECK(rectFmu.getCounts("outputs") == 8);
	BOOST_CHECK(rectFmu.getCounts("parameters") == 23);
	BOOST_CHECK(rectFmu.getCounts("unit") == 13);
	auto vars = rectFmu.getCounts("variables");
	BOOST_CHECK(vars == 190);
	BOOST_CHECK(rectFmu.getCounts("locals") == vars-23-8);
	rectFmu.close();
	BOOST_REQUIRE_MESSAGE(exists(rectDir),"fmu directory does not exist");
	try
	{
		remove_all(rectDir);
	}
	catch (filesystem_error const &e)
	{
		BOOST_WARN_MESSAGE(false, "unable to remove the folder: " + std::string(e.what()));
	}

	//test second fmu with different load mechanics
	std::string fmu2 = fmu_directory + "rectifier2.fmu";
	path rectDir2(fmu_directory + "rectifier2");
	fmiLibrary rect2Fmu(fmu2,rectDir2.string());
	BOOST_CHECK(rect2Fmu.isXmlLoaded());
	BOOST_CHECK(rect2Fmu.getCounts("states") == 4);
	BOOST_CHECK(rect2Fmu.getCounts("outputs") == 1);
	BOOST_CHECK(rect2Fmu.getCounts("parameters") == 0);
	BOOST_CHECK(rect2Fmu.getCounts("units") == 2);
	vars = rect2Fmu.getCounts("variables");
	BOOST_CHECK(vars == 61);
	BOOST_CHECK(rect2Fmu.getCounts("locals") == vars - 2);
	
	rect2Fmu.close();
	BOOST_REQUIRE_MESSAGE(exists(rectDir2), "fmu directory does not exist");
	try
	{
		remove_all(rectDir2);
	}
	catch (filesystem_error const &e)
	{
		BOOST_WARN_MESSAGE(false, "unable to remove the folder: " + std::string(e.what()));
	}


}

#ifdef _WIN32
BOOST_AUTO_TEST_CASE(test_fmi_load_shared)
{
	std::string fmu = fmu_directory + "rectifier.fmu";
	path rectDir(fmu_directory + "rectifier");
	fmiLibrary rectFmu(fmu);
	rectFmu.loadSharedLibrary();
	BOOST_REQUIRE(rectFmu.isSoLoaded());

	auto b = rectFmu.createModelExchangeObject("rctf");
	BOOST_REQUIRE(b);
	b->setMode(fmuMode::initializationMode);
	auto v = b->get<double>("VAC");
	BOOST_CHECK_CLOSE(v, 400.0, 0.0001);
	auto phase = b->get<double>("SineVoltage1.phase");
	BOOST_CHECK_CLOSE(phase, 0.0, 0.0001);

	b->set("VAC", 394.23);
	v = b->get<double>("VAC");
	BOOST_CHECK_CLOSE(v, 394.23, 0.0001);


	try
	{
		remove_all(rectDir);
	}
	catch (filesystem_error const &e)
	{
		BOOST_WARN_MESSAGE(false, "unable to remove the folder: " + std::string(e.what()));
	}
}

#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
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

#endif


BOOST_AUTO_TEST_SUITE_END ()
