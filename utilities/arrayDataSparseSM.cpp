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

#include "arrayDataSparseSM.h"

#include <cassert>
#include <algorithm>

static const index_t kNullLocation((index_t)(-1));

const bool compareLocSM2(pLocBig A, pLocBig B)
{
	return (A.first < B.first);
}


arrayDataSparseSM2::arrayDataSparseSM2(index_t startCount)
{
	dVec.reserve(startCount);
}
/**
* function to clear the data
*/
void arrayDataSparseSM2::clear()
{
	dVec.clear();
}

void arrayDataSparseSM2::assign(index_t X, index_t Y, double num)
{
	assert(X < rowLim);
	assert(Y < colLim);
	assert(std::isfinite(num));

	//dVec.push_back (cLoc (X, Y, num));
	dVec.emplace_back((static_cast<std::uint64_t>(Y)<<32) + static_cast<std::uint64_t>(X), num);
}

const count_t arrayDataSparseSM2::size()
{
	return static_cast<count_t>(dVec.size());
}

const count_t arrayDataSparseSM2::capacity()
{
	return static_cast<count_t> (dVec.capacity());
}

void arrayDataSparseSM2::sortIndex()
{
	std::sort(dVec.begin(), dVec.end(), compareLocSM2);
	sortCount = static_cast<count_t>(dVec.size());
}

void arrayDataSparseSM2::compact()
{
	if (dVec.empty())
	{
		return;
	}

	auto dvb = dVec.begin();
	auto dv2 = dvb;
	++dv2;
	auto dvend = dVec.end();
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
	dVec.resize(++dvb - dVec.begin());
}

const index_t arrayDataSparseSM2::rowIndex(index_t N)
{
	return static_cast<index_t>((dVec[N].first & (std::uint64_t)(0xFFFFFFFF)));
}

const index_t arrayDataSparseSM2::colIndex(index_t N)
{
	return static_cast<index_t>(dVec[N].first >>32);
}

const double arrayDataSparseSM2::val(index_t N)
{
	return dVec[N].second;
}

const double arrayDataSparseSM2::at(index_t rowN, index_t colN)
{
	std::uint64_t cmp = (static_cast<std::uint64_t>(colN) << 32) + static_cast<std::uint64_t>(rowN);
	if (isSorted())
	{
		auto res = std::lower_bound(dVec.begin(), dVec.end(), std::make_pair(cmp, 0), compareLocSM2);
		if (res == dVec.end())
		{
			return 0.0;
		}
		if (res->first == cmp)
		{
			return res->second;
		}
		else
		{
			return 0.0;
		}
	}
	else
	{

		for (const auto &rv : dVec)
		{
			if (rv.first == cmp)
			{
				return rv.second;
			}
		}
		return 0.0;
	}

}