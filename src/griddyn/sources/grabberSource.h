/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include "rampSource.h"

namespace griddyn {
class grabberSet;
namespace sources {
    /** source to grab data from another location and use it in another context*/
    class grabberSource: public rampSource {
      private:
        std::unique_ptr<grabberSet> gset;  //!< the grabberSet to get the data
        std::string field;  //!< the field to grab
        std::string target;  //!< the name of the target
        model_parameter multiplier;  //!< a multiplier on the grabber value
      public:
        grabberSource(const std::string& objName = "grabbersource_#");
        ~grabberSource();

      protected:
        virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;

        virtual void dynObjectInitializeB(const IOdata& inputs,
                                          const IOdata& desiredOutput,
                                          IOdata& fieldSet) override;

        virtual coreObject* clone(coreObject* obj = nullptr) const override;
        /** update the target field of the grabber*/
        void updateField(const std::string& newField);

        /** update the target object of the grabber*/
        void updateTarget(const std::string& newTarget);

        /** update the target object of the grabber directly*/
        void updateTarget(coreObject* obj);

        virtual void setFlag(const std::string& flag, bool val) override;
        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param,
                         double val,
                         units::unit unitType = units::defunit) override;
        virtual double get(const std::string& param,
                           units::unit unitType = units::defunit) const override;

        virtual IOdata getOutputs(const IOdata& inputs,
                                  const stateData& sD,
                                  const solverMode& sMode) const override;
        virtual double getOutput(const IOdata& inputs,
                                 const stateData& sD,
                                 const solverMode& sMode,
                                 index_t outputNum = 0) const override;

        virtual double getOutput(index_t outputNum = 0) const override;

        virtual double getDoutdt(const IOdata& inputs,
                                 const stateData& sD,
                                 const solverMode& sMode,
                                 index_t num = 0) const override;
    };
}  // namespace sources
}  // namespace griddyn
