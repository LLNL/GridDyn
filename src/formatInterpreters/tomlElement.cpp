/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tomlElement.h"

static const std::string nullStr{};

tomlElement::tomlElement(toml::Value vElement, std::string newName):
    name(std::move(newName)), element(std::move(vElement))
{
    elementIndex = 0;

    if (element.type() == toml::Value::ARRAY_TYPE) {
        arraytype = true;
        arrayIndex = 0;
        while ((arrayIndex < static_cast<int>(element.size())) &&
               (element.find(arrayIndex)->empty())) {
            ++arrayIndex;
        }
    }
}

void tomlElement::clear()
{
    element = toml::Value();
    elementIndex = 0;
    arrayIndex = 0;
    arraytype = false;
    name = nullStr;
}
