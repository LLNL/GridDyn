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

#include "solvers/sundialsMatrixData.h"
#include <algorithm>
#include <cstring>

matrixDataSundialsDense::matrixDataSundialsDense ()
{
}

matrixDataSundialsDense::matrixDataSundialsDense (DlsMat mat) : J (mat)
{
  rowLim = static_cast<count_t> (J->M);
  colLim = static_cast<count_t> (J->N);
}
void matrixDataSundialsDense::clear ()
{
  memset (J->data, 0, sizeof(realtype) * J->ldata);
}

void matrixDataSundialsDense::assign (index_t X, index_t Y, double num)
{
  DENSE_ELEM (J, X, Y) += num;
}

void matrixDataSundialsDense::setMatrix (DlsMat mat)
{
  J = mat;
  rowLim = static_cast<count_t> (J->M);
  colLim = static_cast<count_t> (J->N);
}

count_t matrixDataSundialsDense::size () const
{
  return static_cast<count_t> (J->M * J->N);
}

count_t matrixDataSundialsDense::capacity () const
{
  return static_cast<count_t> (J->M * J->N);
}

index_t matrixDataSundialsDense::rowIndex (index_t N) const
{
  return static_cast<index_t> (N % J->N);
}

index_t matrixDataSundialsDense::colIndex (index_t N) const
{
  return static_cast<index_t> (N / J->N);
}

double matrixDataSundialsDense::val (index_t N) const
{
  return J->data[N];
}

double matrixDataSundialsDense::at (index_t rowN, index_t colN) const
{
  return DENSE_ELEM (J, rowN, colN);
}



matrixDataSundialsSparse::matrixDataSundialsSparse ()
{
}

matrixDataSundialsSparse::matrixDataSundialsSparse (SlsMat mat) : J (mat)
{
  rowLim = static_cast<count_t> (J->M);
  colLim = static_cast<count_t> (J->N);
}

void matrixDataSundialsSparse::clear ()
{
  memset (J->data, 0, sizeof(realtype) * J->NNZ);
}

void matrixDataSundialsSparse::assign (index_t X, index_t Y, double num)
{

  int sti = J->colptrs[Y];
  int stp = J->colptrs[Y + 1];
  auto st = J->rowvals + sti;
  while (sti < stp)
    {
      if (*st == static_cast<int> (X))
        {

          J->data[sti] += num;
          break;
        }
      ++st;
      ++sti;
    }
}

void matrixDataSundialsSparse::setMatrix (SlsMat mat)
{
  J = mat;
  rowLim = static_cast<count_t> (J->M);
  colLim = static_cast<count_t> (J->N);
}


count_t matrixDataSundialsSparse::size () const
{
  return static_cast<count_t> (J->colptrs[colLim]);
}

count_t matrixDataSundialsSparse::capacity () const
{
  return static_cast<count_t> (J->NNZ);
}

index_t matrixDataSundialsSparse::rowIndex (index_t N) const
{
  return static_cast<index_t> (J->rowvals[N]);
}

index_t matrixDataSundialsSparse::colIndex (index_t N) const
{
  auto res = std::lower_bound (J->colptrs, &(J->colptrs[colLim]), static_cast<int> (N));
  return *res - 1;
}

void matrixDataSundialsSparse::start()
{
	cur = 0;
	ccol = 0;
}

matrixElement<double> matrixDataSundialsSparse::next()
{
	matrixElement<double> ret{ static_cast<index_t>(J->rowvals[cur]), ccol, J->data[cur] };
	++cur;
	if (static_cast<int>(cur) >= J->colptrs[ccol+1])
	{
		++ccol;
		if (ccol > colLim)
		{
			--cur;
			--ccol;
		}
	}
	return ret;

}

double matrixDataSundialsSparse::val (index_t N) const
{
  return J->data[N];
}

double matrixDataSundialsSparse::at (index_t rowN, index_t colN) const
{
  if (static_cast<int> (colN) > J->M)
    {
      return 0;
    }
  int sti = J->colptrs[colN];
  int stp = J->colptrs[colN + 1];
  for (int kk = sti; kk < stp; ++kk)
    {
      if (J->rowvals[kk] == static_cast<int> (rowN))
        {
          return J->data[kk];
        }
    }
  return 0;
}
