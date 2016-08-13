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
#include "testHelper.h"
#include "simulation/gridDynSimulationFileOps.h"

#include <vectorOps.hpp>
#include <map>
#include <utility>
#include <iostream>
#include <cstdio>
#include <set>

#define COMPUTE_TIMES 1

#if (COMPUTE_TIMES>0)
#include <chrono>
#endif
//test case for gridCoreObject object

static const std::string validationTestDirectory(GRIDDYN_TEST_DIRECTORY "/validation_tests/");

#define INPUT_TEST_DIRECTORY (GRIDDYN_TEST_DIRECTORY "/input_tests/")

BOOST_FIXTURE_TEST_SUITE (validation_tests, gridDynSimulationTestFixture)




BOOST_AUTO_TEST_CASE(matpower_validation_tests)
{
	/* *INDENT-OFF* */
	const std::map<std::string, std::string> compare_cases{
		{ "case4gs.m","case4gs_res.m" },
		{ "case5.m","case5_res.m" },
		{ "case6ww.m","case6ww_res.m" },
		{ "case9.m","case9_res.m" },
		{ "case9Q.m","case9Q_res.m" },
		{ "case9target.m","case9target_res.m" },
		{ "case14.m","case14_res.m" },
		{ "case24_ieee_rts.m","case24_ieee_rts_res.m" },
		{ "case30.m","case30_res.m" },
		{ "case30pwl.m","case30pwl_res.m" },
		{ "case30Q.m","case30Q_res.m" },
		{ "case_ieee30.m","case_ieee30_res.m" },
		{ "case39.m","case39_res.m" },
		{ "case57.m","case57_res.m" },
		{ "case89pegase.m","case89pegase_res.m" },
		{ "case118.m","case118_res.m" },
		{ "case300.m","case300_res.m" },
		{ "case1354pegase.m","case1354pegase_res.m" },
		{ "case2383wp.m","case2383wp_res.m" },
		{ "case2736sp.m","case2736sp_res.m" },
		{ "case2737sop.m","case2737sop_res.m" },
		{ "case2746wop.m","case2746wop_res.m" },
		{ "case2746wp.m","case2746wp_res.m" },
		{ "case2869pegase.m","case2869pegase_res.m" },
		{ "case3012wp.m","case3012wp_res.m" },
		{ "case3120sp.m","case3120sp_res.m" },
		{ "case3375wp.m","case3375wp_res.m" },
		{ "case9241pegase.m","case9241pegase_res.m" },
		
	};
	/* *INDENT-ON* */

	std::vector<double> volts1;
	std::vector<double> ang1;
	std::vector<double> volts2;
	std::vector<double> ang2;

#if (COMPUTE_TIMES>0)
	std::chrono::duration<double> elapsed_time;
#endif

	for (const auto &mp : compare_cases)
	{
		gds = new gridDynSimulation();
		gds->set("consoleprintlevel", GD_SUMMARY_PRINT);
		std::string fname;
		if (mp.first.length() > 25)
		{
			fname = mp.first;
		}
		else
		{
			fname = validationTestDirectory + mp.first;
		}

		loadFile(gds, fname);
		gds->sourceFile = mp.first;
		gds->setFlag("no_powerflow_adjustments");
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
#if (COMPUTE_TIMES>0)
		
		auto start_t = std::chrono::high_resolution_clock::now();
		gds->powerflow();
		
		auto stop_t = std::chrono::high_resolution_clock::now();
		elapsed_time = stop_t - start_t;
		printf("%s completed in %f\n", mp.first.c_str(), elapsed_time.count());
#else
		gds->powerflow();
#endif
		//printf("completed power flow\n");
		if (gds->currentProcessState() != gridDynSimulation::gridState_t::POWERFLOW_COMPLETE)
		{
			std::cout << fname << " did not complete power flow calculation\n";
		}
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

		int cnt=gds->getVoltage(volts1);
		gds->getAngle(ang1);

		volts1.resize(cnt);
		ang1.resize(cnt);
		gds2 = new gridDynSimulation();
		if (mp.second.length() > 25)
		{
			fname = mp.second;
		}
		else
		{
			fname = validationTestDirectory + mp.second;
		}
		gds2->set("consoleprintlevel", GD_SUMMARY_PRINT);
		loadFile(gds2, fname);
		BOOST_REQUIRE(gds2->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
		gds2->pFlowInitialize();
		cnt=gds2->getVoltage(volts2);
		gds2->getAngle(ang2);
		volts2.resize(cnt);
		ang2.resize(cnt);
		auto vdiff = countDiffs(volts1, volts2, 5e-5);
		auto adiff = countDiffs(ang1, ang2, 5e-5);
		if ((adiff > 0) || (vdiff > 0))
		{
			int maxvdiffbus;
			double mvdiff = absMaxDiffLoc(volts1, volts2, maxvdiffbus);
			int maxadiffbus;
			double madiff = absMaxDiffLoc(ang1, ang2, maxadiffbus);
			std::cout << mp.first << "max vdiff [" << maxvdiffbus << "] = " << mvdiff << "|| max adiff[" << maxadiffbus << "] = " << madiff << '\n';
		}
		BOOST_CHECK_EQUAL(vdiff, 0);
		BOOST_CHECK_EQUAL(adiff, 0);
		delete gds;
		delete gds2;
		gds2 = nullptr;
		gds = nullptr;
	}

}

//test cases with gauss-siedel solver
BOOST_AUTO_TEST_CASE(matpower_validation_tests_gs)
{
	/* *INDENT-OFF* */
	const std::map<std::string, std::string> compare_cases{
		{ "case4gs.m","case4gs_res.m" },
		{ "case5.m","case5_res.m" },
		{ "case6ww.m","case6ww_res.m" },
		{ "case9.m","case9_res.m" },
		{ "case9Q.m","case9Q_res.m" },
		//{ "case9target.m","case9target_res.m" },
		{ "case14.m","case14_res.m" },
		{ "case24_ieee_rts.m","case24_ieee_rts_res.m" },
		{ "case30.m","case30_res.m" },
		{ "case30pwl.m","case30pwl_res.m" },
		{ "case30Q.m","case30Q_res.m" },
		{ "case_ieee30.m","case_ieee30_res.m" },
		{ "case39.m","case39_res.m" },
		{ "case57.m","case57_res.m" },
		{ "case89pegase.m","case89pegase_res.m" },
		{ "case118.m","case118_res.m" },
		{ "case300.m","case300_res.m" },
		/*{ "case1354pegase.m","case1354pegase_res.m" },
		{ "case2383wp.m","case2383wp_res.m" },
		{ "case2736sp.m","case2736sp_res.m" },
		{ "case2737sop.m","case2737sop_res.m" },
		{ "case2746wop.m","case2746wop_res.m" },
		{ "case2746wp.m","case2746wp_res.m" },
		{ "case2869pegase.m","case2869pegase_res.m" },
		{ "case3012wp.m","case3012wp_res.m" },
		{ "case3120sp.m","case3120sp_res.m" },
		{ "case3375wp.m","case3375wp_res.m" },
		{ "case9241pegase.m","case9241pegase_res.m" },*/
	};
	/* *INDENT-ON* */

	std::vector<double> volts1;
	std::vector<double> ang1;
	std::vector<double> volts2;
	std::vector<double> ang2;

#if (COMPUTE_TIMES>0)
	std::chrono::duration<double> elapsed_time;
#endif

	for (const auto &mp : compare_cases)
	{
		gds = new gridDynSimulation();
		gds->set("consoleprintlevel", GD_SUMMARY_PRINT);
		std::string fname;
		if (mp.first.length() > 25)
		{
			fname = mp.first;
		}
		else
		{
			fname = validationTestDirectory + mp.first;
		}

		loadFile(gds, fname);
		gds->setFlag("no_powerflow_adjustments");
		gds->set("defpowerflow", "gauss-seidel");
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
#if (COMPUTE_TIMES>0)
		auto start_t = std::chrono::high_resolution_clock::now();
		gds->powerflow();
		auto stop_t = std::chrono::high_resolution_clock::now();
		elapsed_time = stop_t - start_t;
		printf("%s completed in %f\n", mp.first.c_str(), elapsed_time.count());
#else
		gds->powerflow();
#endif
		//printf("completed power flow\n");
		if (gds->currentProcessState() != gridDynSimulation::gridState_t::POWERFLOW_COMPLETE)
		{
			std::cout << fname << " did not complete power flow calculation\n";
		}
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

		int cnt = gds->getVoltage(volts1);
		gds->getAngle(ang1);

		volts1.resize(cnt);
		ang1.resize(cnt);
		double vdiff = 0;
		double adiff = 0;
		size_t maxvdiffbus = 0;
		size_t maxadiffbus = 0;
		double mvdiff = 0;
		double madiff = 0;
		gds2 = new gridDynSimulation();
		if (mp.second.length() > 25)
		{
			fname = mp.second;
		}
		else
		{
			fname = validationTestDirectory + mp.second;
		}
		gds2->set("consoleprintlevel", GD_SUMMARY_PRINT);
		loadFile(gds2, fname);
		BOOST_REQUIRE(gds2->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
		gds2->pFlowInitialize();
		cnt = gds2->getVoltage(volts2);
		gds2->getAngle(ang2);
		volts2.resize(cnt);
		ang2.resize(cnt);
		for (size_t kk = 0; kk < volts1.size(); ++kk)
		{
			double diff = std::abs(volts1[kk] - volts2[kk]);
			if (diff > mvdiff)
			{
				mvdiff = diff;
				maxvdiffbus = kk;
			}
			if (diff> 1e-3)
			{
				std::cout << mp.first << " Voltage difference bus " << kk + 1 << "::" << volts1[kk] << " vs. " << volts2[kk] << "::" << std::abs(volts1[kk] - volts2[kk]) << " puV\n";
				vdiff++;
			}
			diff = std::abs(ang1[kk] - ang2[kk]);
			if (diff > madiff)
			{
				madiff = diff;
				maxadiffbus = kk;
			}
			if (diff> 1e-3)
			{
				std::cout << mp.first << " Angle difference-- bus " << kk + 1 << "::" << ang1[kk] * 180.0 / kPI << " vs. " << ang2[kk] * 180.0 / kPI << "::" << std::abs(ang1[kk] - ang2[kk]) * 180.0 / kPI << " deg\n";
				adiff++;
			}
		}
		if ((adiff > 0) || (vdiff > 0))
		{
			std::cout << mp.first << "max vdiff [" << maxvdiffbus << "] = " << mvdiff << "|| max adiff[" << maxadiffbus << "] = " << madiff << '\n';
		}
		BOOST_CHECK_EQUAL(vdiff, 0);
		BOOST_CHECK_EQUAL(adiff, 0);
		delete gds;
		delete gds2;
		gds2 = nullptr;
		gds = nullptr;
	}

}
#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(matpower_validation_tests_withq)
{
	/* *INDENT-OFF* */
	const std::map<std::string, std::string> compare_cases{
		{ "case4gs.m","case4gs_resqlim.m" },
		{ "case5.m","case5_resqlim.m" },
		{ "case6ww.m","case6ww_resqlim.m" },
		{ "case9.m","case9_resqlim.m" },
		{ "case9Q.m","case9Q_resqlim.m" },
		{ "case9target.m","case9target_resqlim.m" },
		{ "case14.m","case14_resqlim.m" },
		{ "case24_ieee_rts.m","case24_ieee_rts_resqlim.m" },
		{ "case30.m","case30_resqlim.m" },
		{ "case30pwl.m","case30pwl_resqlim.m" },
		{ "case30Q.m","case30Q_resqlim.m" },
		{ "case_ieee30.m","case_ieee30_resqlim.m" },
		{ "case39.m","case39_resqlim.m" },
		{ "case57.m","case57_resqlim.m" },
		{ "case89pegase.m","case89pegase_resqlim.m" },
		{ "case118.m","case118_resqlim.m" },
		{ "case300.m","case300_resqlim.m" },
		{ "case1354pegase.m","case1354pegase_resqlim.m" },
		{ "case2383wp.m","case2383wp_resqlim.m" },
		{ "case2736sp.m","case2736sp_resqlim.m" },
		{ "case2746wop.m","case2746wop_resqlim.m" },
		{ "case2746wp.m","case2746wp_resqlim.m" },
		{ "case2869pegase.m","case2869pegase_resqlim.m" },
		{ "case3012wp.m","case3012wp_resqlim.m" },
		{ "case3120sp.m","case3120sp_resqlim.m" },
		{ "case3375wp.m","case3375wp_resqlim.m" },
		{ "case9241pegase.m","case9241pegase_resqlim.m" },
	};
	/* *INDENT-ON* */

	std::vector<double> volts1;
	std::vector<double> ang1;
	std::vector<double> volts2;
	std::vector<double> ang2;

	
#if (COMPUTE_TIMES>0)
	std::chrono::duration<double> elapsed_time;
#endif

	for (const auto &mp : compare_cases)
	{
		gds = new gridDynSimulation();
		gds->set("consoleprintlevel", GD_SUMMARY_PRINT);
		std::string fname;
		if (mp.first.length() > 25)
		{
			fname = mp.first;
		}
		else
		{
			fname = validationTestDirectory + mp.first;
		}

		loadFile(gds, fname);
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
#if (COMPUTE_TIMES>0)
		auto start_t = std::chrono::high_resolution_clock::now();
		gds->powerflow();
		auto stop_t = std::chrono::high_resolution_clock::now();
		elapsed_time = stop_t - start_t;
		printf("%s completed in %f\n", mp.first.c_str(), elapsed_time.count());
#else
		gds->powerflow();
#endif
		//printf("completed power flow\n");
		if (gds->currentProcessState() != gridDynSimulation::gridState_t::POWERFLOW_COMPLETE)
		{
			std::cout << fname << " did not complete power flow calculation\n";
		}
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

		int cnt = gds->getVoltage(volts1);
		gds->getAngle(ang1);

		volts1.resize(cnt);
		ang1.resize(cnt);
		double vdiff = 0;
		double adiff = 0;
		double maxvdiffbus = 0;
		double maxadiffbus = 0;
		double mvdiff = 0;
		double madiff = 0;
		gds2 = new gridDynSimulation();
		if (mp.second.length() > 25)
		{
			fname = mp.second;
		}
		else
		{
			fname = validationTestDirectory + mp.second;
		}
		gds2->set("consoleprintlevel", GD_SUMMARY_PRINT);
		loadFile(gds2, fname);
		BOOST_REQUIRE(gds2->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
		gds2->pFlowInitialize();
		cnt = gds2->getVoltage(volts2);
		gds2->getAngle(ang2);
		volts2.resize(cnt);
		ang2.resize(cnt);
		for (size_t kk = 0; kk < volts1.size(); ++kk)
		{
			double diff = std::abs(volts1[kk] - volts2[kk]);
			if (diff > mvdiff)
			{
				mvdiff = diff;
				maxvdiffbus = kk;
			}
			if (diff> 1e-6)
			{
				std::cout << mp.first << " Voltage difference bus " << kk + 1 << "::" << volts1[kk] << " vs. " << volts2[kk] << "::" << std::abs(volts1[kk] - volts2[kk]) << " puV\n";
				vdiff++;
			}
			diff = std::abs(ang1[kk] - ang2[kk]);
			if (diff > madiff)
			{
				madiff = diff;
				maxadiffbus = kk;
			}
			if (diff> 1e-6)
			{
				std::cout << mp.first << " Angle difference-- bus " << kk + 1 << "::" << ang1[kk] * 180.0 / kPI << " vs. " << ang2[kk] * 180.0 / kPI << "::" << std::abs(ang1[kk] - ang2[kk]) * 180.0 / kPI << " deg\n";
				adiff++;
			}
		}
		if ((adiff > 0) || (vdiff > 0))
		{
			std::cout << mp.first << "max vdiff [" << maxvdiffbus << "] = " << mvdiff << "|| max adiff[" << maxadiffbus << "] = " << madiff << '\n';
		}
		BOOST_CHECK_EQUAL(vdiff, 0);
		BOOST_CHECK_EQUAL(adiff, 0);
		delete gds;
		delete gds2;
		gds2 = nullptr;
		gds = nullptr;
	}

}
#endif
#ifdef ENABLE_EXPERIMENTAL_TEST_CASES
BOOST_AUTO_TEST_CASE(matpower_validation_tests_problems)
{
	/* *INDENT-OFF* */
	const std::map<std::string, std::string> compare_cases{		
		//{ "case3012wp.m","case3012wp_res.m" },
		//{ "case3120sp.m","case3120sp_res.m" },
		{"case2746wop.m","case2746wop_res.m" },
	};
	/* *INDENT-ON* */

	std::vector<double> volts1;
	std::vector<double> ang1;
	std::vector<double> volts2;
	std::vector<double> ang2;

	std::vector<double> flow1,flow2,flow3,flow4,ldp,ldq,genp,genq;

#if (COMPUTE_TIMES>0)
	std::chrono::duration<double> elapsed_time;
#endif

	for (const auto &mp : compare_cases)
	{
		gds = new gridDynSimulation();
		gds->set("consoleprintlevel", GD_DEBUG_PRINT);
		std::string fname;
		if (mp.first.length() > 25)
		{
			fname = mp.first;
		}
		else
		{
			fname = validationTestDirectory + mp.first;
		}

		loadFile(gds, fname);
		gds->setFlag("no_powerflow_adjustments");
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
#if (COMPUTE_TIMES>0)
		auto start_t = std::chrono::high_resolution_clock::now();
		gds->powerflow();
		auto stop_t = std::chrono::high_resolution_clock::now();
		elapsed_time = stop_t - start_t;
		printf("%s completed in %f\n", mp.first.c_str(), elapsed_time.count());
#else
		gds->powerflow();
#endif
		//printf("completed power flow\n");
		if (gds->currentProcessState() != gridDynSimulation::gridState_t::POWERFLOW_COMPLETE)
		{
			std::cout << fname << " did not complete power flow calculation\n";
		}
		BOOST_REQUIRE(gds->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

		int cnt = gds->getVoltage(volts1);
		gds->getAngle(ang1);

		volts1.resize(cnt);
		ang1.resize(cnt);
		cnt = gds->getLinkRealPower(flow1);
		cnt = gds->getLinkReactivePower(flow2);
		cnt = gds->getBusLoadReal(ldp);
		cnt = gds->getBusGenerationReal(genp);
		gds->getBusLoadReactive(ldq);
		gds->getBusGenerationReactive(genq);
		cnt = gds->getLinkReactivePower(flow2);
		cnt = gds->getLinkRealPower(flow3, 0, 2);
		cnt = gds->getLinkReactivePower(flow4,0,2);
		for (int pp = 0; pp < cnt; ++pp)
		{
			printf("%d, %f, %f, %f, %f\n", pp, flow1[pp], flow2[pp], flow3[pp], flow4[pp]);
		}
		printf("------------------------------------------------------\n");
		for (size_t pp = 0; pp < ldp.size(); ++pp)
		{
			printf("%d, %f, %f, %f, %f \n", static_cast<int>(pp), ldp[pp], ldq[pp],genp[pp],genq[pp]);
		}
		double vdiff = 0;
		double adiff = 0;
		double maxvdiffbus = 0;
		double maxadiffbus = 0;
		double mvdiff = 0;
		double madiff = 0;
		gds2 = new gridDynSimulation();
		if (mp.second.length() > 25)
		{
			fname = mp.second;
		}
		else
		{
			fname = validationTestDirectory + mp.second;
		}
		gds2->set("consoleprintlevel", GD_SUMMARY_PRINT);
		loadFile(gds2, fname);
		BOOST_REQUIRE(gds2->currentProcessState() == gridDynSimulation::gridState_t::STARTUP);
		gds2->pFlowInitialize();
		cnt = gds2->getVoltage(volts2);
		gds2->getAngle(ang2);
		volts2.resize(cnt);
		ang2.resize(cnt);
		for (size_t kk = 0; kk < volts1.size(); ++kk)
		{
			double diff = std::abs(volts1[kk] - volts2[kk]);
			if (diff > mvdiff)
			{
				mvdiff = diff;
				maxvdiffbus = kk;
			}
			if (diff> 1e-3)
			{
				std::cout << mp.first << " Voltage difference bus " << kk + 1 << "::" << volts1[kk] << " vs. " << volts2[kk] << "::" << std::abs(volts1[kk] - volts2[kk]) << " puV\n";
				vdiff++;
			}
			diff = std::abs(ang1[kk] - ang2[kk]);
			if (diff > madiff)
			{
				madiff = diff;
				maxadiffbus = kk;
			}
			if (diff> 0.01 / 180.0*kPI)
			{
				std::cout << mp.first << " Angle difference-- bus " << kk + 1 << "::" << ang1[kk] * 180.0 / kPI << " vs. " << ang2[kk] * 180.0 / kPI << "::" << std::abs(ang1[kk] - ang2[kk]) * 180.0 / kPI << " deg\n";
				adiff++;
			}
		}
		std::cout << mp.first << "max vdiff [" << maxvdiffbus << "] = " << mvdiff << "|| max adiff[" << maxadiffbus << "] = " << madiff << '\n';
		BOOST_CHECK_EQUAL(vdiff, 0);
		BOOST_CHECK_EQUAL(adiff, 0);
		delete gds;
		delete gds2;
		gds2 = nullptr;
		gds = nullptr;
	}

}
#endif

BOOST_AUTO_TEST_SUITE_END()
