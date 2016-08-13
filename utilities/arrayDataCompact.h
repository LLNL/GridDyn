#pragma once
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2014, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef _ARRAY_DATA_COMPACT_H_
#define _ARRAY_DATA_COMPACT_H_

#include "arrayData.h"
#include <array>
#include <cstring>

/** @brief class implementing a dense matrix geared for jacobaian entries
 this arrayData structure is intended to be for small dense matrices with a required fast value lookup
and possibly lots of duplicate entries
*/
template< count_t R, count_t C, class Y=double>
class arrayDataCompact: public arrayData<Y>
{
private:
	std::array<Y, R*C> dVec;         //!< the vector of tuples containing the data
	index_t Rctr = 0;
	index_t Cctr = 0;
public:
	/** @brief compact constructor
	*/
	arrayDataCompact():arrayData<Y>(R,C) {};

	void clear() override
	{
		//std::fill(dVec.begin(), dVec.end(), 0);
		memset(dVec.data(), 0, sizeof(Y)*R*C);
	};
	
	void assign(index_t row, index_t col, Y num) override
	{
		//in column major order
		dVec[col*R + row] += num;
	};

	count_t size() const override
	{
		return R*C;
	};

	count_t capacity() const override
	{
		return R*C;
	};
	
	void setRowLimit(index_t /*lim*/) final
	{};
	void setColLimit(index_t /*lim*/) final
	{}

	index_t rowIndex(index_t N) const override
	{
		return (N % R);
	};

	index_t colIndex(index_t N) const override
	{
		return N / R;  //integer division
	};

	Y val(index_t N) const override
	{
		return dVec[N];
	};

	void start() override
	{
		arrayData<Y>::cur = 0;
		Rctr = 0;
		Cctr = 0;
	}
	
	 data_triple<Y> next() override
	{
		data_triple<Y> tp{ Rctr,Cctr,dVec[arrayData<Y>::cur] };
		++arrayData<Y>::cur;
		++Rctr;
		if (Rctr == R)
		{
			++Rctr = 0;
			++Cctr;
		}
		return tp;
	}

	double at(index_t rowN, index_t colN) const override
	{
		return dVec[colN*R + rowN];
	};
};

#endif

