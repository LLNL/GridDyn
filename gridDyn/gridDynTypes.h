/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
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
#include "gridDyn_time.h"
#include <cstdint>


#ifdef ENABLE_64_BIT_INDEXING
typedef std::uint64_t index_t;
typedef std::uint64_t count_t;
#else
typedef std::uint32_t index_t;
typedef std::uint32_t count_t;
#endif

//at some point gridDyn may move to a different type for the time representation
//typedef double gridDyn_time;

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




typedef timeRepresentation<count_time<9>> gridDyn_time;


inline double operator/(double x, gridDyn_time t)
{
	return x / static_cast<double>(t);
}

inline double operator*(double x, gridDyn_time t)
{
	return x*static_cast<double>(t);
}


inline gridDyn_time operator-(gridDyn_time t, double x)
{
	return t-gridDyn_time(x);
}

inline gridDyn_time operator-(double x, gridDyn_time t)
{
	return gridDyn_time(x)-t;
}

inline gridDyn_time operator+(gridDyn_time t, double x)
{
	return t + gridDyn_time(x);
}

inline gridDyn_time operator+(double x, gridDyn_time t)
{
	return gridDyn_time(x) + t;
}

inline double operator/(gridDyn_time t1, gridDyn_time t2)
{
	return static_cast<double>(t1) / static_cast<double>(t2);
}


inline bool operator==(gridDyn_time t1, double rhs)
{
	return (t1 == gridDyn_time(rhs));
}

inline bool operator!=(gridDyn_time t1, double rhs)
{
	return (t1 != gridDyn_time(rhs));
}

inline bool operator>(gridDyn_time t1, double rhs)
{
	return (t1 > gridDyn_time(rhs));
}

inline bool operator<(gridDyn_time t1, double rhs)
{
	return (t1 < gridDyn_time(rhs));
}

inline bool operator>=(gridDyn_time t1,double rhs)
{
	return (t1 >= gridDyn_time(rhs));
}

inline bool operator<=(gridDyn_time t1, double rhs)
{
	return (t1 <= gridDyn_time(rhs));
}

inline bool operator==(double lhs, gridDyn_time t1)
{
	return (gridDyn_time(lhs) == t1);
}

inline bool operator!=(double lhs, gridDyn_time t1)
{
	return (gridDyn_time(lhs) != t1);
}

inline bool operator>(double lhs, gridDyn_time t1)
{
	return (gridDyn_time(lhs) > t1);
}

inline bool operator<(double lhs, gridDyn_time t1)
{
	return (gridDyn_time(lhs) < t1);
}

inline bool operator>=(double lhs, gridDyn_time t1)
{
	return (gridDyn_time(lhs) >= t1);
}

inline bool operator<=(double lhs, gridDyn_time t1)
{
	return (gridDyn_time(lhs) <= t1);
}

const gridDyn_time maxTime=gridDyn_time::max();
const gridDyn_time negTime=gridDyn_time::min();
const gridDyn_time timeZero(0.0);
const gridDyn_time timeOne(1.0);

const gridDyn_time kDayLength(86400.0f);
const gridDyn_time kSmallTime(1e-7);
const gridDyn_time kShortTime(1e-6);

#endif
