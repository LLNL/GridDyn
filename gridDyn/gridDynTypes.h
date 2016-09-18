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

/* define the basic indexing types*/

#ifndef GRIDDYN_TYPES_H_
#define GRIDDYN_TYPES_H_

#include "griddyn-config.h"

#include <cstdint>

#ifdef ENABLE_64_BIT_INDEXING
typedef std::uint64_t index_t;
typedef std::uint64_t count_t;
#else
typedef std::uint32_t index_t;
typedef std::uint32_t count_t;
#endif

//at some point gridDyn may move to a different type for the time representation
typedef double gridDyn_time;

const index_t kNullLocation (static_cast<index_t>(-1));
const index_t kInvalidLocation (static_cast<index_t>(-2));
const count_t kInvalidCount (static_cast<count_t>(-1));

const double kBigNum (1e49);  //!< what GridDyn uses for infinity
const int kBigINT (0x7EDCBA98);  //!< a big arbitrary integer

const double kHalfBigNum (kBigNum/2);  //!< half of a very big number

const double kNullVal (-1.456e47);  //!< what GridDyn will use as a null value for many return functions

/** @brief enumeration of object changes that can occur throughout the simulation */
enum class change_code
{
  not_triggered = -2,                           //!< no potential change was triggered
  execution_failure = -1,               //!< the execution has failed
  no_change = 0,                                //!< there was no change
  non_state_change = 1,                 //!< a change occurred that cannot affect the states
  parameter_change = 2,                 //!< a parameter change occurred
  jacobian_change = 3,                  //!< a change to the number of non-zeros occurred
  object_change = 4,                            //!< a change in the number of number of objects occurred
  state_count_change = 5,               //!< a change in the number of states occurred
};

#endif
