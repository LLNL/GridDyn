/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*-
 */
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

#ifndef FNCS_LIBRARY_HEADER_
#define FNCS_LIBRARY_HEADER_

#include <complex>
#include <string>

void loadFNCSLibrary();

void fncsSendComplex(const std::string& key, double real, double imag);
void fncsSendComplex(const std::string& key, std::complex<double> val);
std::complex<double> fncsGetComplex(const std::string& key);
void fncsSendVal(const std::string& key, double val);
double fncsGetVal(const std::string& key);

#endif
