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

#include "submodels/otherGenModels.h"
#include "submodels/gridDynExciter.h"
#include "submodels/gridDynGovernor.h"
#include "loadModels/otherLoads.h"
#include "core/gridDynExceptions.h"
#include "testHelper.h"
#include "objectFactory.h"

//test case for coreObject object

using namespace gridUnits;
BOOST_AUTO_TEST_SUITE(core_tests)


BOOST_AUTO_TEST_CASE (coreObject_test)
{

  coreObject *obj1 = new coreObject ();
  coreObject *obj2 = new coreObject ();

  BOOST_CHECK_EQUAL (obj1->getName().compare ("object_"+std::to_string(obj1->getID())),0);

  //check to make sure the object m_oid counter is working
 
  BOOST_CHECK (obj2->getID()>obj1->getID());
  //check if the name setting is working
  std::string name1 = "test_object";
  obj1->set ("name",name1);
  BOOST_CHECK_EQUAL (obj1->getName().compare (name1),0);

  double ntime = 1;
  obj1->set ("nextupdatetime",ntime);
  BOOST_CHECK_EQUAL (obj1->getNextUpdateTime(),ntime);

  //use alternative form for set
  obj2->set ("nextupdatetime",0.1);
  BOOST_CHECK_EQUAL (obj2->getNextUpdateTime(),0.1);

  BOOST_CHECK (compareUpdates (obj2,obj1));

  coreObject *obj3 = nullptr;
  obj3 = obj1->clone (obj3);

  //check the copy constructor
  BOOST_CHECK_EQUAL (obj3->getNextUpdateTime(),ntime);
  BOOST_CHECK_EQUAL(obj3->getName().compare ("test_object"),0);
  BOOST_CHECK (obj3->getParent() == nullptr);

  ntime = 3;
  obj1->set ("nextupdatetime",ntime);
  obj3 = obj1->clone (obj3);
  BOOST_CHECK_EQUAL (obj3->getNextUpdateTime(),ntime);
  BOOST_CHECK (obj3->getName().compare ("test_object")==0);
  //check the parameter not found function
  try
  {
	  obj3->set("bob", 0.5);//this should throw and exception
	  BOOST_CHECK(false);
  }
  catch (unrecognizedParameter &)
  {

  }
  delete(obj1);
  delete(obj2);
  delete(obj3);
}

//testcase for gridDynGenModel Object
BOOST_AUTO_TEST_CASE (gridDynGenModel_test)
{

  
  gridDynGenModel *gm = new gridDynGenModel4 ();
  auto id = gm->getID();
  BOOST_CHECK_EQUAL (gm->getName().compare ("genModel4_"+std::to_string(id)),0);
  std::string temp1 = "gen_model 5";
  gm->set ("name",temp1);
  BOOST_CHECK_EQUAL (gm->getName().compare (temp1),0);

  gm->setName("namebbghs");
  BOOST_CHECK_EQUAL(gm->getName().compare("namebbghs"), 0);
  gm->initializeA(timeZero, 0);
  BOOST_CHECK_EQUAL(gm->stateSize(cLocalSolverMode), 6u);
  gm->set ("h",4.5);
  gm->setOffset (0, cLocalSolverMode);
  BOOST_CHECK_EQUAL (gm->findIndex ("freq", cLocalSolverMode), static_cast<index_t>(3));
  delete(gm);
}

BOOST_AUTO_TEST_CASE (gridDynExciter_test)
{

  gridDynExciter *ex = new gridDynExciterIEEEtype1 ();
  std::string temp1 = "exciter 2";
  ex->set ("name",temp1);
  BOOST_CHECK_EQUAL (ex->getName().compare (temp1),0);
  ex->initializeA(0.0, 0);
  BOOST_CHECK_EQUAL(ex->stateSize(cLocalSolverMode), 3u);
  ex->set ("tf",4.5) ;
  ex->setOffset (0, cLocalSolverMode);
  BOOST_CHECK_EQUAL (ex->findIndex ("vr",cLocalSolverMode), static_cast<index_t>(1));
  delete(ex);
}


BOOST_AUTO_TEST_CASE (gridDynGovernor_test)
{

  gridDynGovernor *gov = new gridDynGovernor ();
  std::string temp1 = "gov 2";
  gov->set ("name",temp1);
  BOOST_CHECK_EQUAL (gov->getName().compare (temp1),0);
  gov->initializeA(timeZero, 0);
  BOOST_CHECK_EQUAL (gov->stateSize (cLocalSolverMode), 4u);
  gov->set ("t1",4.5);
  gov->setOffset(0, cLocalSolverMode);
  BOOST_CHECK_EQUAL (gov->findIndex ("pm", cLocalSolverMode), static_cast<index_t>(0));
  delete(gov);
}


BOOST_AUTO_TEST_CASE (test_unit_functions)
{
  //units_t u1;
  //units_t u2;
  double val1;
  double basepower = 100;
//  double basevoltage = 10;
  //power conversions
  val1 = unitConversion (100.0, MW, kW);
  BOOST_CHECK_CLOSE (val1, 100000, 0.0001);
  val1 = unitConversion (100.0, MW, Watt);
  BOOST_CHECK_CLOSE (val1, 100000000, 0.0001);
  val1 = unitConversion (1000000.0, Watt, MW);
  BOOST_CHECK_CLOSE (val1, 1, 0.0001);
  val1 = unitConversion (100000.0, kW, MW);
  BOOST_CHECK_CLOSE (val1, 100, 0.0001);
  val1 = unitConversion (100000.0, kW, puMW,basepower);
  BOOST_CHECK_CLOSE (val1, 1, 0.0001);
  //angle conversions
  val1 = unitConversion (10, deg, rad);
  BOOST_CHECK_CLOSE (val1, 0.17453292, 0.0001);
  val1 = unitConversion (0.17453292, rad, deg);
  BOOST_CHECK_CLOSE (val1, 10, 0.0001);
  //pu conversions
  val1 = unitConversion (1.0, puOhm, Ohm, 0.1, 0.6);
  BOOST_CHECK_CLOSE (val1, 3.600, 0.001);
  val1 = unitConversion (1.0, puA, Amp, 0.1, 0.6);
  BOOST_CHECK_CLOSE (val1, 166.6666666, 0.01);

}

BOOST_AUTO_TEST_CASE (object_factory_test)
{
  auto cof = coreObjectFactory::instance ();
  coreObject *obj = cof->createObject ("load", "basic");
  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  BOOST_CHECK (ld != nullptr);
  delete ld;
  sourceLoad *gsL = dynamic_cast<sourceLoad *> (cof->createObject ("load", "sine"));
  BOOST_CHECK (gsL != nullptr);
  delete gsL;

  obj = cof->createObject("load");
  ld = dynamic_cast<gridLoad *> (obj);
  BOOST_CHECK(ld != nullptr);


}

BOOST_AUTO_TEST_CASE(gridDynTime_tests)
{
	gridDyn_time rt(34.123141512);

	double dval = static_cast<double>(rt);
	BOOST_CHECK_CLOSE(dval, 34.123141512, 0.0000001);
}

BOOST_AUTO_TEST_SUITE_END ()
