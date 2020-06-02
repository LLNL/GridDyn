/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include "../Source.h"

namespace griddyn {
namespace sources {
    /**@brief defines a ramping source
     */
    class rampSource: public Source {
      protected:
        model_parameter mp_dOdt = 0.0;  //!< [1/s] the ramp rate of the output
      public:
        rampSource(const std::string& objName = "rampSource_#", double startVal = 0.0);
        virtual coreObject* clone(coreObject* obj = nullptr) const override;

        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param,
                         double val,
                         units::unit unitType = units::defunit) override;

        virtual double computeOutput(coreTime time) const override;
        virtual double getDoutdt(const IOdata& inputs,
                                 const stateData& sD,
                                 const solverMode& sMode,
                                 index_t num = 0) const override;
        /** @brief clear the ramp (set it to 0)*/
        void clearRamp() { mp_dOdt = 0.0; }
    };
}  // namespace sources
}  // namespace griddyn
