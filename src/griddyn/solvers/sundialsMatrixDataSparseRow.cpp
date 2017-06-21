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

namespace griddyn
{
namespace solvers
{
sundialsMatrixDataSparseRow::sundialsMatrixDataSparseRow (SlsMat mat) : J (mat)
{
    rowLim = static_cast<count_t> (J->M);
    colLim = static_cast<count_t> (J->N);
}

void sundialsMatrixDataSparseRow::clear () { memset (J->data, 0, sizeof (realtype) * J->NNZ); }
void sundialsMatrixDataSparseRow::assign (index_t row, index_t col, double num)
{
    int sti = J->indexptrs[row];
    int stp = J->indexptrs[row + 1];
    auto st = J->indexvals + sti;
    while (sti < stp)
    {
        if (*st == static_cast<int> (col))
        {
            J->data[sti] += num;
            break;
        }
        ++st;
        ++sti;
    }
}

void sundialsMatrixDataSparseRow::setMatrix (SlsMat mat)
{
    J = mat;
    rowLim = static_cast<count_t> (J->M);
    colLim = static_cast<count_t> (J->N);
}

count_t sundialsMatrixDataSparseRow::size () const { return static_cast<count_t> (J->indexptrs[rowLim]); }
count_t sundialsMatrixDataSparseRow::capacity () const { return static_cast<count_t> (J->NNZ); }
matrixElement<double> sundialsMatrixDataSparseRow::element (index_t N) const
{
    matrixElement<double> ret;
    ret.col = static_cast<index_t> (J->indexvals[N]);
    auto res = std::lower_bound (J->indexptrs, &(J->indexptrs[rowLim]), static_cast<int> (N));
    ret.row = static_cast<index_t> (*res - 1);
    ret.data = J->data[N];
    return ret;
}

void sundialsMatrixDataSparseRow::start ()
{
    cur = 0;
    crow = 0;
}

matrixElement<double> sundialsMatrixDataSparseRow::next ()
{
    matrixElement<double> ret{crow, static_cast<index_t> (J->indexvals[cur]), J->data[cur]};
    ++cur;
    if (static_cast<int> (cur) >= J->indexptrs[crow + 1])
    {
        ++crow;
        if (crow > rowLim)
        {
            --cur;
            --crow;
        }
    }
    return ret;
}

double sundialsMatrixDataSparseRow::at (index_t rowN, index_t colN) const
{
    if (static_cast<int> (rowN) > J->M)
    {
        return 0;
    }
    int sti = J->indexptrs[rowN];
    int stp = J->indexptrs[rowN + 1];
    for (int kk = sti; kk < stp; ++kk)
    {
        if (J->indexvals[kk] == static_cast<int> (colN))
        {
            return J->data[kk];
        }
    }
    return 0;
}
}  // namespace solvers
}  // namespace griddyn