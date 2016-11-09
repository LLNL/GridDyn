#pragma once

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

#ifndef _MATRIX_DATA_ORDERED_H_
#define _MATRIX_DATA_ORDERED_H_

#include "matrixData.h"
#include <vector>
#include <utility>


/** @brief class implementing an expandable sparse matrix with an expandable data vector for each row
 *also adding a function to get all the data in a particular row.  
*/
template <class Y = double>
class matrixDataRowOrdered : public matrixData<Y>
{
private:
	std::vector<std::vector<std::pair<index_t,Y>>> dVec; //!< vector of data vectors for each Row		

	decltype(dVec[0].begin()) cptr; //!< ptr to the beginning of the sequence;
	decltype(dVec[0].end()) iend; //!< ptr to the end of the current sequence;
	index_t ci = 0;			//!< indicator of which vector of the array we are sequencing on;
public:
	/** @brief default constructor*/
	matrixDataRowOrdered()
	{
		
	}
	/** @brief constructor
	@param[in] rows  the number of elements to reserve
	*/
	explicit matrixDataRowOrdered(index_t rows):matrixData<Y>(rows,rows)
	{
		dVec.resize(rows);
	};
	void clear() override
	{
		for (auto &dvk : dVec)
		{
			dvk.clear();
		}
	};
	

	void assign(index_t row, index_t col, Y num) override
	{
		auto iI = dVec[row].begin();
		auto iEnd = dVec[row].end();
		while (iI!=iEnd)
		{
			if (iI->first==col)
			{
				iI->second += num;
				break;
			}
			if (iI->first>col)
			{
				dVec[row].insert(iI, std::make_pair(col, num));
				break;
			}
			++iI;
		}
		if (iI==iEnd)
		{
			dVec[row].emplace_back(col, num);
		}
	}

	virtual void setRowLimit(index_t limit) override
	{
		matrixData<Y>::rowLim = limit;
		dVec.resize(limit);
	}
	virtual void reserve(count_t reserveSize) override
	{
		for (auto &dvk : dVec)
		{
			dvk.reserve(3*(reserveSize/ matrixData<Y>::rowLim));
		}
	}
	virtual count_t size() const override
	{
		count_t sz = 0;
		for (auto &dvk : dVec)
		{
			sz += static_cast<count_t>(dvk.size());
		}
		return sz;
	}

	virtual count_t capacity() const override
	{
		count_t sz = 0;
		for (auto &dvk : dVec)
		{
			sz += static_cast<count_t>(dvk.capacity());
		}
		return sz;
	}
	
	virtual index_t rowIndex(index_t N) const override
	{
		index_t ii = 0;
		size_t sz2 = dVec[0].size();
		while (N >= sz2)
		{
			++ii;
			sz2 += dVec[ii].size();
		}
		return ii;

	}
	virtual index_t colIndex(index_t N) const override
	{
		index_t ii = 0;
		size_t sz1 = 0;
		size_t sz2 = dVec[0].size();
		while (N >= sz2)
		{
			sz1 += dVec[ii].size();
			++ii;
			sz2 += dVec[ii].size();
		}
		return (dVec[ii][N - sz1].first);
	}

	virtual Y val(index_t N) const override
	{
		index_t ii = 0;
		size_t sz1 = 0;
		size_t sz2 = dVec[0].size();
		while (N >= sz2)
		{
			sz1 += dVec[ii].size();
			++ii;
			sz2 += dVec[ii].size();
		}
		return (dVec[ii][N - sz1].second);
	}

	virtual void start() override
	{
		ci = 0;
		while (dVec[ci].empty())
		{
			++ci;
			if (ci == matrixData<Y>::rowLim)
			{
				cptr = dVec[ci-1].begin();
				iend = dVec[ci-1].end();
				break;
			}
		}
		if (ci != matrixData<Y>::rowLim)
		{
			cptr = dVec[ci].begin();
			iend = dVec[ci].end();
		}
		
	}

	matrixElement<Y> next() override
	{
		matrixElement<Y> tp{ ci,cptr->first, cptr->second };
		++cptr;
		if (cptr == iend)
		{
			++ci;
			if (ci < matrixData<Y>::rowLim)
			{
				while (dVec[ci].empty())
				{
					++ci;
					if (ci == matrixData<Y>::rowLim)
					{
						cptr = dVec[ci].begin();
						iend = dVec[ci].end();
						break;
					}
				}
				if (ci<matrixData<Y>::rowLim)
				{
					cptr = dVec[ci].begin();
					iend = dVec[ci].end();
				}
				
			}
		}
		return tp;
	}

	bool moreData() override
	{
		return (ci < matrixData<Y>::rowLim);
	}


	Y at(index_t rowN, index_t colN) const override
	{
		for (auto &de:dVec[rowN])
		{
			if (de.first==colN)
			{
				return de.second;
			}
			else if (de.first>colN)
			{
				return Y(0);
			}
		}
		return Y(0);
	}

	const std::vector<std::pair<index_t, Y>> &getRow(index_t rowN) const
	{
		return dVec[rowN];
	}
};


#endif