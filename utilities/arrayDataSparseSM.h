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

#ifndef _ARRAY_DATA_SPARSESM_H_
#define _ARRAY_DATA_SPARSESM_H_

#include "arrayData.h"
#include <vector>
#include <utility>
#include <algorithm>
#include <array>
#include <iterator>
#include <type_traits>
#include <cassert>

/** @brief enumeration specifying sparse ordering patterns*/
enum class sparse_ordering
{
	row_ordered = 0,
	column_ordered = 1,
};

/** @brief simple class for compute the keyIndex from a row and column and inverting it*/
template <class X, sparse_ordering M>
class keyCompute
{//default is row ordered
public:
	inline X keyGen(index_t rowIndex, index_t colIndex) const
	{
		return ((static_cast<X>(rowIndex) << vshift) + static_cast<X>(colIndex));
	};

	inline index_t row(X key) const
	{
		return static_cast<index_t>(key >> vshift);
	};
	inline index_t col(X key) const
	{
		return static_cast<index_t>(key & mask);
	};
private:
	static const unsigned int vshift = 4 * sizeof(X);
	static const X mask = (X(1) << vshift) - 1;
};

template <class X>
class keyCompute<X,sparse_ordering::column_ordered>
{
public:
	inline X keyGen(index_t rowIndex, index_t colIndex) const
	{
		return ((static_cast<X>(colIndex) << vshift) + static_cast<X>(rowIndex));
	};

	inline index_t col(X key) const
	{
		return static_cast<index_t>(key >> vshift);
	};
	inline index_t row(X key) const
	{
		return static_cast<index_t>(key & mask);
	};
private:
	static const unsigned int vshift = 4 * sizeof(X);
	static const X mask = (X(1) << vshift) - 1;
};


/** @brief simple class for compute the blockIndex from a row and column and inverting it*/
template <count_t K, sparse_ordering M>
class blockCompute
{
public:
	inline index_t blockIndexGen(index_t row, index_t col) const
	{
		return (((M == sparse_ordering::row_ordered) ? row : col) + bias) >> kShift;
	};

	void setMaxIndex(index_t rowMax, index_t colMax)
	{
		index_t keyMax = (M == sparse_ordering::row_ordered) ? rowMax : colMax;
		index_t remaining = keyMax;
		int shiftCount = 0;
		while (remaining > 1) {
			remaining >>= 1;
			shiftCount++;
		}
		//set kShift to get 2^K bins from shifting the Y column
		kShift =shiftCount - K + 1;
		//figure out how many elements should be in the first and last buckets
		index_t extraElements = keyMax - (1 << kShift)*((1<<K)-2);
		bias = (1 << kShift)-(extraElements>>1);
	}
private:
	int kShift = 0;					//!< the shift factor used for column determination
	int bias = 0;					//!< the bias added to the column to even out the number of points in the first and last columns
};


/** @brief simple class for compute the blockIndex from a row and column and inverting it*/
template <sparse_ordering M>
class blockCompute<1,M>
{
public:
	inline index_t blockIndexGen(index_t row, index_t col) const
	{
		return (((M == sparse_ordering::row_ordered) ? row : col)>=split);
	};

	void setMaxIndex(index_t rowMax, index_t colMax)
	{
		index_t key = (M == sparse_ordering::row_ordered) ? rowMax : colMax;
		split = (key >> 1);
	}
private:
	index_t split = 0;					//!< the shift factor used for column determination

};


