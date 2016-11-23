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
#include <cmath>

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

/** prototype class for representing time in GridDyn (incomplete yet)
@details implements time as a count of 1/2^30 seconds  roughly corresponding to 1 ns (actually about 0.93 ns)
this is done for performance because many mathematical operations are needed on the time and this way
it could be implemented using shift and masks for some conversions to floating point operations
*/

class gridDyn_time2
{
private:
	static const long long int scalar = (1<<30);
	static const double scalarf;
	long long int nseconds = 0;
public:
	//implicit conversion requested
	gridDyn_time2(double t)
	{
		double intpart;
		double frac=std::modf(t, &intpart);
		nseconds = static_cast<long long int>(intpart)*scalar + static_cast<long long int>(frac*scalarf);
	}

	gridDyn_time2(const gridDyn_time2 &x):nseconds(x.nseconds)
	{
		
	}

	gridDyn_time2(long long int nsec) :nseconds(nsec)
	{

	}

	gridDyn_time2& operator= (const gridDyn_time2 &x)
	{
		nseconds = x.nseconds;
		return *this;
	}

	gridDyn_time2& operator= (const double x)
	{
		double intpart;
		double frac = std::modf(x, &intpart);
		nseconds = (static_cast<long long>(intpart)<<30) + static_cast<long long>(frac*scalarf);
		return *this;
	}

	operator double()
	{
		return (static_cast<double>(nseconds >> 30) + static_cast<double>(0x0000'0000'3FFF'FFFF & nseconds) / scalarf);
	}

	gridDyn_time2 &operator+=(const gridDyn_time2 &rhs) {
		nseconds += rhs.nseconds;
		return *this;
	}

	gridDyn_time2 &operator-=(const gridDyn_time2 &rhs) {
		nseconds -= rhs.nseconds;
		return *this;
	}

	gridDyn_time2 operator+(const gridDyn_time2 &other) const 
	{
		return gridDyn_time2(nseconds + other.nseconds);
	}

	gridDyn_time2 operator-(const gridDyn_time2 &other) const
	{
		return gridDyn_time2(nseconds - other.nseconds);
	}

	gridDyn_time2 operator*(int multiplier) const
	{
		return gridDyn_time2(nseconds*multiplier);
	}

	gridDyn_time2 operator*(double multiplier) const
	{
		return gridDyn_time2(static_cast<double>(nseconds)*multiplier);
	}

	gridDyn_time2 operator/(int divisor) const
	{
		return gridDyn_time2(nseconds/divisor);
	}

	gridDyn_time2 operator/(double divisor) const
	{
		return gridDyn_time2(static_cast<double>(nseconds)/divisor);
	}

	bool operator==(const gridDyn_time2 &rhs) const
	{
		return (nseconds == rhs.nseconds);
	}

	bool operator!=(const gridDyn_time2 &rhs) const
	{
		return (nseconds != rhs.nseconds);
	}

	bool operator>(const gridDyn_time2 &rhs) const
	{
		return (nseconds > rhs.nseconds);
	}

	bool operator<(const gridDyn_time2 &rhs) const
	{
		return (nseconds < rhs.nseconds);
	}

	bool operator>=(const gridDyn_time2 &rhs) const
	{
		return (nseconds >= rhs.nseconds);
	}

	bool operator<=(const gridDyn_time2 &rhs) const
	{
		return (nseconds <= rhs.nseconds);
	}


};


inline double operator/(double x, gridDyn_time2 t)
{
	return x / static_cast<double>(t);
}

inline gridDyn_time2 operator*(double x, gridDyn_time2 t)
{
	return t*x;
}


#endif
