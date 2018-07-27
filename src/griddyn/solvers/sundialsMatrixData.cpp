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
std::unique_ptr<matrixData<double>> makeSundialsMatrixData (SUNMatrix J)
{
    switch (SUNMatGetID (J))
    {
    case SUNMATRIX_DENSE:
        return std::make_unique<sundialsMatrixDataDense> (J);
    case SUNMATRIX_SPARSE:
        if (SM_SPARSETYPE_S (J) == CSR_MAT)
        {
            return std::make_unique<sundialsMatrixDataSparseRow> (J);
        }
        else
        {
            return std::make_unique<sundialsMatrixDataSparseColumn> (J);
        }
    case SUNMATRIX_CUSTOM:
    default:
        return nullptr;
    }
}

}  // namespace solvers

}  // namespace griddyn