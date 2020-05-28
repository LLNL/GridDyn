/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMI_GENMODEL_H_
#define FMI_GENMODEL_H_

#include "fmiMEWrapper.hpp"
#include "griddyn/GenModel.h"

namespace griddyn {
namespace fmi {
    class fmiMESubModel;

    class fmiGenModel: public fmiMEWrapper<GenModel> {
      public:
        fmiGenModel(const std::string& objName = "fmiGenModel_#");
        virtual coreObject* clone(coreObject* obj = nullptr) const override;
        // virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
        // virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput,
        // IOdata &fieldSet) override;

        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param,
                         double val,
                         units::unit unitType = units::defunit) override;
    };

}  // namespace fmi
}  // namespace griddyn
#endif
