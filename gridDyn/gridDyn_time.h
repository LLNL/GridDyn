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
#include <type_traits>

#define MAXLL 0x7FFF'FFFF'FFFF'FFFF

/** prototype class for representing time
@details implements time as a count of 1/2^N seconds  
this is done for performance because many mathematical operations are needed on the time and this way
it could be implemented using shift and masks for some conversions to floating point operations
*/
template <int N, typename base=long long int>
class integer_time
{
private:
	static const base scalar = (1 << N);
	static const base fracMask = ((1 << N) - 1);
public:
	typedef base baseType;
	static baseType maxVal()
	{
		return MAXLL;
	}
	static baseType minVal()
	{
		return -(MAXLL - 1);
	}

	static baseType convert(double t)
	{
		if (t < -1e12)
		{
			return minVal();
		}
		double intpart;
		double frac = std::modf(t, &intpart);
		baseType nseconds = (static_cast<base>(intpart) << N) + static_cast<base>(ldexp(frac, N));
		return nseconds;
	}

	static double toDouble(baseType val)
	{
		return (static_cast<double>(val >> N) + std::ldexp(static_cast<double>(fracMask & val), -N));
	}
	
	static long long int seconds(baseType val)
	{
		return static_cast<long long int>(val >> N);
	}
};


const long long int fac10[16]{ 1,10,100,1000,10'000,100'000,
1'000'000,10'000'000,100'000'000,1'000'000'000,10'000'000'000,100'000'000'000,
1'000'000'000'000,10'000'000'000'000,100'000'000'000'000,1'000'000'000'000'000 };

const double fac10f[16]{ 1.0,10.0,100.0,1000.0,10'000.0,100'000.0,
1'000'000.0,10'000'000.0,100'000'000.0,1'000'000'000.0,10'000'000'000.0,100'000'000'000.0,
1'000'000'000'000.0,10'000'000'000'000.0,100'000'000'000'000.0,1'000'000'000'000'000.0 };

template <int N, typename base = long long int>
class count_time
{
public:
	typedef base baseType;
	static baseType maxVal()
	{
		return MAXLL;
	}
	static baseType minVal()
	{
		return -(MAXLL - 1);
	}
	static baseType convert(double t)
	{
		if (t < -1e12)
		{
			return minVal();
		}
		return (static_cast<baseType>(t*fac10f[N]));
	}

	static double toDouble(baseType val)
	{
		return (static_cast<double>(val / fac10[N]) + static_cast<double>(val%fac10[N]) / fac10f[N]);
	}
	static long long int seconds(baseType val)
	{
		return static_cast<long long int>(val / fac10[N]);
	}
};

template<typename base=double>
class double_time
{
public:
	typedef base baseType;
	static baseType convert(double t)
	{
		return t;
	}

	static double toDouble(baseType val)
	{
		return val;
	}
	static baseType maxVal()
	{
		return (1e49);
	}
	static baseType minVal()
	{
		return (-1.456e47);
	}
	static long long int seconds(baseType val)
	{
		return static_cast<long long int>(val);
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
	timeRepresentation(double t):timecode(Tconv::convert(t))
	{
		DOUBLETIME
	}

	timeRepresentation(const timeRepresentation &x) :timecode(x.timecode)
	{
		DOUBLETIME
	}

	static timeRepresentation maxVal()
	{
		timeRepresentation tret;
		tret.timecode = Tconv::maxVal();
		return tret;
	}
	static timeRepresentation minVal()
	{
		timeRepresentation tret;
		tret.timecode = Tconv::minVal();
		return tret;
	}

	long long int seconds() const
	{
		return Tconv::seconds(timecode);
	}

	timeRepresentation& operator= (const timeRepresentation &x)
	{
		timecode = x.timecode;
		DOUBLETIME
			return *this;
	}

	timeRepresentation& operator= (double t)
	{   
		timecode = Tconv::convert(t);
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

	timeRepresentation operator%(const timeRepresentation &other) const
	{
		timeRepresentation trep;
		if (std::is_integral<baseType>::value)
		{
			trep.timecode = timecode%other.timecode;
		}
		else
		{
			trep.timecode = Tconv::convert(std::fmod(Tconv::toDouble(timecode), Tconv::toDouble(other.timecode)));
		}
		return trep;
	}

	timeRepresentation &operator%=(const timeRepresentation &other)
	{
		
		if (std::is_integral<baseType>::value)
		{
			timecode = timecode%other.timecode;
		}
		else
		{
			timecode = Tconv::convert(std::fmod(Tconv::toDouble(timecode), Tconv::toDouble(other.timecode)));
		}
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