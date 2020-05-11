/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

// libraries

// headers
#include "dimeRunner.h"

#include "CLI11/CLI11.hpp"
#include "dimeInterface.h"
#include "fileInput/fileInput.h"
#include "griddyn/gridDynSimulation.h"
#include <chrono>
#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>

namespace griddyn {
namespace dimeLib {

    dimeRunner::dimeRunner()
    {
        loadDimeLibrary();
        m_gds = std::make_shared<gridDynSimulation>();
    }

    dimeRunner::~dimeRunner() = default;

    dimeRunner::dimeRunner(std::shared_ptr<gridDynSimulation> sim): GriddynRunner(std::move(sim))
    {
        loadDimeLibrary();
    }

    std::shared_ptr<CLI::App> dimeRunner::generateLocalCommandLineParser(readerInfo& ri)
    {
        loadDimeReaderInfoDefinitions(ri);

        auto parser =
            std::make_shared<CLI::App>("options related to helics executable", "helics_options");
        parser->add_option("--broker", "specify the broker address");
        parser->add_option("--period", "specify the synchronization period");
        parser->allow_extras();
        return parser;
    }

    /*
    if (dimeOptions.count("test") != 0u)
    {
        if (griddyn::helicsLib::runDimetests())
        {
            std::cout << "HELICS tests passed\n";
        }
        else
        {
            std::cout << "HELICS tests failed\n";
        }
        return 1;
    }

*/
    coreTime dimeRunner::Run() { return GriddynRunner::Run(); }

    coreTime dimeRunner::Step(coreTime time)
    {
        auto retTime = GriddynRunner::Step(time);
        // coord->updateOutputs(retTime);
        return retTime;
    }

    void dimeRunner::Finalize() { GriddynRunner::Finalize(); }

}  // namespace dimeLib
}  // namespace griddyn
