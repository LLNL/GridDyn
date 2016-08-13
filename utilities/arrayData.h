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

#ifndef _ARRAY_DATA_H_
#define _ARRAY_DATA_H_

#include <cstdint>

#ifdef ENABLE_64_BIT_INDEXING
typedef std::uint64_t index_t;
typedef std::uint64_t count_t;
#else
typedef std::uint32_t index_t;
typedef std::uint32_t count_t;
#endif


/** @brief convenience structure for returning data
	*/
	template <class X>
	class data_triple
	{
	public:
		index_t row; //!< row
		index_t col; //!< column
		X data;  //!< value
	};
	
/** @brief class implementing a matrix entry class for handling jacobian entries
  this is a purely virtual interface a specific instantiation is required
 */
template <class X=double>
class arrayData
{
protected:
	index_t rowLim = (index_t)(-1);  //!< the maximum row index
	index_t colLim = (index_t)(-1);  //!< the maximum column index
	index_t cur = 0;
public:
	
	/** @brief constructor
	@param[in] rows  the initial row limit
	@parma[in] cols the initial column limit*/
	arrayData(index_t rows = (index_t)(-1), index_t cols = (index_t)(-1)) : rowLim(rows), colLim(cols)
	{};
	/** @brief virtual destructor*/
	virtual ~arrayData() {};
  /**
  * @brief function to clear the data
  */
  virtual void clear() = 0;

  /**
  * @brief add a new jacobian element
  * @param[in] row,col the row  and column of the element
  * @param[in] num the value of the element
  */
  virtual void assign(index_t row, index_t col, X num) = 0;
  /** @brief assign with a check on the row
  *  add a new jacobian element if the arguments are valid (row<rowNum)
  * @param[in] row,col the row and column of the element
  * @param[in] num the value of the element
  */
  void assignCheckRow(index_t row, index_t col, X num)
  {
	if (row < rowLim)
	{
		assign(row, col, num);
	}
  }

  /** @brief assign with a check on the col
  *  add a new jacobian element if the arguments are valid (col<colLim)
  * @param[in] row,col the row  and column of the element
  * @param[in] num the value of the element
  */
  void assignCheckCol(index_t row, index_t col, X num)
  {
	  if (col < colLim)
	  {
		  assign(row, col, num);
	  }
  }


  /** @brief assign with a check on the row and column
  *  add a new jacobian element if the arguments are valid (X<rowLim) &&(Y<colLim)
  * @param[in] X,Y the row (X) and column(Y) of the element
  * @param[in] num the value of the element
  */
  void assignCheck(index_t row, index_t col, X num)
  {
	  if ((row < rowLim) && (col < colLim))
	  {
		  assign(row, col, num);
	  }
  }


  /**
  * @brief get the maximum row number
  * @return the max row value
  */
  count_t rowLimit() const { return rowLim; };
  /**
  * @brief get the maximum row number
  * @return the max col value
  */
  count_t colLimit() const { return colLim; };
  /**
  * @brief set the maximum row count
  * @param[in] lim the new row limit
  */
  virtual void setRowLimit(index_t lim) { rowLim = lim; };
  /**
  * @brief set the maximum number of columns
  * @param[in] lim the new column limit
  */
  virtual void setColLimit(index_t lim) { colLim = lim; };
  /**
  * @brief get the number of points
  * @return the number of points
  */
  virtual count_t size() const = 0;
  /**
  * @brief get the current capacity
  * @return the maximum number of points
  */
  virtual count_t capacity() const = 0;

  /**
  * @brief reserve space in the array for 
  */
  virtual void reserve(count_t /*maxNonZeros*/) {};

  /**
  @brief get the value at a specific location
  @param[in] rowN  the row number
  @param[in] colN  the column number
  */
  virtual double at(index_t rowN, index_t colN) const = 0;

  //virtual arrayIterator begin() const;
 // virtual arrayIterator end() const;
  /**
  * @brief get the row value
  * @param[in] N the element number to return
  * @return the row of the corresponding index
  */
  virtual index_t rowIndex(index_t N) const = 0;

  /**
  * @brief get the column value
  * @param[in] N the element number to return
  * @return the column of the corresponding index
  */
  virtual index_t colIndex(index_t N) const = 0;
  /**
  * @brief get the value
  * @param[in] N the element number to return
  * @return the value of the corresponding index
  */
  virtual X val(index_t N) const = 0;

  /** @brief change the arrayData to a compact sorted form
  */
  virtual void compact()
  {
	  //many arrays would already be in compact form so this should do nothing in the default case
  }

  /** @brief set the array to begin a sequence of retrievals
  */
  virtual void start()
  {
	  cur = 0;
  }

  /** @brief gets the next data triple
   if called after all the elements have been retrieved should return invalid row and column index;
  @return a triple with the row/col/val of the first element
  */
  virtual data_triple<X> next()
  {
	  return{ rowIndex(cur),colIndex(cur),val(cur++) };
  }
  /** @brief check if the data sequence is at its end
  @return true if there are more points to grab false if not
  */
  virtual bool moreData()
  {
	  return (cur < size());
  }
  /**
  * @brief merge 2 arrayData structures together
  * @param[in] a2 the arrayData to merge in
  */
  virtual void merge(arrayData<X> *a2)
  {
	  count_t count = a2->size();
	  a2->start();
	  for (index_t nn = 0; nn < count; ++nn)
	  {
		  auto tp = a2->next();
		  assign(tp.row, tp.col, tp.data);
	  }
  }
  /**
  * @brief merge 2 arrayData structures together
  * @param[in] a2 the arrayData to merge in
  @param[in] scale a double scaler for each of the elemnets in the second matrix;
  */
  virtual void merge(arrayData<X> *a2, X scale)
  {
	  count_t count = a2->size();
	  a2->start();
	  for (index_t nn = 0; nn < count; ++nn)
	  {
		  auto tp = a2->next();
		  assign(tp.row, tp.col, tp.data*scale);
	  }
  }
  /**
  * @merge copy and translate a row from a2 into the calling arrayData
  * @param[in] a2 the arrayData to  copy and translate
  @param[in] origRow  the original row
  @param[in] newRow the new row Value
  */
  virtual void copyTranslateRow(arrayData<X> *a2, index_t origRow, index_t newRow)
  {
	  count_t count = a2->size();
	  a2->start();
	  for (index_t nn = 0; nn < count; ++nn)
	  {
		  if (a2->rowIndex(nn) == origRow)
		  {
			  assign(newRow, a2->colIndex(nn), a2->val(nn));
		  }
	  }
  }
};


#endif
