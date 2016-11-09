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

#ifndef _MATRIX_DATA_SPARSE_H_
#define _MATRIX_DATA_SPARSE_H_

#include "matrixData.h"
#include <vector>
#include <tuple>
#include <cassert>
#include <cmath>
#include <algorithm>


/**
* class for storing data from the Jacobian computation
*/

const int adRow = 0;
const int adCol = 1;
const int adVal = 2;
/** @brief class implementing an expandable sparse matrix geared for Jacobian entries*/
template <class Y=double>
class matrixDataSparse: public matrixData<Y>
{
public:
	typedef std::tuple<index_t, index_t, Y> cLoc;
private:
	std::vector<cLoc> data;         //!< the vector of tuples containing the data			
	count_t sortCount = 0;			//!< count of the last sort operation
	decltype(data.begin()) cptr; //!< ptr to the beginning of the sequence;
protected:
	static bool compareLocRow(cLoc A, cLoc B)
	{
		bool ans;
		if (std::get<adRow>(A) < std::get<adRow>(B))
		{
			ans = true;
		}
		else if (std::get<adRow>(A) > std::get<adRow>(B))
		{
			ans = false;
		}
		else       //A(0)==B(0)  so check the column
		{
			ans = (std::get<adCol>(A) < std::get<adCol>(B)) ? true : false;
		}
		return ans;
	}

	static bool compareLocCol(cLoc A, cLoc B)
	{
		bool ans;
		if (std::get<adCol>(A) < std::get<adCol>(B))
		{
			ans = true;
		}
		else if (std::get<adCol>(A) > std::get<adCol>(B))
		{
			ans = false;
		}
		else       //A(1)==B(1)  so check the row
		{
			ans = (std::get<adRow>(A) < std::get<adRow>(B)) ? true : false;
		}
		return ans;
	}

	static bool compareLocRowQuick(cLoc A, cLoc B)
	{
		return (std::get<adRow>(A) < std::get<adRow>(B));
	}

	static bool compareLocColQuick(cLoc A, cLoc B)
	{
		return (std::get<adCol>(A) < std::get<adCol>(B));
	}
public:	
	/** @brief constructor 
	@param[in] startCount  the number of elements to allocate space for initially
	*/
	explicit matrixDataSparse(index_t startCount = 50)
	{
		data.reserve(startCount);
	}
	/**
	* function to clear the data
	*/
	void clear() override
	{
		data.clear();
	}
	void assign(index_t row, index_t col, Y num) override
	{
		assert(row != ((index_t)(-1)));
		assert(row < matrixData<Y>::rowLim);
		assert(col < matrixData<Y>::colLim);
		assert(std::isfinite(num));

		//data.push_back (cLoc (X, Y, num));
		data.emplace_back(row, col, num);
	}

	/**
	* @brief reserve space for the count of the Jacobian elements
	* @param[in] reserveSize the amount of space to reserve
	*/
	void reserve(count_t reserveSize) override
	{
		data.reserve(reserveSize);
	}
	count_t size() const override
	{
		return static_cast<count_t>(data.size());
	}

	count_t capacity() const override
	{
		return static_cast<count_t> (data.capacity());
	}

	/**
	* sort the index based first on column number then column number
	*/
	void sortIndex()
	{
		std::sort(data.begin(), data.end(), compareLocCol);
		sortCount = static_cast<count_t>(data.size());
	}
	/**
	* sort the index based first on column number then column number
	*/
	void sortIndexCol()
	{
		std::sort(data.begin(), data.end(), compareLocCol);
		sortCount = static_cast<count_t>(data.size());
	}

