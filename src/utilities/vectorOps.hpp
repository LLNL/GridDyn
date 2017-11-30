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

 /** @file
 *  @brief define template operations on vectors
 */
#ifndef VECTOR_OPS_H
#define VECTOR_OPS_H
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <numeric>
#include <type_traits>
#include <vector>
/** solve a 2x2 matrix problem 
@details solve a 2 variable set of equations Vx=y solve for x
v11*x1+v12*x2=y1
v21*x1+v22*x2=y2
@param[in] v11
@param[in] v12
@param[in] v21
@param[in] v22
@param[in] y1  the result of the first equation
@param[in] y2 the result of the second equation
@param[out] x1 the computed first element
@param[out] x2 the computed second result
@return the value of the determinant
*/
double solve2x2 (double v11, double v12, double v21, double v22, double y1, double y2, double &x1, double &x2);

/** solve Ax=b where A is a [3x3] matrix
@param[in] input a 3x3 array
@param[in] vals b in the equation Ax=b
@return a 3x1 array of the results*/
std::array<double, 3>
solve3x3 (const std::array<std::array<double, 3>, 3> &input, const std::array<double, 3> &vals);

/** perform a linear interpolation 
@param[in] timeIn a vector of time values
@param[in] valIn the values of the known vector
@param[in] timeOut  the desired times in the output vector
@return the computed values corresponding to timeOut
*/
std::vector<double> interpolateLinear (const std::vector<double> &timeIn,
                                       const std::vector<double> &valIn,
                                       const std::vector<double> &timeOut);

#ifdef _MSC_VER
#if _MSC_VER < 1900
#define KEY_QUAL inline const
#endif
#endif
#ifndef KEY_QUAL
#define KEY_QUAL constexpr
#endif
/** force a value to be between two limits
@details if val is between the two limits it returns val if it is not it returns the appropriate limit
requires that the < operator be defined on the type
@tparam valType the value type to compare must define a < operator
@param[in] val the value to operate on
@param[in] LowerLim the lower limit of the valid range
@param[in] UpperLim the upper limit of the valid range
@return the clamped value */
template <class valType>
KEY_QUAL valType valLimit (valType val, valType lowerLim, valType upperLim)
{
    return (val < upperLim) ? ((val < lowerLim) ? lowerLim : val):upperLim;
}

/** force a value to be at or below an upper Limit
@details if val is below the upper limit it returns val if it is not it returns the limit
@tparam valType the value type to compare must define a < operator
@param[in] val the value to operate on
@param[in] upperLim the upper limit of the valid range
@return the clamped value */
template <class valType>
KEY_QUAL valType valUpperLimit (valType val, valType upperLim)
{
    return (val < upperLim) ? val:upperLim;
}

/** force a value to be at or above a lower Limit
@details if val is greater than the lower limit it returns val if it is not it returns the limit
@tparam valType the value type to compare must define a < operator
@param[in] val the value to operate on
@param[in] lowerLim the upper limit of the valid range
@return the clamped value */
template <class valType>
KEY_QUAL valType valLowerLimit (valType val, valType lowerLim)
{
    return (val < lowerLim) ? lowerLim : val;
}

/** get the sign of a number (-1) for numbers less than 1 for greater than 0 and 0 for == 0
@details this could be used with other types if they define a constructor which take and integer argument
and a less than operator and != operator
@tparam M the value type to compare must define a < operator and != operator
@param[in] x the value to operate on
@return the sign of the value*/
template <typename M>
KEY_QUAL M signn (M x)
{
    return ((x < M(0)) ? M(-1) : ((x != M(0)) ? M(1) : M(0)));
}

/** sum a vector
@tparam x the type of the vector
@param[in] a the vector to sum the contents
@return the sum as the same type as the vector
*/
template <class X>
X sum (const std::vector<X> &a)
{
    X sum_of_vector = std::accumulate (a.cbegin (), a.cend (), X(0));
    return sum_of_vector;
}

