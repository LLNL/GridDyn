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

#ifndef _MATRIX_DATA_SCALE_H_
#define _MATRIX_DATA_SCALE_H_

#include "matrixData.h"

/** @brief class implementation for a scaling array data
 all data is multiplied by a factor before being sent to the underlying matrixData object
*/
template<class Y = double>
class matrixDataScale : public matrixData<Y>
{
public:
	matrixData<Y> *ad;  //!< the matrix to translate to
private:
	Y scalingFactor;
public:
	/** @brief constructor
	*/
	matrixDataScale(matrixData<Y> *input, Y scaleFactor):scalingFactor(scaleFactor)
	{
		setArray(input);
	};

	void clear() override
	{
		ad->clear();
	};

	void assign(index_t row, index_t col, Y num) override
	{
		ad->assign(row, col, num*scalingFactor);
	};

	count_t size() const override
	{
		return ad->size();
	};

	void reserve(count_t maxNonZeros) override
	{
		ad->reserve(maxNonZeros);
	}

	count_t capacity() const override
	{
		return ad->capacity();
	};

	index_t rowIndex(index_t N) const override
	{
		return ad->rowIndex(N);
	};

	index_t colIndex(index_t N) const override
	{
		return ad->colIndex(N);
	};

	Y val(index_t N) const override
	{
		return ad->val(N);
	};

	void compact() override
	{
		ad->compact();
	}

	virtual matrixIterator<Y> begin() const override
	{
		return ad->begin();
	}

	virtual matrixIterator<Y> end() const override
	{
		return ad->end();
	}

	void start() override
	{
		ad->start();
	}

	matrixElement<Y> next() override
	{
		return ad->next();
	}

	bool moreData() override
	{
		return ad->moreData();
	}

	Y at(index_t rowN, index_t colN) const override
	{
		return ad->at(rowN, colN);
	};
	/** set the matrixData object to translate to
	@param[in] newAd  the new matrixData object
	*/
	void setArray(matrixData<Y> *newAd)
	{
		ad = newAd;
		matrixData<Y>::colLim = ad->colLimit();
		matrixData<Y>::rowLim = ad->rowLimit();
	}
	/** @brief set the scale factor for the array
	@param[in] scaleFactor  the input row to translate
	*/
	void setScale(Y scaleFactor)
	{
		scalingFactor = scaleFactor;
	}
};

#endif

