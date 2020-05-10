/*
* LLNS Copyright Start
* Copyright (c) 2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#ifndef DISABLE_GRIDDYN

#    include "paradaeArrayData.h"

namespace griddyn {
namespace paradae {
    paradaeArrayData::paradaeArrayData(SparseMatrix* mat)
    {
        J = mat;
        setRowLimit(static_cast<count_t>(mat->GetM()));

        setColLimit(static_cast<count_t>(mat->GetM()));
    }

    void paradaeArrayData::clear() { J->FillKeepingStruct(0); }

    void paradaeArrayData::assign(index_t X, index_t Y, double num) { (*J)(X, Y) += num; }

    void paradaeArrayData::setMatrix(SparseMatrix* mat)
    {
        J = mat;
        setRowLimit(static_cast<count_t>(mat->GetM()));
        setColLimit(static_cast<count_t>(mat->GetM()));
    }

    count_t paradaeArrayData::size() const { return static_cast<count_t>(J->GetNNZ()); }

    count_t paradaeArrayData::capacity() const { return static_cast<count_t>(J->GetNNZ()); }

    index_t paradaeArrayData::rowIndex(index_t N) const { return J->GetRowIndex(N); }

    index_t paradaeArrayData::colIndex(index_t N) const { return J->GetColIndex(N); }

    double paradaeArrayData::val(index_t N) const { return J->GetValue(N); }

    double paradaeArrayData::at(index_t rowN, index_t colN) const { return (*J)(rowN, colN); }

    matrixElement<double> paradaeArrayData::element(index_t N) const
    {
        return {J->GetRowIndex(N), J->GetColIndex(N), J->GetValue(N)};
    }

}  // namespace paradae
}  // namespace griddyn

#endif
