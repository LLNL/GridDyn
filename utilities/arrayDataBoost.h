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

#ifndef _ARRAY_DATA_BOOST_H_
#define _ARRAY_DATA_BOOST_H_

#include "arrayData.h"
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <cstdio>

/** @brief class implementing an expandable sparse matrix based on the boost matrices geared for jacobaian entries*/
template< class type>
class arrayDataBoost : public arrayData
{
public:
	boost::numeric::ublas::mapped_matrix<type, boost::numeric::ublas::column_major> m; //!<boost matrix to interface 
public:
	/** @brief constructor
	@param[in] R  number of rows
	@param[in] C  number of columns
	*/
	arrayDataBoost(count_t R, count_t C):arrayData(R,C),m(R,C) {};
	/**
	* function to clear the data
	*/
	void clear() override
	{
		m.clear();
	}

	void assign(index_t X, index_t Y, const double num) override
	{
		m(X, Y) += num;
		//printf("insert [%d,%d]=%f\n", X, Y, num);
	}
	
	count_t size() const override
	{
		return static_cast<count_t>(m.nnz());
	}
	count_t capacity() const override
	{
		return rows*cols;
	}
	/**
	* @brief don't use this function
	*/
	index_t rowIndex(index_t N) const override
	{
		return 0;
	}
	/**
	* @brief don't use this function
	*/
	index_t colIndex(index_t N) const override
	{
		return 0;
	}
	/**
	* @brief don't use this function
	*/
	double val(index_t N) const override
	{
		return 0;
	}

	/**
	* @brief don't use this function
	*/
	data_triple start() override
	{
		return{ rowLimit,colLimit,0 };
	}
	/**
	* @brief don't use this function
	*/
	data_triple next() override
	{
		return{ rowLimit,colLimit,0 };
	}

	double at(index_t rowN, index_t colN) const override
	{
		return m(rowN, colN);
	}
	
};

#endif

