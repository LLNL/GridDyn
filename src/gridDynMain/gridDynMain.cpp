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

// headers
#include "griddyn-config.h"
#include "griddyn.h"

#include "gridDynRunner.h"

#ifdef HELICS_EXECUTABLE
#include "helics/helicsRunner.h"
#endif

#ifdef DIME_ENABLE
#include "zmqlib/dimeRunner.h"
#endif

#ifdef BUILD_SHARED_FMI_LIBRARY
#include "fmi_export/fmuBuilder.h"
#endif

enum class execMode_t
{
    normal = 0,
    mpicount = 1,
    helics = 2,
    buildfmu = 3,
    dime = 4,
	buildgdz=5,
};

using namespace griddyn;
// main
int main (int argc, char *argv[])
{
    auto gds = std::make_shared<gridDynSimulation> ();

    // Store the simulation pointer somewhere so that it can be accessed in other modules.
    gridDynSimulation::setInstance (gds.get ());  // peer to gridDynSimulation::GetInstance ();

    auto execMode = execMode_t::normal;
    // check for different options
    for (int ii = 1; ii < argc; ++ii)
    {
        if (strcmp ("--mpicount", argv[ii])==0)
        {
            execMode = execMode_t::mpicount;
            break;
        }
		if (strncmp("--buildgdz", argv[ii], 10) == 0)
		{
			execMode = execMode_t::buildgdz;
			break;
		}
#ifdef BUILD_SHARED_FMI_LIBRARY
        if (strncmp ("--buildfmu", argv[ii], 10)==0)
        {
            execMode = execMode_t::buildfmu;
            break;
        }
#endif
#ifdef HELICS_EXECUTABLE
        if (strcmp ("--helics", argv[ii])==0)
        {
            execMode = execMode_t::helics;

            break;
        }
#endif
#ifdef DIME_ENABLE
        if (strcmp ("--dime", argv[ii])==0)
        {
            execMode = execMode_t::dime;
            break;
        }
#endif
    }

    switch (execMode)
    {
    case execMode_t::normal:
    {
        auto runner = std::make_unique<GriddynRunner> (gds);
        auto ret = runner->Initialize (argc, argv);
        if (ret > 0)
        {
            return 0;
        }
        if (ret < 0)
        {
            return ret;
        }
        runner->simInitialize ();
        runner->Run ();
    }
    break;
    case execMode_t::mpicount:
    {
        auto runner = std::make_unique<GriddynRunner> (gds);
        auto ret = runner->Initialize (argc, argv);
        if (ret > 0)
        {
            return 0;
        }
        if (ret < 0)
        {
            return ret;
        }
        gds->countMpiObjects (true);
    }
        return 0;
    case execMode_t::buildfmu:
#ifdef BUILD_SHARED_FMI_LIBRARY
    {
		gds->log(nullptr, print_level::summary,
			std::string("Building FMI through FMI builder"));
        auto builder = std::make_unique<fmi::fmuBuilder> (gds);
        auto ret = builder->Initialize (argc, argv);
        if (ret < 0)
        {
            return ret;
        }
        builder->MakeFmu ();
    }
#endif
        return 0;
    case execMode_t::helics:
    {
#ifdef HELICS_EXECUTABLE
        auto runner = std::make_unique<helicsLib::helicsRunner> (gds);
		gds->log(nullptr, print_level::summary,
			std::string("Executing through HELICS runner"));
        auto ret = runner->Initialize (argc, argv);
        if (ret > 0)
        {
            return 0;
        }
        if (ret < 0)
        {
            return ret;
        }
        runner->simInitialize ();
        runner->Run ();
#endif
    }
    break;
    case execMode_t::dime:
    {
#ifdef DIME_ENABLE
        auto runner = std::make_unique<dimeLib::dimeRunner> (gds);
		gds->log(nullptr, print_level::summary,
			std::string("Executing through DIME runner"));
        auto ret = runner->Initialize (argc, argv);
        if (ret > 0)
        {
            return 0;
        }
        if (ret < 0)
        {
            return ret;
        }
        runner->simInitialize ();
        runner->Run ();
#endif
    }
	case execMode_t::buildgdz:
		gds->log(nullptr, print_level::error,
			std::string("GDZ builder not implemented yet"));
		return (-4);
	default:
		gds->log(nullptr, print_level::error,
			std::string("unknown execution mode"));
		return (-4);
    break;
    };

    auto pState = gds->currentProcessState ();
    if (pState >= gridDynSimulation::gridState_t::DYNAMIC_COMPLETE)
    {
        auto ssize = gds->getInt ("dynstatesize");
        auto jsize = gds->getInt ("dynnonzeros");
        gds->log (nullptr, print_level::summary,
                  std::string ("simulation final Dynamic statesize= ") + std::to_string (ssize) + ", " +
                    std::to_string (jsize) + " non zero elements in Jacobian\n");
    }
    else  // if (pState <= gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED)
    {
        auto ssize = gds->getInt ("pflowstatesize");
        auto jsize = gds->getInt ("pflownonzeros");
        gds->log (nullptr, print_level::summary,
                  std::string ("simulation final Power flow statesize= ") + std::to_string (ssize) + ", " +
                    std::to_string (jsize) + " non zero elements in Jacobian\n");
    }

    return 0;
}
