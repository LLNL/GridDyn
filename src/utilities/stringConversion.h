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

#ifndef STRINGCONVERSION_H_
#define STRINGCONVERSION_H_


#include "charMapper.h"
#include "stringOps.h"
#include <type_traits>

extern const utilities::charMapper<bool> numCheck;
extern const utilities::charMapper<bool> numCheckEnd;

// templates for single numerical conversion
template <typename X>
inline X numConv (const std::string &V)
{
    return (std::is_integral<X>::value) ? X (numConv<long long> (V)) : X (numConv<double> (V));
}

// template definition for double conversion
template <>
inline double numConv (const std::string &V)
{
    return std::stod (V);
}

// template definition for long double conversion
template <>
inline long double numConv (const std::string &V)
{
    return std::stold (V);
}

// template definition for integer conversion
template <>
inline int numConv (const std::string &V)
{
    return std::stoi (V);
}

// template definition for unsigned long conversion
template <>
inline unsigned long numConv (const std::string &V)
{
    return std::stoul (V);
}
// template definition for unsigned long long conversion
template <>
inline unsigned long long numConv (const std::string &V)
{
    return std::stoull (V);
}

// template definition for long long conversion
template <>
inline long long numConv (const std::string &V)
{
    return std::stoll (V);
}

// template for numeric conversion returning the position
template <class X>
inline X numConvComp (const std::string &V, size_t &rem)
{
    return (std::is_integral<X>::value) ? X (numConvComp<long long> (V, rem)) :
                                          X (numConvComp<double> (V, rem));
}

template <>
inline float numConvComp (const std::string &V, size_t &rem)
{
    return std::stof (V, &rem);
}

template <>
inline double numConvComp (const std::string &V, size_t &rem)
{
    return std::stod (V, &rem);
}

template <>
inline long double numConvComp (const std::string &V, size_t &rem)
{
    return std::stold (V, &rem);
}


template <>
inline int numConvComp (const std::string &V, size_t &rem)
{
    return std::stoi (V, &rem);
}

template <>
inline unsigned long numConvComp (const std::string &V, size_t &rem)
{
    return std::stoul (V, &rem);
}

template <>
inline unsigned long long numConvComp (const std::string &V, size_t &rem)
{
    return std::stoull (V, &rem);
}

template <>
inline long long numConvComp (const std::string &V, size_t &rem)
{
    return std::stoll (V, &rem);
}

/** check if the first character of the string is a valid numerical value*/
inline bool nonNumericFirstCharacter (const std::string &V)
{
    return ((V.empty ()) || (numCheck[V[0]] == false));
}


/** check if the first character of the string is a valid numerical value*/
inline bool nonNumericFirstOrLastCharacter (const std::string &V)
{
    return ((V.empty ()) || (numCheck[V[0]] == false) || (numCheckEnd[V.back ()] == false));
}

template <typename X>
X numeric_conversion (const std::string &V, const X defValue)
{
    if (nonNumericFirstCharacter (V))
    {
        return defValue;
    }
    try
    {
        return numConv<X> (V);
    }
    catch (std::invalid_argument)
    {
        return defValue;
    }
}

/** do a numeric conversion of the complete string
 */
template <typename X>
X numeric_conversionComplete (const std::string &V, const X defValue)
{
    if (nonNumericFirstOrLastCharacter (V))
    {
        return defValue;
    }
    try
    {
        size_t rem;
        X res = numConvComp<X> (V, rem);
        while (rem < V.length ())
        {
            if (!(isspace (V[rem])))
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
template <typename X>
std::vector<X> str2vector (const std::string &line, const X defValue, const std::string &delimiters = ",;")
{
    auto tempVec = stringOps::splitline (line, delimiters);
    std::vector<X> av;
    av.reserve (tempVec.size ());
    for (const auto &str : tempVec)
    {
        av.push_back (numeric_conversion<X> (str, defValue));
    }
    return av;
}

/** @brief  convert a vector of strings into doubles
@param[in] tokens the vector of string to convert
@param[in] defValue  the default numerical return value if conversion fails
@return a vector of double precision numbers converted from the string
*/
template <typename X>
std::vector<X> str2vector (const stringVector &tokens, const X defValue)
{
    std::vector<X> av;
    av.reserve (tokens.size ());
    for (const auto &str : tokens)
    {
        av.push_back (numeric_conversion<X> (str, defValue));
    }
    return av;
}


#endif
