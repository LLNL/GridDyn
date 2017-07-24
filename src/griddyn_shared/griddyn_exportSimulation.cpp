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

#include "griddyn_export.h"
#include "gridDyn_export_internal.h"
#include "gridDyn.h"
#include "fileInput/gridDynRunner.h"
#include "fileInput/fileInput.h"

using namespace griddyn;





gridDynSimReference griddynSimulation_create(const char *type, const char *name)
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


void griddynSimulation_free(gridDynSimReference sim)
{
	if (sim != nullptr)
	{
		delete reinterpret_cast<GriddynRunner *>(sim);
	}
}

int griddynSimulation_initializeFromString(gridDynSimReference sim, const char *initializationString)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	return runner->InitializeFromString(initializationString);
}

int griddynSimulation_initializeFromArgs(gridDynSimReference sim, int argc, char *argv[], bool ignoreUnrecognized)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	return runner->Initialize(argc,argv,ignoreUnrecognized);
}

int griddynSimulation_loadfile(gridDynSimReference sim, const char *filename, const char *fileType)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
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
		return EXECUTION_SUCCESS;
	}
	catch(...)
	{
		return FILE_LOAD_FAILURE;
	}
}

int griddynSimulation_addCommand(gridDynSimReference sim, const char *command)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	gridDynAction action(command);
	if (action.command != gridDynAction::gd_action_t::invalid)
	{
		runner->getSim()->add(action);
		return EXECUTION_SUCCESS;
	}
	return ADD_FAILURE;
	
}

int griddynSimulation_run(gridDynSimReference sim, double *actualTime)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	try
	{
		runner->Run();
		*actualTime = runner->getSim()->getCurrentTime();
	}
	catch (...)
	{
		*actualTime = runner->getSim()->getCurrentTime();
		return SOLVE_ERROR;

	}
	return EXECUTION_SUCCESS;
}

int griddynSimulation_runTo(gridDynSimReference sim, double runToTime, double *actualTime)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	try
	{
		runner->Step(runToTime);
		*actualTime = runner->getSim()->getCurrentTime();
	}
	catch (...)
	{
		*actualTime = runner->getSim()->getCurrentTime();
		return SOLVE_ERROR;
	}
	return EXECUTION_SUCCESS;
}

int griddynSimulation_Step(gridDynSimReference sim, double *actualTime)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto ret=runner->getSim()->step();
	*actualTime = runner->getSim()->getCurrentTime();
	return ret;
}


gridDynObject getSimulationObject(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	return creategridDynObject(runner->getSim().get());
}

int griddynSimulation_powerflowInitialize(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	return runner->getSim()->pFlowInitialize();
}

int griddynSimulation_powerflow(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	return runner->getSim()->powerflow();
}

int griddynSimulation_dynamicInitialize(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	runner->simInitialize();
	return EXECUTION_SUCCESS;
}

int gridDynSimulation_reset(gridDynSimReference sim)
{
	auto runner = reinterpret_cast<GriddynRunner *>(sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	return runner->Reset();
}