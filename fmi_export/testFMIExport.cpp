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
#include <boost/filesystem.hpp>
#include "gridDyn.h"
#include "fmi_export/fmiEvent.h"
#include "fmi_export/fmiCollector.h"
#include "fmi_export/fmiCoordinator.h"
#include "fmi_export/loadFMIExportObjects.h"
#include "fmi/fmi_import/fmiImport.h"
#include "gridDyn/loadModels/gridLoad3Phase.h"
#include "gridDynFileInput.h"
#include "utilities/vectorOps.hpp"
#include "gridBus.h"
#include "simulation/diagnostics.h"
#include "readerInfo.h"

//test case for fmi_export functions

#include "testHelper.h"
#include "fmi_export/fmiRunner.h"
#include "fmi/fmi_import/fmiObjects.h"

static const std::string fmi_test_directory(GRIDDYN_TEST_DIRECTORY "/fmi_export_tests/");




using namespace boost::filesystem;
BOOST_FIXTURE_TEST_SUITE(fmi_export_tests, gridDynSimulationTestFixture)
//Test that fmi events(inputs) are created with the correct names
BOOST_AUTO_TEST_CASE(test_fmi_events)
{
	auto fmiCon = std::make_shared<fmiCoordinator>();

	gds = std::make_unique<gridDynSimulation>();
	gds->addsp(fmiCon);
	
	auto fmc = gds->find("fmiCoordinator");
	BOOST_REQUIRE(fmc != nullptr);
	BOOST_CHECK_EQUAL(fmc->getID(), fmiCon->getID());

	readerInfo ri;
	loadFmiExportReaderInfoDefinitions(ri);
	std::string file = fmi_test_directory + "fmi_export_fmiinput.xml";
		loadFile(gds.get(), file,&ri,"xml");


		auto &ev = fmiCon->getInputs();
		BOOST_REQUIRE_EQUAL(ev.size(), 1u);

		BOOST_CHECK_EQUAL(ev[0].second.name, "power");
		
}

BOOST_AUTO_TEST_CASE(test_fmi_output)
{
	auto fmiCon = std::make_shared<fmiCoordinator>();

	gds = std::make_unique<gridDynSimulation>();
	gds->addsp(fmiCon);

	auto fmc = gds->find("fmiCoordinator");
	BOOST_REQUIRE(fmc != nullptr);
	BOOST_CHECK_EQUAL(fmc->getID(), fmiCon->getID());

	readerInfo ri;
	loadFmiExportReaderInfoDefinitions(ri);
	std::string file = fmi_test_directory + "fmi_export_fmioutput.xml";
	loadFile(gds.get(), file, &ri, "xml");


	auto &ev = fmiCon->getOutputs();
	BOOST_REQUIRE_EQUAL(ev.size(), 1u);

	BOOST_CHECK_EQUAL(ev[0].second.name, "load");

}

