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

#ifndef VECTOR_OPS_H
#define VECTOR_OPS_H
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <array>
#include <functional>
#include <type_traits>


double solve2x2 (double a, double b, double c, double d, double y1, double y2, double &x1, double &x2);

std::array<double,3> solve3x3 (std::array <std::array<double, 3>,3> &input, std::array<double, 3> &vals);


std::vector<double> interpolateLinear (const std::vector<double> &timeIn, const std::vector<double> &valIn, const std::vector<double> &timeOut);

#ifdef _MSC_VER
#if _MSC_VER < 1900
#define KEY_QUAL inline const
#endif
#endif
#ifndef KEY_QUAL
#define KEY_QUAL constexpr
#endif

template<class N>
KEY_QUAL N valLimit (N val, N lowerLim, N upperLim)
{
  return (val > upperLim) ? upperLim : ((val < lowerLim) ? lowerLim : val);
}

template<class N>
KEY_QUAL N valUpperLimit (N val, N upperLim)
{
  return (val > upperLim) ? upperLim : val;
}

template<class N>
KEY_QUAL N valLowerLimit (N val, N lowerLim)
{
  return (val < lowerLim) ? lowerLim : val;
}

template<typename M>
KEY_QUAL M signn (M x)
{
  return ((x > 0) ? 1 : ((x < 0) ? (-1) : 0));
}

template<class X>
X sum (const std::vector<X> &a)
{
  X sum_of_vector = std::accumulate (a.begin (),a.end (),0.0);
  return sum_of_vector;
}

template<class X>
X mean (const std::vector<X> &a)
{
  X sum_of_vector = std::accumulate (a.begin (), a.end (), 0.0);
  if (!a.empty ())
    {
      sum_of_vector /= static_cast<X> (a.size ());
    }

  return sum_of_vector;
}

template<class X>
X absSum (const std::vector<X> &a)
{
  X sum_of_vector = std::accumulate (a.begin (), a.end (), 0.0, [](X a1, X a2) -> X {
    return a1 + std::abs (a2);
  });
  return sum_of_vector;
}


template<class X>
X absMax (const std::vector<X> &a)
{
  auto result = std::minmax_element (a.begin (), a.end ());
  return std::max (std::abs (*(result.first)),std::abs (*(result.second)));
}

template<class X>
X absMaxLoc (const std::vector<X> &a, int &loc)
{
  auto result = std::minmax_element (a.begin (), a.end ());
  X a1 = std::abs (*(result.first));
  X a2 = std::abs (*(result.second));
  if (a1 > a2)
    {
      loc = result.first - a.begin ();
      return a1;
    }
  else
    {
      loc = result.second - a.begin ();
      return a2;
    }
}

template<class X>
X absMin (const std::vector<X> &a)
{
  auto result = std::minmax_element (a.begin (), a.end ());
  return std::min (std::abs (*(result.first)), std::abs (*(result.second)));
}

template<class X>
X absMinLoc (const std::vector<X> &a, int &loc)
{
  auto result = std::minmax_element (a.begin (), a.end ());
  X a1 = std::abs (*(result.first));
  X a2 = std::abs (*(result.second));
  if (a1 <= a2)
    {
      loc = result.first - a.begin ();
      return a1;
    }
  else
    {
      loc = result.second - a.begin ();
      return a2;
    }
}

template<class X>
X maxLoc (const std::vector<X> &a, int &loc)
{
  auto result = std::max_element (a.begin (), a.end ());
  X a1 = *(result);
  loc = result - a.begin ();
  return a1;
}

template<class X>
X minLoc (const std::vector<X> &a, int &loc)
{
  auto result = std::min_element (a.begin (), a.end ());
  X a1 = *(result);
  loc = result - a.begin ();
  return a1;
}


