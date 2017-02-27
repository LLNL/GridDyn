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
#pragma once
#ifndef NUMERIC_ESTIMATION_FUNCTIONS_H_

#define NUMERIC_ESTIMATION_FUNCTIONS_H_

#include "gridDynDefinitions.h"

class gridObject;
class stateData;


template<class X>
class matrixData;

class solverMode;

/**
*@brief numerically compute the partial derivatives of the internal states with respect to inputs and other internal states
@details numerically computes the elements of the Jacobian matrix of the internal states of the given object
@param[in] obj the object to compute the partial derivatives for
@param[in] inputs the inputs for the secondary object
* @param[in] sD the current state data for the simulation
* @param[out] ad  the array to store the information in
* @param[in] inputLocs the vector of input argument locations
* @param[in] sMode the operations mode
**/

void numericJacobianCalculation(gridObject *obj, const IOdata & inputs, const stateData &sD, matrixData<double> &ad, const IOlocs & inputLocs, const solverMode & sMode);

/**
@brief function to copy the local state of an object from one data to another
@param[in] obj  the object to copy the state for
@param[in] state the current state vector
@param[out]  the location to copy the state information
@param[in] sMode the solver mode corresponding to the state
*/
void copyObjectState(const gridObject *obj, const double state[], double newstate[], const solverMode &sMode);

/** @brief get a vector of all the local state locations of an object
@param[in] obj  the object get all the state locations
@param[in] sMode the solver mode to get the locations for
@return a vector containing the indices of the states
*/
std::vector<index_t> getObjectStates(const gridObject *obj, const solverMode &sMode);
#endif
