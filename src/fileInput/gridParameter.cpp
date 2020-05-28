/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "gridParameter.h"

#include "core/coreExceptions.h"
#include "gmlc/utilities/string_viewConversion.h"
#include "griddyn/gridDynDefinitions.hpp"

namespace griddyn {
gridParameter::gridParameter() = default;
gridParameter::gridParameter(const std::string& str)
{
    fromString(str);
}
gridParameter::gridParameter(std::string fld, double val):
    field(std::move(fld)), value(val), valid(true)
{
}
gridParameter::gridParameter(std::string fld, std::string val):
    field(std::move(fld)), strVal(std::move(val)), valid(true), stringType(true)
{
}

void gridParameter::reset()
{
    valid = false, stringType = false, paramUnits = units::defunit;
    applyIndex.resize(0);
}

void gridParameter::fromString(const std::string& str)
{
    using namespace gmlc::utilities::string_viewOps;
    using gmlc::utilities::string_view;
    string_view strv(str);
    valid = false;
    size_t rlc = strv.find_last_of('=');
    if (rlc == std::string::npos) {
        throw(invalidParameterValue(str));
    }
    valid = true;
    auto fld = trim(strv.substr(0, rlc));

    // now read the value
    strVal = strv.substr(rlc + 1).to_string();
    value = gmlc::utilities::numeric_conversionComplete(strVal, kNullVal);
    stringType = (value == kNullVal);

    rlc = fld.find_first_of('(');
    if (rlc != string_view::npos) {
        size_t rlc2 = fld.find_last_of(')');
        paramUnits = units::unit_cast_from_string(fld.substr(rlc + 1, rlc2 - rlc - 1).to_string());
        field = fld.substr(0, rlc).to_string();
    } else {
        field = fld.to_string();
    }
}

}  // namespace griddyn