template<class X>
X absMaxDiffLoc(const std::vector<X> &a, const std::vector<X> &b, int &loc)
{
	loc = -1;
	X mdiff = 0;
	auto cnt = (std::min)(a.size(), b.size());
	auto abeg = a.begin();
	auto acur = abeg;
	auto bcur = b.begin();
	auto aend = abeg + cnt;
	while (acur < aend)
	{
		if (std::abs(*acur - *bcur) > mdiff)
		{
			loc = acur - abeg;
			mdiff = std::abs(*acur - *bcur);
		}
		++acur;
		++bcur;
	}
	return mdiff;
}

template<class X>
X product (const std::vector<X> &a)
{
  X prod_of_vector = std::accumulate (a.begin (), a.end (), 0.0, [](X a1, X a2) -> X {
    return a1 * a2;
  });
  return prod_of_vector;
}


template<class X>
X rms (const std::vector<X> &a)
{
  X sum_of_vector = std::accumulate (a.begin (), a.end (), 0.0, [](X a1, X a2) -> X {
    return (a1 + a2 * a2);
  });
  return std::sqrt (sum_of_vector);
}

template<class X>
X stdev (const std::vector<X> &a)
{
  X mv = mean (a);
  X sum_of_vector = std::accumulate (a.begin (), a.end (), 0.0, [mv](X a1, X a2) -> X {
    return (a1 + (a2 - mv) * (a2 - mv));
  });
  X ret = 0;
  if (!a.empty ())
    {
      ret = std::sqrt (sum_of_vector / static_cast<X> (a.size ()));
    }
  return ret;
}

template<class X>
X median (std::vector<X> &a)
{
  size_t n = a.size () / 2;
  std::nth_element (a.begin (), a.begin () + n, a.end ());
  if (a.size () % 2 == 1)
    {
      return a[n];
    }
  else
    {
      std::nth_element (a.begin (), a.begin () + n - 1, a.end ());
      return static_cast<X> (0.5 * (a[n] + a[n - 1]));
    }

}

template<class X>
std::vector<X> diff (std::vector<X> &a)
{
  auto cnt = a.size ();
  std::vector<X> d (cnt);
  std::adjacent_difference (a.begin (), a.end (), d.begin ());
  return d;
}

//finding index values to meet conditions
template<class X>
auto vecFindeq (const std::vector<X> &a, X match)->std::vector<decltype (a.size ())>
{
  auto cnt = a.size ();
  std::vector<decltype (cnt)> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] == match)
        {
          locs.push_back (ii);
        }
    }
  return locs;
}

template<class X>
auto vecFindne (const std::vector<X> &a, X match)->std::vector<decltype (a.size ())>
{
  auto cnt = a.size ();
  std::vector<decltype (cnt)> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] != match)
        {
          locs.push_back (ii);
        }
    }
  return locs;
}

template<class X>
auto vecFindne (const std::vector<X> &a, X match, size_t start, size_t end)->std::vector<decltype (a.size ())>
{
  auto cnt = a.size ();
  cnt = std::min (end, cnt);
  std::vector<decltype (cnt)> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = start; ii < cnt; ++ii)
    {
      if (a[ii] != match)
        {
          locs.push_back (ii);
        }
    }
  return locs;
}


template<class X>
auto vecFindlt (const std::vector<X> &a, X val)->std::vector<decltype (a.size ())>
{
  auto cnt = a.size ();
  std::vector<decltype (cnt)> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] < val)
        {
          locs.push_back (ii);
        }
    }
  return locs;
}

template<class X>
auto vecFindlte (const std::vector<X> &a, X val)->std::vector<decltype (a.size ())>
{
  auto cnt = a.size ();
  std::vector<decltype (cnt)> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] <= val)
        {
          locs.push_back (ii);
        }
    }
  return locs;
}

template<class X>
auto vecFindgt (const std::vector<X> &a, X val)->std::vector<decltype (a.size ())>
{
  auto cnt = a.size ();
  std::vector<decltype (cnt)> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] > val)
        {
          locs.push_back (ii);
        }
    }
  return locs;
}