/** check that a vector has the requested number of elements if not resize it*/
template <class X>
void ensureSizeAtLeast (std::vector<X> &a, size_t minRequiredSize)
{
    if (a.size () < minRequiredSize)
    {
        a.resize (minRequiredSize);
    }
}

/** check that a vector has the requested number of elements if not resize it with all new elements having value = defArg*/
template <class X>
void ensureSizeAtLeast (std::vector<X> &a, size_t minRequiredSize, const X &defArg)
{
    if (a.size () < minRequiredSize)
    {
        a.resize (minRequiredSize, defArg);
    }
}

/** generate the mean of a vector sum(X)/size(X)*/
template <class X>
X mean (const std::vector<X> &a)
{
    X sum_of_vector = std::accumulate (a.begin (), a.end (), X (0));
    if (!a.empty ())
    {
        sum_of_vector /= static_cast<X> (a.size ());
    }

    return sum_of_vector;
}

/** calculate the sum of the absolute values of a vector 
@tparam X the type of the vector must define a negation operator and a < operator
*/
template <class X>
X absSum (const std::vector<X> &a)
{
    X sum_of_vector =
      std::accumulate (a.begin (), a.end (), X (0), [](X a1, X a2) -> X { return a1 + std::abs(a2); });
    return sum_of_vector;
}

/** calculate the maximum absolute value of a vector
@tparam X the type of the vector must define a negation operator and a < operator
*/
template <class X>
X absMax (const std::vector<X> &a)
{
    // auto result = std::minmax_element (a.begin (), a.end ());
    // return std::max (std::abs (*(result.first)),std::abs (*(result.second)));

    X max_of_vector = std::accumulate (a.begin (), a.end (), X (0),
		[](X a1, X a2) -> X {auto absA = std::abs(a2); return (a1 <absA) ? absA : a1; });
    return max_of_vector;
}

/** calculate the maximum absolute value of a vector and return its location
@tparam X the type of the vector must define a negation operator and a < operator
@return a pair the first element is the absMax value the second is the index into the vector
*/
template <class X>
std::pair<X, int> absMaxLoc (const std::vector<X> &a)
{
    auto result = std::minmax_element (a.begin (), a.end ());
    X a1 = std::abs (*(result.first));
    X a2 = std::abs (*(result.second));
    return (a1 > a2) ? std::make_pair (a1, result.first - a.begin ()) :
                       std::make_pair (a2, result.second - a.begin ());
}

/** calculate the maximum absolute value of a vector
@tparam X the type of the vector must define a negation operator and a < operator
*/
template <class X>
X absMin (const std::vector<X> &a)
{
    auto result = std::minmax_element (a.begin (), a.end ());
    return std::min (std::abs (*(result.first)), std::abs (*(result.second)));
}

/** calculate the minimum absolute value of a vector and return its location
@tparam X the type of the vector must define a negation operator and a < operator
@return a pair the first element is the absMax value the second is the index into the vector
*/
template <class X>
std::pair<X, int> absMinLoc (const std::vector<X> &a)
{
    auto result = std::minmax_element (a.begin (), a.end ());
    X a1 = std::abs (*(result.first));
    X a2 = std::abs (*(result.second));
    return (a1 <= a2) ? std::make_pair (a1, result.first - a.begin ()) :
                        std::make_pair (a2, result.second - a.begin ());
}

/** calculate the maximum value of a vector and return its location
@tparam X the type of the vector must define a negation operator and a < operator
@return a pair the first element is the Max value the second is the index into the vector
*/
template <class X>
std::pair<X, int> maxLoc (const std::vector<X> &a)
{
    auto result = std::max_element (a.begin (), a.end ());
    X a1 = *(result);
    return std::make_pair (a1, result - a.begin ());
}

/** calculate the minimum value of a vector and return its location
@tparam X the type of the vector must define a negation operator and a < operator
@return a pair the first element is the Max value the second is the index into the vector
*/
template <class X>
std::pair<X, int> minLoc (const std::vector<X> &a)
{
    auto result = std::min_element (a.begin (), a.end ());
    X a1 = *(result);
    return std::make_pair (a1, result - a.begin ());
}