	/**
	* @brief sort the index based first on row number then row number
	*/
	void sortIndexRow()
	{
		std::sort(data.begin(), data.end(), compareLocRow);
		sortCount = static_cast<count_t>(data.size());
	}
	/**
	* @brief compact the index merging values with the same row and column number together
	*/
	void compact() override
	{
		if (data.empty())
		{
			return;
		}
		if (!isSorted())
		{
			sortIndex();
		}
		auto currentDataLocation = data.begin();
		auto testDataLocation = currentDataLocation + 1;
		auto dataEnd = data.end();
		while (testDataLocation != dataEnd)
		{
			//Check if the next is equal to the previous in location 
			//if they are add them if not shift the new one to the right location and move on
			if ((std::get<adCol>(*testDataLocation) == std::get<adCol>(*currentDataLocation)) && (std::get<adRow>(*testDataLocation) == std::get<adRow>(*currentDataLocation)))
			{
				std::get<adVal>(*currentDataLocation) += std::get<adVal>(*testDataLocation);

			}
			else
			{
				++currentDataLocation;
				*currentDataLocation = *testDataLocation;
			}
			++testDataLocation;
		}
		//reduce the size and indicate that we are still sorted. 

		data.resize(++currentDataLocation - data.begin());
		sortCount = static_cast<count_t>(data.size());

	}
	/**
	* @brief get the row value
	* @param[in] N the element number to return
	* @return the row of the corresponding index
	*/
	index_t rowIndex(index_t N) const override
	{
		return std::get<adRow>(data[N]);
	}
	/**
	* @brief get the column value
	* @param[in] N the element number to return
	* @return the column of the corresponding index
	*/
	index_t colIndex(index_t N) const override
	{
		return std::get<adCol>(data[N]);
	}
	/**
	* @brief get the value
	* @param[in] N the element number to return
	* @return the value of the corresponding index
	*/
	Y val(index_t N) const override
	{
		return std::get<adVal>(data[N]);
	}

	virtual matrixIterator<Y> begin() const override
	{
		return matrixIterator<Y>(new matrixIteratorSparse(this, 0));
	}

	virtual matrixIterator<Y> end() const override
	{
		return matrixIterator<Y>(new matrixIteratorSparse(this, size()));
	}

	void start() override
	{
		cptr = data.begin();
	}

	matrixElement<Y> next() override
	{
		matrixElement<Y> tp{ std::get<adRow>(*cptr), std::get<adCol>(*cptr), std::get<adVal>(*cptr) };
		++cptr;
		return tp;
	}

	bool moreData() override
	{
		return (cptr != data.end());
	}

	/**
	* @brief get the number nonzero of elements in each row
	* @return a vector of the column counts
	*/
	std::vector<count_t> columnCount()
	{
		if (!isSorted())
		{
			sortIndexCol();
		}
		auto dataEnd = data.end();
		std::vector<count_t> colCount(std::get<adRow>(*(dataEnd - 1)), 0);
		count_t cnt = 1;

		index_t testRow = std::get<adRow>(data.front());
		for (auto testData = data.begin(); testData != dataEnd; ++testData)
		{
			if (testRow != std::get<adRow>(*testData))
			{
				colCount[testRow] = cnt;
				cnt = 0;
				testRow = std::get<adRow>(*testData);
			}
			++cnt;
		}
		colCount[testRow] = cnt;
		return colCount;

	}
	/** @brief check if the sparse array is sorted
	@return bool indicating sorted status
	*/
	bool isSorted() const
	{
		return (sortCount == static_cast<count_t>(data.size()));
	}

	Y at(index_t rowN, index_t colN) const override
	{
		if (isSorted())
		{
			auto res = std::lower_bound(data.begin(), data.end(), cLoc(rowN, colN, 0.0), compareLocCol);
			if (res == data.end())
			{
				return Y(0);
			}
			if ((std::get<adRow>(*res) == rowN) && (std::get<adCol>(*res) == colN))
			{
				return std::get<adVal>(*res);
			}
			else
			{
				return Y(0);
			}
		}
		else
		{
			for (const auto &rv : data)
			{
				if ((std::get<adRow>(rv) == rowN) && (std::get<adCol>(rv) == colN))
				{
					return std::get<adVal>(rv);
				}
			}
			return Y(0);
		}

	}

