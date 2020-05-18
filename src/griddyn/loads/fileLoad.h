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
#include "gmlc/utilities/TimeSeriesMulti.hpp"
#include "rampLoad.h"

namespace griddyn {
namespace loads {
    /** @brief a load that generates its value from files*/
    class fileLoad: public rampLoad {
      public:
        enum file_load_flags {
            use_absolute_time_flag = object_flag7,
            use_step_change_flag = object_flag8,
        };

      protected:
        std::string fileName_;  //!< the name of the file
        gmlc::utilities::TimeSeriesMulti<double, coreTime>
            schedLoad;  //!< time series containing the load information
        units::unit inputUnits = units::defunit;
        double scaleFactor = 1.0;  //!< scaling factor on the load
        index_t currIndex = 0;  //!< the current index on timeSeries
        count_t count = 0;
        double qratio = kNullVal;
        std::vector<int> columnkey;

      public:
        explicit fileLoad(const std::string& objName = "fileLoad_$");
        fileLoad(const std::string& objName, std::string fileName);
        coreObject* clone(coreObject* obj = nullptr) const override;
        virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;

        virtual void updateA(coreTime time) override;

        virtual void
            timestep(coreTime time, const IOdata& inputs, const solverMode& sMode) override;

        virtual void setFlag(const std::string& flag, bool val = true) override;
        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param,
                         double val,
                         units::unit unitType = units::defunit) override;

      private:
        count_t loadFile();
    };
}  // namespace loads
}  // namespace griddyn
