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

/* define the basic indexing types*/
#pragma once
#ifndef CORE_TYPES_H_
#define CORE_TYPES_H_

#include "griddyn-config.h"
#include "timeRepresentation.h"
#include <cstdint>
#include <string>
#include <vector>


constexpr double kBigNum(1e49);  //!< a very large number
constexpr int kBigINT(0x7EDCBA98);  //!< a big arbitrary integer

constexpr double kHalfBigNum(kBigNum / 2.0);  //!< half of a very big number

constexpr double kNullVal(-1.456e47);  //!< a very large negative number used for an empty value

#define FUNCTION_EXECUTION_SUCCESS (0)
#define FUNCTION_EXECUTION_FAILURE (-1)

//bookkeeping object changes
#define UPDATE_TIME_CHANGE (1435)
#define OBJECT_NAME_CHANGE (1445)
#define OBJECT_ID_CHANGE (1446)
#define OBJECT_IS_SEARCHABLE (1455)
#define UPDATE_REQUIRED (2010)                            //!< indicator that an object is using an update function
#define UPDATE_NOT_REQUIRED (2011)                        //!< indicator that an object is no longer using an update function

#ifdef ENABLE_64_BIT_INDEXING
using index_t = std::uint64_t;
using count_t= std::uint64_t;
#else
using index_t = std::uint32_t;
using count_t = std::uint32_t;
#endif

constexpr index_t kNullLocation(static_cast<index_t>(-1));
constexpr index_t kInvalidLocation(static_cast<index_t>(-2));
constexpr count_t kInvalidCount(static_cast<count_t>(-1));

/** enumeration of print levels for logging
*/
enum class print_level
{
	no_print = 0, //!< never print
	error = 1,	//!< only print errors
	warning = 2,	//!< print/log warning and errors
	summary = 3,	//!< print a summary
	normal = 4,	//!< defualt print level
	debug = 5,	//!< debug level prints
	trace = 6,	//!< trace level printing 
};

using coreTime= timeRepresentation<count_time<9>>;
using stringVec = std::vector<std::string>;

/** defining some additional operators for coreTime that were not necessarily covered 
by the class definition
*/
/** division operator with double as the numerator*/
inline double operator/(double x, coreTime t)
{
	return x / static_cast<double>(t);
}
/** we are distinguishing here between a time as the first operator and time as the second
@details it is a semantic difference time as the first element of a multiplication should produce a time
time as the second should be treated as a number and produce another number*/
inline double operator*(double x, coreTime t)
{
	return x*static_cast<double>(t);
}

/** dividing two times is a ratio and should produce a number*/
inline double operator/(coreTime t1, coreTime t2)
{
	return static_cast<double>(t1) / static_cast<double>(t2);
}

inline coreTime operator-(coreTime t, double x)
{
	return t - coreTime(x);
}

inline coreTime operator-(double x, coreTime t)
{
	return coreTime(x) - t;
}

inline coreTime operator+(coreTime t, double x)
{
	return t + coreTime(x);
}

inline coreTime operator+(double x, coreTime t)
{
	return coreTime(x) + t;
}


inline bool operator==(coreTime t1, double rhs)
{
	return (t1 == coreTime(rhs));
}

inline bool operator!=(coreTime t1, double rhs)
{
	return (t1 != coreTime(rhs));
}

inline bool operator>(coreTime t1, double rhs)
{
	return (t1 > coreTime(rhs));
}

inline bool operator<(coreTime t1, double rhs)
{
	return (t1 < coreTime(rhs));
}

inline bool operator>=(coreTime t1, double rhs)
{
	return (t1 >= coreTime(rhs));
}

inline bool operator<=(coreTime t1, double rhs)
{
	return (t1 <= coreTime(rhs));
}

inline bool operator==(double lhs, coreTime t1)
{
	return (coreTime(lhs) == t1);
}

inline bool operator!=(double lhs, coreTime t1)
{
	return (coreTime(lhs) != t1);
}

inline bool operator>(double lhs, coreTime t1)
{
	return (coreTime(lhs) > t1);
}

inline bool operator<(double lhs, coreTime t1)
{
	return (coreTime(lhs) < t1);
}

inline bool operator>=(double lhs, coreTime t1)
{
	return (coreTime(lhs) >= t1);
}

inline bool operator<=(double lhs, coreTime t1)
{
	return (coreTime(lhs) <= t1);
}
/** commonly used time expressions*/
constexpr coreTime maxTime=coreTime::maxVal();
constexpr coreTime negTime=coreTime::minVal();
constexpr coreTime timeZero=coreTime::zeroVal();

constexpr coreTime timeOneSecond(1.0);

constexpr coreTime kDayLength(86400.0f);
constexpr coreTime kSmallTime(1e-7);
constexpr coreTime kShortTime(1e-6);

#endif
