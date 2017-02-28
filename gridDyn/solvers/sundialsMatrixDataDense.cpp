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

#include "solvers/sundialsMatrixData.h"
#include <algorithm>
#include <cstring>

sundialsMatrixDataDense::sundialsMatrixDataDense ()
{
}

sundialsMatrixDataDense::sundialsMatrixDataDense (DlsMat mat) : J (mat)
{
  rowLim = static_cast<count_t> (J->M);
  colLim = static_cast<count_t> (J->N);
}
void sundialsMatrixDataDense::clear ()
{
  memset (J->data, 0, sizeof(realtype) * J->ldata);
}

void sundialsMatrixDataDense::assign (index_t X, index_t Y, double num)
{
  DENSE_ELEM (J, X, Y) += num;
}

void sundialsMatrixDataDense::setMatrix (DlsMat mat)
{
  J = mat;
  rowLim = static_cast<count_t> (J->M);
  colLim = static_cast<count_t> (J->N);
}

count_t sundialsMatrixDataDense::size () const
{
  return static_cast<count_t> (J->M * J->N);
}

count_t sundialsMatrixDataDense::capacity () const
{
  return static_cast<count_t> (J->M * J->N);
}

matrixElement<double> sundialsMatrixDataDense::element(index_t N) const
{
	return{ N % J->N ,N / J->N ,J->data[N] };
}

double sundialsMatrixDataDense::at (index_t rowN, index_t colN) const
{
  return DENSE_ELEM (J, rowN, colN);
}



