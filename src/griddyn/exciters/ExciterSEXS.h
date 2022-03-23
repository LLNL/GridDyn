/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EXCITER_SEXS_H_
#define EXCITER_SEXS_H_

#include "Exciter.h"
namespace griddyn {
namespace exciters {
    /** @brief SEXS exciter
     */
    class ExciterSEXS: public Exciter {
      protected:
        double Te{0.0};
        double Tb{1.0};

      public:
        explicit ExciterSEXS(const std::string& objName = "exciterSEXS_#");
        virtual coreObject* clone(coreObject* obj = nullptr) const override;

        virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;
        virtual void dynObjectInitializeB(const IOdata& inputs,
                                          const IOdata& desiredOutput,
                                          IOdata& fieldSet) override;

        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param,
                         double val,
                         units::unit unitType = units::defunit) override;

        virtual stringVec localStateNames() const override;

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

        virtual void rootTest(const IOdata& inputs,
                              const stateData& sD,
                              double root[],
                              const solverMode& sMode) override;
        virtual change_code rootCheck(const IOdata& inputs,
                                      const stateData& sD,
                                      const solverMode& sMode,
                                      check_level_t level) override;
        // virtual void setTime(coreTime time){prevTime=time;};
      protected:
        /** @brief the Jacobian entries for the limiter
    @param[in] V the voltage
    @param[in] Vloc the location of the voltage
    @param[in] refloc  the location of the reference
    @param[in] cj  the differential scale variable
    @param[out] md the array structure to store the Jacobian data in
    */
        virtual void
            limitJacobian(double V, int Vloc, int refLoc, double cj, matrixData<double>& md);
    };
}  // namespace exciters
}  // namespace griddyn

#endif  // EXCITER_DC1A_H_