/** calculate the maximum absolute difference between values in two vectors and return the maximum difference and the location
@tparam X the type of the vector must have std::abs defined
@return a pair the first element is the absMax value the second is the index into the vector
*/
template <class X>
auto absMaxDiffLoc (const std::vector<X> &a, const std::vector<X> &b)
{
    int loc = -1;
    
    auto cnt = (std::min) (a.size (), b.size ());
    auto abeg = a.begin ();
    auto acur = abeg;
    auto bcur = b.begin ();
    auto aend = abeg + cnt;
	auto mdiff = decltype(std::abs(*acur - *bcur))(0);
    while (acur < aend)
    {
		auto adiff = std::abs(*acur - *bcur);
        if (adiff > mdiff)
        {
            loc = acur - abeg;
            mdiff = adiff;
        }
        ++acur;
        ++bcur;
    }
    return std::make_pair (mdiff, loc);
}

/** calculate the maximum absolute difference between values in two vectors and return the maximum difference
@tparam X the type of the vector must have std::abs defined
@return the maximum absolute value of the difference between two vectors
*/
template <class X>
auto absMaxDiff (const std::vector<X> &a, const std::vector<X> &b)
{
    auto res = absMaxDiffLoc (a, b);
    return res.first;
}

/** calculate the product of all the values in a vector
@tparam X the type of the vector must have std::abs defined
@return the product of all the value in a vector
*/
template <class X>
X product (const std::vector<X> &a)
{
    X prod_of_vector = std::accumulate (a.begin (), a.end (), X (0), [](X a1, X a2) { return a1 * a2; });
    return prod_of_vector;
}

/** calculate the rms value of a vector 
@tparam X the type of the vector must have std::abs defined
@return the computed rms value
*/
template <class X>
X rms (const std::vector<X> &a)
{
    X sum_of_vector = std::accumulate (a.begin (), a.end (), 0.0, [](X a1, X a2) -> X { return (a1 + a2 * a2); });
    return std::sqrt (sum_of_vector);
}

/** compute the std deviation of a vector*/
template <class X>
X stdev (const std::vector<X> &a)
{
    X mv = mean (a);
    X sum_of_vector =
      std::accumulate (a.begin (), a.end (), 0.0, [mv](X a1, X a2) -> X { return (a1 + (a2 - mv) * (a2 - mv)); });
    X ret = X(0);
    if (!a.empty ())
    {
        ret = std::sqrt (sum_of_vector / static_cast<X> (a.size ()));
    }
    return ret;
}

/** compute the median value in a vector and partially reorders the vector according to nth_element
@return the median value*/
template <class X>
X medianReorder (std::vector<X> &a)
{
    size_t n = a.size () / 2;
    std::nth_element (a.begin (), a.begin () + n, a.end ());
    if (a.size () % 2 == 1)
    {
        return a[n];
    }
    std::nth_element (a.begin (), a.begin () + n - 1, a.end ());
    return static_cast<X> (0.5 * (a[n] + a[n - 1]));
}
/** compute the median value in a const vector
@return the median value*/
template <class X>
X median (const std::vector<X> &a)
{
    std::vector<X> b(a);  // copy the vector
    return medianReorder (b);
}

/** compute the ordered difference between elements in a vector
@param[in] a the input vector
@return a new vector whose elements contain the differences in adjacent elements*/
template <class X>
auto diff (const std::vector<X> &a)
{
    auto cnt = a.size ();
    std::vector<decltype(a[1]-a[0])> d (cnt);
    std::adjacent_difference (a.begin (), a.end (), d.begin ());
    return d;
}

/** generate a vector of indices where the values of a vector are equal to a given value
@param a the vector of value to compare
@param op the operation to check a bool function which takes a value as an input
@return a vector of indices into a with the matching condition

*/
template <class X>
auto vecFindOp(const std::vector<X> &a, std::function<bool(X)> op) 
{
	auto cnt = a.size();
	std::vector<decltype (cnt)> locs;
	locs.reserve(cnt);
	for (decltype (cnt) ii = 0; ii < cnt; ++ii)
	{
		if (op(a[ii]))
		{
			locs.push_back(ii);
		}
	}
	return locs;
}

