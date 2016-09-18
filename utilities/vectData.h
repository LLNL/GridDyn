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

#ifndef _VECT_DATA_H_
#define _VECT_DATA_H_


//TODO make this into a template

#include <cstdint>
#ifdef ENABLE_64_BIT_INDEXING
typedef std::uint64_t index_t;
typedef std::uint64_t count_t;
#else
typedef std::uint32_t index_t;
typedef std::uint32_t count_t;
#endif

#include <vector>
#include <utility>

typedef std::pair<index_t, double> vLoc;

bool compareLocVectData (vLoc A, vLoc B);
/**
* class for storing data from the Jacobian computation
*/
class vectData
{
private:
  std::vector<vLoc> dVec;         //!< the vector of pairs containing the data
public:
  vectData ()
  {
  }
  /**
  * function to clear the data
  */
  void reset ()
  {
    dVec.resize (0);
  }
  count_t svsize = 1000000000;
  /**
  * add a new Jacobian element
  * @param[in] X,Y the row (X) and column(Y) of the element
  * @param[in] num the value of the element
  */
  void assign(index_t X, double num);
  /** assign with a check
  * add a new Jacobian element if the arguments are valid (X>=0) &&(Y>=0)
  * @param[in] X,Y the row (X) and column(Y) of the element
  * @param[in] num the value of the element
  */
  void assignCheck(index_t X, double num);
  /**
  * reserve space for the cound of the jacobians
  * @param[in] size the amount of space to reserve
  */
  void reserve (count_t size)
  {
    dVec.reserve (size);
  }
  /**
  * get the number of points
  * @return the number of points
  */
  count_t points () const
  {
    return static_cast<count_t> (dVec.size ());
  }
  /**
  * get the maximum number of points the vector can hold
  * @return the number of points
  */
  count_t capacity () const
  {
    return static_cast<count_t> (dVec.capacity ());
  }
  /**
  * sort the index based first on row number than column number
  */
  void sortIndex();
  /**
  * compact the index merging values with the same row and column number together
  */
  void compact ();
  /**
  * get the row value
  * @param[in] N the element number to return
  * @return the row of the corresponding index
  */
  index_t rowIndex (index_t N)
  {
    return dVec[N].first;
  }
  /**
  * get the column value
  * @param[in] N the element number to return
  * @return the column of the corresponding index
  */

  double val (index_t N)
  {
    return dVec[N].second;
  }
  /**
  * get the number nonzero of elements in each row
  * @return a vector of ints with the column counts
  */

  double find (index_t rowN);
  void scale (double factor, index_t start = 0, count_t count = 0x0FFFFFFFF);
};



#endif
