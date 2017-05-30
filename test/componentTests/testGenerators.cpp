/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
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
#include "gridBus.h"
#include "generators/gridDynGenerator.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include "utilities/timeSeriesMulti.h"

#define GEN_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/gen_tests/"

BOOST_FIXTURE_TEST_SUITE (gen_tests, gridDynSimulationTestFixture)

//TODO convert to a BOOST_DATA_TEST
BOOST_AUTO_TEST_CASE(gen_test_remote)
{

  std::string fname = std::string(GEN_TEST_DIRECTORY "test_gen_remote.xml");
  detailedStageCheck(fname, gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
}

BOOST_AUTO_TEST_CASE(gen_test_remoteb)
{

  std::string fname = std::string(GEN_TEST_DIRECTORY "test_gen_remote_b.xml");
  detailedStageCheck(fname, gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
}


BOOST_AUTO_TEST_CASE(gen_test_remote2)
{

  std::string fname = std::string(GEN_TEST_DIRECTORY "test_gen_dualremote.xml");
  detailedStageCheck(fname, gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
 
}


BOOST_AUTO_TEST_CASE(gen_test_remote2b)
{
  std::string fname = std::string(GEN_TEST_DIRECTORY "test_gen_dualremote_b.xml");
  detailedStageCheck(fname, gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
}

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(gen_test_isoc)
{
	std::string fname = std::string(GEN_TEST_DIRECTORY "test_isoc2.xml");

	gds = readSimXMLFile(fname);

	gds->set("recorddirectory", GEN_TEST_DIRECTORY);

	gds->run();

	std::string recname = std::string(GEN_TEST_DIRECTORY "datafile.dat");
	timeSeriesMulti<> ts3(recname);
	BOOST_REQUIRE(ts3.size() > 30);
	BOOST_CHECK(ts3.data(0,30) < 0.995);
	BOOST_CHECK(ts3[0].back() > 1.0);

	BOOST_CHECK((ts3.data(1,0) - ts3[1].back()) > 0.199);
	remove(recname.c_str());

}
#endif
BOOST_AUTO_TEST_SUITE_END()