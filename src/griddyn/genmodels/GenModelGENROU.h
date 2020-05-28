/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GENMODELGENROU_H_
#define GENMODELGENROU_H_

#include "GenModel5.h"

namespace griddyn {
namespace genmodels {
    class GenModelGENROU: public GenModel5 {
      protected:
      public:
        explicit GenModelGENROU(const std::string& objName = "genrou_#");
        virtual coreObject* clone(coreObject* obj = nullptr) const override;
        virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;
        virtual void dynObjectInitializeB(const IOdata& inputs,
                                          const IOdata& desiredOutput,
                                          IOdata& fieldSet) override;

        virtual stringVec localStateNames() const override;
        // dynamics
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
        virtual void algebraicUpdate(const IOdata& inputs,
                                     const stateData& sD,
                                     double update[],
                                     const solverMode& sMode,
                                     double alpha) override;
    };

}  // namespace genmodels
}  // namespace griddyn
#endif  // GENMODELGENROU_H_
