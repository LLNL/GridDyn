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

#include "readerElement.h"
#include "utilities/stringConversion.h"

readerAttribute::readerAttribute () {}
readerAttribute::readerAttribute (std::string attName, std::string attText) : name (std::move(attName)), text (std::move(attText)) {}
void readerAttribute::set (const std::string &attName, const std::string &attText)
{
    name = attName;
    text = attText;
}

double readerAttribute::getValue () const { return numeric_conversion<double> (text, readerNullVal); }
constexpr int64_t nullLong = int64_t(0x8000'0000'0000'0000);
int64_t readerAttribute::getInt () const { return numeric_conversion<int64_t> (text, nullLong); }
readerElement::~readerElement() = default;
