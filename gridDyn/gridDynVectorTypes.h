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

#include "gridDynTypes.h"
#include <vector>
#include <string>

typedef std::vector<std::string> stringVec;

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

typedef boost::container::small_vector<double, 4> IOdata;
typedef boost::container::small_vector<index_t, 4> IOlocs;
#else
typedef std::vector<double> IOdata;
typedef std::vector<index_t> IOlocs;
#endif

#endif

