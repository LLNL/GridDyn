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
#include <boost/test/floating_point_comparison.hpp>

/** these test cases test out the value converters
 */
#include "griddyn.h"

static const std::string ieee_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/");
static const std::string matlab_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/matlab_test_files/");
static const std::string other_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/other_test_cases/");

BOOST_AUTO_TEST_SUITE (basic_tests, * boost::unit_test::label("quick"))

/** just a check that in the simple case we do actually get the time back we requested*/
BOOST_AUTO_TEST_CASE (simple_load_test) 
{
    gridDynSimReference sim = gridDynSimulation_create("", "sim1");
    BOOST_CHECK(sim != nullptr);
    std::string file = ieee_test_directory + "ieee14.cdf";
    auto res = gridDynSimulation_loadfile(sim, file.c_str(), "");
    BOOST_CHECK(res == griddyn_ok);
    res = gridDynSimulation_run(sim);

    double time = gridDynSimulation_getCurrentTime(sim);

    BOOST_CHECK_CLOSE(time, 30.0,0.0001);
    auto obj=getSimulationObject(sim);

    gridDynObject_free(obj);
    BOOST_CHECK(res == griddyn_ok);

    gridDynSimulation_free(sim);
}

BOOST_AUTO_TEST_CASE(getResult_test)
{
    gridDynSimReference sim = gridDynSimulation_create("", "sim1");
    BOOST_CHECK(sim != nullptr);
    std::string file = ieee_test_directory + "ieee14.cdf";
    auto res = gridDynSimulation_loadfile(sim, file.c_str(), "");
    BOOST_CHECK(res == griddyn_ok);
    res = gridDynSimulation_powerflow(sim);

    int cnt = gridDynSimulation_busCount(sim);

    BOOST_CHECK_EQUAL(cnt,14);
    std::vector<double> voltages(cnt);
    std::vector<double> angles(cnt);
    int act = gridDynSimulation_getResults(sim, "voltage",voltages.data(), cnt);
    BOOST_REQUIRE_EQUAL(cnt,act);
    cnt = gridDynSimulation_getResults(sim, "angles",angles.data(), cnt);
    BOOST_REQUIRE_EQUAL(cnt, act);
    BOOST_CHECK_CLOSE(voltages[0], 1.06,0.001);
    BOOST_CHECK_SMALL(angles[0],  0.00001);
    BOOST_CHECK_CLOSE(voltages[1], 1.045, 0.001);
    BOOST_CHECK_CLOSE(voltages[2], 1.01, 0.001);
    BOOST_CHECK_CLOSE(voltages[8], 1.056, 0.1);
    BOOST_CHECK_CLOSE(angles[5], -14.22*3.1415927 / 180.0, 0.1);

    gridDynSimulation_free(sim);
}

BOOST_AUTO_TEST_CASE(getObject_tests)
{
    gridDynSimReference sim = gridDynSimulation_create("", "sim1");
    BOOST_CHECK(sim != nullptr);
    std::string file = ieee_test_directory + "ieee14.cdf";
    auto res = gridDynSimulation_loadfile(sim, file.c_str(), "");
    BOOST_CHECK(res == griddyn_ok);
    res = gridDynSimulation_powerflow(sim);

    auto obj = getSimulationObject(sim);

    auto bus2 = gridDynObject_getSubObject(obj, "bus", 8);
    gridDynObject_free(obj); //just making sure the bus object is disconnected from obj
    BOOST_CHECK(bus2 != nullptr);

    double result;
    auto status = gridDynObject_getValue(bus2, "voltage", &result);
    BOOST_CHECK(status == griddyn_ok);
    BOOST_CHECK_CLOSE(result, 1.056, 0.1);

    char name[50];
    int count= gridDynObject_getString(bus2, "name", name,50);

    std::string namestr(name);
    BOOST_CHECK_EQUAL(count, static_cast<int>(namestr.size()));
    BOOST_CHECK_EQUAL(namestr.compare(0, 5, "Bus 9"), 0);
    gridDynObject_free(bus2);
    gridDynSimulation_free(sim);
}

BOOST_AUTO_TEST_CASE(build_small_test_case)
{
	gridDynSimReference sim = gridDynSimulation_create("", "sim1");
	BOOST_CHECK(sim != nullptr);
	auto obj = getSimulationObject(sim);

	auto bus1 = gridDynObject_create("bus", "infinite");
	gridDynObject_setString(bus1, "name", "bus1");
	gridDynObject_add(obj, bus1);

	auto bus2 = gridDynObject_create("bus", "");
	gridDynObject_setString(bus2, "name", "bus2");
	gridDynObject_add(obj, bus2);

	auto ld1 = gridDynObject_create("load", "");
	gridDynObject_setValue(ld1, "p", 0.7);
	gridDynObject_setValue(ld1, "q", 0.3);
	auto status=gridDynObject_add(bus2, ld1);
	BOOST_CHECK(status == 0);

	auto lnk = gridDynObject_create("link", "ac");
	gridDynObject_setValue(lnk, "x", 0.1);
	gridDynObject_setValue(lnk, "r", 0.03);

	gridDynObject_add(obj, lnk);
	gridDynObject_setString(lnk, "from", "bus1");
	gridDynObject_setString(lnk, "to", "bus2");

	gridDynSimulation_powerflow(sim);

	double V;
	gridDynObject_getValueUnits(bus2, "voltage", "pu", &V);
	BOOST_CHECK(V < 1.0);
	double A;
	gridDynObject_getValueUnits(bus2, "angle", "deg", &A);
	BOOST_CHECK(A < 0);
	gridDynObject_free(bus1);
	gridDynObject_free(ld1);
	gridDynObject_free(lnk);
	gridDynSimulation_free(sim);

	double V2;
	gridDynObject_getValue(bus2, "voltage", &V2);
	BOOST_CHECK_EQUAL(V, V2);
	gridDynObject_free(bus2);
    gridDynObject_free(obj);
}

BOOST_AUTO_TEST_SUITE_END ()
