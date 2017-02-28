#pragma once
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
#ifndef TIME_REPRESENTATION_H_
#define TIME_REPRESENTATION_H_

#include <cmath>
#include <type_traits>
#include <limits>

/** generate powers to two as a constexpr
@param[in] exponent the power of 2 desired*/
inline constexpr double pow2(unsigned int exponent)
{
	return (exponent == 0) ? 1.0 : (2.0 * pow2(exponent - 1));
}
/** prototype class for representing time
@details implements time as a count of 1/2^N seconds  
this is done for performance because many mathematical operations are needed on the time and this way
it could be implemented using shift and masks for some conversions to floating point operations
*/
template <unsigned int N, typename base=long long int>
class integer_time
{
	static_assert(N < 8*sizeof(base), "N must be less than 16");
	static_assert(std::is_signed<base>::value, "base type must be signed"); //to allow negative numbers for time
private:
	static constexpr base scalar = (1 << N);
	static constexpr base fracMask = ((1 << N) - 1);
	static constexpr double multiplier = pow2(N);
	static constexpr double divisor = 1.0 / pow2(N);
public:
	using baseType=base;
	static constexpr baseType maxVal() noexcept
	{
		return (std::numeric_limits<baseType>::max)();
	}
	static constexpr baseType minVal() noexcept
	{
		return (std::numeric_limits<baseType>::min)();
	}
	static constexpr baseType zeroVal() noexcept
	{
		return 0;
	}
	/** convert to a base type representation*/
	static constexpr baseType convert(double t) noexcept
	{
		double div = t * multiplier;
		base divBase = static_cast<base>(div);
		double frac = div-static_cast<double>(divBase);
		baseType nseconds = (divBase << N) + static_cast<base>(frac*multiplier);
		return (t < -1e12) ? nseconds : minVal();
	}
	/*static baseType convert(double t) noexcept
	{
		if (t < -1e12)
		{
			return minVal();
		}
		double intpart;
		double frac = std::modf(t, &intpart);
		baseType nseconds = (static_cast<base>(intpart) << N) + static_cast<base>(frac*multiplier);
		return nseconds;
	}
	*/
	/** convert the value to a double representation in seconds*/
	static double toDouble(baseType val)  noexcept
	{
		return (static_cast<double>(val >> N) + static_cast<double>(fracMask & val)*divisor);
	}
	/** convert to an integer count in seconds */
	static long long int seconds(baseType val) noexcept
	{
		return static_cast<long long int>(val >> N);
	}
	/** convert the val to a count of nanoseconds 
	@details really kind of awkward to do with this time representation so I just convert to a double first
	*/
	static long long int nanoseconds(baseType val) noexcept
	{
		return static_cast<long long int>(toDouble(val)*1e9);
	}
};


