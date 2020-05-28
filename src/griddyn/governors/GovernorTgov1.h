/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GOVERNOR_TGOV1_H_
#define GOVERNOR_TGOV1_H_
#pragma once

#include "GovernorIeeeSimple.h"

namespace griddyn {
namespace governors {
    class GovernorTgov1: public GovernorIeeeSimple {
      public:
      protected:
        double Dt = 0.0;  //!< speed damping constant
      public:
        explicit GovernorTgov1(const std::string& objName = "govTgov1_#");
        virtual coreObject* clone(coreObject* obj = nullptr) const override;
        virtual ~GovernorTgov1();
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
        virtual void derivative(const IOdata& inputs,
                                const stateData& sD,
                                double deriv[],
                                const solverMode& sMode) override;
        virtual void jacobianElements(const IOdata& inputs,
                                      const stateData& sD,
                                      matrixData<double>& md,
                                      const IOlocs& inputLocs,
                                      const solverMode& sMode) override;
        virtual void
            timestep(coreTime time, const IOdata& inputs, const solverMode& sMode) override;
        virtual void rootTest(const IOdata& inputs,
                              const stateData& sD,
                              double roots[],
                              const solverMode& sMode) override;
        virtual void rootTrigger(coreTime time,
                                 const IOdata& inputs,
                                 const std::vector<int>& rootMask,
                                 const solverMode& sMode) override;
    };
}  // namespace governors
}  // namespace griddyn

#endif  // GOVERNOR_TGOV_1