template<class X>
auto vecFindgte (const std::vector<X> &a, X val)->std::vector<decltype (a.size ())>
{
  auto cnt = a.size ();
  std::vector<decltype (cnt)> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] >= val)
        {
          locs.push_back (ii);
        }
    }
  return locs;
}

//return in specified type vector
template<class X, class Y>
std::vector<Y> vecFindeq (const std::vector<X> &a, X match)
{
  auto cnt = a.size ();
  std::vector<Y> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] == match)
        {
          locs.push_back (static_cast<Y> (ii));
        }
    }
  return locs;
}

template<class X, class Y>
std::vector<Y> vecFindne (const std::vector<X> &a, X match)
{
  auto cnt = a.size ();
  std::vector<Y> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] != match)
        {
          locs.push_back (static_cast<Y> (ii));
        }
    }
  return locs;
}

template<class X, class Y>
std::vector<Y> vecFindne (const std::vector<X> &a, X match, size_t start, size_t end)
{
  auto cnt = a.size ();
  cnt = std::min (end, cnt);
  std::vector<Y> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = start; ii < cnt; ++ii)
    {
      if (a[ii] != match)
        {
          locs.push_back (static_cast<Y> (ii));
        }
    }
  return locs;
}


template<class X, class Y>
std::vector<Y> vecFindlt (const std::vector<X> &a, X val)
{
  auto cnt = a.size ();
  std::vector<Y> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] < val)
        {
          locs.push_back (static_cast<Y> (ii));
        }
    }
  return locs;
}

template<class X, class Y>
std::vector<Y> vecFindlte (const std::vector<X> &a, X val)
{
  auto cnt = a.size ();
  std::vector<Y> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] <= val)
        {
          locs.push_back (static_cast<Y> (ii));
        }
    }
  return locs;
}

template<class X, class Y>
std::vector<Y> vecFindgt (const std::vector<X> &a, X val)
{
  auto cnt = a.size ();
  std::vector<Y> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] > val)
        {
          locs.push_back (static_cast<Y> (ii));
        }
    }
  return locs;
}

template<class X, class Y>
std::vector<Y> vecFindgte (const std::vector<X> &a, X val)
{
  auto cnt = a.size ();
  std::vector<Y> locs;
  locs.reserve (cnt);
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (a[ii] >= val)
        {
          locs.push_back (static_cast<Y> (ii));
        }
    }
  return locs;
}

template <class X, class Y>
X ind_sum (const std::vector<X> &a, const std::vector<Y> &b, Y match)
{
  typename std::vector<X>::size_type cnt = (std::min)(a.size (), b.size ());
  X sum_of_vector = 0;
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      if (b[ii] == match)
        {
          sum_of_vector += a[ii];
        }
    }
  return sum_of_vector;
}

template<class X>
X mult_sum (const std::vector<X> &a, const std::vector<X> &b)
{
  X sum_of_vector_mult = 0;
  typename std::vector<X>::size_type cnt = (std::min)(a.size (), b.size ());
  for (decltype (cnt)ii = 0; ii < cnt; ++ii)
    {
      sum_of_vector_mult += a[ii] * b[ii];
    }
  return sum_of_vector_mult;
}

template<class X>
void vectorAdd (std::vector<X> &a, const std::vector<X> &b)
{

  auto cnt = (std::min)(a.size (), b.size ());
  std::transform (a.begin (), a + cnt, b.begin (), a.begin (), std::plus<X> ());
}

template<class X>
void vectorSubtract (std::vector<X> &a, const std::vector<X> &b)
{

  auto cnt = (std::min)(a.size (), b.size ());
  std::transform (a.begin (), a + cnt, b.begin (), a.begin (), std::minus<X> ());
}

template<class X>
void vectorMult (const std::vector<X> &a, const std::vector<X> &b, std::vector<X> &M)
{

  auto cnt = (std::min)(a.size (), b.size ());
  std::transform (a.begin (), a + cnt, b.begin (), M.begin (), std::multiplies<X> ());
}

