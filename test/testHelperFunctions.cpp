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

#include "testHelper.h"
#include "loadModels/gridLoad.h"
#include "gridDyn.h"
#include "simulation/diagnostics.h"
#include "gridDynFileInput.h"
#include <iostream>
#include <cmath>
#include <boost/test/unit_test.hpp>

gridDynSimulationTestFixture::gridDynSimulationTestFixture()
{
	readerConfig::setPrintMode(0);
}

gridDynSimulationTestFixture::~gridDynSimulationTestFixture()
{

}

std::ostream& operator<<(std::ostream& os, const gridDynSimulation::gridState_t state)
{
	switch (state)
	{
	case gridDynSimulation::gridState_t::STARTUP:
		os << "startup";
		break;
	case gridDynSimulation::gridState_t::INITIALIZED:
		os << "initialized";
		break;
	case gridDynSimulation::gridState_t::POWERFLOW_COMPLETE:
		os << "powerflow_complete";
		break;
	case gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED:
		os << "dynamic init";
		break;
	case gridDynSimulation::gridState_t::DYNAMIC_PARTIAL:
		os << "dynamic partial";
		break;
	case gridDynSimulation::gridState_t::DYNAMIC_COMPLETE:
		os << "dynamic complete";
		break;
	case gridDynSimulation::gridState_t::GD_ERROR:
		os << "error";
		break;
	case gridDynSimulation::gridState_t::HALTED:
		os << "halted";
		break;
	}

	return os;
}

