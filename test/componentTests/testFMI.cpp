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

#include "fmi/fmiGDinfo.h"
#include "fmi/fmi_import/fmiImport.h"
#include "fmi/fmi_import/fmiObjects.h"
#include "griddyn/gridBus.h"
#include "griddyn/simulation/diagnostics.h"
#include "gmlc/utilities/vectorOps.hpp"
#include <boost/filesystem.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

// test case for coreObject object

#include "griddyn/loads/approximatingLoad.h"
#include "fmi/fmi_models/fmiMELoad3phase.h"
#include "../testHelper.h"


static const std::string fmi_test_directory (GRIDDYN_TEST_DIRECTORY "/fmi_tests/");
static const std::string fmu_directory (GRIDDYN_TEST_DIRECTORY "/fmi_tests/test_fmus/");

// create a test fixture that makes sure everything gets deleted properly

using namespace boost::filesystem;
BOOST_FIXTURE_TEST_SUITE (fmi_tests, gridDynSimulationTestFixture, * boost::unit_test::label("quick"))

BOOST_AUTO_TEST_CASE (test_fmi_xml)
{
    std::string fmu = fmu_directory + "Rectifier.fmu";
    path rectDir (fmu_directory + "Rectifier");
    fmiLibrary rectFmu (fmu);
    BOOST_CHECK (rectFmu.isXmlLoaded ());
    BOOST_CHECK (rectFmu.getCounts ("states") == 4);
    BOOST_CHECK (rectFmu.getCounts ("outputs") == 8);
    BOOST_CHECK (rectFmu.getCounts ("parameters") == 23);
    BOOST_CHECK (rectFmu.getCounts ("unit") == 13);
    auto vars = rectFmu.getCounts ("variables");
    BOOST_CHECK (vars == 190);
    BOOST_CHECK (rectFmu.getCounts ("locals") == vars - 23 - 8);
    rectFmu.close ();
    BOOST_REQUIRE_MESSAGE (exists (rectDir), "fmu directory does not exist");
    try
    {
        remove_all (rectDir);
    }
    catch (filesystem_error const &e)
    {
        BOOST_WARN_MESSAGE (false, "unable to remove the folder: " + std::string (e.what ()));
    }

    // test second fmu with different load mechanics
    std::string fmu2 = fmu_directory + "Rectifier2.fmu";
    path rectDir2 (fmu_directory + "Rectifier2");
    fmiLibrary rect2Fmu (fmu2, rectDir2.string ());
    BOOST_CHECK (rect2Fmu.isXmlLoaded ());
    BOOST_CHECK (rect2Fmu.getCounts ("states") == 4);
    BOOST_CHECK (rect2Fmu.getCounts ("outputs") == 1);
    BOOST_CHECK (rect2Fmu.getCounts ("parameters") == 0);
    BOOST_CHECK (rect2Fmu.getCounts ("units") == 2);
    vars = rect2Fmu.getCounts ("variables");
    BOOST_CHECK (vars == 61);
    BOOST_CHECK (rect2Fmu.getCounts ("locals") == vars - 2);

    rect2Fmu.close ();
    BOOST_REQUIRE_MESSAGE (exists (rectDir2), "fmu directory does not exist");
    try
    {
        remove_all (rectDir2);
    }
    catch (filesystem_error const &e)
    {
        BOOST_WARN_MESSAGE (false, "unable to remove the folder: " + std::string (e.what ()));
    }
}

#ifdef _WIN32
BOOST_AUTO_TEST_CASE (test_fmi_load_shared)
{
    std::string fmu = fmu_directory + "rectifier.fmu";
    path rectDir (fmu_directory + "rectifier");
    fmiLibrary rectFmu (fmu);
    rectFmu.loadSharedLibrary ();
    BOOST_REQUIRE (rectFmu.isSoLoaded ());

    auto b = rectFmu.createModelExchangeObject ("rctf");
    BOOST_REQUIRE (b);
    b->setMode (fmuMode::initializationMode);
    auto v = b->get<double> ("VAC");
    BOOST_CHECK_CLOSE (v, 400.0, 0.0001);
    auto phase = b->get<double> ("SineVoltage1.phase");
    BOOST_CHECK_CLOSE (phase, 0.0, 0.0001);

    b->set ("VAC", 394.23);
    v = b->get<double> ("VAC");
    BOOST_CHECK_CLOSE (v, 394.23, 0.0001);

	rectFmu.close();
    try
    {
        remove_all (rectDir);
    }
    catch (filesystem_error const &e)
    {
        BOOST_WARN_MESSAGE (false, "unable to remove the folder: " + std::string (e.what ()));
    }
}

