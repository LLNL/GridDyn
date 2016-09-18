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

#ifndef _ARRAY_DATA_SPARSE_H_
#define _ARRAY_DATA_SPARSE_H_

#include "arrayData.h"
#include <vector>
#include <tuple>

typedef std::tuple<index_t, index_t, double> cLoc;

/**
* class for storing data from the Jacobian computation
*/

const int adRow = 0;
const int adCol = 1;
const int adVal = 2;
/** @brief class implementing an expandable sparse matrix geared for jacobaian entries*/
class arrayDataSparse: public arrayData<double>
{
private:
	std::vector<cLoc> data;         //!< the vector of tuples containing the data			
	count_t sortCount = 0;			//!< count of the last sort operation
	decltype(data.begin()) cptr; //!< ptr to the begining of the sequence;
public:	
	/** @brief constructor 
	@param[in] startCount  the number of elements to allocate space for initially
	*/
	arrayDataSparse(index_t startCount = 50);
	/**
	* function to clear the data
	*/
	void clear() override;
	void assign(index_t row, index_t col, double num) override;

	/**
	* @brief reserve space for the cound of the jacobians
	* @param[in] size the amount of space to reserve
	*/
	void reserve(count_t reserveSize) override
	{
		data.reserve(reserveSize);
	}
	count_t size() const override;

	count_t capacity() const override;
	/**
	* sort the index based first on column number then column number
	*/
	void sortIndex();
	/**
	* sort the index based first on column number then column number
	*/
	void sortIndexCol();
	/**
	* @brief sort the index based first on row number then row number
	*/
	void sortIndexRow();
	/**
	* @brief compact the index merging values with the same row and column number together
	*/
	void compact() override;
	/**
	* @brief get the row value
	* @param[in] N the element number to return
	* @return the row of the corresponding index
	*/
	index_t rowIndex(index_t N) const override;
	/**
	* @brief get the column value
	* @param[in] N the element number to return
	* @return the column of the corresponding index
	*/
	index_t colIndex(index_t N) const override;
	/**
	* @brief get the value
	* @param[in] N the element number to return
	* @return the value of the corresponding index
	*/
	double val(index_t N) const override;


	void start() override;

	data_triple<double> next() override;

	bool moreData() override;

	/**
	* @brief get the number nonzero of elements in each row
	* @return a vector of ints with the column counts
	*/
	std::vector<count_t> columnCount();
	/** @brief check if the sparse array is sorted
	@return bool indicating sorted status
	*/
	bool isSorted() const
	{
		return (sortCount == static_cast<count_t>(data.size()));
	}

	double at(index_t rowN, index_t colN) const override;
	/** @brief scale a subset of the elements 
	@param[in] factor the scaling factor
	@param[in] start the starting index
	@param[in] count the number of elements to scale
	*/
	void scale(double factor, index_t start = 0, count_t count = 0x0FFFFFFFF);
	/** @brief scale all the elements of a particular row
	@param[in] row the row to scale
	@param[in] factor the scaling factor
	*/
	void scaleRow(index_t row, double factor);
	/** @brief scale all the elements of a particular column
	@param[in] col the row to scale
	@param[in] factor the scaling factor
	*/
	void scaleCol(index_t col, double factor);
	/** @brief translate all the elements in a particular row to some other row
	@param[in] origRow the row to change
	@param[in] newRow the row to change origRow into
	*/
	void translateRow(index_t origRow, index_t newRow);
	/** @brief translate all the elements in a particular column to some other column
	@param[in] origCol the column to change
	@param[in] newCol the column to change origCol into
	*/
	void translateCol(index_t origCol, index_t newCol);
	using arrayData::copyTranslateRow;
	/** @brief translate all the elements in a particular row in a2 and translate row origRow to newRow
	@param[in] a2  the arrayDataSparse object to copy from
	@param[in] origRow the column to change
	@param[in] newRow the column to change origRow inot
	*/
	void copyTranslateRow(arrayDataSparse *a2, index_t origRow, index_t newRow);
	/** @brief translate all the elements in a particular column in a2 and translate column origCol to newCol
	@param[in] a2  the arrayDataSparse object to copy from
	@param[in] origCol the column to change
	@param[in] newCol the column to change origCol into
	*/
	void copyTranslateCol(arrayDataSparse *a2, index_t origCol, index_t newCol);
	/** @brief copy all data from a2 to the array and translate origCol into a set of new indices and scale factors
	  this function is useful when the input to some object is a summation of other states,  this function allows the translation into a large 
	sparse data object
	@param[in] a2  the arrayDataSparse object to copy from
	@param[in] origCol the column to change
	@param[in] newIndices a vector of indictes to change
	@param[in] mult the scaler multipler for each fo the new indices
	*/
	void copyReplicate(arrayDataSparse *a2, index_t origCol, std::vector<index_t> newIndices, std::vector<double> mult);
	/** @brief remove invalid rows or those given by the testrow
	@param[in] rowTest,  the row index to remove*/
	void filter(index_t rowTest = (index_t)(-1));
	void cascade(arrayDataSparse *a2, index_t element);
	using arrayData::merge;
	void merge(arrayDataSparse *a2);
	void saveFile(double time, const std::string &filename, bool append);
	void transpose();
	void diagMultiply(std::vector<double> diag);
	std::vector<double> vectorMult(std::vector<double> V);
};

std::vector<index_t> findMissing(arrayDataSparse *ad);
std::vector<std::vector<index_t>> findRank(arrayDataSparse *ad);

#endif
