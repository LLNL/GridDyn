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

#ifndef GRIDDYN_DIAGNOSTICS_H_
#define GRIDDYN_DIAGNOSTICS_H_

#include <memory>
#include <string>
#include "core/coreDefinitions.h"

class gridDynSimulation;
class solverMode;
class solverInterface;
class coreObject;

template<class X>
class matrixData;

const double resid_check_tol = 1e-5;
const double jac_check_tol = 1e-5;
/** @brief function check on the Jacobian
  function does a comparison between the computed Jacobian via the jacobianElements function call and a numerically calculated
version from the residual,  It will not check Jacobian elements dependent on other state derivatives
This function is mostly useful for diagnosing problems and is used throughout the test suite
@param[in] gds the gridDynSimulation object to test
@param[in] sMode the solverMode to check the Jacobian for
@param[in] jacTol  the tolerance to check matches
@param[in] useStateNames  set to true to collect and print state names (vs numbers) for any mismatch on the Jacobian check
@return the number of mismatches
*/
int JacobianCheck (gridDynSimulation *gds, const solverMode &sMode, double jacTol = jac_check_tol, bool useStateNames = false);

/** @brief do a residual check
  function checks for any non-zero residuals usually used after an initialization or step.
@param[in] gds the griddynSimulation object to test
@param[in] sMode the solverMode to check the residual
@param[in] residTol  the tolerance to check matches
@param[in] useStateNames  set to true to collect and print state names (vs numbers) for any mismatch on the Jacobian check
@return the number of mismatches
*/
int residualCheck (gridDynSimulation *gds, const solverMode &sMode, double residTol = resid_check_tol, bool useStateNames = false);
/** @brief do a residual check
  function checks for any non-zero residuals usually used after an initialization or step.
@param[in] gds the griddynSimulation object to test
@param[in] time the time to check the residual
@param[in] sMode the solverMode to check the residual
@param[in] residTol  the tolerance to check matches
@param[in] useStateNames  set to true to collect and print state names (vs numbers) for any mismatch on the Jacobian check
@return the number of mismatches
*/
int residualCheck (gridDynSimulation *gds, coreTime time, const solverMode &sMode, double residTol = resid_check_tol, bool useStateNames = false);

std::pair<double, int> checkResid (gridDynSimulation *gds, const std::shared_ptr<solverInterface> &sd);

std::pair<double, int> checkResid (gridDynSimulation *gds, coreTime time, const std::shared_ptr<solverInterface> &sd);

std::pair<double, int> checkResid (gridDynSimulation *gds, coreTime time, const solverMode &sMode);

int algebraicCheck (gridDynSimulation *gds, coreTime time, const solverMode &sMode, double residTol = resid_check_tol, bool useStateNames = false);

int derivativeCheck (gridDynSimulation *gds, coreTime time, const solverMode &sMode, double derivTol = resid_check_tol, bool useStateNames = false);

/** @brief do a convergence test on the solver
*/
void dynamicSolverConvergenceTest (gridDynSimulation *gds, const solverMode &sMode, const std::string &file, unsigned int pts = 100000, int mode = 0);

/** @brief print out the structure and count of the Jacobian entries and counts
@param[in] ad the matrix data object to analyze
@param[in] gds the gridDynSimulation object to work with
@param[in] the solver mode in use
*/
void jacobianAnalysis(matrixData<double> &ad, gridDynSimulation *gds, const solverMode &sMode, int level);



/** @brief check object equivalence 
@details checks if the objects are equivalent in function and if instructed spits out messages of the differences
@param[in] obj1 the first object to compare
@param[in] obj2 the second object to compare
@param[in] printMessage bool indicating that messages should be printed
@return true if the objects are deemed equivalent*/

bool checkObjectEquivalence(const coreObject *obj1, const coreObject *obj2, bool printMessage = true);

#endif