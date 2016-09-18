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
#include <boost/test/floating_point_comparison.hpp>
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "simulation/gridDynSimulationFileOps.h"
#include "testHelper.h"
#include "simulation/diagnostics.h"
#include "gridBus.h"

#include <vectorOps.hpp>
#include <map>
#include <utility>
#include <iostream>
#include <cstdio>
#include <set>
#include <chrono>


BOOST_FIXTURE_TEST_SUITE (performance_tests, gridDynSimulationTestFixture)

static const std::string validationTestDirectory(GRIDDYN_TEST_DIRECTORY "/validation_tests/");

BOOST_AUTO_TEST_CASE(performance_tests1)
{
	/* *INDENT-OFF* */
	const stringVec perf_cases{ "case1354pegase.m",
		"case2869pegase.m",
		"case3012wp.m","case3375wp.m", "case9241pegase.m"
	};
	/* *INDENT-ON* */
	std::chrono::duration<double> pflow_time(0);
	std::chrono::duration<double> load_time(0);
	for (const auto &mp : perf_cases)
	{

		std::string fname;
		if (mp.length() > 25)
		{
			fname = mp;
		}
		else
		{
			fname = validationTestDirectory + mp;
		}
		for (int kk = 0; kk < 10; ++kk)  //Do this 10 time
		{
			gds = new gridDynSimulation();
			gds->set("consoleprintlevel", GD_SUMMARY_PRINT);
			auto start_t = std::chrono::high_resolution_clock::now();
			loadFile(gds, fname);
			gds->setFlag("no_powerflow_adjustments");
			auto stop_t = std::chrono::high_resolution_clock::now();
			load_time += (stop_t - start_t);

			BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);

			start_t = std::chrono::high_resolution_clock::now();
			gds->powerflow();
			stop_t = std::chrono::high_resolution_clock::now();
			pflow_time += (stop_t - start_t);

			if (gds->currentProcessState() != gridDynSimulation::gridState_t::POWERFLOW_COMPLETE)
			{
				std::cout << fname << " did not complete power flow calculation\n";
				break;
			}
			BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
			delete gds;
			gds = nullptr;
		}
		printf("%s load in %f powerflow in %f\n", mp.c_str(), load_time.count() / 10.0, pflow_time.count() / 10.0);


	}

}


BOOST_AUTO_TEST_CASE(performance_tests_scaling_pFlow)
{
	std::string testFile= std::string(GRIDDYN_TEST_DIRECTORY "/performance_tests/block_grid2.xml");
	
	std::vector<int> elements = { 3,4,5,6,7,8,9,10,11,12,14,16,18,20,24,28,32,36,40,45,50,56,60,66,70,74,80,84,86,88,90,92,100,110,120,132,136,140,148,156 };
	//std::vector<int> elements = {212,240,260};
	//std::vector<int> elements{ 800 };
	std::chrono::duration<double> pflow_time(0);
	std::chrono::duration<double> load_time(0);

	std::vector<double> ldtime(elements.size());
	std::vector<double> pftime(elements.size());
	int ii = 0;
	for (auto gsize : elements)
	{
		gds = new gridDynSimulation();
		readerInfo ri;
		ri.addLockedDefinition("garraySize", std::to_string(gsize));
		gds->set("consoleprintlevel", GD_SUMMARY_PRINT);
		auto start_t = std::chrono::high_resolution_clock::now();
		loadFile(gds, testFile,&ri);
		gds->setFlag("no_powerflow_adjustments");
		auto stop_t = std::chrono::high_resolution_clock::now();
		load_time = (stop_t - start_t);
		ldtime[ii] = load_time.count();

		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);

		start_t = std::chrono::high_resolution_clock::now();
		gds->powerflow();
		stop_t = std::chrono::high_resolution_clock::now();
		gds->updateLocalCache();
		pflow_time = (stop_t - start_t);
		pftime[ii] =pflow_time.count();
		int ss=gds->getInt("statesize");
        int nnz=gds->getInt("nonzeros");
        int rcount=gds->getInt("residcount");
        int jcount=gds->getInt("jaccount");
		printf("%d size=%d, nnz=%d,ld time=%f, pflow time=%f\n", gsize,ss, nnz,load_time.count(), pflow_time.count());
printf("%d residual calls, %d Jacobian call\n",rcount,jcount);
		auto bus = static_cast<gridBus *>(gds->findByUserID("bus", 10000000));
		printf("slack bus gen p=%f, gen q =%f\n", bus->getGenerationReal(), bus->getGenerationReactive());
		
		std::vector<double> v;
		int cnt=gds->getVoltage(v);
		int locMin;
		double minV=minLoc(v,locMin);
		int locMax;
		double maxV=maxLoc(v,locMax);
		printf("cnt=%d vmin=%f at %d, vmax=%f at %d \n",cnt, minV,locMin,maxV,locMax);
		//savePowerFlowCSV(gds,"bigCSV.csv");
		delete gds;
		gds = nullptr;
	}

}


#ifdef ENABLE_IN_DEVELOPMENT_CASES
#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
//test pjm case
BOOST_AUTO_TEST_CASE(test_pjm_pflow)
{

	std::string fname = std::string(OTHER_TEST_DIRECTORY "pf.output.raw");
	gds = new gridDynSimulation();
	readerInfo ri;
	ri.flags = addflags(0, "ignore_step_up_transformers");
	loadFile(gds, fname, &ri);
	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
	std::vector<double> gv1, gv2;
	gds->getVoltage(gv1);
	gds->pFlowInitialize();
	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::INITIALIZED);
	int mmatch = residualCheck(gds, cPflowSolverMode, 0.2, true);
	if (mmatch>0)
	{
		printf("Mmatch failures=%d\n", mmatch);
	}
	//BOOST_REQUIRE(mmatch == 0);



	gds->powerflow();
	BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
	gds->getVoltage(gv2);
	BOOST_REQUIRE(gv1.size() == gv2.size());
	int diffc = 0;
	double bdiff = 0;
	count_t bdiffi = 0;
	for (size_t kk = 0; kk<gv1.size(); ++kk)
	{
		if (std::abs(gv1[kk] - gv2[kk])>0.0015)
		{
			++diffc;
			// printf("vstate %d, %f to %f\n",kk,gv1[kk],gv2[kk]);
			if (std::abs(gv1[kk] - gv2[kk])>bdiff)
			{
				bdiff = std::abs(gv1[kk] - gv2[kk]);
				bdiffi = kk;
			}
		}
	}
	 BOOST_CHECK(diffc==0);
	 if (diffc>0)
	 {
	  printf("%d diffs, difference bus %d orig=%f, result=%f\n",diffc, bdiffi,gv1[bdiffi],gv2[bdiffi]);
	}
}

#endif
#endif
	BOOST_AUTO_TEST_SUITE_END()