/** generate a vector of indices where the values of a vector are equal to a given value
@param a the vector of value to compare
@param match the matching value
@return a vector of indices into a with the matching condition
*/
template <class X>
auto vecFindeq (const std::vector<X> &a, X match)  //->std::vector<decltype (a.size ())>
{
    auto cnt = a.size ();
    std::vector<decltype (cnt)> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] == match)
        {
            locs.push_back (ii);
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are not equal to a given value
@param a the vector of value to compare
@param match the matching value
@return a vector of indices into a with the matching condition
*/
template <class X>
auto vecFindne (const std::vector<X> &a, X match)
{
    auto cnt = a.size ();
    std::vector<decltype (cnt)> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] != match)
        {
            locs.push_back (ii);
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are not equal to a given value within a defined range
@param a the vector of value to compare
@param match the matching value
@param start the starting index
@param end the last index to compare
@return a vector of indices into a with the matching condition
*/
template <class X>
auto vecFindne (const std::vector<X> &a, X match, size_t start, size_t end)
{
    auto cnt = a.size ();
    cnt = std::min (end, cnt);
    std::vector<decltype (cnt)> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = start; ii < cnt; ++ii)
    {
        if (a[ii] != match)
        {
            locs.push_back (ii);
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are less than a given value
@param a the vector of value to compare
@param match the comparison value
@return a vector of indices into a with the matching condition
*/
template <class X>
auto vecFindlt (const std::vector<X> &a, X val)  //->std::vector<decltype (a.size ())>
{
    auto cnt = a.size ();
    std::vector<decltype (cnt)> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] < val)
        {
            locs.push_back (ii);
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are less than or equal to a given value
@param a the vector of value to compare
@param match the comparison value
@return a vector of indices into a with the matching condition
*/
template <class X>
auto vecFindlte (const std::vector<X> &a, X val)  //->std::vector<decltype (a.size ())>
{
    auto cnt = a.size ();
    std::vector<decltype (cnt)> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] <= val)
        {
            locs.push_back (ii);
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are greater than a given value
@tparam X the type of the values
@param a the vector of value to compare
@param match the comparison value
@return a vector of indices into a with the matching condition
*/
template <class X>
auto vecFindgt (const std::vector<X> &a, X val)
{
    auto cnt = a.size ();
    std::vector<decltype (cnt)> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] > val)
        {
            locs.push_back (ii);
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are greate or equal to a given value
@tparam X the type of the values
@param a the vector of value to compare
@param match the comparison value
@return a vector of indices into a with the matching condition
*/
template <class X>
auto vecFindgte (const std::vector<X> &a, X val)
{
    auto cnt = a.size ();
    std::vector<decltype (cnt)> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] >= val)
        {
            locs.push_back (ii);
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are equal to a given value
@tparam X the type of the values
@tparam Y the type of the indices desired
@param a the vector of value to compare
@param match the comparison value
@return a vector of indices into a with the matching condition
*/
template <class X, class Y>
std::vector<Y> vecFindeq (const std::vector<X> &a, X match)
{
    auto cnt = a.size ();
    std::vector<Y> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] == match)
        {
            locs.push_back (static_cast<Y> (ii));
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are not equal to a given value
@tparam X the type of the values
@tparam Y the type of the indices desired
@param a the vector of value to compare
@param match the comparison value
@return a vector of indices into a with the matching condition
*/
template <class X, class Y>
std::vector<Y> vecFindne (const std::vector<X> &a, X match)
{
    auto cnt = a.size ();
    std::vector<Y> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] != match)
        {
            locs.push_back (static_cast<Y> (ii));
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are not equal to a given value within a defined range
@tparam X the type of the values
@tparam Y the type of the indices desired
@param a the vector of value to compare
@param match the matching value
@param start the starting index
@param end the last index to compare
@return a vector of indices into a with the matching condition
*/
template <class X, class Y>
std::vector<Y> vecFindne (const std::vector<X> &a, X match, size_t start, size_t end)
{
    auto cnt = a.size ();
    cnt = std::min (end+1, cnt);
    std::vector<Y> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = start; ii < cnt; ++ii)
    {
        if (a[ii] != match)
        {
            locs.push_back (static_cast<Y> (ii));
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are less than a given value
@tparam X the type of the values
@tparam Y the type of the indices desired
@param a the vector of value to compare
@param val the threshold value
@return a vector of indices into a with the matching condition
*/
template <class X, class Y>
std::vector<Y> vecFindlt (const std::vector<X> &a, X val)
{
    auto cnt = a.size ();
    std::vector<Y> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] < val)
        {
            locs.push_back (static_cast<Y> (ii));
        }
    }
    return locs;
}

/** generate a vector of indices where the values of a vector are less than or equal to a given value
@tparam X the type of the values
@tparam Y the type of the indices desired
@param a the vector of value to compare
@param val the threshold value
@return a vector of indices into a with the matching condition
*/
template <class X, class Y>
std::vector<Y> vecFindlte (const std::vector<X> &a, X val)
{
    auto cnt = a.size ();
    std::vector<Y> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] <= val)
        {
            locs.push_back (static_cast<Y> (ii));
        }
    }
    return locs;
}
/** generate a vector of indices where the values of a vector are greater than a given value
@tparam X the type of the values
@tparam Y the type of the indices desired
@param a the vector of value to compare
@param val the threshold value
@return a vector of indices into a with the matching condition
*/
template <class X, class Y>
std::vector<Y> vecFindgt (const std::vector<X> &a, X val)
{
    auto cnt = a.size ();
    std::vector<Y> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] > val)
        {
            locs.push_back (static_cast<Y> (ii));
        }
    }
    return locs;
}
/** generate a vector of indices where the values of a vector are greater than or equal to a given value
@tparam X the type of the values
@tparam Y the type of the indices desired
@param a the vector of value to compare
@param val the threshold value
@return a vector of indices into a with the matching condition
*/
template <class X, class Y>
std::vector<Y> vecFindgte (const std::vector<X> &a, X val)
{
    auto cnt = a.size ();
    std::vector<Y> locs;
    locs.reserve (cnt);
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (a[ii] >= val)
        {
            locs.push_back (static_cast<Y> (ii));
        }
    }
    return locs;
}

