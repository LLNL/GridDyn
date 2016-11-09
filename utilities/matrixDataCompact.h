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

#ifndef _MATRIX_DATA_COMPACT_H_
#define _MATRIX_DATA_COMPACT_H_

#include "matrixData.h"
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

protected:
	class matrixIteratorCompact :public matrixIteratorActual<Y>
	{
	public:
		explicit matrixIteratorCompact(const matrixDataCompact<R,C,Y> *matrixData, index_t start = 0) :matrixIteratorActual<Y>(matrixData, start), mDC(matrixData)
		{
			if (start==mDC->size())
			{
				Rctr = R;
				Cctr = C;
			}

		}
		matrixIteratorCompact(const matrixIteratorCompact *it2) :matrixIteratorActual<Y>(it2->mDC,it2->currentElement), mDC(it2->mDC)
		{
			Rctr = it2->Rctr;
			Cctr = it2->Cctr;
		}

		virtual matrixIteratorActual<Y> *clone() const override
		{
			return new matrixIteratorCompact(this);
		}

		virtual void increment() override
		{
			matrixIteratorActual<Y>::increment();
			++Rctr;
			if (Rctr == R)
			{
				Rctr = 0;
				++Cctr;
			}
		}


		virtual matrixElement<Y> operator*() const override
		{
			return{ Rctr,Cctr,mDC->dVec[matrixIteratorActual<Y>::currentElement] };
		}
	private:
		const matrixDataCompact<R, C, Y> *mDC = nullptr;
		index_t Rctr = 0;
		index_t Cctr = 0;

	};
};

#endif

