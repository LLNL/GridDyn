#pragma once
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

#ifndef GRIDDYN_TIME_H_
#define GRIDDYN_TIME_H_

#include <cmath>

#define MAXLL 0x7FFF'FFFF'FFFF'FFFF

/** prototype class for representing time
@details implements time as a count of 1/2^N seconds  
this is done for performance because many mathematical operations are needed on the time and this way
it could be implemented using shift and masks for some conversions to floating point operations
*/
template <int N>
class integer_time
{
private:
	static const long long int scalar = (1 << N);
	static const long long int fracMask = ((1 << N) - 1);
public:
	typedef typename long long int baseType;
	static long long int convert(double t)
	{
		double intpart;
		double frac = std::modf(t, &intpart);
		long long int nseconds = (static_cast<long long int>(intpart) << N) + static_cast<long long int>(ldexp(frac, N));
		return nseconds;
	}

	static double toDouble(long long int val)
	{
		return (static_cast<double>(val >> N) + std::ldexp(static_cast<double>(fracMask & val), -N));
	}
	static long long int max()
	{
		return MAXLL;
	}
	static long long int min()
	{
		return -(MAXLL - 1);
	}
};


const long long int fac10[16]{ 1,10,100,1000,10'000,100'000,
1'000'000,10'000'000,100'000'000,1'000'000'000,10'000'000'000,100'000'000'000,
1'000'000'000'000,10'000'000'000'000,100'000'000'000'000,1'000'000'000'000'000 };

const double fac10f[16]{ 1.0,10.0,100.0,1000.0,10'000.0,100'000.0,
1'000'000.0,10'000'000.0,100'000'000.0,1'000'000'000.0,10'000'000'000.0,100'000'000'000.0,
1'000'000'000'000.0,10'000'000'000'000.0,100'000'000'000'000.0,1'000'000'000'000'000.0 };

template <int N>
class count_time
{
public:
	typedef typename long long int baseType;

	static long long int convert(double t)
	{
		return (static_cast<long long int>(t*fac10f[N]));
	}

	static double toDouble(long long int val)
	{
		return (static_cast<double>(val / fac10[N]) + static_cast<double>(val%fac10[N]) / fac10f[N]);
	}
	static long long int max()
	{
		return MAXLL;
	}
	static long long int min()
	{
		return -(MAXLL - 1);
	}
};


class double_time
{
public:
	typedef typename double baseType;
	static double convert(double t)
	{
		return t;
	}

	static double toDouble(double val)
	{
		return val;
	}
	static double max()
	{
		return (1e48);
	}
	static double min()
	{
		return -(1e48);
	}
};

/** prototype class for representing time
@details implements time as a count of 1/2^30 seconds  roughly corresponding to 1 ns (actually about 0.93 ns)
this is done for performance because many mathematical operations are needed on the time and this way
it could be implemented using shift and masks for some conversions to floating point operations
*/
template<class Tconv>
class timeRepresentation
{

public:
	typedef typename Tconv::baseType baseType;
private:
	
	baseType timecode;
#ifdef _DEBUG
	//this is a debugging aid to display the time as a double when looking at debug output
	//it isn't involved in any calculations and is removed when not in debug mode
	double dtime;
#define DOUBLETIME dtime=static_cast<double>(*this);
#else
#define DOUBLETIME
#endif 
public:
	//implicit conversion requested
	timeRepresentation() {};
	timeRepresentation(double t)
	{
		timecode = Tconv::convert(t);
		DOUBLETIME
	}

	timeRepresentation(const timeRepresentation &x) :timecode(x.timecode)
	{
		DOUBLETIME
	}

	static timeRepresentation max()
	{
		return timeRepresentation(Tconv::max());
	}
	static timeRepresentation min()
	{
		return timeRepresentation(Tconv::min());
	}

	timeRepresentation& operator= (const timeRepresentation &x)
	{
		timecode = x.timecode;
		DOUBLETIME
			return *this;
	}

	timeRepresentation& operator= (double t)
	{
		*this = timeRepresentation(t);
		DOUBLETIME
			return *this;
	}

	operator double() const
	{
		return Tconv::toDouble(timecode);
	}

	timeRepresentation &operator+=(const timeRepresentation &rhs) {
		timecode += rhs.timecode;
		DOUBLETIME
			return *this;
	}

	timeRepresentation &operator-=(const timeRepresentation &rhs) {
		timecode -= rhs.timecode;
		DOUBLETIME
			return *this;
	}

	timeRepresentation &operator*=(int multiplier) {
		timecode *= multiplier;
		DOUBLETIME
			return *this;
	}
	timeRepresentation &operator*=(double multiplier) {
		timeRepresentation nt(Tconv::toDouble(timecode)*multiplier);
		timecode = nt.timecode;
		DOUBLETIME
			return *this;
	}

	timeRepresentation &operator/=(int divisor) {
		timecode /= divisor;
		DOUBLETIME
			return *this;
	}

	timeRepresentation &operator/=(double divisor) {
		timeRepresentation nt(Tconv::toDouble(timecode) / divisor);
		timecode = nt.timecode;
		DOUBLETIME
			return *this;
	}

	timeRepresentation operator+(const timeRepresentation &other) const
	{
		timeRepresentation trep;
		trep.timecode=timecode + other.timecode;
		return trep;
	}

	timeRepresentation operator-(const timeRepresentation &other) const
	{
		timeRepresentation trep;
		trep.timecode = timecode - other.timecode;
		return trep;
	}



	timeRepresentation operator*(int multiplier) const
	{
		timeRepresentation trep;
		trep.timecode = timecode*multiplier;
		return trep;
	}

	timeRepresentation operator*(double multiplier) const
	{
		return timeRepresentation(Tconv::toDouble(timecode)*multiplier);
	}


	timeRepresentation operator/(int divisor) const
	{
		timeRepresentation trep;
		trep.timecode = timecode / divisor;
		return trep;
	}

	timeRepresentation operator/(double divisor) const
	{
		return timeRepresentation(Tconv::toDouble(timecode) / divisor);
	}

	bool operator==(const timeRepresentation &rhs) const
	{
		return (timecode == rhs.timecode);
	}

	bool operator!=(const timeRepresentation &rhs) const
	{
		return (timecode != rhs.timecode);
	}

	bool operator>(const timeRepresentation &rhs) const
	{
		return (timecode > rhs.timecode);
	}

	bool operator<(const timeRepresentation &rhs) const
	{
		return (timecode < rhs.timecode);
	}

	bool operator>=(const timeRepresentation &rhs) const
	{
		return (timecode >= rhs.timecode);
	}

	bool operator<=(const timeRepresentation &rhs) const
	{
		return (timecode <= rhs.timecode);
	}


};


#endif