#endif

#ifdef _WIN32
#ifndef _WIN64
BOOST_AUTO_TEST_CASE(test_3phase_fmu)
{
	std::string fmu = fmu_directory+"DUMMY_0CYMDIST.fmu";
	fmiLibrary loadFmu(fmu);
	BOOST_CHECK(loadFmu.isXmlLoaded());
	auto states = loadFmu.getCounts("states");
	BOOST_CHECK_EQUAL(states, 0);
	auto outputs = loadFmu.getCounts("outputs");
	BOOST_CHECK_EQUAL(outputs, 6);
	auto inputs = loadFmu.getCounts("inputs");
	BOOST_CHECK_EQUAL(inputs, 6);
	//auto param = loadFmu.getCounts("parameters");
	//auto units = loadFmu.getCounts("unit");
	auto fm = loadFmu.createModelExchangeObject("meload");
	BOOST_REQUIRE(fm);
	double inp[6];
	double out[6];
	double out2[6];
	fm->setMode(fmuMode::continuousTimeMode);
	fm->getCurrentInputs(inp);
	inp[0] = 1.0;
	inp[1] = 1.0;
	inp[2] = 1.0;
	fm->setInputs(inp);
	fm->getCurrentInputs(inp);
	BOOST_CHECK_EQUAL(inp[2], 1.0);
	fm->getOutputs(out);
	inp[0] = 0.97;
	inp[1] = 0.97;
	inp[2] = 0.97;
	auto res = fm->get<double>("a0");
	BOOST_CHECK_EQUAL(res, 1.0);
	fm->setInputs(inp);
	fm->getOutputs(out2);
	BOOST_CHECK_LT(out2[0], out[0]);
	loadFmu.close();
}


BOOST_AUTO_TEST_CASE(test_fmi_approx_load)
{
	using namespace griddyn::loads;
	approximatingLoad apload("apload1");

	auto ld1 = new griddyn::fmi::fmiMELoad3phase("fmload1");
    std::string fmu = fmu_directory + "DUMMY_0CYMDIST.fmu";

	apload.add(ld1);
    ld1->set("a2", 0.0);
    ld1->set("b2", 0);
    ld1->set("c2", 0.0);
	ld1->setFlag("current_output",true);
	apload.pFlowInitializeA(0, 0);
	apload.pFlowInitializeB();
	auto p = apload.get("p");
	auto q = apload.get("q");
    auto ip = apload.get("ip");
    auto iq = apload.get("iq");
	auto yp = apload.get("yp");
	auto yq = apload.get("yq");

    BOOST_CHECK_SMALL(p, 0.0001);
    BOOST_CHECK_CLOSE(ip, 3.0, 0.0001);
    BOOST_CHECK_CLOSE(yp, 3.0, 0.0001);

    BOOST_CHECK_CLOSE(q, 0.0, 0.0001);
    BOOST_CHECK_CLOSE(iq, 0.0, 0.0001);
    BOOST_CHECK_CLOSE(yq, 0.0, 0.0001);

    ld1->set("a0", 2.0);
    ld1->set("b0", 2.0);
    ld1->set("c0", 2.0);
    apload.updateA(0);
    apload.updateB();
    auto p2 = apload.get("p");
    auto ip2 = apload.get("ip");
    auto yp2 = apload.get("yp");
    BOOST_CHECK_SMALL(p2, 0.0001);
    BOOST_CHECK_CLOSE(ip*2.0, ip2, 0.00002);
    BOOST_CHECK_CLOSE(yp, yp2, 0.001);

    ld1->set("angle0", 10, gridUnits::deg);
    apload.updateA(0);
    apload.updateB();
    auto iq2 = apload.get("iq");
    auto yq2 = apload.get("yq");

    BOOST_CHECK_GT(std::abs(iq2), 0.001);
    BOOST_CHECK_CLOSE(iq2/2.0, yq2,0.0001);
	ld1 = nullptr;
}


