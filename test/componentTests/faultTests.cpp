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
#include "objectFactory.h"
#include "generators/gridDynGenerator.h"
#include "submodels/gridDynGenModel.h"
#include "submodels/gridDynExciter.h"
#include "submodels/gridDynGovernor.h"
#include "simulation/diagnostics.h"
#include "relays/fuse.h"
#include "relays/breaker.h"
#include "linkModels/acLine.h"
#include "gridBus.h"
#include <cmath>

/** these test cases test out the various generator components ability to handle faults
*/

static const std::string fault_test_directory( GRIDDYN_TEST_DIRECTORY "/fault_tests/" );

BOOST_FIXTURE_TEST_SUITE(fault_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE(fault_test1)
{
	std::string fname = fault_test_directory+"fault_test1.xml";

	
	auto cof = coreObjectFactory::instance();
	gridCoreObject *obj = nullptr;
	
	auto genlist = cof->getTypeNames("genmodel");

	for (auto &gname : genlist)
	{
		gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
		gds->consolePrintLevel = 0;
		obj = cof->createObject("genmodel", gname);
		BOOST_REQUIRE(obj != nullptr);

		gridDynGenerator *gen = gds->getGen(0);
		gen->add(obj);

		//run till just after the fault
		gds->run(1.01);
		if (gds->hasDynamics())
		{
			BOOST_CHECK_MESSAGE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE, "Model " << gname << " failed to run past fault");
			auto mmatch = runJacobianCheck(gds, cDaeSolverMode);
			BOOST_CHECK_MESSAGE(mmatch==0, "Model " << gname << " Jacobian failure after fault");
		}
		else
		{
			delete gds;
			gds = nullptr;
			continue;
		}
		
		//run till just after the fault clears
		gds->run(1.2);

			BOOST_CHECK_MESSAGE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE, "Model " << gname << " failed to run past fault clear");
			auto mmatch = runJacobianCheck(gds, cDaeSolverMode);
			
			BOOST_CHECK_MESSAGE(mmatch==0, "Model " << gname << " Jacobian failure");
		
		gds->run();
		BOOST_CHECK_MESSAGE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,"Model "<<gname<<" failed to run to completion");


		std::vector<double> volts;
		gds->getVoltage(volts);
		
		BOOST_CHECK_MESSAGE(volts[1] > 0.96,"Model "<<gname<<" voltage issue v="<<volts[1]);
		delete gds;
		gds = nullptr;
	}
}

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
//testing with an exciter added
BOOST_AUTO_TEST_CASE(fault_test2)
{
	std::string fname = fault_test_directory + "fault_test1.xml";


	auto cof = coreObjectFactory::instance();
	auto genlist = cof->getTypeNames("genmodel");

	for (auto &gname : genlist)
	{
		
		gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
		gds->consolePrintLevel = GD_NO_PRINT;
		auto obj = cof->createObject("genmodel", gname);
		BOOST_REQUIRE(obj != nullptr);

		gridDynGenerator *gen = gds->getGen(0);
		gen->add(obj);

		auto exc = cof->createObject("exciter", "type1");
		gen->add(exc);
		//run till just after the fault
		gds->run(1.01);
		
			BOOST_CHECK_MESSAGE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE, "Model " << gname << " failed to run past fault");
			auto mmatch = runJacobianCheck(gds, cDaeSolverMode);
			BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure after fault");
		

		//run till just after the fault clears
		gds->run(1.2);
		
			BOOST_CHECK_MESSAGE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE, "Model " << gname << " failed to run past fault clear");
			mmatch = runJacobianCheck(gds, cDaeSolverMode);

			BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure");
	

		gds->run();
		BOOST_CHECK_MESSAGE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE, "Model " << gname << " failed to run to completion");
		
		std::vector<double> volts;
		gds->getVoltage(volts);

		BOOST_CHECK_MESSAGE(volts[1] > 0.96, "Model " << gname << " voltage issue v=" << volts[1]);
		delete gds;
		gds = nullptr;
	}
}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
//testing with a governor added
BOOST_AUTO_TEST_CASE(fault_test3)
{
	std::string fname = fault_test_directory + "fault_test1.xml";


	auto cof = coreObjectFactory::instance();
	auto genlist = cof->getTypeNames("genmodel");

	for (auto &gname : genlist)
	{
		
		gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
		gds->consolePrintLevel = GD_NO_PRINT;
		auto obj = cof->createObject("genmodel", gname);
		BOOST_REQUIRE(obj != nullptr);

		gridDynGenerator *gen = gds->getGen(0);
		gen->add(obj);

		auto exc = cof->createObject("exciter", "type1");
		gen->add(exc);

		auto gov = cof->createObject("governor", "tgov1");
		gen->add(gov);
		//run till just after the fault
		gds->run(1.01);

		BOOST_CHECK_MESSAGE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE, "Model " << gname << " failed to run past fault");
		auto mmatch = runJacobianCheck(gds, cDaeSolverMode);
		BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure after fault");


		//run till just after the fault clears
		gds->run(1.2);

		BOOST_CHECK_MESSAGE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE, "Model " << gname << " failed to run past fault clear");
		mmatch = runJacobianCheck(gds, cDaeSolverMode);

		BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure");


		gds->run();
		BOOST_CHECK_MESSAGE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE, "Model " << gname << " failed to run to completion");

		std::vector<double> volts;
		gds->getVoltage(volts);

		BOOST_CHECK_MESSAGE(volts[1] > 0.96, "Model " << gname << " voltage issue v=" << volts[1]);
		delete gds;
		gds = nullptr;
	}
}
#endif 

