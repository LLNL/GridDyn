/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GOVERNORE_STEAM_TCSR_H_
#define GOVERNORE_STEAM_TCSR_H_
#pragma once

#include "GovernorSteamNR.h"

namespace griddyn {
namespace governors {
    class GovernorSteamTCSR: public GovernorSteamNR {
      public:
      protected:
        model_parameter Trh;  //!< [s] steam reheat chest time constant
        model_parameter Tco;  //!< [s] steam reheat chest time constant
        model_parameter Fch;  //!< [s] steam reheat chest time constant
        model_parameter Fip;  //!< [s] steam reheat chest time constant
        model_parameter Flp;  //!< [s] steam reheat chest time constant
      public:
        GovernorSteamTCSR(const std::string& objName = "govSteamTCSR_#");
        virtual coreObject* clone(coreObject* obj = nullptr) const override;
        virtual ~GovernorSteamTCSR();

        virtual void dynObjectInitializeB(const IOdata& inputs,
                                          const IOdata& desiredOutput,
                                          IOdata& fieldSet) override;

        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param,
                         double val,
                         units::unit unitType = units::defunit) override;
        virtual index_t findIndex(const std::string& field, const solverMode& sMode) const override;

        virtual void residual(const IOdata& inputs,
                              const stateData& sD,
                              double resid[],
                              const solverMode& sMode) override;

        virtual void jacobianElements(const IOdata& inputs,
                                      const stateData& sD,
                                      matrixData<double>& md,
                                      const IOlocs& inputLocs,
                                      const solverMode& sMode) override;
    };

}  // namespace governors
}  // namespace griddyn

#endif  // GOVERNORE_STEAM_TCSR_H_
