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
#include "gridBus.h"
#include "linkModels/acLine.h"
#include "testHelper.h"
#include <cstdio>
#include <vectorOps.hpp>
#include <map>
#include <array>
#include <utility>
#include <iostream>
#include <functional>
//test case for gridCoreObject object

#define INPUT_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/input_tests/"

BOOST_FIXTURE_TEST_SUITE (input_tests, gridDynSimulationTestFixture)



//TODO:: convert to a DATA TEST CASE
BOOST_AUTO_TEST_CASE(test_power_flow_inputs)
{
  /* *INDENT-OFF* */
const std::map<std::string, std::array<int, 2>> baseCDFcase{
  { "ieee14_act.cdf", { { 14, 20 } } },
  { "ieee30_act.cdf", { { 30, 41 } } },
  { "ieee57_act.cdf", { { 57, 80 } } },
  { "ieee118_act.cdf", { { 118, 186 } } },
  { "ieee300.cdf", { { 300, 411 } } },
  { "IEEE39.raw", { { 39, 46 } } },
  { std::string(INPUT_TEST_DIRECTORY "testCSV5k.xml"), { { 5000, 6279 } } },
  };
/* *INDENT-ON* */

std::vector<double> volts1;
std::vector<double> ang1;
std::vector<double> volts2;
std::vector<double> ang2;
std::vector<double> P1;
std::vector<double> P2;
std::vector<double> Q1;
std::vector<double> Q2;


for (const auto &mp : baseCDFcase)
  {
  gds = new gridDynSimulation();
  std::string fname;
  if (mp.first.length() > 25)
    {
    fname = mp.first;
    }
  else
    {
      fname = std::string(IEEE_TEST_DIRECTORY) + mp.first;
    }

  loadFile(gds, fname);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);
  //gds->set("consoleprintlevel", "trace");
  int count=gds->getInt("totalbuscount");
  BOOST_CHECK_EQUAL(count, mp.second[0]);
  //check the linkcount
  count=gds->getInt("totallinkcount");
  BOOST_CHECK_EQUAL(count, mp.second[1]);
  volts1.resize(0);
  ang1.resize(0);
  volts2.resize(0);
  ang2.resize(0);
  P1.resize(0);
  P2.resize(0);
  Q1.resize(0);
  Q2.resize(0);
  auto ct=gds->getVoltage(volts1);
  gds->getAngle(ang1);
  BOOST_CHECK(ct == ang1.size());
  gds->pFlowInitialize();
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED);
  gds->updateLocalCache();
  gds->getBusGenerationReal(P1);
  gds->getBusGenerationReactive(Q1);



  gds->powerflow();
  //printf("completed power flow\n");
  if (gds->currentProcessState() != gridDynSimulation::gridState_t::POWERFLOW_COMPLETE)
  {
    std::cout << fname << " did not complete power flow calculation" << '\n';
  }
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage(volts2);
  gds->getAngle(ang2);
  gds->getBusGenerationReal(P2);
  gds->getBusGenerationReactive(Q2);
  std::function<void(size_t,double,double)> vfunc = [=](size_t index, double v1, double v2)
  {
	  std::cout << mp.first << " Voltage difference bus " << index+ 1 << "::" << v1 << " vs. " << v2 << "::" << v1 - v2 << '\n';
  };

  auto vdiff = countDiffsCallback(volts1, volts2, 0.0008,vfunc);

  std::function<void(size_t, double, double)> afunc = [=](size_t index, double v1, double v2)
  {
	  std::cout << mp.first << " Angle difference bus " << index + 1 << "::" << v1* 180.0 / kPI << " vs. " << v2* 180.0 / kPI << "::" << (v1 - v2)* 180.0 / kPI << '\n';
  };
  auto adiff = countDiffsCallback(ang1, ang2, 0.0009,afunc);

  std::function<void(size_t, double, double)> Pfunc = [=](size_t index, double v1, double v2)
  {
	  std::cout << mp.first << " Power difference-- bus " << index + 1 << "::" << v1 << " vs. " << v2 << '\n';
  };
  auto pdiff = countDiffsIfValidCallback(P1, P2, 0.01,Pfunc);
  std::function<void(size_t, double, double)> Qfunc = [=](size_t index, double v1, double v2)
  {
	  std::cout << mp.first << " Q difference-- bus " << index + 1 << "::" << v1 << " vs. " << v2 << "::" << v1 - v1 << '\n';
  };
  auto qdiff = countDiffsIfValidCallback(Q1, Q2, 0.01,Qfunc);
  BOOST_CHECK_EQUAL(vdiff, 0);
  BOOST_CHECK_EQUAL(adiff, 0);
  BOOST_CHECK_EQUAL(pdiff, 0);
  BOOST_CHECK_EQUAL(qdiff, 0);
  if (qdiff > 0)
  {
	  printf("%f vs %f diff %f\n", sum(Q1), sum(Q2), sum(Q1) - sum(Q2));
  }
  // debugging aids
  /*
  loss = gds->getLoss();
  gen = gds->getGenerationReal();
  ld = gds->getLoadReal();
  */
 
  //check that the reset works correctly
  if (mp.first == "ieee300.cdf")
    { //need to do a custom reset for the peculiarities of this system
    gds->reset(reset_levels::voltage_angle);

    //do a manual reset of the voltages
	//gds->consolePrintLevel = print_level::trace;
    for (int ii = 0; ii < 300; ++ii)
      {
		gridBus *bus = gds->getBus(ii);
      if (bus->getType() == gridBus::busType::PQ)
        {
        bus->set("voltage", 1.0);
        }
      }
    int cnt = 0;
    for (int ii = 0; ii < 411; ++ii)
      {
		gridLink *lnk = gds->getLink(ii);
      if (dynamic_cast<adjustableTransformer *>(lnk))
        {
        cnt++;
        if ((cnt >= 2)&(cnt <= 3))
          {
          lnk->reset(reset_levels::full);
          lnk->set("center", "target");
          break;
          }

        }
      }
    }
  else
    {
    gds->reset(reset_levels::full);
    }
 
  gds->powerflow();
  //printf("completed power flow\n");
  if (gds->currentProcessState() != gridDynSimulation::gridState_t::POWERFLOW_COMPLETE)
  {
    std::cout << fname << " did not complete power flow calculation 2" << '\n';
  }
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);

  gds->getVoltage(volts1);
  gds->getAngle(ang1);
  gds->getBusGenerationReal(P2);
  gds->getBusGenerationReactive(Q2);

  vdiff = countDiffs(volts1,volts2,0.0005);
  adiff = countDiffs(ang1, ang2, 0.0009);
  
  BOOST_CHECK_EQUAL(vdiff, 0);
  BOOST_CHECK_EQUAL(adiff, 0);

  delete gds;
  gds = nullptr;

  }
	
}