/** @brief class implementing an expandable sparse matrix with multiple column vectors
 sparseSMB implements a sparse matrix using a vector and not doing the sort until absolutely necessary
the vectors contains a single 64 bit unsigned number composed by using col<<32+row allowing support for up to 2^32-2 rows and columns
to make the sorting faster when done  
the template parameter K indicates that the structure should use 2^K vectors for data storage
the space allocated is approximately 2x that used in the single column structure
*/
template <count_t K, class X=std::uint32_t, class Y=double,sparse_ordering M=sparse_ordering::column_ordered>
class arrayDataSparseSMB : public arrayData<Y>
{
	/*class arrayIterator :public std::iterator<std::input_iterator_tag, double>
	{
	public:
		arrayIterator() {};
		virtual ~arrayIterator() {};
		virtual index_t row();
		virtual index_t col();
		virtual double val();
		virtual arrayIterator &operator++();
		virtual bool operator==(arrayIterator it2);
	private:
		index_t currN;
	};
	*/
	static_assert (std::is_integral<X>::value, "class X must be of integral type");
	static_assert (std::is_unsigned<X>::value, "class X must be of an unsigned type");
	typedef std::pair<X, Y> pLoc;
private:
	keyCompute<X, M> key_computer;    //!< object that generators the keys and extracts row and column information
	std::array<std::vector<pLoc>,(1<<K)> dVec;         //!< the vector of pairs containing the data			
	count_t sortCount = 0;			//!< count of the last sort operation
	blockCompute<K, M> block_computer;  //!< object that generates the appropriate block index;
	decltype(dVec[0].begin()) cptr; //!< ptr to the begining of the sequence;
	decltype(dVec[0].end()) iend; //!< ptr to the end of the current sequence;
	int ci = 0;						//!< indicator of which vector of the array we are sequencing on;
public:
	/** @brief constructor 
	  the actual storage space used is approx 2x startCount due to expected unevenness in the array
	@param[in] startCount  the number of elements to reseve
	*/
	arrayDataSparseSMB(index_t startCount = 1000)
	{
		for (auto &dvk : dVec)
		{
			dvk.reserve(startCount>>(K-1));
		}
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
		auto temp = block_computer.blockIndexGen(row, col);
		assert(temp < (1 << K));
		dVec[temp].emplace_back(key_computer.keyGen(row,col), num);

	}

	/**
	* @brief reserve space for the cound of the jacobians
	* @param[in] size the amount of space to reserve
	*/
	void reserve(count_t reserveSize) override
	{
		for (auto &dvk : dVec)
		{
			dvk.reserve(reserveSize >> (K-1));
		}
	}
	count_t size() const override
	{
		count_t sz = 0;
		for (auto &dvk : dVec)
		{
			sz+=static_cast<count_t>(dvk.size());
		}
		return sz;
	}
	
	void setColLimit(index_t lim) override
	{
		arrayData<Y>::colLim = lim;
		block_computer.setMaxIndex(arrayData<Y>::rowLim, lim);
	}

	void setRowLimit(index_t lim) override
	{
		arrayData<Y>::rowLim = lim;
		block_computer.setMaxIndex(lim,arrayData<Y>::colLim);
	}

	count_t capacity() const override
	{
		count_t sz = 0;
		for (auto &dvk : dVec)
		{
			sz += static_cast<count_t>(dvk.capacity());
		}
		return sz;
	}
	/**
	* @brief sort the index based first on column number then column number
	*/
	void sortIndex()
	{
		//std::sort(dVec.begin(), dVec.end(), compareLocSM);
		auto fp = [](pLoc A, pLoc B) {return (A.first < B.first); };
		for (auto &dvk : dVec)
		{
			std::sort(dvk.begin(), dvk.end(), fp);
		}
		sortCount = size();
	}
	/**
	* @brief sort the index based first on row number then row number
	*/
	
	void compact() override
	{
		if (!isSorted())
		{
			sortIndex();
		}
		for (auto &dvk : dVec)
		{
			if (dvk.empty())
			{
				return;
			}

			auto dvb = dvk.begin();
			auto dv2 = dvb;
			++dv2;
			auto dvend = dvk.end();
			while (dv2 != dvend)
			{
				if (dv2->first == dvb->first)
				{
					dvb->second += dv2->second;

				}
				else
				{
					++dvb;
					*dvb = *dv2;
				}
				++dv2;
			}
			dvk.resize(++dvb - dvk.begin());
		}
		sortCount = size();
	}

	index_t rowIndex(index_t N) const override
	{
		int ii = 0;
		size_t sz1 = 0;
		size_t sz2 = dVec[0].size();
		while (N >= sz2)
		{
			sz1 += dVec[ii].size();
			++ii;
			sz2+= dVec[ii].size();
		}
		assert(ii < (1 << K));
		return (key_computer.row(dVec[ii][N-sz1].first));

	}
	index_t colIndex(index_t N) const override
	{
		int ii = 0;
		size_t sz1 = 0;
		size_t sz2 = dVec[0].size();
		while (N >= sz2)
		{
			sz1 += dVec[ii].size();
			++ii;
			sz2 += dVec[ii].size();
		}
		assert(ii < (1 << K));
		return (key_computer.row(dVec[ii][N - sz1].first));
	}
	Y val(index_t N) const override
	{
		int ii = 0;
		size_t sz1 = 0;
		size_t sz2 = dVec[0].size();
		while (N >= sz2)
		{
			sz1 += dVec[ii].size();
			++ii;
			sz2 += dVec[ii].size();
		}
		assert(ii < (1 << K));
		return (dVec[ii][N - sz1].second);
	}

