/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */
#pragma once

#include "../fmi_models/fmiCoSimSubModel.h"
#include "fmiWrapper.hpp"
namespace griddyn {
namespace fmi {
    template<class BaseObj>
    class fmiCoSimWrapper: public fmiWrapper<fmiCoSimSubModel, BaseObj> {
      public:
        fmiCoSimWrapper(const std::string& objName): fmiWrapper<fmiCoSimSubModel, BaseObj>(objName)
        {
        }
        coreObject* clone(coreObject* obj) const override
        {
            auto nobj =
                cloneBase<fmiCoSimWrapper, fmiWrapper<fmiCoSimSubModel, BaseObj>>(this, obj);
            if (nobj == nullptr) {
                return obj;
            }
            return nobj;
        }

        void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override
        {
            if (fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->isLoaded()) {
                fmiWrapper<fmiCoSimSubModel, BaseObj>::setupFmiIo();
                SET_CONTROLFLAG(flags, force_constant_pflow_initialization);

                BaseObj::pFlowObjectInitializeA(time0, flags);

            } else {
                this->disable();
            }
        }

        void residual(const IOdata& inputs,
                      const stateData& sD,
                      double resid[],
                      const solverMode& sMode) override
        {
            fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->residual(inputs, sD, resid, sMode);
        }

        void derivative(const IOdata& inputs,
                        const stateData& sD,
                        double deriv[],
                        const solverMode& sMode) override
        {
            fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->derivative(inputs, sD, deriv, sMode);
        }

        void outputPartialDerivatives(const IOdata& inputs,
                                      const stateData& sD,
                                      matrixData<double>& md,
                                      const solverMode& sMode) override
        {
            fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->outputPartialDerivatives(inputs,
                                                                                    sD,
                                                                                    md,
                                                                                    sMode);
        }
        void ioPartialDerivatives(const IOdata& inputs,
                                  const stateData& sD,
                                  matrixData<double>& md,
                                  const IOlocs& inputLocs,
                                  const solverMode& sMode) override
        {
            fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->ioPartialDerivatives(
                inputs, sD, md, inputLocs, sMode);
        }
        void jacobianElements(const IOdata& inputs,
                              const stateData& sD,
                              matrixData<double>& md,
                              const IOlocs& inputLocs,
                              const solverMode& sMode) override
        {
            fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->jacobianElements(
                inputs, sD, md, inputLocs, sMode);
        }

        void rootTest(const IOdata& inputs,
                      const stateData& sD,
                      double roots[],
                      const solverMode& sMode) override
        {
            fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->rootTest(inputs, sD, roots, sMode);
        }
        void rootTrigger(coreTime time,
                         const IOdata& inputs,
                         const std::vector<int>& rootMask,
                         const solverMode& sMode) override
        {
            fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->rootTrigger(time,
                                                                       inputs,
                                                                       rootMask,
                                                                       sMode);
        }

        void setState(coreTime time,
                      const double state[],
                      const double dstate_dt[],
                      const solverMode& sMode) override
        {
            fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->setState(time, state, dstate_dt, sMode);
        }

        index_t findIndex(const std::string& field, const solverMode& sMode) const override
        {
            return fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->findIndex(field, sMode);
        }

        void timestep(coreTime time, const IOdata& inputs, const solverMode& sMode) override
        {
            BaseObj::prevTime = time;
            fmiWrapper<fmiCoSimSubModel, BaseObj>::fmisub->timestep(time, inputs, sMode);
        }
    };
}  //namespace fmi
}  //namespace griddyn
