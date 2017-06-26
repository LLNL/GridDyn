#pragma once/*
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

#ifndef STRVIEWCONVERSION_H_
#define STRVIEWCONVERSION_H_


#include "string_viewOps.h"
#include "charMapper.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4127 4459)
#endif
#include <boost/spirit/home/x3.hpp>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <stdexcept>
extern const utilities::charMapper<bool> numCheck;
extern const utilities::charMapper<bool> numCheckEnd;

using utilities::string_view;

template< typename X>
X strViewToInteger(string_view input, size_t *rem = nullptr)
{
	static_assert(std::is_integral<X>::value,"requested type is not integral");
	X ret = 0;
	bool inProcess = false;
	int sign = 1;
	if (rem)
	{
		*rem = input.length();
	}
	auto v1 = input.cbegin();
	auto vend = input.cend();
	while ((!inProcess)&&(v1!=vend))
	{
		switch (*v1)
		{
		case'1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			ret = (*v1 - '0');
			inProcess = true;
			break;
		case '-':
			sign = -1;
			inProcess = true;
			break;
		case '+':case '0':
			inProcess = true;
			break;
		case ' ': case '\t': case '\r': case '\0': case '\n':
			break;
		default:
			throw(std::invalid_argument("unable to convert string"));
		}
		++v1;
	}
	if (!inProcess)
	{
		throw(std::invalid_argument("unable to convert string"));
	}

	while (v1 != vend)
	{
		if (isdigit(*v1))
		{
			ret *= 10;
			ret += (*v1 - '0');
		}
		else
		{
			if (rem)
			{
				*rem = (v1 - input.cbegin());
			}
			break;
		}
		++v1;
	}
	
	return ret*sign;
}

//templates for single numerical conversion
template <typename X>
inline X numConv(string_view V)
{
	return (std::is_integral<X>::value) ? strViewToInteger<X>(V) : X(numConv<double>(V));
}

//template definition for double conversion
template <>
inline double numConv(string_view V)
{
	namespace x3 = boost::spirit::x3;
	double retVal=-1e49;
	x3::parse(V.cbegin(), V.cend(), x3::double_, retVal);
	return retVal;
}

template <>
inline float numConv(utilities::string_view V)
{
	namespace x3 = boost::spirit::x3;
	float retVal= - 1e25f;
	x3::parse(V.cbegin(), V.cend(), x3::float_, retVal);
	return retVal;
}

//template definition for long double conversion
template <>
inline long double numConv(string_view V)
{
	return std::stold(V.to_string());
}

//template for numeric conversion returning the position
template <class X>
inline X numConvComp(string_view V, size_t &rem)
{
	return (std::is_integral<X>::value) ? strViewToInteger<X>(V,&rem) : X(numConvComp<double>(V,rem));
}

template <>
inline double numConvComp(string_view V, size_t &rem)
{
	return std::stod(V.to_string(), &rem);
}

template <>
inline long double numConvComp(string_view V, size_t &rem)
{
	return std::stold(V.to_string(), &rem);
}



/** check if the first character of the string is a valid numerical value*/
inline bool nonNumericFirstCharacter(string_view V)
{
	return ((V.empty()) || (numCheck[V[0]] == false));
}


/** check if the first character of the string is a valid numerical value*/
inline bool nonNumericFirstOrLastCharacter(string_view V)
{
	return ((V.empty()) || (numCheck[V[0]] == false) || (numCheckEnd[V.back()] == false));
}

template<typename X>
X numeric_conversion(string_view V, const X defValue)
{
	if (nonNumericFirstCharacter(V))
	{
		return defValue;
	}
	try
	{
		return numConv<X>(V);
	}
	catch (std::invalid_argument)
	{
		return defValue;
	}
}

/** do a numeric conversion of the complete string
*/
template<typename X>
X  numeric_conversionComplete(string_view V, const X defValue)
{
	if (nonNumericFirstCharacter(V))
	{
		return defValue;
	}
	try
	{
		size_t rem;
		X res = numConvComp<X>(V, rem);
		while (rem < V.length())
		{
			if (!(isspace(V[rem])))
			{
				res = defValue;
				break;
			}
			++rem;
		}
		return res;

	}
	catch (std::invalid_argument)
	{
		return defValue;
	}
}

/** @brief  convert a string into a vector of double precision numbers
@param[in] line the string to convert
@param[in] defValue  the default numerical return value if conversion fails
@param[in] delimiters  the delimiters to use to separate the numbers
@return a vector of double precision numbers converted from the string
*/
template<typename X>
std::vector<X> str2vector(string_view line, const X defValue, string_view delimiters = utilities::string_viewOps::default_delim_chars)
{
	auto tempVec = utilities::string_viewOps::split(line, delimiters);
	std::vector<X> av;
	av.reserve(tempVec.size());
	for (const auto &str : tempVec)
	{
		av.push_back(numeric_conversion<X>(str, defValue));
	}
	return av;
}

/** @brief  convert a vector of strViews into doubles
@param[in] tokens the vector of strViews to convert
@param[in] defValue  the default numerical return value if conversion fails
@return a vector of double precision numbers converted from the string
*/
template <typename X>
std::vector<X> str2vector(const utilities::string_viewVector &tokens, const X defValue)
{
	std::vector<X> av;
	av.reserve(tokens.size());
	for (const auto &str : tokens)
	{
		av.push_back(numeric_conversion<X>(str, defValue));
	}
	return av;
}


#endif

