/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "coreExceptions.h"
namespace griddyn {
coreObjectException::coreObjectException(const coreObject* obj) noexcept: throwingObject(obj) {}
std::string coreObjectException::who() const noexcept
{
    return fullObjectName(throwingObject);
}
}  // namespace griddyn
