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

#include "griddyn_export.h"
#include "internal/griddyn_export_internal.h"
#include "griddyn/gridDynSimulation.h"
#include "fileInput/gridDynRunner.h"
#include "fileInput/fileInput.h"
#include "core/coreExceptions.h"

using namespace griddyn;





gridDynSimReference gridDynSimulation_create(const char *type, const char *name)
{
	GriddynRunner *sim;
	std::string typeStr(type);
	if (typeStr == "helics")
	{
		return nullptr;
	}
	else if (typeStr == "buildfmu")
	{
		return nullptr;
	}
	else if (typeStr == "dime")
	{
		return nullptr;
	}
	else if (typeStr == "buildgdz")
	{
		return nullptr;
	}
	else
	{
		sim = new GriddynRunner();
		
	}
	if (sim != nullptr)
	{
		sim->getSim()->setName(name);
	}
	

	return reinterpret_cast<gridDynSimReference>(sim);
	
}


void gridDynSimulation_free(gridDynSimReference sim)
{
	if (sim != nullptr)
	{
		delete reinterpret_cast<GriddynRunner *>(sim);
	}
}

griddyn_status gridDynSimulation_initializeFromString(gridDynSimReference sim, const char *initializationString)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	return runner->InitializeFromString(initializationString);
}

griddyn_status gridDynSimulation_initializeFromArgs(gridDynSimReference sim, int argc, char * argv[], int ignoreUnrecognized)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	return runner->Initialize(argc,argv,(ignoreUnrecognized!=0));
}

griddyn_status gridDynSimulation_loadfile(gridDynSimReference sim, const char *filename, const char *fileType)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	
	try
	{
		auto typestr = std::string(fileType);
		if (typestr.empty())
		{
			loadFile(runner->getSim().get(), filename);
		}
		else
		{
			loadFile(runner->getSim().get(), filename, nullptr, typestr);
		}
		return griddyn_ok;
	}
	catch(...)
	{
		return griddyn_file_load_failure;
	}
}

griddyn_status gridDynSimulation_addCommand(gridDynSimReference sim, const char *command)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	gridDynAction action(command);
	if (action.command != gridDynAction::gd_action_t::invalid)
	{
		runner->getSim()->add(action);
		return griddyn_ok;
	}
	return griddyn_add_failure;
	
}

griddyn_status gridDynSimulation_run(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	try
	{
		runner->Run();
	}
	catch (...)
	{
		return griddyn_solve_error;

	}
	return griddyn_ok;
}

griddyn_status gridDynSimulation_runTo(gridDynSimReference sim, double runToTime)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	try
	{
		runner->Step(runToTime);
	}
	catch (...)
	{
		return griddyn_solve_error;
	}
	return griddyn_ok;
}

griddyn_status gridDynSimulation_Step(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	auto ret=runner->getSim()->step();
	return ret;
}


griddyn_status gridDynSimulation_runAsync(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	try
	{
		runner->RunAsync();
	}
	catch (const executionFailure &)
	{
		return FUNCTION_EXECUTION_FAILURE;
	}
	return 0;
}


griddyn_status gridDynSimulation_runToAsync(gridDynSimReference sim, double runToTime)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	try
	{
		runner->StepAsync(runToTime);
	}
	catch (const executionFailure &)
	{
		return FUNCTION_EXECUTION_FAILURE;
	}
	return 0;
}


griddyn_status gridDynSimulation_StepAsync(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	return 0;
}

int gridDynSimulation_getStatus(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	coreTime tRet;
	auto res=runner->getStatus(tRet);
	return res;
}


gridDynObject getSimulationObject(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return nullptr;
	}
    runner->getSim()->addOwningReference();
	return creategridDynObject(runner->getSim().get());
}

griddyn_status gridDynSimulation_powerflowInitialize(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	return runner->getSim()->pFlowInitialize();
}

griddyn_status gridDynSimulation_powerflow(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	return runner->getSim()->powerflow();
}

griddyn_status gridDynSimulation_dynamicInitialize(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	runner->simInitialize();
	return griddyn_ok;
}

griddyn_status gridDynSimulation_reset(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return griddyn_invalid_object;
	}
	return runner->Reset();
}

double gridDynSimulation_getCurrentTime(gridDynSimReference sim)
{
    auto runner = reinterpret_cast<GriddynRunner *>(sim);

    if (runner == nullptr)
    {
        return kNullVal;
    }
    return static_cast<double>(runner->getSim()->getSimulationTime());
}