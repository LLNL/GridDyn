/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GENMODEL6_TYPE2_H_
#define GENMODEL6_TYPE2_H_

#include "GenModel5type2.h"

namespace griddyn {
namespace genmodels {

    class GenModel6type2: public GenModel5type2 {
      protected:
      public:
        explicit GenModel6type2(const std::string& objName = "genModel6type2_#");
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
#endif  // GENMODEL6_TYPE2_H_