	void start() override
	{
		cptr = dVec[0].begin();
		iend = dVec[0].end();
		ci = 0;
	}

	data_triple<Y> next() override
	{
		data_triple<Y> tp{key_computer.row(cptr->first), key_computer.col(cptr->first), cptr->second };
		++cptr;
		if (cptr == iend)
		{
			++ci;
			if (ci < (1 << K))
			{
				cptr = dVec[ci].begin();
				iend = dVec[ci].end();
			}
		}
		return tp;
	}

	bool moreData() override
	{
		return (ci<(1 << K));
	}

	/** @brief check if the sparse array is sorted
	@return bool indicating sorted status
	*/
	bool isSorted() const
	{
		return (sortCount == size());
	}

	double at(index_t rowN, index_t colN) const override
	{
		X cmp = key_computer.keyGen(rowN, colN);
		int I = block_computer.blockIndexGen(rowN,colN);
		if (isSorted())
		{
			auto res = std::lower_bound(dVec[I].begin(), dVec[I].end(), std::make_pair(cmp, 0), [](pLoc A, pLoc B) {return (A.first < B.first); });
			if (res == dVec[I].end())
			{
				return 0.0;
			}
			if (res->first == cmp)
			{
				return res->second;
			}
			else
			{
				return 0.0;
			}
		}
		else
		{

			for (const auto &rv : dVec[I])
			{
				if (rv.first == cmp)
				{
					return rv.second;
				}
			}
			return 0.0;
		}
	}

};

/** @brief class implementing an expandable sparse matrix geared for jacobaian entries
 sparseSM implements a sparse matrix using a vector and not doing the sort until absolutely necessary
the vector contains a single 32 bit unsigned number composed by using col<<16+row to make the sorting faster when done
*/
template <class X,class Y,sparse_ordering M>
class arrayDataSparseSMB<0,X,Y,M> : public arrayData<Y>
{
	/*class arrayIterator :public std::iterator<std::input_iterator_tag, double>
	{
	public:
	arrayIterator() {};
	virtual ~arrayIterator() {};
	virtual index_t row();
	virtual index_t col();
	virtual double val();
	virtual arrayIterator &operator++();
	virtual bool operator==(arrayIterator it2);
	private:
	index_t currN;
	};
	*/
	static_assert (std::is_integral<X>::value, "class X must be of integral type");
	static_assert (std::is_unsigned<X>::value, "class X must be of an unsigned type");
private:
	typedef std::pair<X, Y> pLoc;
	std::vector<pLoc> dVec;         //!< the vector of pairs containing the data			
	count_t sortCount = 0;			//!< count of the last sort operation
	decltype(dVec.begin()) cptr; //!< ptr to the begining of the sequence;
	keyCompute<X, M> key_computer;    //!< object that generators the keys and extracts row and column information
	//static const unsigned int vshift = 4 * sizeof(X);
	//static const X mask = (X(1) << vshift) - 1;
public:
	/** @brief constructor
	  the actual storage space used is approx 2x startCount due to expected unevenness in the array
	@param[in] startCount  the number of elements to reseve
	*/
	arrayDataSparseSMB(index_t startCount = 1000)
	{
		dVec.reserve(startCount);
	};
	void clear() override
	{
		dVec.clear();
	};


	void assign(index_t row, index_t col, Y num) override
	{
		dVec.emplace_back(key_computer.keyGen(row,col), num);
	}

	/**
	* @brief reserve space for the cound of the jacobians
	* @param[in] size the amount of space to reserve
	*/
	void reserve(count_t reserveSize) override
	{
		dVec.reserve(reserveSize);
	}
	count_t size() const override
	{
		return static_cast<count_t>(dVec.size());
	}

	count_t capacity() const override
	{
		return static_cast<count_t>(dVec.capacity());
	}
	/**
	* @brief sort the index based first on column number then column number
	*/
	void sortIndex()
	{
		std::sort(dVec.begin(), dVec.end(), [](pLoc A, pLoc B) {return (A.first < B.first); });
	}
	/**
	* @brief sort the index based first on row number then row number
	*/