/** sum a vector elements where an indicator function matches a defined value
@param[in] a vector to sum specif elements of
@param[in] b an indicator vector
@param match if an element of b == match then add the corresponding value of a to the sum
@return the summed value where the indicators match
*/
template <class X, class Y>
X ind_sum (const std::vector<X> &a, const std::vector<Y> &b, Y match)
{
    typename std::vector<X>::size_type cnt = (std::min) (a.size (), b.size ());
    X sum_of_vector = 0;
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        if (b[ii] == match)
        {
            sum_of_vector += a[ii];
        }
    }
    return sum_of_vector;
}

/** multiply two vectors and sum the result
@param[in] a vector 1
@param[in] b vector 2
*/
template <class X>
X mult_sum (const std::vector<X> &a, const std::vector<X> &b)
{
    X sum_of_vector_mult = 0;
    typename std::vector<X>::size_type cnt = (std::min) (a.size (), b.size ());
    for (decltype (cnt) ii = 0; ii < cnt; ++ii)
    {
        sum_of_vector_mult += a[ii] * b[ii];
    }
    return sum_of_vector_mult;
}

/** add a vector from another and store the result in the original vector
@param[in,out] a vector 1
@param[in] b vector 2
*/
template <class X>
void vectorAdd (std::vector<X> &a, const std::vector<X> &b)
{
    auto cnt = (std::min) (a.size (), b.size ());
    std::transform (a.begin (), a + cnt, b.begin (), a.begin (), std::plus<X> ());
}

