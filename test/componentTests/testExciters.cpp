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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include "utilities/vectorOps.hpp"
#include "core/objectFactory.h"
#include "generators/gridDynGenerator.h"

#include <cmath>
//test case for coreObject object

#define EXCITER_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/exciter_tests/"

BOOST_FIXTURE_TEST_SUITE (exciter_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (root_exciter_test)
{
  std::string fname = std::string (EXCITER_TEST_DIRECTORY "test_root_exciter.xml");

  readerConfig::setPrintMode (0);
  gds = readSimXMLFile(fname);

  int retval = gds->dynInitialize ();
  BOOST_CHECK_EQUAL (retval, 0);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

  std::vector<double> st = gds->getState ();



  gds->run ();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
  std::vector<double> st2 = gds->getState ();

  //check for stability
  auto diff = countDiffs(st, st2, 0.0001);
  BOOST_CHECK_EQUAL (diff, 0u);

}

BOOST_AUTO_TEST_CASE(basic_stability_test1)
{
	static const std::map<std::string, std::vector<std::pair<std::string, double>>> parameters
	{
		{"basic",{{"ta",0.2},{"ka",11.0}}},
		{"dc1a",{ { "ta",0.1 },{ "ka",6.0 } } },
		{ "dc2a",{ { "ta",0.1 },{ "ka",6.0 } }},
	};

	std::string fname = std::string(EXCITER_TEST_DIRECTORY "test_exciter_stability.xml");

	auto cof = coreObjectFactory::instance();

	auto exclist = cof->getTypeNames("exciter");

	//exclist.insert(exclist.begin(), "none");
	for (auto &excname : exclist)
	{

		gds = readSimXMLFile(fname);
		gridDynGenerator *gen = gds->getGen(0);
		gds->consolePrintLevel = print_level::no_print;
		auto obj = cof->createObject("exciter", excname);
		BOOST_CHECK(obj != nullptr);
		auto fnd = parameters.find(excname);

		if (fnd != parameters.end())
		{
			for (auto &pp : fnd->second)
			{
				obj->set(pp.first, pp.second);
			}
		}

		gen->add(obj);
		

		int retval = gds->dynInitialize();
		BOOST_REQUIRE_EQUAL(gds->currentProcessState(),gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

		BOOST_CHECK_EQUAL(retval, 0);

		int badresid = runResidualCheck(gds, cDaeSolverMode,false);

		BOOST_REQUIRE_MESSAGE(badresid == 0,"exciter type "<<excname<<"resid issue\n");
		int badjacobian = runJacobianCheck(gds, cDaeSolverMode,false);
		BOOST_REQUIRE_MESSAGE(badjacobian == 0, "exciter type " << excname << "Jacobian issue\n");

		std::vector<double> volt1;
		gds->getVoltage(volt1);

		gds->run();

		BOOST_REQUIRE_MESSAGE(gds->getCurrentTime() >= 30.0,"exciter type "<<excname<<" didn't complete\n");
		std::vector<double> volt2;
		gds->getVoltage(volt2);
		BOOST_CHECK((volt2[0] > 0.95) && (volt2[0] < 1.00));
		BOOST_CHECK((volt2[1] > 0.95) && (volt2[1] < 1.000));

		//check for stability
	}

}

BOOST_AUTO_TEST_CASE(basic_stability_test2)
{
	static const std::map<std::string, std::vector<std::pair<std::string, double>>> parameters
	{
		{ "basic",{ { "ta",0.2 },{ "ka",11.0 } } },
		{ "dc1a",{ { "ta",0.1 },{ "ka",6.0 } } },
		{ "dc2a",{ { "ta",0.1 },{ "ka",6.0 } } },
	};

	std::string fname = std::string(EXCITER_TEST_DIRECTORY "test_exciter_stability2.xml");

	auto cof = coreObjectFactory::instance();
	auto exclist = cof->getTypeNames("exciter");

	//exclist.insert(exclist.begin(), "none");
	for (auto &excname : exclist)
	{
		gds = readSimXMLFile(fname);
		gridDynGenerator *gen = gds->getGen(0);
		gds->consolePrintLevel = print_level::no_print;
		auto obj = cof->createObject("exciter", excname);
		BOOST_CHECK(obj != nullptr);
		auto fnd = parameters.find(excname);

		if (fnd != parameters.end())
		{
			for (auto &pp : fnd->second)
			{
				obj->set(pp.first, pp.second);
			}
		}

		gen->add(obj);


		int retval = gds->dynInitialize();
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

		BOOST_CHECK_EQUAL(retval, 0);

		int badresid = runResidualCheck(gds, cDaeSolverMode,false);
		BOOST_REQUIRE_MESSAGE(badresid == 0, "Exciter " << excname << " residual issue");
		int badjacobian = runJacobianCheck(gds, cDaeSolverMode,false);

		BOOST_REQUIRE_MESSAGE(badjacobian == 0,"Exciter "<<excname<<" Jacobian issue");

		std::vector<double> volt1;
		gds->getVoltage(volt1);

		gds->run();
		if (gds->getCurrentTime() < 30.0)
		{
			printf("exciter didn't complete %s\n", excname.c_str());
			gds->saveRecorders();
		}
		BOOST_REQUIRE(gds->getCurrentTime() >= 30.0);
		std::vector<double> volt2;
		gds->getVoltage(volt2);
		BOOST_CHECK((volt2[0] > 1.00) && (volt2[0] < 1.05));
		BOOST_CHECK((volt2[1] > 0.99) && (volt2[1] < 1.04));

		//check for stability
	}

}

BOOST_AUTO_TEST_CASE(basic_stability_test3)
{
	static const std::map<std::string, std::vector<std::pair<std::string, double>>> parameters
	{
		//{ "basic",{ { "ta",0.2 },{ "ka",11.0 } } },
		{ "dc1a",{ { "ta",0.1 },{ "ka",6.0 } } },
		{ "dc2a",{ { "ta",0.3 },{ "ka",6.0 } } },
	};

	std::string fname = std::string(EXCITER_TEST_DIRECTORY "test_exciter_stability3.xml");

	auto cof = coreObjectFactory::instance();
	coreObject *obj = nullptr;

	auto exclist = cof->getTypeNames("exciter");

	//exclist.insert(exclist.begin(), "none");
	for (auto &excname : exclist)
	{
		if (excname == "dc1a")
		{
			//TODO: this doesn't work for now (unknown)
			continue;
		}
		gds = readSimXMLFile(fname);
		gridDynGenerator *gen = gds->getGen(0);
		gds->consolePrintLevel = print_level::no_print;
		obj = cof->createObject("exciter", excname);
		BOOST_CHECK(obj != nullptr);
		auto fnd = parameters.find(excname);

		if (fnd != parameters.end())
		{
			for (auto &pp : fnd->second)
			{
				obj->set(pp.first, pp.second);
			}
		}

		gen->add(obj);


		int retval = gds->dynInitialize();
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

		BOOST_CHECK_EQUAL(retval, 0);

		int badresid = runResidualCheck(gds, cDaeSolverMode,false);

		BOOST_REQUIRE_MESSAGE(badresid == 0, "Exciter " << excname << " residual issue");
		int badjacobian = runJacobianCheck(gds, cDaeSolverMode,false);

		BOOST_REQUIRE_MESSAGE(badjacobian == 0, "Exciter " << excname << " Jacobian issue");

		std::vector<double> volt1;
		gds->getVoltage(volt1);

		gds->run();
		if (gds->getCurrentTime() < 30.0)
		{
			printf("exciter didn't complete %s\n", excname.c_str());
			gds->saveRecorders();
		}
		BOOST_REQUIRE(gds->getCurrentTime() >= 30.0);
		std::vector<double> volt2;
		gds->getVoltage(volt2);
		BOOST_CHECK((volt2[0] > 0.98) && (volt2[0] < 1.02));
		BOOST_CHECK((volt2[1] > 0.97) && (volt2[1] < 1.02));
		//check for stability
	}

}

BOOST_AUTO_TEST_CASE(basic_stability_test4)
{
	static const std::map<std::string, std::vector<std::pair<std::string, double>>> parameters
	{
		//{ "basic",{ { "ta",0.2 },{ "ka",11.0 } } },
		{ "dc1a",{ { "ta",0.1 },{ "ka",6.0 } } },
		{ "dc2a",{ { "ta",0.3 },{ "ka",6.0 } } },
	};

	std::string fname = std::string(EXCITER_TEST_DIRECTORY "test_exciter_stability4.xml");

	auto cof = coreObjectFactory::instance();
	coreObject *obj = nullptr;

	auto exclist = cof->getTypeNames("exciter");

	//exclist.insert(exclist.begin(), "none");
	for (auto &excname : exclist)
	{
		if (excname == "dc1a")
		{
			//TODO: this doesn't work for now (unknown)
			continue;
		}
		gds = readSimXMLFile(fname);
		gridDynGenerator *gen = gds->getGen(0);
		gds->consolePrintLevel = print_level::no_print;
		obj = cof->createObject("exciter", excname);
		BOOST_CHECK(obj != nullptr);
		auto fnd = parameters.find(excname);

		if (fnd != parameters.end())
		{
			for (auto &pp : fnd->second)
			{
				obj->set(pp.first, pp.second);
			}
		}

		gen->add(obj);


		int retval = gds->dynInitialize();
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

		BOOST_CHECK_EQUAL(retval, 0);

		int badresid = runResidualCheck(gds, cDaeSolverMode,false);

		BOOST_REQUIRE_MESSAGE(badresid == 0, "Exciter " << excname << " residual issue");
		int badjacobian = runJacobianCheck(gds, cDaeSolverMode,false);
		BOOST_REQUIRE_MESSAGE(badjacobian == 0 , "Exciter " << excname << " residual issue");

		std::vector<double> volt1;
		gds->getVoltage(volt1);

		gds->run();
		if (gds->getCurrentTime() < 30.0)
		{
			printf("exciter didn't complete %s\n", excname.c_str());
			gds->saveRecorders();
		}
		BOOST_REQUIRE(gds->getCurrentTime() >= 30.0);
		std::vector<double> volt2;
		gds->getVoltage(volt2);
		BOOST_CHECK((volt2[0] > 0.98) && (volt2[0] < 1.02));
		BOOST_CHECK((volt2[1] > 0.97) && (volt2[1] < 1.02));
		//check for stability
	}

}

#ifdef LOAD_CVODE
BOOST_AUTO_TEST_CASE(exciter_test2_alg_diff_tests)  //test the algebraic updates and derivative updates
{
	static const std::map<std::string, std::vector<std::pair<std::string, double>>> parameters
	{
		{ "basic",{ { "ta",0.2 },{ "ka",11.0 } } },
		{ "dc1a",{ { "ta",0.1 },{ "ka",6.0 } } },
		{ "dc2a",{ { "ta",0.1 },{ "ka",6.0 } } },
	};

	std::string fname = std::string(EXCITER_TEST_DIRECTORY "test_exciter_stability.xml");

	auto cof = coreObjectFactory::instance();

	auto exclist = cof->getTypeNames("exciter");

	//exclist.insert(exclist.begin(), "none");
	for (auto &excname : exclist)
	{

		gds = readSimXMLFile(fname);
		gridDynGenerator *gen = gds->getGen(0);
		gds->consolePrintLevel = print_level::no_print;
		auto obj = cof->createObject("exciter", excname);
		BOOST_CHECK(obj != nullptr);
		auto fnd = parameters.find(excname);

		if (fnd != parameters.end())
		{
			for (auto &pp : fnd->second)
			{
				obj->set(pp.first, pp.second);
			}
		}

		gen->add(obj);

		int retval = gds->dynInitialize();

		BOOST_CHECK_EQUAL(retval, 0);
		auto mmatch = runResidualCheck(gds, cDaeSolverMode,false);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Exciter " << excname << " residual issue");
		mmatch = runDerivativeCheck(gds, cDaeSolverMode,false);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Exciter " << excname << " derivative issue");
		mmatch = runAlgebraicCheck(gds, cDaeSolverMode,false);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Exciter " << excname << " algebraic issue");
	}

		

		

}

BOOST_AUTO_TEST_CASE(exciter_alg_diff_jacobian_tests)  //test the algebraic updates and derivative updates
{
	static const std::map<std::string, std::vector<std::pair<std::string, double>>> parameters
	{
		{ "basic",{ { "ta",0.2 },{ "ka",11.0 } } },
		{ "dc1a",{ { "ta",0.1 },{ "ka",6.0 } } },
		{ "dc2a",{ { "ta",0.1 },{ "ka",6.0 } } },
	};

	std::string fname = std::string(EXCITER_TEST_DIRECTORY "test_exciter_stability.xml");

	auto cof = coreObjectFactory::instance();

	auto exclist = cof->getTypeNames("exciter");

	//exclist.insert(exclist.begin(), "none");
	for (auto &excname : exclist)
	{

		gds = readSimXMLFile(fname);
		gridDynGenerator *gen = gds->getGen(0);
		gds->consolePrintLevel = print_level::no_print;
		auto obj = cof->createObject("exciter", excname);
		BOOST_CHECK(obj != nullptr);
		auto fnd = parameters.find(excname);

		if (fnd != parameters.end())
		{
			for (auto &pp : fnd->second)
			{
				obj->set(pp.first, pp.second);
			}
		}

		gen->add(obj);
		int retval = gds->dynInitialize();

		BOOST_CHECK_EQUAL(retval, 0);
		auto mmatch = runJacobianCheck(gds, cDynDiffSolverMode,false);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Exciter " << excname << " Jacobian dynDiff issue");
		mmatch = runJacobianCheck(gds, cDynAlgSolverMode,false);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Exciter " << excname << " Jacobian dynAlg issue");
	}

}
#endif

BOOST_AUTO_TEST_SUITE_END ()


