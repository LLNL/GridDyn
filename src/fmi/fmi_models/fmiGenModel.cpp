/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fmiGenModel.h"

#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "fmiMESubModel.h"
#include "gmlc/utilities/stringOps.h"
#include "griddyn/gridBus.h"

namespace griddyn {
namespace fmi {
    fmiGenModel::fmiGenModel(const std::string& objName): fmiMEWrapper<GenModel>(objName) {}

    coreObject* fmiGenModel::clone(coreObject* obj) const
    {
        auto nobj = cloneBase<fmiGenModel, fmiMEWrapper<GenModel>>(this, obj);
        if (nobj == nullptr) {
            return obj;
        }

        return nobj;
    }

    void fmiGenModel::set(const std::string& param, const std::string& val)
    {
        if (param.empty()) {
        } else {
            fmiMEWrapper<GenModel>::set(param, val);
        }
    }

    void fmiGenModel::set(const std::string& param, double val, units::unit unitType)
    {
        if (param.empty()) {
        } else {
            fmiMEWrapper<GenModel>::set(param, val, unitType);
        }
    }

}  // namespace fmi
}  // namespace griddyn
