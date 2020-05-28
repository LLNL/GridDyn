/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "readerElement.h"

#include "gmlc/utilities/stringConversion.h"

using namespace gmlc::utilities;

readerAttribute::readerAttribute() = default;
readerAttribute::readerAttribute(std::string attName, std::string attText):
    name(std::move(attName)), text(std::move(attText))
{
}
void readerAttribute::set(const std::string& attName, const std::string& attText)
{
    name = attName;
    text = attText;
}

double readerAttribute::getValue() const
{
    return numeric_conversion<double>(text, readerNullVal);
}
constexpr int64_t nullLong = int64_t(0x8000'0000'0000'0000);
int64_t readerAttribute::getInt() const
{
    return numeric_conversion<int64_t>(text, nullLong);
}
readerElement::~readerElement() = default;
