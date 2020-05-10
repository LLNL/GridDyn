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

#ifndef CYMEDIST_LOAD_H_
#define CYMEDIST_LOAD_H_

#include "fmiMELoad3phase.h"

namespace griddyn {
namespace fmi {
    class CymeDistLoadME: public fmiMELoad3phase {
        static std::atomic<int> indexCounter;
        std::string configFile;
        index_t configIndex = 0;

      public:
        CymeDistLoadME(const std::string& objName = "cymedist_$");
        virtual coreObject* clone(coreObject* obj = nullptr) const override;

        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param,
                         double val,
                         units::unit unitType = units::defunit) override;

      private:
        void loadConfigFile(const std::string& configFileName);
    };

}  // namespace fmi
}  // namespace griddyn

#endif  // CYMEDIST_LOAD_COSIM_H_
