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
#ifndef MATRIX_OPS_H
#define MATRIX_OPS_H

#include  "matrixData.h"
#include <vector>
/** multiply a matrix data object by a vector
@details assumes vec has the appropriate size otherwise undefined behavior will occur
@param[in] ad the matrix to multiply size MxN
@param[in] vec the vector data to multiply at least size N
@param[out] res the location to store the results
@return a vector with the results the vector will be of length M
*/
void matrixDataMultiply(matrixData<double> &ad, const double vec[], double res[]);

/** multiply a matrix data object by a vector
@details assumes vec has the appropriate size otherwise undefined behavior will occur
@param[in] ad the matrix to multiply size MxN
@param[in] vec the vector data to multiply at least size N
@return a vector with the results the vector will be of length M
*/
std::vector<double> matrixDataMultiply(matrixData<double> &ad, const double vec[]);


#endif