	void compact() override
	{
		if (dVec.empty())
		{
			return;
		}
		if (!isSorted())
		{
			sortIndex();
		}
		auto dvb = dVec.begin();
		auto dv2 = dvb;
		++dv2;
		auto dvend = dVec.end();
		while (dv2 != dvend)
		{
			if (dv2->first == dvb->first)
			{
				dvb->second += dv2->second;

			}
			else
			{
				++dvb;
				*dvb = *dv2;
			}
			++dv2;
		}
		dVec.resize(++dvb - dVec.begin());
		sortCount = size();

	}

	index_t rowIndex(index_t N) const override
	{
		return (key_computer.row(dVec[N].first));
	}

	index_t colIndex(index_t N) const override
	{
		return (key_computer.col(dVec[N].first));
	}

	Y val(index_t N) const override
	{
		return dVec[N].second;
	}

	void start() override
	{
		cptr = dVec.begin();
	}

	data_triple<Y> next() override
	{
		data_triple<Y> tp{ key_computer.row(cptr->first), key_computer.col(cptr->first), cptr->second };
		++cptr;
		return tp;
	}

	bool moreData() override
	{
		return (cptr != dVec.end());
	}

	/** @brief check if the sparse array is sorted
	@return bool indicating sorted status
	*/
	bool isSorted() const
	{
		return (sortCount == size());
	}

	Y at(index_t rowN, index_t colN) const override
	{
		index_t cmp = key_computer.keyGen(rowN,colN);
		if (isSorted())
		{
			auto res = std::lower_bound(dVec.begin(), dVec.end(), std::make_pair(cmp, 0), [](pLoc A, pLoc B) {return (A.first < B.first); });
			if (res == dVec.end())
			{
				return 0.0;
			}
			if (res->first == cmp)
			{
				return res->second;
			}
			else
			{
				return 0.0;
			}
		}
		else
		{

			for (const auto &rv : dVec)
			{
				if (rv.first == cmp)
				{
					return rv.second;
				}
			}
			return 0.0;
		}
	}

};

/** @brief class implementing an expandable sparse matrix with multiple column vectors
 sparseSMB implements a sparse matrix using a vector and not doing the sort until absolutely necessary
the vectors contains a single 64 bit unsigned number composed by using col<<32+row allowing support for up to 2^32-2 rows and columns
to make the sorting faster when done
the template parameter K indicates that the structure should use 2^K vectors for data storage
the space allocated is approximately 2x that used in the single column structure
*/
template <class X, class Y, sparse_ordering M>
class arrayDataSparseSMB<1,X,Y,M> : public arrayData<Y>
{
	/*class arrayIterator :public std::iterator<std::input_iterator_tag, double>
	{
	public:
	arrayIterator() {};
	virtual ~arrayIterator() {};
	virtual index_t row();
	virtual index_t col();
	virtual double val();
	virtual arrayIterator &operator++();
	virtual bool operator==(arrayIterator it2);
	private:
	index_t currN;
	};
	*/
	static_assert (std::is_integral<X>::value, "class X must be of integral type");
	static_assert (std::is_unsigned<X>::value, "class X must be of an unsigned type");
	typedef std::pair<X, Y> pLoc;
private:
	std::array<std::vector<pLoc>, 2> dVec;         //!< the vector of pairs containing the data			
	count_t sortCount = 0;			//!< count of the last sort operation

	decltype(dVec[0].begin()) cptr; //!< ptr to the begining of the sequence;
	decltype(dVec[0].end()) iend; //!< ptr to the end of the current sequence;
	int ci = 0;						//!< indicator of which vector of the array we are sequencing on;
	keyCompute<X, M> key_computer;    //!< object that generators the keys and extracts row and column information
	blockCompute<1, M> block_computer;    //!< object that generators the appropriate block to use
	//static const unsigned int vshift = 4 * sizeof(X);
	//static const X mask = (X(1) << vshift) - 1;
public:
	/** @brief constructor
	  the actual storage space used is approx 2x startCount due to expected unevenness in the array
	@param[in] startCount  the number of elements to reseve
	*/
	arrayDataSparseSMB(index_t startCount = 1000)
	{
		dVec[0].reserve(startCount);
		dVec[1].reserve(startCount);
	};
	void clear() override
	{
		dVec[0].clear();
		dVec[1].clear();
	};


