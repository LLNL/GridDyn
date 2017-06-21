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
sundialsMatrixDataSparseColumn::sundialsMatrixDataSparseColumn (SlsMat mat) : J (mat)
{
    rowLim = static_cast<count_t> (J->M);
    colLim = static_cast<count_t> (J->N);
}

void sundialsMatrixDataSparseColumn::clear () { memset (J->data, 0, sizeof (realtype) * J->NNZ); }
void sundialsMatrixDataSparseColumn::assign (index_t row, index_t col, double num)
{
    int sti = J->indexptrs[col];
    int stp = J->indexptrs[col + 1];
    auto st = J->indexvals + sti;
    while (sti < stp)
    {
        if (*st == static_cast<int> (row))
        {
            J->data[sti] += num;
            break;
        }
        ++st;
        ++sti;
    }
}

void sundialsMatrixDataSparseColumn::setMatrix (SlsMat mat)
{
    J = mat;
    rowLim = static_cast<count_t> (J->M);
    colLim = static_cast<count_t> (J->N);
}

count_t sundialsMatrixDataSparseColumn::size () const { return static_cast<count_t> (J->indexptrs[colLim]); }
count_t sundialsMatrixDataSparseColumn::capacity () const { return static_cast<count_t> (J->NNZ); }
matrixElement<double> sundialsMatrixDataSparseColumn::element (index_t N) const
{
    matrixElement<double> ret;
    ret.row = static_cast<index_t> (J->indexvals[N]);
    auto res = std::lower_bound (J->indexptrs, &(J->indexptrs[colLim]), static_cast<int> (N));
    ret.col = static_cast<index_t> (*res - 1);
    ret.data = J->data[N];
    return ret;
}

void sundialsMatrixDataSparseColumn::start ()
{
    cur = 0;
    ccol = 0;
}

matrixElement<double> sundialsMatrixDataSparseColumn::next ()
{
    matrixElement<double> ret{static_cast<index_t> (J->indexvals[cur]), ccol, J->data[cur]};
    ++cur;
    if (static_cast<int> (cur) >= J->indexptrs[ccol + 1])
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

double sundialsMatrixDataSparseColumn::at (index_t rowN, index_t colN) const
{
    if (static_cast<int> (colN) > J->M)
    {
        return 0.0;
    }
    int sti = J->indexptrs[colN];
    int stp = J->indexptrs[colN + 1];
    for (int kk = sti; kk < stp; ++kk)
    {
        if (J->indexvals[kk] == static_cast<int> (rowN))
        {
            return J->data[kk];
        }
    }
    return 0.0;
}

}  // namespace solvers
}  // namespace griddyn