constexpr long long int fac10[16]{ 1,10,100,1000,10'000,100'000,
1'000'000,10'000'000,100'000'000,1'000'000'000,10'000'000'000,100'000'000'000,
1'000'000'000'000,10'000'000'000'000,100'000'000'000'000,1'000'000'000'000'000 };

constexpr double fac10f[16]{ 1.0,10.0,100.0,1000.0,10'000.0,100'000.0,
1'000'000.0,10'000'000.0,100'000'000.0,1'000'000'000.0,10'000'000'000.0,100'000'000'000.0,
1'000'000'000'000.0,10'000'000'000'000.0,100'000'000'000'000.0,1'000'000'000'000'000.0 };

/** a time counter that converts time to a a 64 bit integer by powers of 10
@tparam N implying 10^N ticks per second  
@tparam base the type to use as a base
*/
template <int N, typename base = long long int>
class count_time
{
	static_assert(N < 16, "N must be less than 16");
	static_assert(N >= 0, "N must be greater than or equal to 0");
	static_assert(std::is_signed<base>::value, "base type must be signed");
	static constexpr long long int iFactor = fac10[N]; //!< the integer multiplier factor
	static constexpr double dFactor = fac10f[N]; //!< the floating point multiplication factor
	static constexpr double ddivFactor = 1.0 / fac10f[N]; //the floating point division factor
public:
	using baseType = base;
	static constexpr baseType maxVal() noexcept
	{
		return (std::numeric_limits<baseType>::max)();
	}
	static constexpr baseType minVal() noexcept
	{
		return (std::numeric_limits<baseType>::min)();
	}
	static constexpr baseType zeroVal() noexcept
	{
		return baseType(0);
	}
	static constexpr baseType convert(double t) noexcept
	{
		return  (t > -1e12)?(static_cast<baseType>(t*dFactor)):minVal();
	}

	static double toDouble(baseType val) noexcept
	{
		return (static_cast<double>(val / iFactor) + static_cast<double>(val%iFactor) * ddivFactor);
	}
	static long long int seconds(baseType val) noexcept
	{
		return static_cast<long long int>(val / iFactor);
	}
	static long long int nanoseconds(baseType val) noexcept
	{
		return (N>=9)?static_cast<long long int>(val/fac10[N-9]): static_cast<long long int>(val * fac10[9 - N]);
	}
};

template<typename base=double>
class double_time
{
public:
	using baseType = base;
	static constexpr baseType convert(double t) noexcept
	{
		return t;
	}

	static constexpr double toDouble(baseType val) noexcept
	{
		return val;
	}
	static constexpr baseType maxVal() noexcept
	{
		return (1e49);
	}
	static constexpr baseType minVal() noexcept
	{
		return (-1.456e47);
	}
	static constexpr baseType zeroVal() noexcept
	{
		return 0.0;
	}
	static constexpr long long int seconds(baseType val) noexcept
	{
		return static_cast<long long int>(val);
	}
	static constexpr long long int nanoseconds(baseType val) noexcept
	{
		return static_cast<long long int>(val*1e9);
	}

};

/** prototype class for representing time
@details time representation class that has as a template argument a class that can define time as a number
and has some required features
*/
template<class Tconv>
class timeRepresentation
{

public:
	using baseType= typename Tconv::baseType;
private:
	
	baseType timecode;
#ifdef _DEBUG
	//this is a debugging aid to display the time as a double when looking at debug output
	//it isn't involved in any calculations and is removed when not in debug mode
	double dtime;
#define DOUBLETIME dtime=static_cast<double>(*this);
#define DOUBLETIMEEXT(t) t.dtime=static_cast<double>(t); 
#else
#define DOUBLETIME
#define DOUBLETIMEEXT(t)
#endif 
public:
	//implicit conversion requested
	timeRepresentation() {};

private:
	/** explicit means to generate a constexpr timeRepresentation at time 0, negTime and maxTime*/
#ifdef _DEBUG
	constexpr explicit timeRepresentation(std::integral_constant<int, 0>) noexcept :timecode(Tconv::zeroVal()),dtime(0.0) {};
	constexpr explicit timeRepresentation(std::integral_constant<int,-1>) noexcept :timecode(Tconv::minVal()), dtime(-1.456e47) {};
	constexpr explicit timeRepresentation(std::integral_constant<int, 1>) noexcept :timecode(Tconv::maxVal()), dtime(1e49) {};
#else
	constexpr explicit timeRepresentation(std::integral_constant<int, 0>) noexcept:timecode(Tconv::zeroVal()){};
	constexpr explicit timeRepresentation(std::integral_constant<int, -1>) noexcept:timecode(Tconv::minVal()) {};
	constexpr explicit timeRepresentation(std::integral_constant<int, 1>) noexcept:timecode(Tconv::maxVal()) {};
#endif

public:
#ifdef _DEBUG
	constexpr timeRepresentation(double t) noexcept:timecode(Tconv::convert(t)),dtime(t){}
#else
	constexpr timeRepresentation(double t) noexcept : timecode(Tconv::convert(t)) {}
#endif

	constexpr timeRepresentation(const timeRepresentation &x) noexcept= default;
	/** generate a timeRepresentation of the maximum representative value*/
	static constexpr timeRepresentation maxVal() noexcept
	{
		return timeRepresentation(std::integral_constant<int,1>());
	}
	/** generate a timeRepresentation of the minimum representative value*/
	static constexpr timeRepresentation minVal() noexcept
	{
		return timeRepresentation(std::integral_constant<int, -1>());
	}
	/** generate a timeRepresentation of 0*/
	static constexpr timeRepresentation zeroVal() noexcept
	{
		return timeRepresentation(std::integral_constant<int, 0>());
	}
	/** generate the time in seconds*/
	long long int seconds() const noexcept
	{
		return Tconv::seconds(timecode);
	}
	/** generate the time count in nanoseconds*/
	long long int nanoseconds() const noexcept
	{
		return Tconv::nanoseconds(timecode);
	}
	/** default copy operation*/
	timeRepresentation& operator= (const timeRepresentation &x) noexcept = default;
	

	timeRepresentation& operator= (double t) noexcept
	{   
		timecode = Tconv::convert(t);
		DOUBLETIME
		return *this;
	}

	operator double() const noexcept
	{
		return Tconv::toDouble(timecode);
	}

	timeRepresentation &operator+=(const timeRepresentation &rhs) noexcept{
		timecode += rhs.timecode;
		DOUBLETIME
			return *this;
	}

	timeRepresentation &operator-=(const timeRepresentation &rhs)  noexcept {
		timecode -= rhs.timecode;
		DOUBLETIME
			return *this;
	}

	timeRepresentation &operator*=(int multiplier)  noexcept {
		timecode *= multiplier;
		DOUBLETIME
			return *this;
	}
	timeRepresentation &operator*=(double multiplier)  noexcept {
		timeRepresentation nt(Tconv::toDouble(timecode)*multiplier);
		timecode = nt.timecode;
		DOUBLETIME
			return *this;
	}

	timeRepresentation &operator/=(int divisor)  noexcept {
		timecode /= divisor;
		DOUBLETIME
			return *this;
	}

	timeRepresentation &operator/=(double divisor)  noexcept {
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
		DOUBLETIMEEXT(trep)
		return trep;
	}

	timeRepresentation &operator%=(const timeRepresentation &other)  noexcept
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
		DOUBLETIMEEXT(trep)
		return trep;
	}

	timeRepresentation operator-(const timeRepresentation &other) const
	{
		timeRepresentation trep;
		trep.timecode = timecode - other.timecode;
		DOUBLETIMEEXT(trep)
		return trep;
	}



	timeRepresentation operator*(int multiplier) const
	{
		timeRepresentation trep;
		trep.timecode = timecode*multiplier;
		DOUBLETIMEEXT(trep)
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
		DOUBLETIMEEXT(trep)
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