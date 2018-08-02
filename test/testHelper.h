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
#pragma once

#include "fileInput/fileInput.h"
#include "griddyn/gridDynSimulation.h"
#include <iosfwd>
#include <string>
#include <vector>
//#define WINDOWS_MEMORY_DEBUG

#ifdef WINDOWS_MEMORY_DEBUG
#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#endif
#endif

#ifndef GRIDDYN_TEST_DIRECTORY
#define GRIDDYN_TEST_DIRECTORY "./test_files/"
#endif

#define IEEE_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/"
#define MATLAB_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/matlab_test_files/"
#define OTHER_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/other_test_cases/"

static const std::string ieee_test_directory = std::string (GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/");
static const std::string matlab_test_directory = std::string (GRIDDYN_TEST_DIRECTORY "/matlab_test_files/");
static const std::string other_test_directory = std::string (GRIDDYN_TEST_DIRECTORY "/other_test_cases/");

#define ENABLE_IN_DEVELOPMENT_CASES

namespace griddyn
{
class Load;
class solverMode;
}  // namespace griddyn

// create a test fixture that makes sure everything gets deleted properly
struct gridDynSimulationTestFixture
{
    /** constructor*/
    gridDynSimulationTestFixture ();
    /** remove copy constructor*/
    gridDynSimulationTestFixture (const gridDynSimulationTestFixture &) = delete;
    /**destructor*/
    ~gridDynSimulationTestFixture ();
    /** remove copy assignment*/
    gridDynSimulationTestFixture &operator= (const gridDynSimulationTestFixture &) = delete;

    std::unique_ptr<griddyn::gridDynSimulation> gds; /** first simulation object*/
    std::unique_ptr<griddyn::gridDynSimulation> gds2; /** second simulation object*/

    /** run a simple test
    @details loads a file, runs the simulation, checks if the simulation completed*/
    void simpleRunTestXML (const std::string &fileName);
    /** run a test
    @details loads a file, runs the simulation, checks if the simulation is in the specified state
    */
    void runTestXML (const std::string &fileName, griddyn::gridDynSimulation::gridState_t finalState);
    /** run the simulation and verify it progressed properly along each step of the process
     */
    void detailedStageCheck (const std::string &fileName, griddyn::gridDynSimulation::gridState_t finalState);
    void simpleStageCheck (const std::string &fileName, griddyn::gridDynSimulation::gridState_t finalState);
    void dynamicInitializationCheck (const std::string &fileName);

    /** check that gds is in the specified state*/
    void checkState (griddyn::gridDynSimulation::gridState_t state);
    /** require that gds is in the specified state*/
    void requireState (griddyn::gridDynSimulation::gridState_t state);
    /** check that gds2 is in the specified state*/
    void checkState2 (griddyn::gridDynSimulation::gridState_t state);
    /** require that gds2 is in the specified state*/
    void requireState2 (griddyn::gridDynSimulation::gridState_t state);
};

/** specialized fixture for load tests
 */
struct gridLoadTestFixture
{
    gridLoadTestFixture ();
    gridLoadTestFixture (const gridLoadTestFixture &) = delete;
    ~gridLoadTestFixture ();
    gridLoadTestFixture &operator= (const gridLoadTestFixture &) = delete;

    griddyn::Load *ld1 = nullptr;
    griddyn::Load *ld2 = nullptr;
};

/** struct defining any global config for the test scripts
@details currently doesn't do much
*/
struct glbconfig
{
    glbconfig ();
    ~glbconfig ();
};

/** forward declaration of the operator for streaming gridDyn states
@details this is used in the BOOST_REQUIRE_EQUALS functions for nice printing if things go wrong
*/
std::ostream &operator<< (std::ostream &os, griddyn::gridDynSimulation::gridState_t state);

/** convert a gridState_t into a string for printing purposes*/
const std::string &to_string (griddyn::gridDynSimulation::gridState_t state);

/** check two states to see if they are the same*/
void checkStates (griddyn::gridDynSimulation::gridState_t state1, griddyn::gridDynSimulation::gridState_t state2);

/** require two states to be the same
@details calls the BOOST_REQUIRE to ensure the states are equal
*/
void requireStates (griddyn::gridDynSimulation::gridState_t state1,
                    griddyn::gridDynSimulation::gridState_t state2);
/** run a Jacobian check on the current simulation object
@param[in] gds the simulation object to use
@param[in] sMode the solverMode to check
@param checkRequired set to true to halt the check procedure if there was any mismatches in the Jacobian
@return the count of the number of mismatches between the calculated and numeric calculation
*/
int runJacobianCheck (std::unique_ptr<griddyn::gridDynSimulation> &gds,
                      const griddyn::solverMode &sMode,
                      bool checkRequired = true);

/** run a Jacobian check on the current simulation object
@param[in] gds the simulation object to use
@param[in] sMode the solverMode to check
@param tol the tolerance to use for the check
@param checkRequired set to true to halt the check procedure if there was any mismatches in the Jacobian
@return the count of the number of mismatches between the calculated and numeric calculation
*/
int runJacobianCheck (std::unique_ptr<griddyn::gridDynSimulation> &gds,
                      const griddyn::solverMode &sMode,
                      double tol,
                      bool checkRequired = true);

/** run a residual check on the current simulation object
@param[in] gds the simulation object to use
@param[in] sMode the solverMode to check
@param checkRequired set to true to halt the check procedure if there was any mismatches in the Jacobian
@return the count of the number of residual values that are sufficiantly far from 0
*/
int runResidualCheck (std::unique_ptr<griddyn::gridDynSimulation> &gds,
                      const griddyn::solverMode &sMode,
                      bool checkRequired = true);

/** run a derivative check on the current simulation object
@param[in] gds the simulation object to use
@param[in] sMode the solverMode to check
@param checkRequired set to true to halt the check procedure if there was any mismatches in the Jacobian
@return the count of the number of mismatches between the calculated and numeric calculation
*/
int runDerivativeCheck (std::unique_ptr<griddyn::gridDynSimulation> &gds,
                        const griddyn::solverMode &sMode,
                        bool checkRequired = true);

/** run an algebraic check on the current simulation object
@param[in] gds the simulation object to use
@param[in] sMode the solverMode to check
@param checkRequired set to true to halt the check procedure if there was any mismatches in the Jacobian
@return the count of the number of mismatches between the calculated and numeric calculation
*/
int runAlgebraicCheck (std::unique_ptr<griddyn::gridDynSimulation> &gds,
                       const griddyn::solverMode &sMode,
                       bool checkRequired = true);

/** print any differences in bus voltages and angles between two vectors
@details targetted at checking two sets of values between different simulations to make sure they produce the same
result
@param V1 the voltages of the first set
@param V2 the voltages of the second set
@param A1 the angles of the first set
@param A2 the angles of the second set
*/
void printBusResultDeviations (const std::vector<double> &V1,
                               const std::vector<double> &V2,
                               const std::vector<double> &A1,
                               const std::vector<double> &A2);
