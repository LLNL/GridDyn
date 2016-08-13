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

#include "vectData.h"

#include <algorithm>
#include <cmath>
#include <cassert>

static const index_t kNullLocation((index_t)(-1));

bool compareLocVectData (vLoc A, vLoc B)
{
  return (A.first < B.first);
}


void vectData::compact ()
{
  if (dVec.empty ())
    {
      return;
    }

  auto dvb = dVec.begin ();
  auto dv2 = dvb;
  ++dv2;
  auto dvend = dVec.end ();
  while (dv2 != dvend)
    {
      if (dv2->first == dvb->first)
        {
          dvb->second += dv2->second;

        }
      else
        {
          ++dvb;
          *dvb = *dv2;
        }
      ++dv2;
    }
  dVec.resize (++dvb - dVec.begin ());
}

void vectData::assign(index_t X, double num)
{
	assert(!std::isnan(num));

	dVec.emplace_back(X, num);
}

void vectData::assignCheck(index_t X, double num)
{
	if (X != kNullLocation)
	{
		assert(X < svsize);
		assert(!std::isnan(num));
		dVec.emplace_back(X, num);
	}
}

void vectData::sortIndex()
{
	std::sort(dVec.begin(), dVec.end(), compareLocVectData);
}

double vectData::find (index_t rowN)
{  //NOTE: function assumes vectdata is sorted and compacted
  auto res = std::lower_bound (dVec.begin (), dVec.end (), vLoc (rowN, 0.0), compareLocVectData);
  if (res == dVec.end ())
    {
      return 0;
    }
  if (res->first == rowN)
    {
      return res->second;
    }
  else
    {
      return 0.0;
    }
}

void vectData::scale (double factor, index_t start, count_t count)
{
  if (start >= dVec.size ())
    {
      return;
    }
  auto res = dVec.begin () + start;
  auto term = dVec.begin () + std::min (start + count, static_cast<count_t> (dVec.size ()));

  while (res != term)
    {
      res->second *= factor;
      ++res;
    }
}
