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

#ifndef _ARRAY_DATA_SUNDIALS_H_
#define _ARRAY_DATA_SUNDIALS_H_

#include "arrayData.h"
#include "sundials/sundials_dense.h"
#include "sundials/sundials_sparse.h"

/** @brief class implementing an arrayData wrapper around the sundials dense matrix*/
class arrayDataSundialsDense : public arrayData<double>
{
private:
  DlsMat J = nullptr;             //!< the vector of tuples containing the data
public:
  /** @brief compact constructor
  */
  arrayDataSundialsDense ();
  /** @brief alternate constructor defining the Dense matrix to fill
  @param[in] mat the dense sundials matrix*/
  arrayDataSundialsDense (DlsMat mat);

  void clear () override;

  void assign (index_t X, index_t Y, double num) override;

  /** set the sundials matrix
  @param[in] mat the dense sundials matrix
  */
  void setMatrix (DlsMat mat);

  count_t size () const override;

  count_t capacity () const override;

  index_t rowIndex (index_t N) const override;

  index_t colIndex (index_t N) const override;

  double val (index_t N) const override;

  double at (index_t rowN, index_t colN) const override;
};

/** @brief class implementing an arrayData wrapper around the sundials dense matrix*/
class arrayDataSundialsSparse : public arrayData<double>
{
private:
  SlsMat J;               //!< the vector of tuples containing the data
  index_t ccol = 0;
public:
  /** @brief compact constructor
  */
  arrayDataSundialsSparse ();
  /** @brief alternate constructor defining the Sparse matrix to fill*/
  arrayDataSundialsSparse (SlsMat mat);

  void clear () override;

  void assign (index_t X, index_t Y, double num) override;

  /** set the sundials matrix
  @param[in] mat the sparse sundials matrix
  */
  void setMatrix (SlsMat mat);

  count_t size () const override;

  count_t capacity () const override;

  index_t rowIndex (index_t N) const override;

  index_t colIndex (index_t N) const override;

  double val (index_t N) const override;

  double at (index_t rowN, index_t colN) const override;

  virtual void start() override;

  virtual data_triple<double> next() override;
};

#endif