	void assign(index_t row, index_t col,  Y num) override
	{
		dVec[block_computer.blockIndexGen(row,col)].emplace_back(key_computer.keyGen(row,col), num);
		//dVec[col / (colLim >> K)].emplace_back((col << 16) + row, num);
	}

	/**
	* @brief reserve space for the cound of the jacobians
	* @param[in] size the amount of space to reserve
	*/
	void reserve(count_t reserveSize) override
	{
		dVec[0].reserve(reserveSize);
		dVec[1].reserve(reserveSize);
	}
	count_t size() const override
	{
		return static_cast<count_t>(dVec[0].size() + dVec[1].size());
	}

	void setColLimit(index_t lim) override
	{
		block_computer.setMaxIndex(arrayData<Y>::rowLim, lim);
		arrayData<Y>::colLim = lim;

	}
	void setRowLimit(index_t lim) override
	{
		block_computer.setMaxIndex(lim,arrayData<Y>::colLim);
		arrayData<Y>::rowLim = lim;
	}

	count_t capacity() const override
	{
		return static_cast<count_t>(dVec[0].capacity() + dVec[1].capacity());
	}
	/**
	* @brief sort the index based first on column number then column number
	*/
	void sortIndex()
	{
		//std::sort(dVec.begin(), dVec.end(), compareLocSM);
		auto fp = [](pLoc A, pLoc B) {return (A.first < B.first); };

			std::sort(dVec[0].begin(), dVec[0].end(), fp);
			std::sort(dVec[1].begin(), dVec[1].end(), fp);
		sortCount = size();
	}

	void compact() override
	{
		if (!isSorted())
		{
			sortIndex();
		}
		for (auto &dvk : dVec)
		{
			if (dvk.empty())
			{
				return;
			}

			auto dvb = dvk.begin();
			auto dv2 = dvb;
			++dv2;
			auto dvend = dvk.end();
			while (dv2 != dvend)
			{
				if (dv2->first == dvb->first)
				{
					dvb->second += dv2->second;

				}
				else
				{
					++dvb;
					*dvb = *dv2;
				}
				++dv2;
			}
			dvk.resize(++dvb - dvk.begin());
		}
		sortCount = size();

	}

	index_t rowIndex(index_t N) const override
	{
		return static_cast<index_t>(key_computer.row((N >= dVec[0].size()) ? dVec[1][N - dVec[0].size()].first : dVec[0][N].first));

	}
	index_t colIndex(index_t N) const override
	{
		return static_cast<index_t>(key_computer.col((N >= dVec[0].size()) ? dVec[1][N - dVec[0].size()].first : dVec[0][N].first));
	}
	double val(index_t N) const override
	{
		return ((N >= dVec[0].size()) ? dVec[1][N - dVec[0].size()].second : dVec[0][N].second);
	}

	void start() override
	{
		cptr = dVec[0].begin();
		iend = dVec[0].end();
		ci = 0;
	}

	data_triple<Y> next() override
	{
		data_triple<Y> tp{ key_computer.row(cptr->first), key_computer.col(cptr->first), cptr->second };
		++cptr;
		if (cptr == iend)
		{
			++ci;
			if (ci < 2)
			{
				cptr = dVec[1].begin();
				iend = dVec[1].end();
			}
		}
		return tp;
	}

	bool moreData() override
	{
		return (ci<2);
	}

	/** @brief check if the sparse array is sorted
	@return bool indicating sorted status
	*/
	bool isSorted() const
	{
		return (sortCount == size());
	}

	double at(index_t rowN, index_t colN) const override
	{
		X cmp = key_computer.keyGen(rowN, colN);
		int I = block_computer.blockIndexGen(rowN,colN);
		if (isSorted())
		{
			auto res = std::lower_bound(dVec[I].begin(), dVec[I].end(), std::make_pair(cmp, 0), [](pLoc A, pLoc B) {return (A.first < B.first); });
			if (res == dVec[I].end())
			{
				return 0.0;
			}
			if (res->first == cmp)
			{
				return res->second;
			}
			else
			{
				return 0.0;
			}
		}
		else
		{

			for (const auto &rv : dVec[I])
			{
				if (rv.first == cmp)
				{
					return rv.second;
				}
			}
			return 0.0;
		}
	}

};
#endif