BOOST_AUTO_TEST_CASE(test_fmi_approx_load_xml)
{
    using namespace griddyn::loads;
    std::string fileName = fmi_test_directory+ "approxLoad_testfmi.xml";



    gds = griddyn::readSimXMLFile(fileName);
    BOOST_REQUIRE(gds);
    auto obj = gds->getBus(1);
    BOOST_REQUIRE(obj != nullptr);
    auto apload = dynamic_cast<approximatingLoad *>(gds->getBus(1)->getLoad(0));

    auto ld1 = dynamic_cast<griddyn::Load *>(apload->getSubObject("subobject",0));
    ld1->set("a2", 0.0);
    ld1->set("b2", 0);
    ld1->set("c2", 0.0);
    ld1->setFlag("current_output", true);
    gds->pFlowInitialize();
    auto p = apload->get("p");
    auto q = apload->get("q");
    auto ip = apload->get("ip");
    auto iq = apload->get("iq");
    auto yp = apload->get("yp");
    auto yq = apload->get("yq");

    BOOST_CHECK_SMALL(p, 0.0001);
    BOOST_CHECK_CLOSE(ip, 3.0, 0.0001);
    BOOST_CHECK_CLOSE(yp, 3.0, 0.0001);

    BOOST_CHECK_SMALL(q, 0.0001);
    BOOST_CHECK_SMALL(iq, 0.0001);
    BOOST_CHECK_SMALL(yq, 0.0001);


    ld1 = nullptr;
}


#endif
#endif

#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE (fmi_test1)
{
    loadFmiLibrary ();
    // test the loading of a single bus
    auto tsb = new fmiLoad (FMU_LOC "ACMotorFMU.fmu");
    tsb->set ("a_in", "#");
    stringVec b;
    tsb->getParameterStrings (b, paramStringType::numeric);
    tsb->set ("a_in", 0.7);
    tsb->pFlowInitializeA (0, 0);
    auto outP = tsb->getRealPower ();
    auto outQ = tsb->getReactivePower ();
    tsb->dynInitializeA (0, 0);
    IOdata oset;
    tsb->dynInitializeB ({1.0, 0}, oset);

    auto out = tsb->timestep (6, {1.0, 0}, cLocalSolverMode);
    outP = tsb->getRealPower ();
    outQ = tsb->getReactivePower ();
    BOOST_CHECK_CLOSE (out, 0.7, 0.001);
    tsb->set ("a_in", 0.9);
    out = tsb->timestep (14, {1.0, 0}, cLocalSolverMode);
    outP = tsb->getRealPower ();
    outQ = tsb->getReactivePower ();
    BOOST_CHECK_CLOSE (out, 0.9, 0.001);
    tsb->set ("a_in", 0.6);
    out = tsb->timestep (22, {1.0, 0}, cLocalSolverMode);
    outP = tsb->getRealPower ();
    outQ = tsb->getReactivePower ();
    BOOST_CHECK_CLOSE (out, 0.6, 0.001);
    delete tsb;
}

BOOST_AUTO_TEST_CASE (fmi_xml1)
{
    std::string fileName = std::string (FMI_TEST_DIRECTORY "fmimotorload_test1.xml");

    readerConfig::setPrintMode (0);
    gds = readSimXMLFile (fileName);

    int retval = gds->pFlowInitialize ();
    BOOST_CHECK_EQUAL (retval, 0);


    int mmatch = JacobianCheck (gds, cPflowSolverMode);
    if (mmatch > 0)
    {
        printStateNames (gds, cPflowSolverMode);
    }
    BOOST_REQUIRE_EQUAL (mmatch, 0);
    gds->powerflow ();

    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds->dynInitialize ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
    std::vector<double> v;
    gds->getVoltage (v);
    mmatch = residualCheck (gds, cDaeSolverMode);
    if (mmatch > 0)
    {
        printStateNames (gds, cDaeSolverMode);
    }
    BOOST_REQUIRE_EQUAL (mmatch, 0);
    mmatch = JacobianCheck (gds, cDaeSolverMode);
    if (mmatch > 0)
    {
        printStateNames (gds, cDaeSolverMode);
    }
    BOOST_REQUIRE_EQUAL (mmatch, 0);

    gds->run ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}

