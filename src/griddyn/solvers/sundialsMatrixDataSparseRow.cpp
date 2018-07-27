/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "sundialsMatrixData.h"
#include <algorithm>
#include <cstring>

namespace griddyn
{
namespace solvers
{
sundialsMatrixDataSparseRow::sundialsMatrixDataSparseRow (SUNMatrix mat)
    : matrixData<double> (static_cast<count_t> (SM_ROWS_S (mat)), static_cast<count_t> (SM_COLUMNS_S (mat))),
      J (mat)
{
}

void sundialsMatrixDataSparseRow::clear () { SUNMatZero (J); }
void sundialsMatrixDataSparseRow::assign (index_t row, index_t col, double num)
{
    int sti = SM_INDEXPTRS_S (J)[row];
    int stp = SM_INDEXPTRS_S (J)[row + 1];
    auto st = SM_INDEXVALS_S (J) + sti;
    while (sti < stp)
    {
        if (*st == static_cast<int> (col))
        {
            SM_DATA_S (J)[sti] += num;
            break;
        }
        ++st;
        ++sti;
    }
}

void sundialsMatrixDataSparseRow::setMatrix (SUNMatrix mat)
{
    J = mat;
    setRowLimit (static_cast<count_t> (SM_ROWS_S (J)));
    setColLimit (static_cast<count_t> (SM_COLUMNS_S (J)));
}

count_t sundialsMatrixDataSparseRow::size () const
{
    return static_cast<count_t> (SM_INDEXPTRS_S (J)[rowLimit ()]);
}
count_t sundialsMatrixDataSparseRow::capacity () const { return static_cast<count_t> (SM_NNZ_S (J)); }
matrixElement<double> sundialsMatrixDataSparseRow::element (index_t N) const
{
    matrixElement<double> ret;
    ret.col = static_cast<index_t> (SM_INDEXVALS_S (J)[N]);
    auto res = std::lower_bound (SM_INDEXPTRS_S (J), &(SM_INDEXPTRS_S (J)[rowLimit ()]), static_cast<int> (N));
    ret.row = static_cast<index_t> (*res - 1);
    ret.data = SM_DATA_S (J)[N];
    return ret;
}

void sundialsMatrixDataSparseRow::start ()
{
    cur = 0;
    crow = 0;
}

matrixElement<double> sundialsMatrixDataSparseRow::next ()
{
    matrixElement<double> ret{crow, static_cast<index_t> (SM_INDEXVALS_S (J)[cur]), SM_DATA_S (J)[cur]};
    ++cur;
    if (static_cast<int> (cur) >= SM_INDEXPTRS_S (J)[crow + 1])
    {
        ++crow;
        if (crow > rowLimit ())
        {
            --cur;
            --crow;
        }
    }
    return ret;
}

double sundialsMatrixDataSparseRow::at (index_t rowN, index_t colN) const
{
    if (static_cast<int> (rowN) > SM_ROWS_S (J))
    {
        return 0.0;
    }
    int sti = SM_INDEXPTRS_S (J)[rowN];
    int stp = SM_INDEXPTRS_S (J)[rowN + 1];
    for (int kk = sti; kk < stp; ++kk)
    {
        if (SM_INDEXVALS_S (J)[kk] == static_cast<int> (colN))
        {
            return SM_DATA_S (J)[kk];
        }
    }
    return 0.0;
}
}  // namespace solvers
}  // namespace griddyn