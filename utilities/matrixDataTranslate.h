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

#ifndef _MATRIX_DATA_TRANSLATE_H_
#define _MATRIX_DATA_TRANSLATE_H_

#include "matrixDataContainer.h"
#include <array>

/** @brief class implementation translation for another matrixData object
 most function are just simple forwarding to the underlying matrixData object
except the assign and at operator which basically means the matrixData can interact with a small subset of a bigger matrixData object
though rowIndex, colIndex, and val will still return the original values.  The intent of this class is not to replace the interactions with another
it is to act as a filter in cases where elements need to be added but the row needs a translation,  using it outside that purpose could lead to issues
*/
template< int CT, class Y=double>
class matrixDataTranslate : public matrixDataContainer<Y>
{
private:
	std::array<index_t, CT> Trow;         //!< the vector of translations
public:
	/** @brief constructor
	*/
	matrixDataTranslate() 
	{
		Trow.fill((index_t)(-1));
		//setRowLimit(CT);
	};

	void assign(index_t row, index_t col, Y num) override
	{
		//for this to work the assignment must be from a small number to some other index
		//and we do automatic checking of the translation and if it isn't valid don't do the assignment
		if ((row< CT) && (Trow[row] < matrixData<Y>::rowLim))
		{
			ad->assign(Trow[row], col, num);
		}
	};

	Y at(index_t rowN, index_t colN) const override
	{
		if ((rowN < CT) && (Trow[rowN] < matrixData<Y>::rowLim))
		{
			return ad->at(Trow[rowN], colN);
		}
		else
		{
			return Y(0);
		}
	};
	
	/** set the translation array
	@param[in] input  the input row to translate
	@param[in] output the rowIndex to that input should be translated to
	*/
	void setTranslation(index_t input, index_t output)
	{
		if (input < CT)
		{
			Trow[input] = output;
		}
	}
};

#endif
