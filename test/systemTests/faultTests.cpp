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

#include "../testHelper.h"
#include "core/objectFactory.hpp"
#include "gmlc/utilities/vectorOps.hpp"
#include "griddyn/Exciter.h"
#include "griddyn/GenModel.h"
#include "griddyn/Generator.h"
#include "griddyn/Governor.h"
#include "griddyn/gridBus.h"
#include "griddyn/gridDynSimulation.h"
#include "griddyn/links/acLine.h"
#include "griddyn/relays/breaker.h"
#include "griddyn/relays/fuse.h"
#include "griddyn/simulation/diagnostics.h"
#include <cmath>

#include <boost/test/unit_test.hpp>

#include <boost/test/floating_point_comparison.hpp>

/** these test cases test out the various generator components ability to handle faults
*/

using namespace griddyn;

static const std::string fault_test_directory(GRIDDYN_TEST_DIRECTORY "/fault_tests/");

BOOST_FIXTURE_TEST_SUITE(fault_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE(fault_test1, *boost::unit_test::label("quick"))
{
    std::string fileName = fault_test_directory + "fault_test1.xml";

    auto cof = coreObjectFactory::instance();
    coreObject* obj = nullptr;

    auto genlist = cof->getTypeNames("genmodel");

    for (auto& gname : genlist) {
        gds = readSimXMLFile(fileName);
        gds->consolePrintLevel = print_level::no_print;
        obj = cof->createObject("genmodel", gname);
        BOOST_REQUIRE(obj != nullptr);

        Generator* gen = gds->getGen(0);
        gen->add(obj);

        // run till just after the fault
        gds->run(1.01);
        if (gds->hasDynamics()) {
            BOOST_CHECK_MESSAGE(gds->currentProcessState() ==
                                    gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,
                                "Model " << gname << " failed to run past fault");
            auto mmatch = runJacobianCheck(gds, cDaeSolverMode);
            BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure after fault");
        } else {
            continue;
        }

        // run till just after the fault clears
        gds->run(1.2);

        BOOST_CHECK_MESSAGE(gds->currentProcessState() ==
                                gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,
                            "Model " << gname << " failed to run past fault clear");
        auto mmatch = runJacobianCheck(gds, cDaeSolverMode);

        BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure");

        gds->run();
        BOOST_CHECK_MESSAGE(gds->currentProcessState() ==
                                gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,
                            "Model " << gname << " failed to run to completion");

        std::vector<double> volts;
        gds->getVoltage(volts);

        BOOST_CHECK_MESSAGE(volts[1] > 0.96, "Model " << gname << " voltage issue v=" << volts[1]);
    }
}

// testing with an exciter added
BOOST_AUTO_TEST_CASE(fault_test2, *boost::unit_test::label("quick"))
{
    std::string fileName = fault_test_directory + "fault_test1.xml";

    auto cof = coreObjectFactory::instance();
    auto genlist = cof->getTypeNames("genmodel");

    for (auto& gname : genlist) {
        gds = readSimXMLFile(fileName);
        gds->consolePrintLevel = print_level::no_print;
        auto obj = cof->createObject("genmodel", gname);
        BOOST_REQUIRE(obj != nullptr);

        Generator* gen = gds->getGen(0);
        gen->add(obj);

        auto exc = cof->createObject("exciter", "type1");
        gen->add(exc);
        // run till just after the fault
        gds->run(1.01);

        BOOST_CHECK_MESSAGE(gds->currentProcessState() ==
                                gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,
                            "Model " << gname << " failed to run past fault");
        auto mmatch = runJacobianCheck(gds, cDaeSolverMode);
        BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure after fault");

        // run till just after the fault clears
        gds->run(1.2);

        BOOST_CHECK_MESSAGE(gds->currentProcessState() ==
                                gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,
                            "Model " << gname << " failed to run past fault clear");
        mmatch = runJacobianCheck(gds, cDaeSolverMode);

        BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure");

        gds->run();
        BOOST_CHECK_MESSAGE(gds->currentProcessState() ==
                                gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,
                            "Model " << gname << " failed to run to completion");

        std::vector<double> volts;
        gds->getVoltage(volts);

        BOOST_CHECK_MESSAGE(volts[1] > 0.96, "Model " << gname << " voltage issue v=" << volts[1]);
    }
}
//#endif

// testing with a governor added
BOOST_AUTO_TEST_CASE(fault_test3, *boost::unit_test::label("quick"))
{
    std::string fileName = fault_test_directory + "fault_test1.xml";

    auto cof = coreObjectFactory::instance();
    auto genlist = cof->getTypeNames("genmodel");

    for (auto& gname : genlist) {
        gds = readSimXMLFile(fileName);
        gds->consolePrintLevel = print_level::no_print;
        auto obj = cof->createObject("genmodel", gname);
        BOOST_REQUIRE(obj != nullptr);

        Generator* gen = gds->getGen(0);
        gen->add(obj);

        auto exc = cof->createObject("exciter", "type1");
        gen->add(exc);

        auto gov = cof->createObject("governor", "tgov1");
        gen->add(gov);
        // run till just after the fault
        gds->run(1.01);

        BOOST_CHECK_MESSAGE(gds->currentProcessState() ==
                                gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,
                            "Model " << gname << " failed to run past fault");
        auto mmatch = runJacobianCheck(gds, cDaeSolverMode);
        BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure after fault");

        // run till just after the fault clears
        gds->run(1.2);

        BOOST_CHECK_MESSAGE(gds->currentProcessState() ==
                                gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,
                            "Model " << gname << " failed to run past fault clear");
        mmatch = runJacobianCheck(gds, cDaeSolverMode);

        BOOST_CHECK_MESSAGE(mmatch == 0, "Model " << gname << " Jacobian failure");

        gds->run();
        BOOST_CHECK_MESSAGE(gds->currentProcessState() ==
                                gridDynSimulation::gridState_t::DYNAMIC_COMPLETE,
                            "Model " << gname << " failed to run to completion");

        std::vector<double> volts;
        gds->getVoltage(volts);

        BOOST_CHECK_MESSAGE(volts[1] > 0.96, "Model " << gname << " voltage issue v=" << volts[1]);
    }
}

BOOST_AUTO_TEST_CASE(geco_fault_case)
{
    std::string fileName = fault_test_directory + "geco_fault_uncoupled.xml";

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::debug;
    int retval = gds->dynInitialize();

    BOOST_CHECK_EQUAL(retval, 0);

    int mmatch = runJacobianCheck(gds, cDaeSolverMode);

    BOOST_REQUIRE_EQUAL(mmatch, 0);
    mmatch = runResidualCheck(gds, cDaeSolverMode);

    gds->run();
    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    // simpleRunTestXML(fileName);
}

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_dynamic)
{
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);

    // test a bunch of different link parameters to make sure all the solve properly
    std::string fileName = fault_test_directory + "link_fault2.xml";

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::trace;
    gds->consolePrintLevel = print_level::warning;

    gds->run();

    std::vector<double> v;
    gds->getVoltage(v);
    BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) { return (a > 0.75); }));

    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}

