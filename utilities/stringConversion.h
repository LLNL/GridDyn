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

#ifndef STRINGCONVERSION_H_
#define STRINGCONVERSION_H_


#include "stringOps.h"

const charMapper<bool> numCheck("numericstart");

//templates for single numerical conversion
template <typename X>
inline X numConv(const std::string &V)
{
	return X(std::stod(V));
}

template <>
inline double numConv(const std::string &V)
{
	return std::stod(V);
}

template <>
inline long double numConv(const std::string &V)
{
	return std::stold(V);
}

template <>
inline int numConv(const std::string &V)
{
	return std::stoi(V);
}
template <>
inline unsigned long numConv(const std::string &V)
{
	return std::stoul(V);
}
template <>
inline unsigned long long numConv(const std::string &V)
{
	return std::stoull(V);
}

template <>
inline long long numConv(const std::string &V)
{
	return std::stoll(V);
}

//template for numeric conversion returning the position
template <class X>
inline X numConvComp(const std::string &V,size_t &rem)
{
	return X(std::stod(V, &rem));
}

template <>
inline double numConvComp(const std::string &V, size_t &rem)
{
	return std::stod(V, &rem);
}

template <>
inline long double numConvComp(const std::string &V, size_t &rem)
{
	return std::stold(V, &rem);
}


template <>
inline int numConvComp(const std::string &V, size_t &rem)
{
	return std::stoi(V,&rem);
}

template <>
inline unsigned long numConvComp(const std::string &V, size_t &rem)
{
	return std::stoul(V,&rem);
}

template <>
inline unsigned long long numConvComp(const std::string &V, size_t &rem)
{
	return std::stoull(V,&rem);
}

template <>
inline long long numConvComp(const std::string &V, size_t &rem)
{
	return std::stoll(V, &rem);
}

template<typename X>
X numeric_conversion(const std::string &V, const X defValue)
{
	if ((V.empty()) || (numCheck[V[0]] == false))
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

template<typename X>
X  numeric_conversionComplete(const std::string &V, const X defValue)
{
	if ((V.empty()) || (numCheck[V[0]] == false))
	{
		return defValue;
	}
	try
	{
		size_t rem;
		X res = numConvComp<X>(V, &rem);
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
std::vector<X> str2vector(const std::string &line, const X defValue, const std::string &delimiters = ",;")
{
	auto tempVec = splitline(line, delimiters);
	std::vector<X> av;
	av.reserve(tempVec.size());
	for (const auto &str : tempVec)
	{
		av.push_back(numeric_conversion<X>(str, defValue));
	}
	return av;
}

/** @brief  convert a vector of strings into doubles
@param[in] tokens the vector of string to convert
@param[in] defValue  the default numerical return value if conversion fails
@return a vector of double precision numbers converted from the string
*/
template <typename X>
std::vector<X> str2vector(const stringVector &tokens, const X defValue)
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