	/** @brief scale a subset of the elements 
	@param[in] factor the scaling factor
	@param[in] start the starting index
	@param[in] count the number of elements to scale
	*/
	void scale(Y factor, index_t startIndex = 0, count_t count = 0x0FFFFFFFF)
	{
		if (startIndex >= data.size())
		{
			return;
		}
		auto res = data.begin() + startIndex;
		auto term = data.end();
		if (count < data.size())
		{
			term = data.begin() + std::min(startIndex + count, static_cast<count_t> (data.size()));
		}
		while (res != term)
		{
			std::get<adVal>(*res) *= factor;
			++res;
		}
	}
	/** @brief scale all the elements of a particular row
	@param[in] row the row to scale
	@param[in] factor the scaling factor
	*/
	void scaleRow(index_t row, Y factor)
	{
		auto res = data.begin();
		auto term = data.end();
		while (res != term)
		{
			if (std::get<adRow>(*res) == row)
			{
				std::get<adVal>(*res) *= factor;
			}
			++res;
		}
	}
	/** @brief scale all the elements of a particular column
	@param[in] col the row to scale
	@param[in] factor the scaling factor
	*/
	void scaleCol(index_t col, Y factor)
	{
		auto res = data.begin();
		auto term = data.end();
		while (res != term)
		{
			if (std::get<adCol>(*res) == col)
			{
				std::get<adVal>(*res) *= factor;
			}
			++res;
		}
	}
	/** @brief translate all the elements in a particular row to some other row
	@param[in] origRow the row to change
	@param[in] newRow the row to change origRow into
	*/
	void translateRow(index_t origRow, index_t newRow)
	{
		auto res = data.begin();
		auto term = data.end();
		while (res != term)
		{
			if (std::get<adRow>(*res) == origRow)
			{
				std::get<adRow>(*res) = newRow;
			}
			++res;
		}
	}
	/** @brief translate all the elements in a particular column to some other column
	@param[in] origCol the column to change
	@param[in] newCol the column to change origCol into
	*/
	void translateCol(index_t origCol, index_t newCol)
	{
		auto res = data.begin();
		auto term = data.end();
		while (res != term)
		{
			if (std::get<adCol>(*res) == origCol)
			{
				std::get<adCol>(*res) = newCol;
			}
			++res;
		}
	}
	using matrixData<Y>::copyTranslateRow;

	/** @brief translate all the elements in a particular row in a2 and translate row origRow to newRow
	@param[in] a2  the matrixDataSparse object to copy from
	@param[in] origRow the column to change
	@param[in] newRow the column to change origRow into
	*/
	void copyTranslateRow(matrixDataSparse<Y> *a2, index_t origRow, index_t newRow)
	{
		auto res = a2->data.begin();
		auto term = a2->data.end();

		while (res != term)
		{
			if (std::get<adRow>(*res) == origRow)
			{
				data.emplace_back(newRow, std::get<adCol>(*res), std::get<adVal>(*res));
			}
			++res;
		}
	}
	/** @brief translate all the elements in a particular column in a2 and translate column origCol to newCol
	@param[in] a2  the matrixDataSparse object to copy from
	@param[in] origCol the column to change
	@param[in] newCol the column to change origCol into
	*/
	void copyTranslateCol(matrixDataSparse<Y> *a2, index_t origCol, index_t newCol)
	{
		auto res = a2->data.begin();
		auto term = a2->data.end();

		while (res != term)
		{
			if (std::get<adCol>(*res) == origCol)
			{
				data.emplace_back(std::get<adRow>(*res), newCol, std::get<adVal>(*res));
			}
			++res;
		}
	}

