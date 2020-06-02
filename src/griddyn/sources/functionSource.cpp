/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "functionSource.h"

#include "core/coreObjectTemplates.hpp"

namespace griddyn {
namespace sources {
    functionSource::functionSource(const std::string& objName): Source(objName) {}
    coreObject* functionSource::clone(coreObject* obj) const
    {
        auto gS = cloneBase<functionSource, gridSubModel>(this, obj);
        if (gS == nullptr) {
            return obj;
        }
        gS->sourceFunc = sourceFunc;
        return gS;
    }

    IOdata functionSource::getOutputs(const IOdata& /*inputs*/,
                                      const stateData& sD,
                                      const solverMode& /*sMode*/) const
    {
        return {sourceFunc(sD.time)};
    }
    double functionSource::getOutput(const IOdata& /*inputs*/,
                                     const stateData& sD,
                                     const solverMode& /*sMode*/,
                                     index_t outputNum) const
    {
        return (outputNum == 0) ? sourceFunc(sD.time) : kNullVal;
    }

    double functionSource::getOutput(index_t outputNum) const
    {
        return (outputNum == 0) ? sourceFunc(prevTime) : kNullVal;
    }
    double functionSource::getDoutdt(const IOdata& /*inputs*/,
                                     const stateData& sD,
                                     const solverMode& /*sMode*/,
                                     index_t outputNum) const
    {
        return (outputNum == 0) ? ((sourceFunc(sD.time + 1e-7) - sourceFunc(sD.time)) / 1e-7) : 0.0;
    }

    void functionSource::setFunction(std::function<double(double)> calcFunc)
    {
        sourceFunc = std::move(calcFunc);
    }
}  // namespace sources
}  // namespace griddyn