//TODO:: convert to a DATA TEST CASE
BOOST_AUTO_TEST_CASE(compare_cases)
{
  /* *INDENT-OFF* */
const std::vector<stringVec> compareCases{
  { "ieee118_act.cdf", "ieee118.psp"/*, "IEEE 118 Bus.EPC"*/ },
  { "ieee14_act.cdf", /*"IEEE 14 bus.epc",*/ "IEEE 14 bus.raw" },
  { "IEEE39.raw", "ieee39_v29.raw" },
  { "ieee30_no_limit.cdf", std::string(INPUT_TEST_DIRECTORY "testCSV.xml") },
  };
/* *INDENT-ON* */
std::vector<double> volts1;
std::vector<double> ang1;
std::vector<double> volts2;
std::vector<double> ang2;

for (const auto &mp : compareCases)
  {
  gds = new gridDynSimulation();
  
  std::string fname = std::string(IEEE_TEST_DIRECTORY) + mp[0];
  loadFile(gds, fname);

  int bcount=gds->getInt("totalbuscount");

  //check the linkcount
  int lcount=gds->getInt("totallinkcount");

  //now run the power flows of both of them
  gds->powerflow();

  gds->getVoltage(volts1);
  gds->getAngle(ang1);

  //now load the equivalent files
  for (size_t ns=1;ns<mp.size();++ns)
    {
    std::string fname2 = mp[ns];
	std::string nf = mp[ns];
    gds2 = new gridDynSimulation();
    if (fname2.size() < 25)
      {
      fname2 = std::string(IEEE_TEST_DIRECTORY) + nf;
      }

    loadFile(gds2, fname2);

    int count=gds2->getInt("totalbuscount");
    BOOST_CHECK_EQUAL(count, bcount);
    /*for (kk = 0; kk < count; ++kk)
      {
      b2 = gds2->getBus(kk);
      b1 = gds->getBus(kk);
      cmp = compareBus(b2, b1, true);
      if (cmp == false)
      {
      std::cout << "bus " << kk << " does not match" << '\n';
      cmp = compareBus(b1, b2, true);
      }
      b2->setVoltageAngle(volts1[kk], ang1[kk]);

      }*/
    //check the linkcount
    count=gds2->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, lcount);

    gds2->powerflow();
    //printf("completed power flow\n");
   
    BOOST_REQUIRE_MESSAGE(gds2->currentProcessState() == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE,fname2 <<" failed to complete");


    gds2->getVoltage(volts2);
    gds2->getAngle(ang2);

    int vdiff = 0;
    int adiff = 0;


    for (size_t kk = 0; kk<volts1.size(); ++kk)
      {

      if (std::abs(volts1[kk] - volts2[kk])>0.0008)
        {
        std::cout <<mp[0]<<" vs. "<< nf << " Voltage difference bus " << kk + 1 << "::" << volts1[kk] << " vs. " << volts2[kk] << '\n';
        vdiff++;
        }

      if (std::abs(ang1[kk] - ang2[kk]) > 0.0009)
        {
        std::cout << mp[0] << " vs. " << nf << " Angle difference-- bus " << kk + 1 << "::" << ang1[kk] * 180.0 / kPI << " vs. " << ang2[kk] * 180.0 / kPI << "::" << std::abs(ang1[kk] - ang2[kk]) * 180.0 / kPI << " deg" << '\n';
        adiff++;
        }

      }
    BOOST_CHECK_EQUAL(vdiff, 0);
    BOOST_CHECK_EQUAL(adiff, 0);
    delete gds2;
    gds2 = nullptr;
    }
  delete gds;
  gds = nullptr;
  }
  }


  //TODO:: convert to a DATA TEST CASE