void gridDynSimulationTestFixture::simpleRunTestXML(const std::string &fileName)
{
	gds = readSimXMLFile(fileName);
	gds->consolePrintLevel = print_level::no_print;
	gds->run();
	BOOST_REQUIRE_EQUAL(gds->currentProcessState(), gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}


void gridDynSimulationTestFixture::simpleStageCheck(const std::string &fileName, gridDynSimulation::gridState_t finalState)
{
	readerConfig::setPrintMode(0);
	int retval = 0;
	gds = readSimXMLFile(fileName);
	BOOST_REQUIRE_EQUAL(readerConfig::warnCount,0);
	BOOST_REQUIRE_EQUAL(gds->currentProcessState(),gridDynSimulation::gridState_t::STARTUP);
	switch (finalState)
	{
	case gridDynSimulation::gridState_t::STARTUP:
		return;
	case gridDynSimulation::gridState_t::INITIALIZED:
		retval = gds->pFlowInitialize();
		BOOST_CHECK_EQUAL(retval, 0);
		return;
	case gridDynSimulation::gridState_t::POWERFLOW_COMPLETE:
		retval = gds->powerflow();
		BOOST_CHECK_EQUAL(retval, 0);
		return;
	case gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED:
		retval = gds->dynInitialize();
		BOOST_CHECK_EQUAL(retval, 0);
		return;
	case gridDynSimulation::gridState_t::DYNAMIC_COMPLETE:
		BOOST_CHECK_EQUAL(gds->run(), 0);
		return;
	default:
		gds->run();
		BOOST_CHECK_EQUAL(gds->currentProcessState(),finalState);
		return;
	}
}

void gridDynSimulationTestFixture::detailedStageCheck(const std::string &fileName, gridDynSimulation::gridState_t finalState)
{
	readerConfig::setPrintMode(0);
	gds = readSimXMLFile(fileName);
	BOOST_REQUIRE_EQUAL(readerConfig::warnCount,0);
	BOOST_REQUIRE_EQUAL(gds->currentProcessState(),gridDynSimulation::gridState_t::STARTUP);
	int retval = gds->pFlowInitialize();
	BOOST_CHECK_EQUAL(retval, 0);
	BOOST_REQUIRE_EQUAL(gds->currentProcessState(),gridDynSimulation::gridState_t::INITIALIZED);
	runJacobianCheck(gds, cPflowSolverMode);

	if (finalState == gridDynSimulation::gridState_t::INITIALIZED)
	{
		return;
	}
	gds->powerflow();

	BOOST_REQUIRE_EQUAL(gds->currentProcessState(),gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

	if (finalState == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE)
	{
		return;
	}
	gds->dynInitialize();
	runResidualCheck(gds, cDaeSolverMode);
	
	runJacobianCheck(gds, cDaeSolverMode);
	
	BOOST_REQUIRE_EQUAL(gds->currentProcessState(),gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
	if (finalState == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED)
	{
		return;
	}
	retval = gds->run();
	BOOST_REQUIRE_EQUAL(retval,0);
	if (gds->hasDynamics())
	{
		BOOST_REQUIRE_EQUAL(gds->currentProcessState(),gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
	}
	else
	{
		BOOST_REQUIRE_EQUAL(gds->currentProcessState(),gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	}
}

void gridDynSimulationTestFixture::dynamicInitializationCheck(const std::string &fileName)
{
	readerConfig::setPrintMode(0);
	gds = readSimXMLFile(fileName);

	int  retval = gds->dynInitialize();

	BOOST_CHECK_EQUAL(retval, 0);

	int mmatch = runJacobianCheck(gds, cDaeSolverMode);

	BOOST_REQUIRE_EQUAL(mmatch, 0);
	mmatch = runResidualCheck(gds, cDaeSolverMode);
	
	BOOST_REQUIRE_EQUAL(mmatch, 0);
}

gridLoadTestFixture::gridLoadTestFixture()
{
	readerConfig::setPrintMode(0);
}

gridLoadTestFixture::~gridLoadTestFixture()
{
  if (ld1)
    {
      delete ld1;
    }

  if (ld2)
    {
      delete ld2;
    }
}


glbconfig::glbconfig()
{

}

glbconfig::~glbconfig()
{
#ifdef _CRTDBG_MAP_ALLOC
	_CrtDumpMemoryLeaks();
#endif
}


int runJacobianCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode,bool checkRequired)
{
	int mmatch = JacobianCheck(gds.get(), sMode);
	if (mmatch > 0)
	{
		printStateNames(gds.get(), sMode);
		if (checkRequired)
		{
			BOOST_REQUIRE_EQUAL(mmatch, 0);
		}
	}
	return mmatch;
}

int runJacobianCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode, double tol, bool checkRequired)
{
	int mmatch = JacobianCheck(gds.get(), sMode,tol);
	if (mmatch > 0)
	{
		printStateNames(gds.get(), sMode);
		if (checkRequired)
		{
			BOOST_REQUIRE_EQUAL(mmatch, 0);
		}
	}
	return mmatch;
}

int runResidualCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode, bool checkRequired)
{
	int mmatch = residualCheck(gds.get(), sMode);
	if (mmatch > 0)
	{
		printStateNames(gds.get(), sMode);
		if (checkRequired)
		{
			BOOST_REQUIRE_EQUAL(mmatch, 0);
		}
	}
	
	return mmatch;
}

int runDerivativeCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode, bool checkRequired)
{
	int mmatch = derivativeCheck(gds.get(), gds->getCurrentTime(),sMode);
	if (mmatch > 0)
	{
		printStateNames(gds.get(), sMode);
		if (checkRequired)
		{
			BOOST_REQUIRE_EQUAL(mmatch, 0);
		}
	}
	return mmatch;
}

int runAlgebraicCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode, bool checkRequired)
{
	int mmatch = algebraicCheck(gds.get(), gds->getCurrentTime(), sMode);
	if (mmatch > 0)
	{
		printStateNames(gds.get(), sMode);
		if (checkRequired)
		{
			BOOST_REQUIRE_EQUAL(mmatch, 0);
		}
		
	}
	return mmatch;
}

void printBusResultDeviations(const std::vector<double> &V1, const std::vector<double> &V2, const std::vector<double> &A1, const std::vector<double> &A2)
{
	for (size_t kk = 0; kk < V1.size(); ++kk)
	{
		if ((std::abs(V1[kk] - V2[kk]) > 0.0001)|| (std::abs(A1[kk] - A2[kk]) > 0.0001))
		{
			std::cout << "Bus " << kk + 1 << "::" << V1[kk] << "vs." << V2[kk] << "::" << A1[kk] * 180.0 / kPI << "vs." << A2[kk] * 180.0 / kPI << "::" << V1[kk] - V2[kk] << ',' << (A1[kk] - A2[kk])*180.0 / kPI << "\n";
		}
	}
}