BOOST_AUTO_TEST_CASE(test_fmi_simulation)
{
	auto fmiCon = std::make_shared<fmiCoordinator>();

	gds = std::make_unique<gridDynSimulation>();
	gds->addsp(fmiCon);

	auto fmc = gds->find("fmiCoordinator");
	BOOST_REQUIRE(fmc != nullptr);
	BOOST_CHECK_EQUAL(fmc->getID(), fmiCon->getID());

	readerInfo ri;
	loadFmiExportReaderInfoDefinitions(ri);
	std::string file = fmi_test_directory + "simulation.xml";
	loadFile(gds.get(), file, &ri, "xml");


	auto &ev = fmiCon->getInputs();
	BOOST_REQUIRE_EQUAL(ev.size(), 1u);

	BOOST_CHECK_EQUAL(ev[0].second.name, "power");

	auto &ev2 = fmiCon->getOutputs();
	BOOST_REQUIRE_EQUAL(ev2.size(), 1u);

	BOOST_CHECK_EQUAL(ev2[0].second.name, "load");

	gds->run(30);
	BOOST_CHECK_EQUAL(gds->currentProcessState(), gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

}

BOOST_AUTO_TEST_CASE(test_fmi_runner)
{
	auto runner = std::make_unique<fmiRunner>("testsim", fmi_test_directory, nullptr);

	runner->simInitialize();
	runner->UpdateOutputs();

	auto out = runner->Get(2);
	BOOST_CHECK_CLOSE(out, 0.5, 0.000001);

	runner->Set(1, 0.6);
	runner->Step(1.0);
	out = runner->Get(2);
	BOOST_CHECK_CLOSE(out, 0.6, 0.00001);
	runner->Set(1, 0.7);
	runner->Step(2.0);
	out = runner->Get(2);
	BOOST_CHECK_CLOSE(out, 0.7, 0.00001);
	runner->Finalize();
}

BOOST_AUTO_TEST_CASE(load_griddyn_fmu)
{
	std::string fmu = fmi_test_directory + "griddyn.fmu";
	path gdDir(fmi_test_directory + "griddyn");
	fmiLibrary gdFmu(fmu);
	gdFmu.loadSharedLibrary();
	BOOST_REQUIRE(gdFmu.isSoLoaded());

	auto types=gdFmu.getTypes();
	BOOST_CHECK_EQUAL(types, "default");
	auto ver = gdFmu.getVersion();
	BOOST_CHECK_EQUAL(ver, "2.0");

	auto b = gdFmu.createCoSimulationObject("gd1");

	auto b2 = gdFmu.createCoSimulationObject("gd2");

	BOOST_CHECK((b));
	BOOST_CHECK((b2));

	b->setMode(fmuMode::initializationMode);
	b2->setMode(fmuMode::initializationMode);

	BOOST_CHECK(b->getCurrentMode() == fmuMode::initializationMode);
	BOOST_CHECK_EQUAL(b->inputSize(), 1);
	BOOST_CHECK_EQUAL(b->outputSize(), 1);

	double input = 0.6;
	b->setInputs(&input);
	auto inputName = b->getInputNames();
	BOOST_CHECK_EQUAL(inputName[0],"power");
	auto outputName = b->getOutputNames();
	BOOST_CHECK_EQUAL(outputName[0], "load");
	b->setMode(fmuMode::stepMode);
	b2->setMode(fmuMode::stepMode);
	BOOST_CHECK(b2->getCurrentMode() == fmuMode::stepMode);

	auto val = b->getOutput(0);
	auto val2 = b2->getOutput(0);

	BOOST_CHECK_CLOSE(val, 0.6, 0.000001);
	BOOST_CHECK_CLOSE(val2, 0.5, 0.0000001);
	input = 0.7;
	b->setInputs(&input);
	input = 0.3;
	b2->setInputs(&input);
	b->doStep(0,1.0,false);
	b2->doStep(0, 1.0, false);
	val = b->getOutput(0);
	val2 = b2->getOutput(0);

	BOOST_CHECK_CLOSE(val, 0.7, 0.000001);
	BOOST_CHECK_CLOSE(val2, 0.3, 0.0000001);
	
	input = 0.3;
	b->setInputs(&input);
	input = 0.7;
	b2->setInputs(&input);
	b->doStep(1.0, 2.0, false);
	b2->doStep(1.0, 2.0, false);
	val = b->getOutput(0);
	val2 = b2->getOutput(0);

	BOOST_CHECK_CLOSE(val, 0.3, 0.000001);
	BOOST_CHECK_CLOSE(val2, 0.7, 0.0000001);

	b = nullptr;
	b2 = nullptr;
}


BOOST_AUTO_TEST_CASE(test_fmi_runner2)
{
	auto runner = std::make_unique<fmiRunner>("testsim", fmi_test_directory+"/three_phase_fmu", nullptr);
	//auto runner = std::make_unique<fmiRunner>("testsim", "C:\\Users\\top1\\Documents\\codeProjects\\New folder\\resources", nullptr);
	runner->simInitialize();
	runner->UpdateOutputs();

	auto bus = static_cast<gridBus*>(runner->getSim()->getSubObject("bus",11));

	auto ld = dynamic_cast<gridLoad3Phase *>(bus->getLoad(0));
	BOOST_REQUIRE(ld != nullptr);

	auto ret=runner->Step(10);
	BOOST_CHECK_EQUAL(ret, 10.0);

	auto val1 = runner->Get(8);
	auto val2 = runner->Get(9);
	auto val3 = runner->Get(10);
	BOOST_CHECK_GT(val1, 3500.0);
	BOOST_CHECK_LT(val1, 5200.0);
	BOOST_CHECK_GT(val2, 3500.0);
	BOOST_CHECK_LT(val2, 5200.0);
	BOOST_CHECK_GT(val3, 3500.0);
	BOOST_CHECK_LT(val3, 5200.0);
	auto val4 = runner->Get(11);
	auto val5 = runner->Get(12);
	auto val6 = runner->Get(13);
	BOOST_CHECK_GT(val4, -2000.0);
	BOOST_CHECK_GT(val5, -2000.0);
	BOOST_CHECK_GT(val6, -2000.0);
	auto time = 20.0;
	for (int ii = 2; ii < 5; ++ii)
	{
		runner->Set(ii, 100.0);
		ret = runner->Step(time);
		time += 10.0;

		auto val1b = runner->Get(8);
		auto val2b = runner->Get(9);
		auto val3b = runner->Get(10);
		BOOST_CHECK_GT(std::abs(val1b - val1),0.00001);
		BOOST_CHECK_GT(std::abs(val2b - val2), 0.00001);
		BOOST_CHECK_GT(std::abs(val3b - val3), 0.00001);
		val1 = val1b;
		val2 = val2b;
		val3 = val3b;
		auto gb = runner->Get(ii);
		BOOST_CHECK_SMALL(std::abs(gb-100.0), 0.5);  //this won't be that close since it is averaged across 3 phases
	}
	for (int ii = 5; ii < 8; ++ii)
	{
		runner->Set(ii, 0.0+(ii-5)*120.0);
		ret = runner->Step(time);
		time += 10.0;

		auto val1b = runner->Get(8);
		auto val2b = runner->Get(9);
		auto val3b = runner->Get(10);
		BOOST_CHECK_GT(std::abs(val1b - val1), 0.00001);
		BOOST_CHECK_GT(std::abs(val2b - val2), 0.00001);
		BOOST_CHECK_GT(std::abs(val3b - val3), 0.00001);
		val1 = val1b;
		val2 = val2b;
		val3 = val3b;
		auto gb = runner->Get(ii);
		double diff = std::abs(gb - (0.0 + (ii - 5)*120.0));
		if (diff > 359.5)
		{
			diff -= 360.0;
		}
		BOOST_CHECK_SMALL(diff, 0.1);
	}
}
BOOST_AUTO_TEST_SUITE_END()