BOOST_AUTO_TEST_CASE(input_execTest)
{
  /* *INDENT-OFF* */
const std::map<std::string, std::array<int, 4>> executionCases{
  { std::string(MATLAB_TEST_DIRECTORY "case4gs.m"), { { 0, 4, 4, 0 } } },
 // { std::string(MATLAB_TEST_DIRECTORY "d_003.m"), { { 0, 3, 3, 0 } } },
 // { std::string(INPUT_TEST_DIRECTORY "test_mat_dyn.xml"), { { 1, 9, 9, 2 } } },
  { std::string(INPUT_TEST_DIRECTORY "test_2m4bDyn_inputchange.xml"), { { 1, 0, 0, 0 } } },
  { std::string(INPUT_TEST_DIRECTORY "testIEEE39dynamic.xml"), { { 1, 39, 0, 0 } } },
//  { std::string(INPUT_TEST_DIRECTORY "testIEEE39dynamic_relay.xml"), { { 1, 39, 0, 0 } } },
  //{ std::string(INPUT_TEST_DIRECTORY "180busdyn_test.xml"),{ { 1, 179, 0, 1 } } },
  };
/* *INDENT-ON* */
int count;
for (const auto &mp : executionCases)
  {
  auto fname = mp.first;

  gds = new gridDynSimulation();

  loadFile(gds, fname);
  BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::STARTUP);

  if (mp.second[1] > 0)
    {
    count=gds->getInt("totalbuscount");
    BOOST_CHECK_EQUAL(count, mp.second[1]);
    }
  //check the linkcount
  if (mp.second[2] > 0)
    {
    count=gds->getInt("totallinkcount");
    BOOST_CHECK_EQUAL(count, mp.second[2]);
    }
  if (mp.second[3] > 0)
    {
    count=gds->getInt("eventcount");
    BOOST_CHECK_EQUAL(count, mp.second[3]);
    }
  if (mp.second[0] == 0)
    {
    gds->powerflow();
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::POWERFLOW_COMPLETE);
    }
  else if (mp.second[0] == 1)
    {
    gds->run();
    //gds->captureJacState("dynCap.dat", true);
    BOOST_REQUIRE (gds->currentProcessState () == gridDynSimulation::gridState_t::DYNAMIC_COMPLETE);
    }
  delete gds;
  gds = nullptr;
  }


}


BOOST_AUTO_TEST_SUITE_END()