template<class X>
void vectorMultAdd (const std::vector<X> &a, const std::vector<X> &b, const X M,std::vector<X> &res)
{

  auto cnt = (std::min)(a.size (), b.size ());
  for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
    {
      res[ii] = std::fma (M, b[ii],a[ii]);     //fast multiply add
    }
}

template<class X>
X compareVec (const std::vector<X> &a, const std::vector<X> &b,std::vector<X> &diff)
{
  X sum_of_diff = 0;
  typename std::vector<X>::size_type cnt = (std::min)(a.size (), b.size ());
  diff.resize (cnt);
  for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
    {
      diff[ii] = std::abs (a[ii] - b[ii]);
      sum_of_diff += diff[ii];
    }
  return sum_of_diff;
}

template<class X>
X compareVec (const std::vector<X> &a, const std::vector<X> &b)
{
  X sum_of_diff = 0;
  typename std::vector<X>::size_type cnt = (std::min)(a.size (), b.size ());

  for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
    {
      sum_of_diff += std::abs (a[ii] - b[ii]);
    }
  return sum_of_diff;
}


template<class X>
X compareVec (const std::vector<X> &a, const std::vector<X> &b, std::vector<X> &diff,
              typename std::vector<X>::size_type cnt)
{
  X sum_of_diff = 0;
  cnt = (std::min)(a.size (), cnt);
  cnt = (std::min)(b.size (), cnt);
  diff.resize (cnt);
  for (typename std::vector<X>::size_type ii = 0; ii < 0; ++ii)
    {
      diff[ii] = std::abs (a[ii] - b[ii]);
      sum_of_diff += diff[ii];
    }
  return sum_of_diff;
}

template<class X>
X compareVec (const std::vector<X> &a, const std::vector<X> &b,
              typename std::vector<X>::size_type cnt)
{
  X sum_of_diff = 0;
  cnt = (std::min)(a.size (), cnt);
  cnt = (std::min)(b.size (), cnt);
  for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
    {
      sum_of_diff += std::abs (a[ii] - b[ii]);
    }
  return sum_of_diff;
}


template<class X>
auto countDiffs(const std::vector<X> &a, const std::vector<X> &b, X maxAllowableDiff)->typename std::vector<X>::size_type
{
	
	typename std::vector<X>::size_type cnt = (std::min)(a.size(), b.size());
	typename std::vector<X>::size_type diffs = (std::max)(a.size(), b.size())-cnt;
	for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
	{
		if (std::abs(a[ii] - b[ii]) > maxAllowableDiff)
		{
			++diffs;
		}
	}
	return diffs;
}

template<class X>
auto countDiffsIgnoreCommon(const std::vector<X> &a, const std::vector<X> &b, X maxAllowableDiff)->typename std::vector<X>::size_type
{

	typename std::vector<X>::size_type cnt = (std::min)(a.size(), b.size());
	typename std::vector<X>::size_type diffs = (std::max)(a.size(), b.size()) - cnt;
	if (cnt<3)
	{
		return 0;
	}
	X commonDiff1 = a[0] - b[0];
	X commonDiff2 = a[1] - b[1];
	for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
	{
		if (std::abs(a[ii] - b[ii]) > maxAllowableDiff)
		{
			if (std::abs(a[ii] - b[ii] - commonDiff1) > maxAllowableDiff)
			{
				if (std::abs(a[ii] - b[ii] - commonDiff2) > maxAllowableDiff)
				{
					++diffs;
				}
			}
			
		}
	}
	return diffs;
}

template<class X>
auto countDiffs(const std::vector<X> &a, const std::vector<X> &b, X maxAllowableDiff, X maxFracDiff)->typename std::vector<X>::size_type
{

	typename std::vector<X>::size_type cnt = (std::min)(a.size(), b.size());
	typename std::vector<X>::size_type diffs = (std::max)(a.size(), b.size()) - cnt;
	for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
	{
		if ((std::abs(a[ii] - b[ii]) > maxAllowableDiff)&& (std::abs(a[ii] - b[ii]) > maxFracDiff*std::abs(a[ii])))
		{
			++diffs;
		}
	}
	return diffs;
}

