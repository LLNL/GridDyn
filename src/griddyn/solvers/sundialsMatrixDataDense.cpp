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
sundialsMatrixDataDense::sundialsMatrixDataDense (SUNMatrix mat)
    : matrixData<double> (static_cast<count_t> (SM_ROWS_D (mat)), static_cast<count_t> (SM_COLUMNS_D (mat))),
      J (mat)
{
}
void sundialsMatrixDataDense::clear () { SUNMatZero (J); }
void sundialsMatrixDataDense::assign (index_t X, index_t Y, double num) { SM_ELEMENT_D (J, X, Y) += num; }
void sundialsMatrixDataDense::setMatrix (SUNMatrix mat)
{
    J = mat;
    setRowLimit (static_cast<count_t> (SM_ROWS_D (J)));
    setColLimit (static_cast<count_t> (SM_COLUMNS_D (J)));
}

count_t sundialsMatrixDataDense::size () const { return static_cast<count_t> (SM_ROWS_D (J) * SM_COLUMNS_D (J)); }
count_t sundialsMatrixDataDense::capacity () const
{
    return static_cast<count_t> (SM_ROWS_D (J) * SM_COLUMNS_D (J));
}
matrixElement<double> sundialsMatrixDataDense::element (index_t N) const
{
    return {N % static_cast<index_t> (SM_COLUMNS_D (J)), N / static_cast<index_t> (SM_COLUMNS_D (J)),
            SM_DATA_D (J)[N]};
}

double sundialsMatrixDataDense::at (index_t rowN, index_t colN) const { return SM_ELEMENT_D (J, rowN, colN); }
}  // namespace solvers
}  // namespace griddyn
