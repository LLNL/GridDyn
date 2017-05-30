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

#ifndef _MATRIX_DATA_COMPACT_H_
#define _MATRIX_DATA_COMPACT_H_

#include  "utilities/matrixData.h"
#include <array>

/** @brief class implementing a dense matrix geared for Jacobian entries
 this matrixData structure is intended to be for small dense matrices with a required fast value lookup
and possibly lots of duplicate entries
*/
template< count_t R, count_t C, class Y=double>
class matrixDataCompact: public matrixData<Y>
{
private:
	std::array<Y, R*C> dVec;         //!< the array containing the data
	index_t Rctr = 0;
	index_t Cctr = 0;
public:
	/** @brief compact constructor
	*/
	matrixDataCompact():matrixData<Y>(R,C) {};

	void clear() override
	{
		dVec.fill(0);
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
	
	void setRowLimit(index_t /*limit*/) final override
	{};
	void setColLimit(index_t /*limit*/) final override
	{}

	matrixElement<Y> element(index_t N) const override
	{
		return{ N%R, N / R, dVec[N] };
	}

	void start() override
	{
		matrixData<Y>::cur = 0;
		Rctr = 0;
		Cctr = 0;
	}
	
	 matrixElement<Y> next() override
	{
		matrixElement<Y> tp{ Rctr,Cctr,dVec[matrixData<Y>::cur] };
		++matrixData<Y>::cur;
		++Rctr;
		if (Rctr == R)
		{
			Rctr = 0;
			++Cctr;
		}
		return tp;
	}

	Y at(index_t rowN, index_t colN) const override
	{
		return dVec[colN*R + rowN];
	};

	auto begin()
	{
		return matrixIteratorCompact(this, 0);
	}
	auto end()
	{
		return matrixIteratorCompact(this, R*C);
	}
protected:
	class matrixIteratorCompact
	{
	public:
		explicit matrixIteratorCompact(const matrixDataCompact<R,C,Y> *matrixData, index_t start = 0):counter(start)
		{
			if (start==mDC->size())
			{
				Rctr = R;
				Cctr = C;
			}

		}

		virtual matrixIteratorCompact& operator++()
		{
			++counter;
			++Rctr;
			if (Rctr == R)
			{
				Rctr = 0;
				++Cctr;
			}
			return *this;
		}


		virtual matrixElement<Y> operator*() const
		{
			return{ Rctr,Cctr,mDC->dVec[counter] };
		}
	private:
		const matrixDataCompact<R, C, Y> *mDC = nullptr;
		index_t Rctr = 0;
		index_t Cctr = 0;
		index_t counter;

	};
};

#endif

