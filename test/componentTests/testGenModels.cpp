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
#include "core/objectFactory.h"
#include "generators/gridDynGenerator.h"
#include "vectorOps.hpp"

#include <cmath>
//test case for coreObject object


#define GENMODEL_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/genmodel_tests/"

BOOST_FIXTURE_TEST_SUITE (genModel_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (model_test1)
{
  std::string fname = std::string (GENMODEL_TEST_DIRECTORY "test_model1.xml");

  gds = readSimXMLFile(fname);

  int retval = gds->dynInitialize ();
  BOOST_CHECK_EQUAL (retval, 0);
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);

  std::vector<double> st = gds->getState ();
  auto mmatch = runResidualCheck(gds, cDaeSolverMode);
  BOOST_REQUIRE_EQUAL(mmatch, 0);
  //gds->saveJacobian(std::string(GENMODEL_TEST_DIRECTORY "mjac5.bin"));
  gds->run ();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
  std::vector<double> st2 = gds->getState ();

  auto cdiff = countDiffs(st, st2, 0.001, 0.01);
 
  BOOST_CHECK_EQUAL (cdiff, 0u);

}

BOOST_AUTO_TEST_CASE (model_test2)  //Jacobian code check
{
  std::string fname = std::string (GENMODEL_TEST_DIRECTORY "test_model1.xml");


  gds = readSimXMLFile(fname);


  gridDynGenerator *gen = gds->getGen (0);
  auto cof = coreObjectFactory::instance ();
  auto genlist = cof->getTypeNames ("genmodel");

  for (auto &gname : genlist)
    {
      auto obj = cof->createObject ("genmodel", gname);
      BOOST_CHECK (obj != nullptr);
      gen->add (obj);

      int retval = gds->dynInitialize ();

      BOOST_CHECK_EQUAL (retval, 0);
      auto mmatch = runResidualCheck (gds,cDaeSolverMode);
      BOOST_REQUIRE_MESSAGE (mmatch==0,"Model "<<gname<<" residual issue");
      mmatch = runJacobianCheck (gds,cDaeSolverMode);
      BOOST_REQUIRE_MESSAGE(mmatch==0,"Model "<<gname<<" Jacobian issue");
	  mmatch = runDerivativeCheck(gds, cDaeSolverMode);
	  BOOST_REQUIRE_MESSAGE(mmatch == 0, "Model " << gname << " derivative issue");
	  mmatch = runAlgebraicCheck(gds, cDaeSolverMode);
	  BOOST_REQUIRE_MESSAGE(mmatch == 0, "Model " << gname << " algebraic issue");
    }

}

BOOST_AUTO_TEST_CASE(model_test2_withr)  //Jacobian code check
{
	std::string fname = std::string(GENMODEL_TEST_DIRECTORY "test_model1.xml");


	gds = readSimXMLFile(fname);


	gridDynGenerator *gen = gds->getGen(0);
	auto cof = coreObjectFactory::instance();
	auto genlist = cof->getTypeNames("genmodel");

	for (auto &gname : genlist)
	{
		auto obj = cof->createObject("genmodel", gname);
		BOOST_CHECK(obj != nullptr);
		//just set the resistance to make sure the models can handle that parameter
		obj->set("r", 0.001);
		gen->add(obj);

		int retval = gds->dynInitialize();

		BOOST_CHECK_EQUAL(retval, 0);
		auto mmatch = runResidualCheck(gds, cDaeSolverMode);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Model " << gname << " residual r issue");
		mmatch = runJacobianCheck(gds, cDaeSolverMode);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian r issue");
	}

}

#ifdef LOAD_CVODE
BOOST_AUTO_TEST_CASE(model_test2_alg_diff_tests)  //test the algebraic updates and derivative updates
{
	std::string fname = std::string(GENMODEL_TEST_DIRECTORY "test_model1.xml");


	gds = readSimXMLFile(fname);


	gridDynGenerator *gen = gds->getGen(0);
	auto cof = coreObjectFactory::instance();
	
	auto genlist = cof->getTypeNames("genmodel");

	for (auto &gname : genlist)
	{

		auto obj = cof->createObject("genmodel", gname);
		BOOST_CHECK(obj != nullptr);
		//just set the resistance to make sure the models can handle that parameter
		obj->set("r", 0.001);
		gen->add(obj);

		int retval = gds->dynInitialize();

		BOOST_CHECK_EQUAL(retval, 0);
		auto mmatch = runResidualCheck(gds, cDaeSolverMode);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Model " << gname << " residual issue");
		mmatch = runDerivativeCheck(gds, cDaeSolverMode);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Model " << gname << " derivative issue");
		mmatch = runAlgebraicCheck(gds, cDaeSolverMode);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Model " << gname << " algebraic issue");

		mmatch = runJacobianCheck(gds, cDynDiffSolverMode);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian dynDiff issue");
		mmatch = runJacobianCheck(gds, cDynAlgSolverMode);
		BOOST_REQUIRE_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian dynAlg issue");
	}

}
#endif

BOOST_AUTO_TEST_CASE (model_test3)  //Jacobian code check
{
  std::string fname = std::string (GENMODEL_TEST_DIRECTORY "test_model2.xml");

  gds = readSimXMLFile(fname);

  gds->run ();
  BOOST_REQUIRE_EQUAL (gds->currentProcessState (), gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

}


BOOST_AUTO_TEST_SUITE_END ()