/** subtract a vector from another and store the result in the original vector
@param[in,out] a vector 1
@param[in] b vector 2
*/
template <class X>
void vectorSubtract (std::vector<X> &a, const std::vector<X> &b)
{
    auto cnt = (std::min) (a.size (), b.size ());
    std::transform (a.begin (), a + cnt, b.begin (), a.begin (), std::minus<X> ());
}

/** multiply a two vectors and store the result
@param[in] a vector 1
@param[in] b vector 2
@param[out] M the location to store the result
*/
template <class X>
void vectorMult (const std::vector<X> &a, const std::vector<X> &b, std::vector<X> &M)
{
    auto cnt = (std::min) (a.size (), b.size ());
	M.resize(cnt);
    std::transform (a.begin (), a + cnt, b.begin (), M.begin (), std::multiplies<X> ());
}

/** multiply a vector by a constant and add a second vector and store the result
@param[in] a vector 1
@param[in] b vector 2
@param[in] Multiplier the multiplication factor
@param[out] res the location to store the result
*/
template <class X>
void vectorMultAdd (const std::vector<X> &a, const std::vector<X> &b, const X Multiplier, std::vector<X> &res)
{
    auto cnt = (std::min) (a.size (), b.size ());
    for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
    {
        res[ii] = std::fma (Multiplier, b[ii], a[ii]);  // fast multiply add
    }
}

/** sum the absolute differences between two Vectors
@param a the first vector
@param b the secon vector
@param[out] diff a vector the absolute values of the differences
@param cnt the number of elements in the vector to compare and sum
@return the sum of the absolute values of the differences
*/
template <class X>
X compareVec (const std::vector<X> &a,
              const std::vector<X> &b,
              std::vector<X> &diff,
              typename std::vector<X>::size_type cnt=0)
{
    X sum_of_diff = 0;
	cnt = (cnt == 0) ? (a.size()) : (std::min) (a.size(), cnt);
    cnt = (std::min) (b.size (), cnt);
    diff.resize (cnt);
    for (typename std::vector<X>::size_type ii = 0; ii < 0; ++ii)
    {
        diff[ii] = std::abs (a[ii] - b[ii]);
        sum_of_diff += diff[ii];
    }
    return sum_of_diff;
}
/** sum the absolute differences between two Vectors
@param a the first vector
@param b the secon vector
@param cnt the number of elements in the vector to compare and sum
@return the sum of the absolute values of the differences
*/
template <class X>
X compareVec (const std::vector<X> &a, const std::vector<X> &b, typename std::vector<X>::size_type cnt=0)
{
    X sum_of_diff = 0;
	
    cnt = (cnt==0)?(a.size()):(std::min) (a.size (), cnt);
    cnt = (std::min) (b.size (), cnt);
    for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
    {
        sum_of_diff += std::abs (a[ii] - b[ii]);
    }
    return sum_of_diff;
}

/** count the differences between two vectors if the difference is greater than a tolerance
@param a the first vector
@param b the second vector
@param maxAllowableDiff the tolerable difference between two values
@return the number of differences*/
template <class X>
auto countDiffs (const std::vector<X> &a, const std::vector<X> &b, X maxAllowableDiff)
{
    using stype_t = typename std::vector<X>::size_type;
    stype_t cnt = (std::min) (a.size (), b.size ());
    stype_t diffs = (std::max) (a.size (), b.size ()) - cnt;
    for (stype_t ii = 0; ii < cnt; ++ii)
    {
        if (std::abs (a[ii] - b[ii]) > maxAllowableDiff)
        {
            ++diffs;
        }
    }
    return diffs;
}

