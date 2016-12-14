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

#include "matrixDataContainer.h"

/** @brief class implementation for a scaling array data
 all data is multiplied by a factor before being sent to the underlying matrixData object
*/
template<class Y = double>
class matrixDataScale : public matrixDataContainer<Y>
{
private:
	Y scalingFactor;
public:
	/** @brief constructor
	*/
	matrixDataScale(matrixData<Y> &input, Y scaleFactor):matrixDataContainer(input),scalingFactor(scaleFactor)
	{
	};

	void assign(index_t row, index_t col, Y num) override
	{
		ad->assign(row, col, num*scalingFactor);
	};
	/** @brief set the scale factor for the array
	@param[in] scaleFactor  the input row to translate
	*/
	void setScale(Y scaleFactor)
	{
		scalingFactor = scaleFactor;
	}
};

#endif

