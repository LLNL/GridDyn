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

#include "coreExceptions.h"
namespace griddyn
{
coreObjectException::coreObjectException (const coreObject *obj) noexcept : throwingObject (obj) {}
std::string coreObjectException::who () const noexcept { return fullObjectName (throwingObject); }
}  // namespace griddyn