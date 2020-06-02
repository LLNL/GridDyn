/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EXPONENTIALLOAD_H_
#define EXPONENTIALLOAD_H_

#include "../Load.h"

namespace griddyn {
namespace loads {
    /** @brief a load with powers as a exponential function of voltage*/
    class exponentialLoad: public Load {
      public:
      protected:
        model_parameter alphaP = 0.0;  //!< the voltage exponent parameter for the real power output
        model_parameter alphaQ =
            0.0;  //!< the voltage exponent parameter for the reactive power output

      public:
        explicit exponentialLoad(const std::string& objName = "expLoad_$");
        /** constructor taking power arguments
    @param[in] rP the real power of the load
    @param[in] qP the reactive power of the load
    @param[in] objName the name of the object
    */
        exponentialLoad(double rP, double qP, const std::string& objName = "expLoad_$");

        virtual coreObject* clone(coreObject* obj = nullptr) const override;

        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param,
                         double val,
                         units::unit unitType = units::defunit) override;

        virtual void ioPartialDerivatives(const IOdata& inputs,
                                          const stateData& sD,
                                          matrixData<double>& md,
                                          const IOlocs& inputLocs,
                                          const solverMode& sMode) override;
        virtual double getRealPower(const IOdata& inputs,
                                    const stateData& sD,
                                    const solverMode& sMode) const override;
        virtual double getReactivePower(const IOdata& inputs,
                                        const stateData& sD,
                                        const solverMode& sMode) const override;
        virtual double getRealPower(double V) const override;
        virtual double getReactivePower(double V) const override;
        virtual double getRealPower() const override;
        virtual double getReactivePower() const override;
    };
}  // namespace loads
}  // namespace griddyn

#endif
