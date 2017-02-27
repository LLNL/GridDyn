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
#pragma once
#ifndef _MATRIX_DATA_FILTER_H_
#define _MATRIX_DATA_FILTER_H_

#include "matrixDataContainer.h"
#include <vector>
#include <algorithm>

/** @brief class implementation filter for another matrixData object
most functions are just simple forwarding to the underlying matrixData object
except the assign operator, the row index is checked against a set of filtered rows
if it is not in the filtered list it is passed through to the underlying matrix
*/
template<class Y = double>
class matrixDataFilter : public matrixDataContainer<Y>
{
private:
	std::vector<index_t> rowFilter;         //!< the vector of translations
public:
	/** @brief constructor
	*/
	matrixDataFilter() = default;

	explicit matrixDataFilter(matrixData<Y> &input) :matrixDataContainer<Y>(input)
	{

	};

	void assign(index_t row, index_t col, Y num) override
	{
		bool found = std::binary_search(rowFilter.begin(), rowFilter.end(), row);
		if (!found)
		{
			matrixDataContainer<Y>::ad->assign(row, col, num);
		}
	};


	/** add a filter
	@param[in] row  the row to filter out
	*/
	void addFilter(index_t row)
	{
		auto lb = std::lower_bound(rowFilter.begin(), rowFilter.end(),row);
		if (lb == rowFilter.end())
		{
			rowFilter.push_back(row);
		}
		else if (*lb!=row)
		{ 
			rowFilter.insert(lb, row);
		}
	}

	/** add a vector of filters
	@param[in] row  the row to filter out
	*/
	void addFilter(std::vector<index_t> &rows)
	{
		rowFilter.reserve(rowFilter.size() + rows.size());
		rowFilter.insert(rowFilter.end(), rows.begin(), rows.end());
		std::sort(rowFilter.begin(), rowFilter.end());
	}
};

#endif