	/** @brief copy all data from a2 to the array and translate origCol into a set of new indices and scale factors
	  this function is useful when the input to some object is a summation of other states,  this function allows the translation into a large 
	sparse data object
	@param[in] a2  the matrixDataSparse object to copy from
	@param[in] origCol the column to change
	@param[in] newIndices a vector of indices to change
	@param[in] mult the scaler multiplier for each fo the new indices
	*/
	void copyReplicate(matrixDataSparse<Y> *a2, index_t origCol, std::vector<index_t> newIndices, std::vector<Y> mult)
	{
		auto res = a2->data.begin();
		auto term = a2->data.end();

		while (res != term)
		{
			if (std::get<adCol>(*res) == origCol)
			{
				for (index_t nn = 0; nn<newIndices.size(); ++nn)
				{
					//data.push_back(cLoc(std::get<adRow>(*res), newIndices[nn], std::get<adVal>(*res)*mult[nn]));
					data.emplace_back(std::get<adRow>(*res), newIndices[nn], std::get<adVal>(*res)*mult[nn]);
				}
			}
			else
			{
				data.push_back(*res);
			}
			++res;
		}
	}
	/** @brief remove invalid rows or those given by the testrow
	@param[in] rowTest,  the row index to remove*/
	void filter(index_t rowTest = (index_t)(-1))
	{
		if (data.empty())
		{
			return;
		}
		auto dvb = data.begin();
		auto dv2 = dvb;
		auto dvend = data.end();
		while (dv2 != dvend)
		{
			index_t row = std::get<adRow>(*dv2);
			index_t col = std::get<adCol>(*dv2);
			if ((row < matrixData<Y>::rowLim) && (row != rowTest))
			{
				if (col < matrixData<Y>::colLim)
				{
					*dvb = *dv2;
					++dvb;
				}
			}
			++dv2;
		}
		data.resize(dvb - data.begin());
	}

	void cascade(matrixDataSparse<Y> *a2, index_t element)
	{
		auto term = data.size();
		size_t nn = 0;
		Y keyval = 0;
		while (nn != term)
		{
			if (std::get<adCol>(data[nn]) == element)
			{
				size_t mm = 0;
				keyval = std::get<adVal>(data[nn]);
				for (size_t kk = 0; kk < a2->data.size(); kk++)
				{
					if (std::get<adRow>(a2->data[kk]) == element)
					{
						if (mm == 0)
						{
							std::get<adCol>(data[nn]) = std::get<adCol>(a2->data[kk]);
							std::get<adVal>(data[nn]) = std::get<adVal>(a2->data[kk]) * keyval;
							++mm;
						}
						else
						{
							//data.push_back (cLoc (std::get<adRow> (data[nn]), std::get<adCol> (a2->data[kk]), keyval * std::get<adVal> (a2->data[kk])));
							data.emplace_back(std::get<adRow>(data[nn]), std::get<adCol>(a2->data[kk]), keyval * std::get<adVal>(a2->data[kk]));
							++mm;
						}
					}
				}
			}
			++nn;
		}
	}
	using matrixData<Y>::merge;
	void merge(matrixDataSparse<Y> *a2)
	{
		data.insert(data.end(), a2->data.begin(), a2->data.end());
	}

	void transpose()
	{
		for (auto &element:data)
		{
			std::swap(std::get<adCol>(element), std::get<adRow>(element));
		//	int t1 = std::get<adCol>(data[kk]);
		//	std::get<adCol>(data[kk]) = std::get<adRow>(data[kk]);
		//	std::get<adRow>(data[kk]) = t1;
		}
	}
	void diagMultiply(std::vector<Y> diag)
	{
		for (size_t kk = 0; kk < data.size(); kk++)
		{
			std::get<adVal>(data[kk]) *= diag[std::get < adCol >(data[kk])];
		}
	}

