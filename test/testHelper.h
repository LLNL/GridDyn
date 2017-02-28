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

#ifndef TEST_HELPER_HEADER_
#define TEST_HELPER_HEADER_

#include <string>
#include <vector>
#include <iosfwd>
#include "gridDyn.h"
//#define WINDOWS_MEMORY_DEBUG

#ifdef WINDOWS_MEMORY_DEBUG
#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif
#endif

#ifndef GRIDDYN_TEST_DIRECTORY
#define GRIDDYN_TEST_DIRECTORY "./test_files/"
#endif

#include "boost/version.hpp"
#if BOOST_VERSION / 100 % 1000 >= 60
#define ALLOW_DATA_TEST_CASES
#endif


#define IEEE_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/"
#define MATLAB_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/matlab_test_files/"
#define OTHER_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/other_test_cases/"

static const std::string ieee_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/");
static const std::string matlab_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/matlab_test_files/" );
static const std::string other_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/other_test_cases/");

#define ENABLE_IN_DEVELOPMENT_CASES

class gridLoad;
class solverMode;

//create a test fixture that makes sure everything gets deleted properly
struct gridDynSimulationTestFixture
{

  gridDynSimulationTestFixture();
  gridDynSimulationTestFixture(const gridDynSimulationTestFixture &) = delete;
  ~gridDynSimulationTestFixture();
  gridDynSimulationTestFixture &operator=(const gridDynSimulationTestFixture &) = delete;

  std::unique_ptr<gridDynSimulation> gds=nullptr;
  std::unique_ptr<gridDynSimulation> gds2=nullptr;

  void simpleRunTestXML(const std::string &fileName);
  void detailedStageCheck(const std::string &fileName, gridDynSimulation::gridState_t finalState);
  void simpleStageCheck(const std::string &fileName, gridDynSimulation::gridState_t finalState);
  void dynamicInitializationCheck(const std::string &fileName);
};

struct gridLoadTestFixture
{

  gridLoadTestFixture();
  gridLoadTestFixture(const  gridLoadTestFixture &) = delete;
  ~gridLoadTestFixture();
  gridLoadTestFixture &operator=(const  gridLoadTestFixture &) = delete;

  gridLoad *ld1=nullptr;
  gridLoad *ld2=nullptr;
};


struct glbconfig
{
	glbconfig();
	~glbconfig();
};

/** forward declaration of the operator for streaming gridDyn states
@details this is used in the BOOST_REQUIRE_EQUALS functions for nice printing if things go wrong
*/
std::ostream& operator<<(std::ostream& os, const gridDynSimulation::gridState_t state);

int runJacobianCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode, bool checkRequired=true);

int runJacobianCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode, double tol, bool checkRequired = true);

int runResidualCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode, bool checkRequired = true);

int runDerivativeCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode, bool checkRequired = true);

int runAlgebraicCheck(std::unique_ptr<gridDynSimulation> &gds, const solverMode &sMode, bool checkRequired = true);

void printBusResultDeviations(const std::vector<double> &V1, const std::vector<double> &V2, const std::vector<double> &A1, const std::vector<double> &A2);
#endif
