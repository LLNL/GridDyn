/*
* LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#include <boost/test/unit_test.hpp>

#include "core/coreExceptions.h"
#include "core/objectFactory.hpp"
#include "griddyn/loads/zipLoad.h"
#include "griddyn/loads/fileLoad.h"
#include "griddyn/loads/sourceLoad.h"
#include "griddyn/exciters/ExciterIEEEtype1.h"
#include "griddyn/Governor.h"
#include "griddyn/genmodels/GenModel4.h"
#include "../testHelper.h"

// test case for coreObject object

using namespace units;
using namespace griddyn;

BOOST_AUTO_TEST_SUITE (core_tests, * boost::unit_test::label("quick"))


BOOST_AUTO_TEST_CASE (coreObject_test)
{
    coreObject *obj1 = new coreObject ();
    coreObject *obj2 = new coreObject ();

    BOOST_CHECK_EQUAL (obj1->getName ().compare ("object_" + std::to_string (obj1->getID ())), 0);

    // check to make sure the object m_oid counter is working

    BOOST_CHECK (obj2->getID () > obj1->getID ());
    // check if the name setting is working
    std::string name1 = "test_object";
    obj1->setName (name1);
    BOOST_CHECK_EQUAL (obj1->getName ().compare (name1), 0);

    obj1->setName ("test_object");
    BOOST_CHECK_EQUAL (obj1->getName ().compare ("test_object"), 0);

    double ntime = 1;
    obj1->set ("nextupdatetime", ntime);
    BOOST_CHECK_EQUAL (double(obj1->getNextUpdateTime ()), ntime);

    // use alternative form for set
    obj2->set ("nextupdatetime", 0.1);
    BOOST_CHECK_EQUAL (double(obj2->getNextUpdateTime ()), 0.1);

    BOOST_CHECK (compareUpdates (obj2, obj1));

    coreObject *obj3 = nullptr;
    obj3 = obj1->clone (obj3);

    // check the copy constructor
    BOOST_CHECK_EQUAL (double(obj3->getNextUpdateTime ()), ntime);
    BOOST_CHECK_EQUAL (obj3->getName ().compare ("test_object"), 0);
    BOOST_CHECK (obj3->isRoot ());

    ntime = 3;
    obj1->set ("nextupdatetime", ntime);
    obj3 = obj1->clone (obj3);
    BOOST_CHECK_EQUAL (double(obj3->getNextUpdateTime ()), ntime);
    BOOST_CHECK (obj3->getName ().compare ("test_object") == 0);
    // check the parameter not found function
    try
    {
        obj3->set ("bob", 0.5);  // this should throw and exception
        BOOST_CHECK (false);
    }
    catch (unrecognizedParameter &)
    {
    }
    delete (obj1);
    delete (obj2);
    delete (obj3);
}

// testcase for GenModel Object
BOOST_AUTO_TEST_CASE (GenModel_test)
{
    auto gm = new genmodels::GenModel4 ();
    auto id = gm->getID ();
    BOOST_CHECK_EQUAL (gm->getName ().compare ("genModel4_" + std::to_string (id)), 0);
    std::string temp1 = "gen_model 5";
    gm->setName (temp1);
    BOOST_CHECK_EQUAL (gm->getName ().compare (temp1), 0);

    gm->setName ("namebbghs");
    BOOST_CHECK_EQUAL (gm->getName ().compare ("namebbghs"), 0);
    gm->dynInitializeA (timeZero, 0);
    BOOST_CHECK_EQUAL (gm->stateSize (cLocalSolverMode), 6_cnt);
    gm->set ("h", 4.5);
    gm->setOffset (0, cLocalSolverMode);
    BOOST_CHECK_EQUAL (gm->findIndex ("freq", cLocalSolverMode), static_cast<index_t> (3));
    delete (gm);
}

BOOST_AUTO_TEST_CASE (Exciter_test)
{
    Exciter *ex = new exciters::ExciterIEEEtype1 ();
    std::string temp1 = "exciter 2";
    ex->setName (temp1);
    BOOST_CHECK_EQUAL (ex->getName ().compare (temp1), 0);
    ex->dynInitializeA (0.0, 0);
    BOOST_CHECK_EQUAL (ex->stateSize (cLocalSolverMode), 3_ind);
    ex->set ("tf", 4.5);
    ex->setOffset (0, cLocalSolverMode);
    BOOST_CHECK_EQUAL (ex->findIndex ("vr", cLocalSolverMode), static_cast<index_t> (1));
    delete (ex);
}


BOOST_AUTO_TEST_CASE (Governor_test)
{
    Governor *gov = new Governor ();
    std::string temp1 = "gov 2";
    gov->set ("name", temp1);
    BOOST_CHECK_EQUAL (gov->getName ().compare (temp1), 0);
    gov->dynInitializeA (timeZero, 0);
    BOOST_CHECK_EQUAL (gov->stateSize (cLocalSolverMode), 4_cnt);
    gov->set ("t1", 4.5);
    gov->setOffset (0, cLocalSolverMode);
    BOOST_CHECK_EQUAL (gov->findIndex ("pm", cLocalSolverMode), static_cast<index_t> (0));
    delete (gov);
}


BOOST_AUTO_TEST_CASE (test_unit_functions)
{
    // units_t u1;
    // units_t u2;
    double val1;
    double basepower = 100;
    //  double basevoltage = 10;
    // power conversions
    val1 = convert (100.0, MW, kW);
    BOOST_CHECK_CLOSE (val1, 100000, 0.0001);
    val1 = convert (100.0, MW, Watt);
    BOOST_CHECK_CLOSE (val1, 100000000, 0.0001);
    val1 = convert (1000000.0, Watt, MW);
    BOOST_CHECK_CLOSE (val1, 1, 0.0001);
    val1 = convert (100000.0, kW, MW);
    BOOST_CHECK_CLOSE (val1, 100, 0.0001);
    val1 = convert (100000.0, kW, puMW, basepower);
    BOOST_CHECK_CLOSE (val1, 1, 0.0001);
    // angle conversions
    val1 = convert (10, deg, rad);
    BOOST_CHECK_CLOSE (val1, 0.17453292, 0.0001);
    val1 = convert (0.17453292, rad, deg);
    BOOST_CHECK_CLOSE (val1, 10, 0.0001);
    // pu conversions
    val1 = convert (1.0, puOhm, Ohm, 0.1, 0.6);
    BOOST_CHECK_CLOSE (val1, 3.600, 0.001);
    val1 = convert (1.0, puA, Amp, 0.1, 0.6);
    BOOST_CHECK_CLOSE (val1, 166.6666666, 0.01);
}

BOOST_AUTO_TEST_CASE (object_factory_test)
{
    auto cof = coreObjectFactory::instance ();
    coreObject *obj = cof->createObject ("load", "basic");
    auto ld = dynamic_cast<zipLoad *> (obj);
    BOOST_CHECK (ld != nullptr);
    delete ld;
    auto gsL = dynamic_cast<loads::sourceLoad *> (cof->createObject ("load", "sine"));
    BOOST_CHECK (gsL != nullptr);
    delete gsL;

    obj = cof->createObject ("load");
    ld = dynamic_cast<zipLoad *> (obj);
    BOOST_CHECK (ld != nullptr);
    delete ld;
}

BOOST_AUTO_TEST_CASE (gridDynTime_tests)
{
    coreTime rt (34.123141512);

    double dval = static_cast<double> (rt);
    BOOST_CHECK_CLOSE (dval, 34.123141512, 0.0000001);

	coreTime rt2(-2.3);

	double dval2 = static_cast<double> (rt2);
	BOOST_CHECK_CLOSE(dval2, -2.3, 0.0000001);

	coreTime rt3(-1.0);

	double dval3 = static_cast<double> (rt3);
	BOOST_CHECK_CLOSE(dval3, -1.0, 0.0000001);
}

/** test case to construct all objects and do some potentially problematic things to them to ensure the object
doesn't break or cause a fault
*/

