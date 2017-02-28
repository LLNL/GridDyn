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

#ifndef _MATRIX_DATA_CONTAINER_H_
#define _MATRIX_DATA_CONTAINER_H_

#include "matrixData.h"

/** @brief intermediate class for implementing a containing matrix data
@details class is abstract and cannot be instantiated on its own meant to help some 
other classes that do things to the input before transmitting it.  
uses a pointer so it can be reassigned later
*/
template<class Y = double>
class matrixDataContainer : public matrixData<Y>
{
public:
	matrixData<Y> *ad;  //!< the matrix this class contains
public:
	matrixDataContainer()
	{};
	/** @brief constructor
	*/
	explicit matrixDataContainer(matrixData<Y> &input)
	{
		setArray(&input);
	};

	void clear() override
	{
		ad->clear();
	};

	void assign(index_t row, index_t col, Y num) override = 0;

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

	matrixElement<Y> element(index_t N) const override
	{
		return ad->element(N);
	 }

	void compact() override
	{
		ad->compact();
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
	virtual void setArray(matrixData<Y> *newAd)
	{
		ad = newAd;
		matrixData<Y>::colLim = ad->colLimit();
		matrixData<Y>::rowLim = ad->rowLimit();
	}
	/** set the matrixData object to translate to
	@param[in] newAd  the new matrixData object
	*/
	virtual void setArray(matrixData<Y> &newAd)
	{
		setArray(&newAd);
	}

};

#endif

