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

#ifndef FMI_MEWRAPPER_H_
#define FMI_MEWRAPPER_H_

#include "../fmi_models/fmiMESubModel.h"
#include "fmiWrapper.hpp"

namespace griddyn {
namespace fmi {
    template<class BaseObj>
    class fmiMEWrapper: public fmiWrapper<fmiMESubModel, BaseObj> {
      public:
        fmiMEWrapper(const std::string& objName): fmiWrapper<fmiMESubModel, BaseObj>(objName) {}
        virtual coreObject* clone(coreObject* obj) const override
        {
            auto nobj = cloneBase<fmiMEWrapper, fmiWrapper<fmiMESubModel, BaseObj>>(this, obj);
            if (nobj == nullptr) {
                return obj;
            }

            return nobj;
        }

        virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                this->disable();
                return;
            }
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub->isLoaded()) {
                fmiWrapper<fmiMESubModel, BaseObj>::setupFmiIo();
                SET_CONTROLFLAG(flags, force_constant_pflow_initialization);
                BaseObj::pFlowObjectInitializeA(time0, flags);
            } else {
                this->disable();
            }
        }

        virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override
        {
            if (!BaseObj::opFlags[pFlow_initialized]) {  // just to make sure we actually
                                                         // initialized and didn't go directly to
                                                         // the dynamic initialization
                pFlowObjectInitializeA(time0, flags);
            }
        }
        virtual void residual(const IOdata& inputs,
                              const stateData& sD,
                              double resid[],
                              const solverMode& sMode) override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return;
            }
            fmiWrapper<fmiMESubModel, BaseObj>::fmisub->residual(inputs, sD, resid, sMode);
        }

        virtual void derivative(const IOdata& inputs,
                                const stateData& sD,
                                double deriv[],
                                const solverMode& sMode) override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return;
            }
            fmiWrapper<fmiMESubModel, BaseObj>::fmisub->derivative(inputs, sD, deriv, sMode);
        }

        virtual void outputPartialDerivatives(const IOdata& inputs,
                                              const stateData& sD,
                                              matrixData<double>& md,
                                              const solverMode& sMode) override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return;
            }
            fmiWrapper<fmiMESubModel, BaseObj>::fmisub->outputPartialDerivatives(inputs,
                                                                                 sD,
                                                                                 md,
                                                                                 sMode);
        }
        virtual void ioPartialDerivatives(const IOdata& inputs,
                                          const stateData& sD,
                                          matrixData<double>& md,
                                          const IOlocs& inputLocs,
                                          const solverMode& sMode) override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return;
            }
            fmiWrapper<fmiMESubModel, BaseObj>::fmisub->ioPartialDerivatives(
                inputs, sD, md, inputLocs, sMode);
        }
        virtual void jacobianElements(const IOdata& inputs,
                                      const stateData& sD,
                                      matrixData<double>& md,
                                      const IOlocs& inputLocs,
                                      const solverMode& sMode) override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return;
            }
            fmiWrapper<fmiMESubModel, BaseObj>::fmisub->jacobianElements(
                inputs, sD, md, inputLocs, sMode);
        }

        virtual void rootTest(const IOdata& inputs,
                              const stateData& sD,
                              double roots[],
                              const solverMode& sMode) override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return;
            }
            fmiWrapper<fmiMESubModel, BaseObj>::fmisub->rootTest(inputs, sD, roots, sMode);
        }
        virtual void rootTrigger(coreTime time,
                                 const IOdata& inputs,
                                 const std::vector<int>& rootMask,
                                 const solverMode& sMode) override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return;
            }
            fmiWrapper<fmiMESubModel, BaseObj>::fmisub->rootTrigger(time, inputs, rootMask, sMode);
        }

        virtual void setState(coreTime time,
                              const double state[],
                              const double dstate_dt[],
                              const solverMode& sMode) override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return;
            }
            fmiWrapper<fmiMESubModel, BaseObj>::fmisub->setState(time, state, dstate_dt, sMode);
        }

        virtual index_t findIndex(const std::string& field, const solverMode& sMode) const override
        {
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return kInvalidLocation;
            }
            return fmiWrapper<fmiMESubModel, BaseObj>::fmisub->findIndex(field, sMode);
        }

        virtual void timestep(coreTime time, const IOdata& inputs, const solverMode& sMode) override
        {
            BaseObj::prevTime = time;
            if (fmiWrapper<fmiMESubModel, BaseObj>::fmisub == nullptr) {
                return;
            }
            fmiWrapper<fmiMESubModel, BaseObj>::fmisub->timestep(time, inputs, sMode);
        }
    };
}  // namespace fmi
}  // namespace griddyn
#endif