BOOST_AUTO_TEST_CASE (fmi_xml2)
{
    std::string fileName = std::string (FMI_TEST_DIRECTORY "fmiload_test2.xml");

    readerConfig::setPrintMode (0);
    gds = readSimXMLFile (fileName);

    int retval = gds->pFlowInitialize ();
    BOOST_CHECK_EQUAL (retval, 0);


    int mmatch = JacobianCheck (gds, cPflowSolverMode);
    if (mmatch > 0)
    {
        printStateNames (gds, cPflowSolverMode);
    }
    BOOST_REQUIRE_EQUAL (mmatch, 0);
    gds->powerflow ();

    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds->dynInitialize ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
    std::vector<double> v;
    gds->getVoltage (v);
    mmatch = residualCheck (gds, cDaeSolverMode);
    if (mmatch > 0)
    {
        printStateNames (gds, cDaeSolverMode);
    }
    BOOST_REQUIRE_EQUAL (mmatch, 0);
    mmatch = JacobianCheck (gds, cDaeSolverMode);
    if (mmatch > 0)
    {
        printStateNames (gds, cDaeSolverMode);
    }
    BOOST_REQUIRE_EQUAL (mmatch, 0);

    gds->run ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}


BOOST_AUTO_TEST_CASE (fmi_xml3)
{
    std::string fileName = std::string (FMI_TEST_DIRECTORY "fmiload_test3.xml");

    readerConfig::setPrintMode (0);
    gds = readSimXMLFile (fileName);

    int retval = gds->pFlowInitialize ();
    BOOST_CHECK_EQUAL (retval, 0);


    int mmatch = JacobianCheck (gds, cPflowSolverMode);
    if (mmatch > 0)
    {
        printStateNames (gds, cPflowSolverMode);
    }
    BOOST_REQUIRE_EQUAL (mmatch, 0);
    gds->powerflow ();

    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds->dynInitialize ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
    std::vector<double> v;
    gds->getVoltage (v);
    mmatch = residualCheck (gds, cDaeSolverMode);
    if (mmatch > 0)
    {
        printStateNames (gds, cDaeSolverMode);
    }
    BOOST_REQUIRE_EQUAL (mmatch, 0);
    mmatch = JacobianCheck (gds, cDaeSolverMode);
    if (mmatch > 0)
    {
        printStateNames (gds, cDaeSolverMode);
    }
    BOOST_REQUIRE_EQUAL (mmatch, 0);

    gds->run ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
}

BOOST_AUTO_TEST_CASE (fmi_array)
{
    std::string fileName = std::string (FMI_TEST_DIRECTORY "block_grid.xml");

    readerConfig::setPrintMode (0);
    gds = readSimXMLFile (fileName);

    int retval = gds->pFlowInitialize ();
    BOOST_CHECK_EQUAL (retval, 0);

    int cnt = gds->getInt ("totalbuscount");
    if (cnt < 200)
    {
        int mmatch = JacobianCheck (gds, cPflowSolverMode);
        if (mmatch > 0)
        {
            printStateNames (gds, cPflowSolverMode);
        }
        BOOST_REQUIRE_EQUAL (mmatch, 0);
    }
    gds->powerflow ();

    requireState(gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    gds->dynInitialize ();
    requireState(gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED);
    std::vector<double> v;
    gds->getVoltage (v);

    auto bus = static_cast<gridBus *> (gds->find ("bus_1_1"));
    printf ("slk bus p=%f min v= %f\n", bus->getGenerationReal (), *std::min_element (v.begin (), v.end ()));
}

#endif


BOOST_AUTO_TEST_SUITE_END ()
