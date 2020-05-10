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

#ifndef GRIDDYN_EXPORT_ADVANCED_C_H_
#define GRIDDYN_EXPORT_ADVANCED_C_H_

#include "griddyn_export.h"

#ifdef __cplusplus
extern "C" {
#endif

// object mathematical functions*/

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_stateSize(gridDynObject obj, solverKey key);

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_guessState(gridDynObject obj,
                                                       double time,
                                                       double* states,
                                                       double* dstate_dt,
                                                       solverKey key);

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_setState(gridDynObject obj,
                                                     double time,
                                                     const double* states,
                                                     const double* dstate_dt,
                                                     solverKey key);

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_getStateVariableTypes(gridDynObject obj,
                                                                  double* types,
                                                                  solverKey key);

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_residual(gridDynObject obj,
                                                     const double* inputs,
                                                     int inputSize,
                                                     double* resid,
                                                     solverKey key);

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_derivative(gridDynObject obj,
                                                       const double* inputs,
                                                       int inputSize,
                                                       double* deriv,
                                                       solverKey key);

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_algebraicUpdate(gridDynObject obj,
                                                            const double* inputs,
                                                            int inputSize,
                                                            double* update,
                                                            double alpha,
                                                            solverKey key);

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_jacobian(gridDynObject obj,
                                                     const double* inputs,
                                                     int inputSize,
                                                     double cj,
                                                     void (*insert)(int, int, double),
                                                     solverKey key);

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_ioPartialDerivatives(gridDynObject obj,
                                                                 const double* inputs,
                                                                 int inputSize,
                                                                 void (*insert)(int, int, double),
                                                                 solverKey key);

/** get the statesize for an object
    @details this function uses a solverKey.  0 can be used on objects with no subobjects on objects with subobject
    using a key of 0 will generate an error code and not produce results
    @param[in] obj the object to query
    @param[in] key a solverKey obtained from ::gridDynSimulation_getSolverKey
    @return the state size of the object for a particular solverkey
    */
GRIDDYN_EXPORT griddyn_status gridDynObject_outputPartialDerivatives(gridDynObject obj,
                                                                     const double* inputs,
                                                                     int inputSize,
                                                                     void (*insert)(int,
                                                                                    int,
                                                                                    double),
                                                                     solverKey key);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif  // GRIDDYN_EXPORT_C_H_
