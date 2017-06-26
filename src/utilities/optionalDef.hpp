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

#ifndef OPTIONALDEFS_H_
#define OPTIONALDEFS_H_
#pragma once

#include "config.h"

#ifdef HAVE_OPTIONAL
#include <optional>
namespace utilities
{
template <typename X>
using optional = std::optional<X>;
}  // namespace utilities
#else
#ifdef HAVE_EXP_OPTIONAL
#include <experimental/optional>
namespace utilities
{
template <typename X>
using optional = std::experimental::optional<X>;
}  // namespace utilities
#else
#include <boost/optional.hpp>
namespace utilities
{
template <typename X>
using optional = boost::optional<X>;
}  // namespace utilities

#endif
#endif

#endif