template<class X>
auto countDiffsIfValid(const std::vector<X> &a, const std::vector<X> &b, X maxAllowableDiff)->typename std::vector<X>::size_type
{

	typename std::vector<X>::size_type cnt = (std::min)(a.size(), b.size());
	typename std::vector<X>::size_type diffs = (std::max)(a.size(), b.size()) - cnt;
	for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
	{
		if ((std::abs(a[ii] - b[ii]) > maxAllowableDiff)&&(a[ii]!=0))
		{
			++diffs;
		}
	}
	return diffs;
}

template<class X>
auto countDiffsCallback(const std::vector<X> &a, const std::vector<X> &b, 
	X maxAllowableDiff, 
	std::function<void(typename std::vector<X>::size_type,X, X)> &f)->typename std::vector<X>::size_type
{

	typename std::vector<X>::size_type cnt = (std::min)(a.size(), b.size());
	typename std::vector<X>::size_type diffs = (std::max)(a.size(), b.size()) - cnt;
	for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
	{
		if (std::abs(a[ii] - b[ii]) > maxAllowableDiff)
		{
			++diffs;
			f(ii, a[ii], b[ii]);
		}
	}
	return diffs;
}

template<class X>
auto countDiffsIfValidCallback(const std::vector<X> &a, const std::vector<X> &b,
	X maxAllowableDiff,
	std::function<void(typename std::vector<X>::size_type, X, X)> &f)->typename std::vector<X>::size_type
{

	typename std::vector<X>::size_type cnt = (std::min)(a.size(), b.size());
	typename std::vector<X>::size_type diffs = (std::max)(a.size(), b.size()) - cnt;
	for (typename std::vector<X>::size_type ii = 0; ii < cnt; ++ii)
	{
		if ((std::abs(a[ii] - b[ii]) > maxAllowableDiff) && (a[ii] != 0))
		{
			++diffs;
			f(ii, a[ii], b[ii]);
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
	template< typename X, typename Y>
	std::vector<X> vectorConvertActual(const std::vector<Y> &dvec, std::false_type)
	{
		std::vector<X> ret(dvec.size());
		std::transform(dvec.begin(), dvec.end(), ret.begin(), [](Y val) {return static_cast<X>(val); });
		return ret;
	}

	template< typename X, typename Y>
	std::vector<X> vectorConvertActual(std::vector<Y> &&dvec, std::true_type)
	{
		std::vector<X> ret(dvec);
		return ret;
	}

	template< typename X, typename Y>
	std::vector<X> vectorConvertActual(const std::vector<Y> &dvec, std::true_type)
	{
		std::vector<X> ret = dvec;
		return ret;
	}

	
}



template< typename X, typename Y, typename Z = typename Y::baseType>
std::vector<X> vectorConvert(std::vector<Y> &&dvec)
{
	return vectorConvertDetail::vectorConvertActual<X, Y>(dvec, std::is_same<X, Z>{});
}

template< typename X, typename Y>
std::vector<X> vectorConvert(std::vector<Y> &&dvec)
{
	return vectorConvertDetail::vectorConvertActual<X, Y>(dvec, std::is_same<X, Y>{});
}

template< typename X, typename Y, typename Z = typename Y::baseType>
std::vector<X> vectorConvert(const std::vector<Y> &dvec)
{
	return vectorConvertDetail::vectorConvertActual<X, Y>(dvec, std::is_same<X, Z>{});
}

template< typename X, typename Y>
std::vector<X> vectorConvert(const std::vector<Y> &dvec)
{
	return vectorConvertDetail::vectorConvertActual<X, Y>(dvec, std::is_same<X, Y>{});
}

#endif