/** count the differences between two vectors if the difference is greater than a tolerance, ignore a common mode difference between the two
@param a the first vector
@param b the second vector
@param maxAllowableDiff the tolerable difference between two values
@return the number of differences*/
template <class X>
auto countDiffsIgnoreCommon (const std::vector<X> &a, const std::vector<X> &b, X maxAllowableDiff)
{
    using stype_t = typename std::vector<X>::size_type;
    stype_t cnt = (std::min) (a.size (), b.size ());
    stype_t diffs = (std::max) (a.size (), b.size ()) - cnt;
    if (cnt < 3)
    {
        return stype_t(0);
    }
    X commonDiff1 = a[0] - b[0];
    X commonDiff2 = a[1] - b[1];
    for (stype_t ii = 0; ii < cnt; ++ii)
    {
        if (std::abs (a[ii] - b[ii]) > maxAllowableDiff)
        {
            if (std::abs (a[ii] - b[ii] - commonDiff1) > maxAllowableDiff)
            {
                if (std::abs (a[ii] - b[ii] - commonDiff2) > maxAllowableDiff)
                {
                    ++diffs;
                }
            }
        }
    }
    return diffs;
}

/** count the differences between two vectors if the difference is greater than a tolerance
@param a the first vector
@param b the second vector
@param maxAllowableDiff the tolerable difference between two values
@param  maxFracDiff the difference must be greater than maxFracDiff*|a[ii]|
@return the number of differences*/
template <class X>
auto countDiffs (const std::vector<X> &a, const std::vector<X> &b, X maxAllowableDiff, X maxFracDiff)
{
    using stype_t = typename std::vector<X>::size_type;
    stype_t cnt = (std::min) (a.size (), b.size ());
    stype_t diffs = (std::max) (a.size (), b.size ()) - cnt;
    for (stype_t ii = 0; ii < cnt; ++ii)
    {
        if ((std::abs (a[ii] - b[ii]) > maxAllowableDiff) &&
            (std::abs (a[ii] - b[ii]) > maxFracDiff * std::abs (a[ii])))
        {
            ++diffs;
        }
    }
    return diffs;
}

/** count the differences between two vectors if the difference is greater than a tolerance and the a value is valid (ie !=0)
@param a the first vector
@param b the second vector
@param maxAllowableDiff the tolerable difference between two values
@return the number of differences*/
template <class X>
auto countDiffsIfValid (const std::vector<X> &a, const std::vector<X> &b, X maxAllowableDiff)
{
    using stype_t = typename std::vector<X>::size_type;
    stype_t cnt = (std::min) (a.size (), b.size ());
    stype_t diffs = (std::max) (a.size (), b.size ()) - cnt;
    for (stype_t ii = 0; ii < cnt; ++ii)
    {
        if ((std::abs (a[ii] - b[ii]) > maxAllowableDiff) && (a[ii] != 0))
        {
            ++diffs;
        }
    }
    return diffs;
}

/** count the differences between two vectors if the difference is greater than a tolerance and call a callback function for each difference
@param a the first vector
@param b the second vector
@param maxAllowableDiff the tolerable difference between two values
@param  f the callback function takes 3 arguments the index and the values for each of the two vectors in the index
@return the number of differences*/
template <class X>
auto countDiffsCallback (const std::vector<X> &a,
                         const std::vector<X> &b,
                         X maxAllowableDiff,
                         std::function<void(typename std::vector<X>::size_type, X, X)> &f)
{
    using stype_t = typename std::vector<X>::size_type;
    stype_t cnt = (std::min) (a.size (), b.size ());
    stype_t diffs = (std::max) (a.size (), b.size ()) - cnt;
    for (stype_t ii = 0; ii < cnt; ++ii)
    {
        if (std::abs (a[ii] - b[ii]) > maxAllowableDiff)
        {
            ++diffs;
            f (ii, a[ii], b[ii]);
        }
    }
    return diffs;
}

