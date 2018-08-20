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

#include "core/objectFactory.hpp"
#include "griddyn/Relay.h"
#include "griddyn/simulation/diagnostics.h"
#include "griddyn/blocks/blockLibrary.h"
#include "../testHelper.h"
#include "utilities/timeSeriesMulti.hpp"
#include "utilities/vectorOps.hpp"
#include <cstdio>
#include <map>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#define BLOCK_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/block_tests/"

static const std::string block_test_directory = std::string (GRIDDYN_TEST_DIRECTORY "/block_tests/");
BOOST_FIXTURE_TEST_SUITE (block_tests, gridDynSimulationTestFixture, * boost::unit_test::label("quick"))
using namespace griddyn;
using namespace griddyn::blocks;
BOOST_AUTO_TEST_CASE (test_gain_block)
{
    std::string fileName = block_test_directory + "block_tests1.xml";

    gds = readSimXMLFile (fileName);

    dynamicInitializationCheck (fileName);
    gds->set ("recorddirectory", BLOCK_TEST_DIRECTORY);
    gds->run ();
    requireState (gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

    std::string recname = std::string (BLOCK_TEST_DIRECTORY "blocktest.dat");
    timeSeriesMulti<> ts3;
    ts3.loadBinaryFile (recname);
    BOOST_REQUIRE (ts3.size () >= 15);
    BOOST_CHECK_CLOSE (ts3.data (0, 5) * 5, ts3.data (1, 5), 0.000001);
    BOOST_CHECK_CLOSE (ts3.data (0, 15) * 5, ts3.data (1, 15), 0.000001);
    int ret = remove (recname.c_str ());

    BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (block_test2)
{
    std::string fileName = std::string (BLOCK_TEST_DIRECTORY "block_tests2.xml");

    dynamicInitializationCheck (fileName);
    gds->set ("recorddirectory", BLOCK_TEST_DIRECTORY);
    gds->run ();
    requireState (gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

    std::string recname = std::string (BLOCK_TEST_DIRECTORY "blocktest.dat");
    timeSeriesMulti<> ts3 (recname);

    BOOST_CHECK_CLOSE (ts3.data (0, 5) * 5, ts3.data (1, 5), 0.001);
    BOOST_CHECK_CLOSE (ts3.data (0, 280) * 5, ts3.data (1, 280), 0.001);
    int ret = remove (recname.c_str ());

    BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (block_test3)
{
    std::string fileName = std::string (BLOCK_TEST_DIRECTORY "block_tests3.xml");

    dynamicInitializationCheck (fileName);
    gds->set ("recorddirectory", BLOCK_TEST_DIRECTORY);
    gds->run ();
    requireState (gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

    std::string recname = std::string (BLOCK_TEST_DIRECTORY "blocktest.dat");
    timeSeriesMulti<> ts3 (recname);
    // ts3.loadBinaryFile(recname);

    BOOST_CHECK_SMALL (ts3.data (1, 5), 0.00001);
    BOOST_CHECK_SMALL (ts3.data (1, 280), 0.001);
    int ret = remove (recname.c_str ());

    BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (block_test4)
{
    std::string fileName = std::string (BLOCK_TEST_DIRECTORY "block_tests4.xml");

    dynamicInitializationCheck (fileName);

    gds->set ("recorddirectory", BLOCK_TEST_DIRECTORY);
    gds->run ();
    requireState (gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

    std::string recname = std::string (BLOCK_TEST_DIRECTORY "blocktest.dat");
    timeSeriesMulti<> ts3;
    ts3.loadBinaryFile (recname);

    BOOST_CHECK_CLOSE (ts3.data (1, 5), -0.5, 0.01);
    double iv = -0.5;
    index_t pp;
    for (pp = 0; pp < static_cast<index_t> (ts3.size ()); ++pp)
    {
        iv += 100 * ts3.data (0, pp) * 0.01;
    }
    BOOST_CHECK_CLOSE (ts3.data (1, pp - 1), iv, 1);
    int ret = remove (recname.c_str ());

    BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (block_test5)
{
    std::string fileName = std::string (BLOCK_TEST_DIRECTORY "block_tests5.xml");

    dynamicInitializationCheck (fileName);
    gds->set ("recorddirectory", BLOCK_TEST_DIRECTORY);
    gds->run ();
    requireState (gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

    std::string recname = std::string (BLOCK_TEST_DIRECTORY "blocktest.dat");
    timeSeriesMulti<> ts3;
    ts3.loadBinaryFile (recname);

    BOOST_CHECK_SMALL (ts3.data (1, 5), 0.0001);
    auto vm = absMax (ts3[0]);
    auto vm2 = absMax (ts3[1]);
    BOOST_CHECK_CLOSE (vm2, vm * 5, 1);
    int ret = remove (recname.c_str ());

    BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (block_test6)
{
    std::string fileName = std::string (BLOCK_TEST_DIRECTORY "block_tests6.xml");

    dynamicInitializationCheck (fileName);
    gds->set ("recorddirectory", BLOCK_TEST_DIRECTORY);
    gds->run ();
    requireState (gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);

    std::string recname = std::string (BLOCK_TEST_DIRECTORY "blocktest.dat");
    timeSeriesMulti<> ts3 (recname);
    // ts3.loadBinaryFile(recname);

    BOOST_CHECK_CLOSE (ts3.data (1, 5), 1.0, 0.01);

    BOOST_CHECK_CLOSE (ts3.data (1, 2000), 1.0, 0.01);
    remove (recname.c_str ());

    recname = std::string (BLOCK_TEST_DIRECTORY "blocktest2.dat");
    ts3.loadBinaryFile (recname);

    BOOST_CHECK_CLOSE (ts3.data (1, 5), 1.0, 0.01);

    BOOST_CHECK_CLOSE (ts3.data (1, 200), 1.0, 0.01);
    int ret = remove (recname.c_str ());

    BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (deadband_block_test)
{
    std::string fileName = std::string (BLOCK_TEST_DIRECTORY "block_tests_deadband.xml");

    gds = readSimXMLFile (fileName);
    gds->solverSet ("powerflow", "printlevel", 0);
    gds->solverSet ("dynamic", "printlevel", 0);
    int retval = gds->dynInitialize ();

    BOOST_CHECK_EQUAL (retval, 0);

    int mmatch = runJacobianCheck (gds, cDaeSolverMode, 1e-5);

    BOOST_REQUIRE_EQUAL (mmatch, 0);
    mmatch = runResidualCheck (gds, cDaeSolverMode);

    BOOST_REQUIRE_EQUAL (mmatch, 0);
    gds->set ("recorddirectory", BLOCK_TEST_DIRECTORY);
    gds->run ();
    BOOST_REQUIRE_GT (double(gds->getSimulationTime()), 7.9);

    std::string recname = std::string (BLOCK_TEST_DIRECTORY "blocktest.dat");
    timeSeriesMulti<> ts3;
    ts3.loadBinaryFile (recname);

    auto mx =
      std::any_of (ts3[1].begin (), ts3[1].end (), [](double a) { return ((a > 0.400001) && (a < 0.4999999)); });
    BOOST_CHECK_EQUAL (mx, false);

    int ret = remove (recname.c_str ());

    BOOST_CHECK_EQUAL (ret, 0);
}

using blockdescpair = std::pair<std::string, std::vector<std::pair<std::string, double>>>;

const std::vector<blockdescpair> blockparamMap{
  {"basic", {std::make_pair ("k", 2.3), std::make_pair ("max", 0.1)}},
  {"delay", {std::make_pair ("t1", 0.5)}},
  {"integral", {std::make_pair ("iv", 0.14)}},
  {"derivative", {std::make_pair ("t", 0.25)}},
  {"fder", {std::make_pair ("t1", 0.25), std::make_pair ("t2", 0.1)}},
  {"deadband", {std::make_pair ("db", 0.1), std::make_pair ("ramp", 0.03)}},
  {"db", {std::make_pair ("db", 0.08)}},
  {"pid",
   {std::make_pair ("p", 0.7), std::make_pair ("i", 0.02), std::make_pair ("d", 0.28), std::make_pair ("t", 0.2)}},
  {"control", {std::make_pair ("t1", 0.2), std::make_pair ("t2", 0.1)}},
  {"function", {std::make_pair ("gain", kPI), std::make_pair ("bias", -0.05)}},
  {"func", {std::make_pair ("arg", 2.35)}},
};

const std::map<std::string, std::vector<std::pair<std::string, std::string>>> blockparamMapSt{
  {"function", {std::make_pair ("func", "sin")}},
  {"func", {std::make_pair ("func", "pow")}},
  {"db", {std::make_pair ("flags", "shifted")}},
};

namespace data = boost::unit_test::data;

BOOST_DATA_TEST_CASE_F (gridDynSimulationTestFixture, compare_block_test, data::xrange (11), caseIndex)
{
    std::string fileName = std::string (BLOCK_TEST_DIRECTORY "block_test_compare.xml");
    auto bf = coreObjectFactory::instance ()->getFactory ("block");

    gds = readSimXMLFile (fileName);
    BOOST_REQUIRE (gds != nullptr);
    gds->solverSet ("powerflow", "printlevel", 0);
    gds->solverSet ("dynamic", "printlevel", 0);
    gds->set ("recorddirectory", BLOCK_TEST_DIRECTORY);
    gds->consolePrintLevel = print_level::warning;
    auto rel1 = gds->getRelay (0);
    auto rel2 = gds->getRelay (1);
    auto &plist = blockparamMap[caseIndex];

    auto bb1 = static_cast<Block *> (bf->makeObject (plist.first));
    auto bb2 = static_cast<Block *> (bf->makeObject (plist.first));

    BOOST_REQUIRE (bb1 != nullptr);
    BOOST_REQUIRE (bb2 != nullptr);
    for (const auto &pp : plist.second)
    {
        bb1->set (pp.first, pp.second);
        bb2->set (pp.first, pp.second);
    }

    auto kfind = blockparamMapSt.find (plist.first);
    if (kfind != blockparamMapSt.end ())
    {
        auto bps = blockparamMapSt.at (plist.first);
        for (const auto &pp : bps)
        {
            bb1->set (pp.first, pp.second);

            bb2->set (pp.first, pp.second);
        }
    }
    rel1->add (bb1);
    rel2->add (bb2);
    int retval = gds->dynInitialize ();

    BOOST_CHECK_EQUAL (retval, 0);

    int mmatch = runJacobianCheck (gds, cDaeSolverMode, 1e-5);
    if (mmatch > 0)
    {
        printf (" mismatching Jacobian in %s\n", plist.first.c_str ());
        BOOST_REQUIRE_EQUAL (mmatch, 0);
    }

    mmatch = runResidualCheck (gds, cDaeSolverMode);
    if (mmatch > 0)
    {
        printf (" mismatching residual in %s\n", plist.first.c_str ());
        BOOST_REQUIRE_EQUAL (mmatch, 0);
    }

    gds->run ();
    if (gds->getSimulationTime() < 7.99)
    {
        runJacobianCheck (gds, cDaeSolverMode);
        runResidualCheck (gds, cDaeSolverMode);
        BOOST_REQUIRE_MESSAGE (gds->getSimulationTime() > 7.99, "Block " << plist.first << " failed to solve");
    }

    std::string recname = std::string (BLOCK_TEST_DIRECTORY "blocktest.dat");
    timeSeriesMulti<> ts3;
    ts3.loadBinaryFile (recname);
    std::vector<double> df (ts3.size ());
    compareVec (ts3[1], ts3[2], df);
    auto mx = absMax (df);
    auto adf = mean (df);
    BOOST_CHECK ((mx < 1e-2) || (adf < 2e-3));
    int ret = remove (recname.c_str ());

    if ((mx > 1e-2) && (adf > 2e-3))
    {
        printf (" mismatching results in %s\n", plist.first.c_str ());
    }
    BOOST_CHECK_EQUAL (ret, 0);
}

#ifdef LOAD_CVODE
/** test the control block if they can handle a differential only Jacobian and an algebraic only Jacobian
 */
// BOOST_AUTO_TEST_CASE (block_alg_diff_jac_test)
BOOST_DATA_TEST_CASE_F (gridDynSimulationTestFixture, block_alg_diff_jac_test, data::xrange (11), caseIndex)
{
    std::string fileName = std::string (BLOCK_TEST_DIRECTORY "block_test_compare.xml");

    auto bf = coreObjectFactory::instance ()->getFactory ("block");
    auto &plist = blockparamMap[caseIndex];

    gds = readSimXMLFile (fileName);
    BOOST_REQUIRE (gds != nullptr);
    gds->solverSet ("powerflow", "printlevel", 0);
    gds->solverSet ("dynamic", "printlevel", 0);
    gds->set ("recorddirectory", BLOCK_TEST_DIRECTORY);
    gds->consolePrintLevel = print_level::warning;
    auto rel1 = gds->getRelay (0);
    auto rel2 = gds->getRelay (1);

    auto bb1 = static_cast<Block *> (bf->makeObject (plist.first));
    auto bb2 = static_cast<Block *> (bf->makeObject (plist.first));

    BOOST_REQUIRE (bb1 != nullptr);
    BOOST_REQUIRE (bb2 != nullptr);
    for (const auto &pp : plist.second)
    {
        bb1->set (pp.first, pp.second);

        bb2->set (pp.first, pp.second);
    }

    auto kfind = blockparamMapSt.find (plist.first);
    if (kfind != blockparamMapSt.end ())
    {
        auto bps = blockparamMapSt.at (plist.first);
        for (const auto &pp : bps)
        {
            bb1->set (pp.first, pp.second);

            bb2->set (pp.first, pp.second);
        }
    }
    rel1->add (bb1);
    rel2->add (bb2);
    int retval = gds->dynInitialize ();

    BOOST_CHECK_EQUAL (retval, 0);
    auto mmatch = runResidualCheck (gds, cDaeSolverMode);
    BOOST_REQUIRE_MESSAGE (mmatch == 0, "Block " << plist.first << " residual issue");
    mmatch = runDerivativeCheck (gds, cDaeSolverMode);
    BOOST_REQUIRE_MESSAGE (mmatch == 0, "Block " << plist.first << " derivative issue");
    mmatch = runAlgebraicCheck (gds, cDaeSolverMode);
    BOOST_REQUIRE_MESSAGE (mmatch == 0, "Block " << plist.first << " algebraic issue");
    mmatch = runJacobianCheck (gds, cDynDiffSolverMode);
    BOOST_REQUIRE_MESSAGE (mmatch == 0, "Block " << plist.first << " Jacobian dynDiff issue");
    mmatch = runJacobianCheck (gds, cDynAlgSolverMode);
    BOOST_REQUIRE_MESSAGE (mmatch == 0, "Block " << plist.first << " Jacobian dynAlg issue");
}

#endif
BOOST_AUTO_TEST_SUITE_END ()