BOOST_AUTO_TEST_CASE(geco_fault_case)
{
	std::string fname = fault_test_directory + "geco_fault_uncoupled.xml";

	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));
	gds->consolePrintLevel = GD_DEBUG_PRINT;
	int  retval = gds->dynInitialize();

	BOOST_CHECK_EQUAL(retval, 0);

	int mmatch = JacobianCheck(gds, cDaeSolverMode);
	if (mmatch > 0)
	{
		printStateNames(gds, cDaeSolverMode);
	}
	BOOST_REQUIRE_EQUAL(mmatch, 0);
	mmatch = residualCheck(gds, cDaeSolverMode);
	if (mmatch > 0)
	{
		printStateNames(gds, cDaeSolverMode);
	}


	gds->run();
	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
	//simpleRunTestXML(fname);
}

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_dynamic)
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);

	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = fault_test_directory + "link_fault2.xml";

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = GD_TRACE_PRINT;
	gds->consolePrintLevel = GD_WARNING_PRINT;

	gds->run();

	std::vector<double> v;
	gds->getVoltage(v);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) {return (a > 0.75); }));


	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}

#endif 

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_fuse)
{
	//test a fuse
	std::string fname = fault_test_directory + "link_fault_fuse.xml";

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = GD_WARNING_PRINT;
	auto obj = dynamic_cast<fuse *>(gds->getRelay(0));
	BOOST_REQUIRE(obj != nullptr);
	gds->run();
	auto lobj = dynamic_cast<gridLink *>(gds->find("bus2_to_bus3"));
	BOOST_CHECK(lobj->isConnected() == false);
	std::vector<double> v;
	gds->getVoltage(v);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) {return (a > 0.80); }));


	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_fuse2)
{
	//test whether fuses are working properly
	std::string fname = fault_test_directory + "link_fault_fuse2.xml";

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = GD_DEBUG_PRINT;
	auto obj = dynamic_cast<gridLink *>(gds->find("bus8_to_bus9"));
	BOOST_REQUIRE(obj != nullptr);
	gds->run();

	std::vector<double> v;
	gds->getVoltage(v);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) {return (a > 0.80); }));


	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_fuse3)
{
	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = fault_test_directory + "link_fault_fuse3.xml";

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = GD_DEBUG_PRINT;
	//auto obj = dynamic_cast<gridLink *>(gds->find("bus2_to_bus3"));
	gds->dynInitialize();
	int mmatch = JacobianCheck(gds, cDaeSolverMode);
	if (mmatch > 0)
	{
		printStateNames(gds, cDaeSolverMode);
	}
	//BOOST_REQUIRE(obj != nullptr);
	gds->run();
	//BOOST_CHECK(obj->isConnected() == false);
	std::vector<double> v;
	gds->getVoltage(v);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) {return (a > 0.80); }));


	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_breaker)
{
	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = fault_test_directory + "link_fault_breaker.xml";

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = GD_WARNING_PRINT;
	auto obj = dynamic_cast<breaker *>(gds->getRelay(0));
	BOOST_REQUIRE(obj != nullptr);
	gds->run();
	auto lobj = dynamic_cast<gridLink *>(gds->find("bus2_to_bus3"));
	BOOST_CHECK(lobj->isConnected() == false);
	std::vector<double> v;
	gds->getVoltage(v);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) {return (a > 0.80); }));


	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_breaker2)
{
	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = fault_test_directory + "link_fault_breaker2.xml";

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = GD_WARNING_PRINT;
	auto obj = dynamic_cast<breaker *>(gds->getRelay(0));
	BOOST_REQUIRE(obj != nullptr);
	gds->run();
	auto lobj = dynamic_cast<gridLink *>(gds->find("bus8_to_bus9"));
	BOOST_CHECK(lobj->isConnected() == false);
	std::vector<double> v;
	gds->getVoltage(v);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) {return (a > 0.80); }));


	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_breaker3)
{
	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = fault_test_directory + "link_fault_breaker3.xml";

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = GD_WARNING_PRINT;
	auto obj = dynamic_cast<breaker *>(gds->getRelay(0));
	BOOST_REQUIRE(obj != nullptr);
	gds->run();
	auto lobj = dynamic_cast<gridLink *>(gds->find("bus8_to_bus9"));
	BOOST_CHECK(lobj->isConnected() == true);
	std::vector<double> v;
	gds->getVoltage(v);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) {return (a > 0.80); }));


	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_breaker4)
{
	//test a bunch of different link parameters to make sure all the solve properly
	std::string fname = fault_test_directory + "link_fault_breaker4.xml";

	gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
	gds->consolePrintLevel = GD_WARNING_PRINT;
	auto obj = dynamic_cast<breaker *>(gds->getRelay(0));
	BOOST_REQUIRE(obj != nullptr);
	gds->run();
	auto lobj = dynamic_cast<gridLink *>(gds->find("bus8_to_bus9"));
	BOOST_CHECK(lobj->isConnected() == true);
	std::vector<double> v;
	gds->getVoltage(v);
	BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) {return (a > 0.80); }));


	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}
#endif
BOOST_AUTO_TEST_SUITE_END()
