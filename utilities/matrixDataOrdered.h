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

#ifndef _MATRIX_DATA_ORDERED_H_
#define _MATRIX_DATA_ORDERED_H_

#include "matrixData.h"
#include "matrixDataOrdering.h"
#include <vector>



/** @brief class implementing an expandable sparse matrix with an expandable data vector for each row
 *also adding a function to get all the data in a particular row.  
*/
template <sparse_ordering M=sparse_ordering::row_ordered, class Y = double>
class matrixDataOrdered : public matrixData<Y>
{
private:
	std::vector<std::vector<std::pair<index_t,Y>>> dVec; //!< vector of data vectors for each Row		

	decltype(dVec[0].cbegin()) cptr; //!< ptr to the beginning of the sequence;
	decltype(dVec[0].cend()) iend; //!< ptr to the end of the current sequence;
	index_t ci = 0;			//!< indicator of which vector of the array we are sequencing on;
	count_t count = 0;
	count_t primary_max = 0; //!< the limit on the primary key
public:
	/** @brief default constructor*/
	matrixDataOrdered()
	{
		
	}
	/** @brief constructor
	@param[in] rows  the number of elements to reserve
	*/
	explicit matrixDataOrdered(index_t N):matrixData<Y>(N,N)
	{
		dVec.resize(N);
	};
	void clear() override
	{
		count = 0;
		for (auto &dvk : dVec)
		{
			dvk.clear();
		}
	};
	

	void assign(index_t row, index_t col, Y num) override
	{
		auto key = keyOrder<index_t, M>::order(row, col);
		auto iI = dVec[key.first].begin();
		auto iEnd = dVec[key.first].end();
		while (iI!=iEnd)
		{
			if (iI->first== key.second)
			{
				iI->second += num;
				break;
			}
			if (iI->first>key.second)
			{
				dVec[key.first].insert(iI, std::make_pair(key.second, num));
				++count;
				break;
			}
			++iI;
		}
		if (iI==iEnd)
		{
			dVec[key.first].emplace_back(key.second, num);
			++count;
		}
	}

	virtual void setRowLimit(index_t limit) override
	{
		matrixData<Y>::rowLim = limit;
		primary_max = keyOrder<index_t, M>::primary(matrixData<Y>::rowLim, matrixData<Y>::colLim);
		dVec.resize(primary_max);
	}
	virtual void setColLimit(index_t limit) override
	{
		matrixData<Y>::colLim = limit;
		primary_max = keyOrder<index_t, M>::primary(matrixData<Y>::rowLim, matrixData<Y>::colLim);
		dVec.resize(primary_max);

	}
	virtual void reserve(count_t reserveSize) override
	{
		auto rcount = 3 * (reserveSize / dVec.size());
		for (auto &dvk : dVec)
		{
			dvk.reserve(rcount);
		}
	}
	virtual count_t size() const override
	{
		return count;
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
	matrixElement<Y> element(index_t N) const override
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
		auto res = keyOrder<index_t, M>::order(ii, dVec[ii][N - sz1].first);
		return{ res.first,res.second,dVec[ii][N - sz1].second };
	}

	virtual void start() override
	{
		ci = 0;
		while (dVec[ci].empty())
		{
			++ci;
			if (ci == primary_max)
			{
				cptr = dVec[ci-1].cbegin();
				iend = dVec[ci-1].cend();
				break;
			}
		}
		if (ci != primary_max)
		{
			cptr = dVec[ci].cbegin();
			iend = dVec[ci].cend();
		}
		
	}

	matrixElement<Y> next() override
	{
		auto res = keyOrder<index_t, M>::order(ci, cptr->first);
		matrixElement<Y> tp{ res.first,res.second,cptr->second };
		++cptr;
		if (cptr == iend)
		{
			++ci;
			if (ci < primary_max)
			{
				while (dVec[ci].empty())
				{
					++ci;
					if (ci == primary_max)
					{
						cptr = dVec[ci].cbegin();
						iend = dVec[ci].cend();
						break;
					}
				}
				if (ci<primary_max)
				{
					cptr = dVec[ci].cbegin();
					iend = dVec[ci].cend();
				}
				
			}
		}
		return tp;
	}

	bool moreData() override
	{
		return (ci < primary_max);
	}


	Y at(index_t rowN, index_t colN) const override
	{
		auto key = keyOrder<index_t, M>::order(rowN, colN);
		for (auto &de:dVec[key.first])
		{
			if (de.first==key.second)
			{
				return de.second;
			}
			else if (de.first>key.second)
			{
				return Y(0);
			}
		}
		return Y(0);
	}

	const std::vector<std::pair<index_t, Y>> &getSet(index_t N) const
	{
		return dVec[N];
	}
};


#endif