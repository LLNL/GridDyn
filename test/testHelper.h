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
#include "griddyn.h"
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

namespace griddyn
{
class Load;
class solverMode;
}

//create a test fixture that makes sure everything gets deleted properly
struct gridDynSimulationTestFixture
{

  gridDynSimulationTestFixture();
  gridDynSimulationTestFixture(const gridDynSimulationTestFixture &) = delete;
  ~gridDynSimulationTestFixture();
  gridDynSimulationTestFixture &operator=(const gridDynSimulationTestFixture &) = delete;

  std::unique_ptr<griddyn::gridDynSimulation> gds=nullptr;
  std::unique_ptr<griddyn::gridDynSimulation> gds2=nullptr;

  void simpleRunTestXML(const std::string &fileName);
  void detailedStageCheck(const std::string &fileName, griddyn::gridDynSimulation::gridState_t finalState);
  void simpleStageCheck(const std::string &fileName, griddyn::gridDynSimulation::gridState_t finalState);
  void dynamicInitializationCheck(const std::string &fileName);

  void checkState(griddyn::gridDynSimulation::gridState_t state);
  void requireState(griddyn::gridDynSimulation::gridState_t state);
  void checkState2(griddyn::gridDynSimulation::gridState_t state);
  void requireState2(griddyn::gridDynSimulation::gridState_t state);
};

struct gridLoadTestFixture
{

  gridLoadTestFixture();
  gridLoadTestFixture(const  gridLoadTestFixture &) = delete;
  ~gridLoadTestFixture();
  gridLoadTestFixture &operator=(const  gridLoadTestFixture &) = delete;

  griddyn::Load *ld1=nullptr;
  griddyn::Load *ld2=nullptr;
};


struct glbconfig
{
	glbconfig();
	~glbconfig();
};

/** forward declaration of the operator for streaming gridDyn states
@details this is used in the BOOST_REQUIRE_EQUALS functions for nice printing if things go wrong
*/
std::ostream& operator<<(std::ostream& os, griddyn::gridDynSimulation::gridState_t state);

const std::string &to_string(griddyn::gridDynSimulation::gridState_t state);

void checkStates(griddyn::gridDynSimulation::gridState_t state1, griddyn::gridDynSimulation::gridState_t state2);
void requireStates(griddyn::gridDynSimulation::gridState_t state1, griddyn::gridDynSimulation::gridState_t state2);
int runJacobianCheck(std::unique_ptr<griddyn::gridDynSimulation> &gds, const griddyn::solverMode &sMode, bool checkRequired=true);

int runJacobianCheck(std::unique_ptr<griddyn::gridDynSimulation> &gds, const griddyn::solverMode &sMode, double tol, bool checkRequired = true);

int runResidualCheck(std::unique_ptr<griddyn::gridDynSimulation> &gds, const griddyn::solverMode &sMode, bool checkRequired = true);

int runDerivativeCheck(std::unique_ptr<griddyn::gridDynSimulation> &gds, const griddyn::solverMode &sMode, bool checkRequired = true);

int runAlgebraicCheck(std::unique_ptr<griddyn::gridDynSimulation> &gds, const griddyn::solverMode &sMode, bool checkRequired = true);

void printBusResultDeviations(const std::vector<double> &V1, const std::vector<double> &V2, const std::vector<double> &A1, const std::vector<double> &A2);

#endif
