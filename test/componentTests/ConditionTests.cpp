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


#include "griddyn/measurement/Condition.h"
#include "griddyn/links/acLine.h"
#include "griddyn/gridBus.h"

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
// test case for coreObject object


BOOST_AUTO_TEST_SUITE(condition_tests)

using namespace griddyn;

/** test basic operations */
BOOST_AUTO_TEST_CASE(basic_tests)
{
	gridBus B;
	B.setVoltageAngle(1.0, 0.05);

	auto cond=make_condition("voltage", "<", 0.7, &B);

	BOOST_CHECK_CLOSE(cond->evalCondition(), 0.3, 0.0001);

	BOOST_CHECK_EQUAL(cond->checkCondition(), false);
	BOOST_CHECK_CLOSE(cond->getVal(1), 1.0,0.00001);

	BOOST_CHECK_CLOSE(cond->getVal(2), 0.7,0.000001);
}

/** test basic operations */
BOOST_AUTO_TEST_CASE(basic_test2)
{
	gridBus B;
	B.setVoltageAngle(1.0, 0.05);

	auto cond = make_condition("voltage-0.4", "<", 0.7, &B);

	BOOST_CHECK_CLOSE(cond->evalCondition(), -0.1, 0.0001);

	BOOST_CHECK_EQUAL(cond->checkCondition(), true);
	BOOST_CHECK_CLOSE(cond->getVal(1), 0.6, 0.00001);

	BOOST_CHECK_CLOSE(cond->getVal(2), 0.7, 0.000001);
}

/** test basic operations */
BOOST_AUTO_TEST_CASE(link_tests)
{
	gridBus B1;
	B1.setVoltageAngle(1.0, 0.05);
	gridBus B2;
	B2.setVoltageAngle(1.05, -0.05);
	acLine L2;
	L2.set("x", 0.01);
	L2.set("r", 0.001);

	L2.updateBus(&B1, 1);
	L2.updateBus(&B2, 2);
	L2.updateLocalCache();

	auto cond = make_condition("current1>current2", &L2);

	auto C1 = L2.getCurrent(1);
	auto C2 = L2.getCurrent(2);
	BOOST_CHECK_SMALL(cond->evalCondition()-(C1-C2), 0.0001);

	BOOST_CHECK_EQUAL(cond->checkCondition(), C1>C2);
	BOOST_CHECK_CLOSE(cond->getVal(1), C1, 0.00001);
	BOOST_CHECK_CLOSE(cond->getVal(2), C2, 0.00001);

}

/** test basic operations */
BOOST_AUTO_TEST_CASE(link_tests_queries)
{
	gridBus B1;
	B1.setVoltageAngle(1.0, 0.05);
	gridBus B2;
	B2.setVoltageAngle(1.05, -0.05);
	acLine L2;
	L2.set("x", 0.01);
	L2.set("r", 0.001);
	L2.set("g", 0.05);

	L2.updateBus(&B1, 1);
	L2.updateBus(&B2, 2);
	L2.updateLocalCache();

	auto cond = make_condition("current1-current2",">",0.01,&L2);

	auto C1 = L2.getCurrent(1);
	auto C2 = L2.getCurrent(2);

	BOOST_CHECK_CLOSE(cond->getVal(1), C1-C2, 0.00001);
	BOOST_CHECK_CLOSE(cond->getVal(2), 0.01, 0.00001);

}

/** test basic operations */
BOOST_AUTO_TEST_CASE(link_tests_queries2)
{
	gridBus B1;
	B1.setVoltageAngle(1.0, 0.05);
	gridBus B2;
	B2.setVoltageAngle(1.05, -0.05);
	acLine L2;
	L2.set("x", 0.01);
	L2.set("r", 0.001);
	L2.set("g", 0.05);

	L2.updateBus(&B1, 1);
	L2.updateBus(&B2, 2);
	L2.updateLocalCache();

	auto cond = make_condition("(current1-current2)*(current1-current2)", ">", 0.01, &L2);

	auto C1 = L2.getCurrent(1);
	auto C2 = L2.getCurrent(2);

	BOOST_CHECK_CLOSE(cond->getVal(1), (C1 - C2)*(C1-C2), 0.00001);
	BOOST_CHECK_CLOSE(cond->getVal(2), 0.01, 0.00001);

}

/** test basic operations */
BOOST_AUTO_TEST_CASE(link_tests_queries3)
{
	gridBus B1;
	B1.setVoltageAngle(1.0, 0.05);
	gridBus B2;
	B2.setVoltageAngle(1.05, -0.05);
	acLine L2;
	L2.set("x", 0.01);
	L2.set("r", 0.001);
	L2.set("g", 0.05);

	L2.updateBus(&B1, 1);
	L2.updateBus(&B2, 2);
	L2.updateLocalCache();

	auto cond = make_condition("hypot(abs(realcurrent1-realcurrent2),abs(imagcurrent1-imagcurrent2))", ">", 0.01, &L2);

	auto R1 = L2.getRealCurrent(1);
	auto R2 = L2.getRealCurrent(2);
	auto I1 = L2.getImagCurrent(1);
	auto I2 = L2.getImagCurrent(2);

	BOOST_CHECK_CLOSE(cond->getVal(1), std::hypot(std::abs(R1-R2),std::abs(I1-I2)), 0.00001);
	BOOST_CHECK_CLOSE(cond->getVal(2), 0.01, 0.00001);

}

BOOST_AUTO_TEST_SUITE_END()