	std::vector<Y> vectorMult(std::vector<Y> V)
	{
		sortIndexRow();
		auto maxRow = std::get<adRow>(data.back());
		std::vector<Y> out(maxRow, 0);
		auto res = data.begin();
		auto term = data.end();
		for (index_t nn = 0; nn <= maxRow; ++nn)
		{
			while ((res != term) && (std::get<adRow>(*res) == nn))
			{
				out[nn] += std::get<adVal>(*res) * V[std::get < adCol >(*res)];
				++res;
			}
		}
		return out;
	}
protected:
	class matrixIteratorSparse :public matrixIteratorActual<Y>
	{
	public:
		explicit matrixIteratorSparse(const matrixDataSparse<Y> *matrixData, index_t start = 0) :matrixIteratorActual<Y>(matrixData, start), mDS(matrixData)
		{
			if (start == 0)
			{
				cptr = mDS->data.begin();
			}
			else if (start<mDS->size())
			{
				cptr = mDS->data.begin() + start;
			}
			else
			{
				cptr = mDS->data.end();
			}

		}
		matrixIteratorSparse(const matrixIteratorSparse *it2) :matrixIteratorActual<Y>(it2->mDS), mDS(it2->mDS)
		{
			cptr = it2->cptr;
		}

		virtual matrixIteratorActual<Y> *clone() const override
		{
			return new matrixIteratorSparse(this);
		}

		virtual void increment() override
		{
			matrixIteratorActual<Y>::increment();
			++cptr;
		}


		virtual matrixElement<Y> operator*() const override
		{
			return{ std::get<adRow>(*cptr), std::get<adCol>(*cptr), std::get<adVal>(*cptr) };
		}
	private:
		const matrixDataSparse<Y> *mDS = nullptr;
		decltype(mDS->data.begin()) cptr; //!< ptr to the beginning of the sequence;	

	};
};

template<class Y>
std::vector<index_t> findMissing(matrixDataSparse<Y> *ad)
{
	std::vector<index_t> missing;
	ad->sortIndexCol();
	ad->compact();
	ad->sortIndexRow();
	index_t pp = 0;
	bool good = false;
	for (index_t kk = 0; kk < ad->rowLimit(); ++kk)
	{
		good = false;
		while ((pp < ad->size()) && (ad->rowIndex(pp) <= kk))
		{
			if ((ad->rowIndex(pp) == kk) && (std::isnormal(ad->val(pp))))
			{
				good = true;
				++pp;
				break;
			}

			++pp;
			if (pp >= ad->size())
			{
				break;
			}
		}
		if (!good)
		{
			missing.push_back(kk);
		}
	}
	return missing;
}

template <class Y>
std::vector<std::vector<index_t>> findRank(matrixDataSparse<Y> *ad)
{
	std::vector<index_t> vr, vt;
	std::vector<Y> vq, vtq;
	std::vector<std::vector<index_t>> mrows;
	ad->sortIndexCol();
	ad->compact();
	ad->sortIndexRow();
	Y factor = 0;
	index_t pp = 0;
	index_t qq = 0;
	auto mp = ad->size();
	bool good = false;
	for (index_t kk = 0; kk < ad->rowLimit() - 1; ++kk)
	{
		vr.clear();
		vq.clear();
		while (ad->rowIndex(pp) == kk)
		{
			vr.push_back(ad->colIndex(pp));
			vq.push_back(ad->val(pp));
			++pp;
		}
		qq = pp;
		for (index_t nn = kk + 1; nn < ad->rowLimit(); ++nn)
		{
			vt.clear();
			vtq.clear();
			good = false;
			if (ad->colIndex(qq) != vr[0])
			{
				good = true;
			}
			while (ad->rowIndex(qq) == nn)
			{

				if (!good)
				{
					vt.push_back(ad->colIndex(qq));
					vtq.push_back(ad->val(qq));
				}
				++qq;
				if (qq >= mp)
				{
					break;
				}
			}
			if (!good)
			{
				continue;
			}
			if (vr.size() != vtq.size())
			{
				continue;
			}

			for (size_t jj = 0; jj < vr.size(); ++jj)
			{
				if (vt[jj] != vr[jj])
				{
					good = true;
					break;
				}
				else if (jj == 0)
				{
					factor = vtq[jj] / vq[jj];
				}
				else if (std::abs(vtq[jj] / vq[jj] - factor) > 0.000001)
				{
					good = true;
					break;
				}
			}
			if (good)
			{
				continue;
			}
			else
			{
				mrows.push_back({ kk,nn });
			}
		}
	}
	return mrows;
}

#endif