/** count the differences between two vectors if the difference is greater than a tolerance and the a value is valid (ie !=0) and call a callback function for each difference
@param a the first vector
@param b the second vector
@param maxAllowableDiff the tolerable difference between two values
@param  f the callback function takes 3 arguments the index and the values for each of the two vectors in the index
@return the number of differences*/
template <class X>
auto countDiffsIfValidCallback (const std::vector<X> &a,
                                const std::vector<X> &b,
                                X maxAllowableDiff,
                                std::function<void(typename std::vector<X>::size_type, X, X)> &f)
{
    using stype_t = typename std::vector<X>::size_type;
    stype_t cnt = (std::min) (a.size (), b.size ());
    stype_t diffs = (std::max) (a.size (), b.size ()) - cnt;
    for (stype_t ii = 0; ii < cnt; ++ii)
    {
        if ((std::abs (a[ii] - b[ii]) > maxAllowableDiff) && (a[ii] != X(0)))
        {
            ++diffs;
            f (ii, a[ii], b[ii]);
        }
    }
    return diffs;
}
/** functions that action do the vector conversions  if the types are actually the same just copy or
move them if possible.  If they are not the same type do a static cast in a transform to make a new vector
  the code use SFINAE magic with the std::false_type and std::true_type to discriminate which overload to use
the function below them vectorConvert include a type_trait check for if the vector types are the same
*/
namespace vectorConvertDetail
{
/** perform a vector conversion of one type to another
@details uses SFINAE to do some checking to determine if the types are actually the same
this function handles the case where they are not and is an lvalue reference
*/
template <typename X, typename Y>
std::vector<X> vectorConvertActual (const std::vector<Y> &dvec, std::false_type /*unused*/)
{
    std::vector<X> ret (dvec.size ());
    std::transform (dvec.begin (), dvec.end (), ret.begin (), [](Y val) { return static_cast<X> (val); });
    return ret;
}
/** perform a vector conversion of one type to another
@details uses SFINAE to do some checking to determine if the types are actually the same
this function handles the case where they are the same and is an rvalue reference
*/
template <typename X, typename Y>
std::vector<X> vectorConvertActual (std::vector<Y> &&dvec, std::true_type /*unused*/)
{
    std::vector<X> ret (std::move (dvec));
    return ret;
}

/** perform a vector conversion of one type to another
@details uses SFINAE to do some checking to determine if the types are actually the same
this function handles the case where they are the same and is an lvalue reference
*/
template <typename X, typename Y>
std::vector<X> vectorConvertActual (const std::vector<Y> &dvec, std::true_type /*unused*/)
{
    std::vector<X> ret = dvec;
    return ret;
}
}  // namespace vectorConvertDetail

/** convert a vector of one type into a vector of another type
@tparam X the desired resultant type
@tparam Y the original type
@tparam Z the base type of Y  SFINAE check
@param dvec a rvalue reference to a vector to convert
*/
template <typename X, typename Y, typename Z = typename Y::baseType>
std::vector<X> vectorConvert (std::vector<Y> &&dvec)
{
    return vectorConvertDetail::vectorConvertActual<X, Y> (dvec, std::is_same<X, Z>{});
}

/** convert a vector of one type into a vector of another type
@tparam X the desired resultant type
@tparam Y the original type
@param dvec a rvalue reference to a vector to convert
*/
template <typename X, typename Y>
std::vector<X> vectorConvert (std::vector<Y> &&dvec)
{
    return vectorConvertDetail::vectorConvertActual<X, Y> (dvec, std::is_same<X, Y>{});
}

/** convert a vector of one type into a vector of another type
@tparam X the desired resultant type
@tparam Y the original type
@tparam Z the base type of Y  SFINAE check
@param dvec a const lvalue reference to a vector to convert
*/
template <typename X, typename Y, typename Z = typename Y::baseType>
std::vector<X> vectorConvert (const std::vector<Y> &dvec)
{
    return vectorConvertDetail::vectorConvertActual<X, Y> (dvec, std::is_same<X, Z>{});
}

/** convert a vector of one type into a vector of another type
@tparam X the desired resultant type
@tparam Y the original type
@param dvec a lvalue reference to a vector to convert
@return a vector containing the new type with the corresponding values as the original
*/
template <typename X, typename Y>
std::vector<X> vectorConvert (const std::vector<Y> &dvec)
{
    return vectorConvertDetail::vectorConvertActual<X, Y> (dvec, std::is_same<X, Y>{});
}

#endif
