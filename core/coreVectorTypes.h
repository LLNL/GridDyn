/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2014, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

/* define the basic vector types*/

#ifndef GRIDDYN_VECTOR_TYPES_H_
#define GRIDDYN_VECTOR_TYPES_H_

#include "coreTypes.h"
#include <vector>
#include <string>

using stringVec= std::vector<std::string>;

/* May at some point in the future convert all the set/get functions to use this in the function prototypes
to facilitate transfer to a different type*/
using parameterName = const std::string &;

/* may at some point use this type alias to convert all object parameters to this type and then move them to a new class
to enable additional functionality*/
using parameter_t = double;

#include "boost/version.hpp"
#if BOOST_VERSION / 100 % 1000 >= 58
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include "boost/container/small_vector.hpp"
#pragma GCC diagnostic pop
#else
#include "boost/container/small_vector.hpp"
#endif

using IOdata = boost::container::small_vector<double, 4>;
using IOlocs = boost::container::small_vector<index_t, 4>;
#else
using IOdata = std::vector<double>;
using IOlocs = std::vector<index_t>;
#endif

const IOdata noInputs{};
const IOlocs noInputLocs{};

#endif