BOOST_AUTO_TEST_CASE(object_tests_probe)
{
	auto cof = coreObjectFactory::instance();
	auto componentList = cof->getFactoryNames();
	for (auto &comp : componentList)
	{
		auto componentFactory = cof->getFactory(comp);
		auto typeList = componentFactory->getTypeNames();
		for (auto &type:typeList)
		{
			auto obj = componentFactory->makeObject(type);
			BOOST_REQUIRE(obj != nullptr);
			obj->setName("bob");
			BOOST_CHECK_EQUAL(obj->getName(), "bob");
			obj->setName(std::string());
			BOOST_CHECK_EQUAL(obj->getName(), "");
			obj->set("", "empty"); //this should not throw an exception
			obj->set("", 0.34, defUnit);  //this should not throw an exception
			obj->setFlag("", false);  //This should not throw an exception
			obj->set("#unknown", "empty"); //this should not throw an exception
			obj->set("#unknown", 0.34, defUnit);  //this should not throw an exception
			obj->setFlag("#unknown", false);  //This should not throw an exception

			if (obj->isCloneable())
			{
				auto nobj=obj->clone();
				BOOST_CHECK(nobj != nullptr);
				if (nobj != nullptr)
				{
					BOOST_CHECK(typeid(nobj) == typeid(obj));
					delete(nobj);
				}
				else
				{
					std::cout << "unable to clone " << comp << "::" << type << '\n';
				}

			}
			delete obj;


		}
	}

}

BOOST_AUTO_TEST_SUITE_END ()