#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_fuse)
{
    // test a fuse
    std::string fileName = fault_test_directory + "link_fault_fuse.xml";

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::warning;
    auto obj = dynamic_cast<fuse*>(gds->getRelay(0));
    BOOST_REQUIRE(obj != nullptr);
    gds->run();
    auto lobj = dynamic_cast<Link*>(gds->find("bus2_to_bus3"));
    BOOST_CHECK(lobj->isConnected() == false);
    std::vector<double> v;
    gds->getVoltage(v);
    BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) { return (a > 0.80); }));

    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_fuse2)
{
    // test whether fuses are working properly
    std::string fileName = fault_test_directory + "link_fault_fuse2.xml";

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::debug;
    auto obj = dynamic_cast<Link*>(gds->find("bus8_to_bus9"));
    BOOST_REQUIRE(obj != nullptr);
    gds->run();

    std::vector<double> v;
    gds->getVoltage(v);
    BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) { return (a > 0.80); }));

    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_fuse3)
{
    // test a bunch of different link parameters to make sure all the solve properly
    std::string fileName = fault_test_directory + "link_fault_fuse3.xml";

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::debug;
    // auto obj = dynamic_cast<Link *>(gds->find("bus2_to_bus3"));
    gds->dynInitialize();
    int mmatch = JacobianCheck(gds, cDaeSolverMode);
    if (mmatch > 0) {
        printStateNames(gds, cDaeSolverMode);
    }
    // BOOST_REQUIRE(obj != nullptr);
    gds->run();
    // BOOST_CHECK(obj->isConnected() == false);
    std::vector<double> v;
    gds->getVoltage(v);
    BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) { return (a > 0.80); }));

    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_breaker)
{
    // test a bunch of different link parameters to make sure all the solve properly
    std::string fileName = fault_test_directory + "link_fault_breaker.xml";

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::warning;
    auto obj = dynamic_cast<breaker*>(gds->getRelay(0));
    BOOST_REQUIRE(obj != nullptr);
    gds->run();
    auto lobj = dynamic_cast<Link*>(gds->find("bus2_to_bus3"));
    BOOST_CHECK(lobj->isConnected() == false);
    std::vector<double> v;
    gds->getVoltage(v);
    BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) { return (a > 0.80); }));

    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_breaker2)
{
    // test a bunch of different link parameters to make sure all the solve properly
    std::string fileName = fault_test_directory + "link_fault_breaker2.xml";

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::warning;
    auto obj = dynamic_cast<breaker*>(gds->getRelay(0));
    BOOST_REQUIRE(obj != nullptr);
    gds->run();
    auto lobj = dynamic_cast<Link*>(gds->find("bus8_to_bus9"));
    BOOST_CHECK(lobj->isConnected() == false);
    std::vector<double> v;
    gds->getVoltage(v);
    BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) { return (a > 0.80); }));

    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_breaker3)
{
    // test a bunch of different link parameters to make sure all the solve properly
    std::string fileName = fault_test_directory + "link_fault_breaker3.xml";

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::warning;
    auto obj = dynamic_cast<breaker*>(gds->getRelay(0));
    BOOST_REQUIRE(obj != nullptr);
    gds->run();
    auto lobj = dynamic_cast<Link*>(gds->find("bus8_to_bus9"));
    BOOST_CHECK(lobj->isConnected() == true);
    std::vector<double> v;
    gds->getVoltage(v);
    BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) { return (a > 0.80); }));

    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(link_test_fault_breaker4)
{
    // test a bunch of different link parameters to make sure all the solve properly
    std::string fileName = fault_test_directory + "link_fault_breaker4.xml";

    gds = readSimXMLFile(fileName);
    gds->consolePrintLevel = print_level::warning;
    auto obj = dynamic_cast<breaker*>(gds->getRelay(0));
    BOOST_REQUIRE(obj != nullptr);
    gds->run();
    auto lobj = dynamic_cast<Link*>(gds->find("bus8_to_bus9"));
    BOOST_CHECK(lobj->isConnected() == true);
    std::vector<double> v;
    gds->getVoltage(v);
    BOOST_CHECK(std::all_of(v.begin(), v.end(), [](double a) { return (a > 0.80); }));

    BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}
#endif
BOOST_AUTO_TEST_SUITE_END()
