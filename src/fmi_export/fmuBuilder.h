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

#ifndef _FMI_BUILDER_H_
#define _FMI_BUILDER_H_

#include "core/coreOwningPtr.hpp"
#include "runner/gridDynRunner.h"
#include <memory>
#include <string>
#include <vector>

namespace griddyn {
class readerInfo;

namespace fmi {
    class fmiCoordinator;

    // class gridDynSimulation;
    /** object to build an FMI object from a gridDyn simulation file*/
    class fmuBuilder: public GriddynRunner {
      private:
        std::string fmuLoc;  //!< location to place the FMU
        std::vector<unsigned int> vrs;  //!< the value references
        coreOwningPtr<fmiCoordinator> coord_;  //!< coordinator to maintain organize everything
        std::unique_ptr<readerInfo> ri_;  //!< location of readerInfo
        std::string execPath;  //!< location of the executable making the fmu
        std::string platform = "all";  //!< target platform for the fmu
        bool keep_dir = false;
        /** private function for loading the subcomponents*/
        void loadComponents();
        void generateXML(const std::string& xmlfile);

      public:
        fmuBuilder();
        fmuBuilder(std::shared_ptr<gridDynSimulation> gds);
        virtual ~fmuBuilder();

      public:
        virtual std::shared_ptr<CLI::App>
            generateLocalCommandLineParser(readerInfo& ri) override final;

        /** build the FMU at the given location
    @param[in] fmuLocation optional argument to specify the location to build the FMU*/
        void MakeFmu(const std::string& fmuLocation = "");
        const std::string& getOutputFile() const { return fmuLoc; }

      private:
        void copySharedLibrary(const std::string& tempdir);
    };

}  // namespace fmi
}  // namespace griddyn
#endif
