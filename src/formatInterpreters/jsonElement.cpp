/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "jsonElement.h"

static const std::string nullStr = std::string("");

jsonElement::jsonElement(Json::Value vElement, std::string newName):
    name(std::move(newName)), element(std::move(vElement))
{
    elementIndex = 0;

    if (element.isArray()) {
        arraytype = true;
        arrayIndex = 0;
        while ((arrayIndex < element.size()) && (element[arrayIndex].empty())) {
            ++arrayIndex;
        }
    }
}

void jsonElement::clear()
{
    element = Json::nullValue;
    elementIndex = 0;
    arrayIndex = 0;
    arraytype = false;
    name = nullStr;
}
