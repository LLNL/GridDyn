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

#include "readerHelper.h"
#include "fileInput.h"

#include <boost/filesystem.hpp>

// library for printf debug statements

#include <cmath>
namespace griddyn
{
using namespace readerConfig;

void paramStringProcess (gridParameter &param, readerInfo &ri)
{
    if (!param.stringType)
    {
        return;
    }
    double val = interpretString (param.strVal, ri);
    if (!(std::isnan (val)))
    {
        param.value = val;
        param.stringType = false;
    }
    else  // can't be interpreted as a number so do a last check for string redefinitions
    {
        param.strVal = ri.checkDefines (param.strVal);
    }
}

}//namespace griddyn
