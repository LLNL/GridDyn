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
#include "griddyn_export_internal.h"
#include "griddyn/events/Event.h"
#include "fileInput/gridDynRunner.h"
#include "core/coreExceptions.h"
#include "griddyn/gridDynSimulation.h"
#include <memory>
#include <vector>

using namespace griddyn;

gridDynEvent gridDynEvent_create(const char *eventString, gridDynObject obj)
{
	auto evnt = new std::shared_ptr<Event>(make_event(eventString, getComponentPointer(obj)));
	if (evnt)
	{
		return reinterpret_cast<void *>(evnt);
	}
	return nullptr;
}

void gridDynEvent_free(gridDynEvent evnt)
{
	if (evnt != nullptr)
	{
		auto shr_event = reinterpret_cast<std::shared_ptr<Event> *>(evnt);
		delete shr_event;
	}
}

int gridDynEvent_trigger(gridDynEvent evnt)
{
	if (evnt == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto shr_event = reinterpret_cast<std::shared_ptr<Event> *>(evnt);
	if (*shr_event)
	{
		if ((*shr_event)->trigger() >= change_code::no_change)
		{
			return EXECUTION_SUCCESS;
		}
	}
	return FUNCTION_EXECUTION_FAILURE;
}

int gridDynEvent_schedule(gridDynEvent evnt, gridDynSimReference sim)
{
	if (evnt == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto shr_event = reinterpret_cast<std::shared_ptr<Event> *>(evnt);

	auto runner = reinterpret_cast<GriddynRunner *> (sim);

	if (runner == nullptr)
	{
		return INVALID_OBJECT;
	}
	try
	{
		runner->getSim()->add(*shr_event);
		return EXECUTION_SUCCESS;
	}
	catch (...)
	{
		return ADD_FAILURE;
	}
	
}

int gridDynEvent_setValue(gridDynEvent evnt, const char *parameter, double value)
{
	if (evnt == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto shr_event = reinterpret_cast<std::shared_ptr<Event> *>(evnt);
	try
	{
		shr_event->operator->()->set(parameter, value);
		return EXECUTION_SUCCESS;
	}
	catch (const invalidParameterValue &)
	{
		return INVALID_PARAMETER_VALUE;
	}
	catch (const unrecognizedParameter &)
	{
		return UNKNOWN_PARAMETER;
	}
}

int gridDynEvent_setString(gridDynEvent evnt, const char *parameter, const char *value)
{
	if (evnt == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto shr_event = reinterpret_cast<std::shared_ptr<Event> *>(evnt);
	try
	{
		shr_event->operator->()->set(parameter, value);
		return EXECUTION_SUCCESS;
	}
	catch (const invalidParameterValue &)
	{
		return INVALID_PARAMETER_VALUE;
	}
	catch (const unrecognizedParameter &)
	{
		return UNKNOWN_PARAMETER;
	}
}

int gridDynEvent_setFlag(gridDynEvent evnt, const char *flag, int val)
{
	if (evnt == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto shr_event = reinterpret_cast<std::shared_ptr<Event> *>(evnt);
	try
	{
		shr_event->operator->()->setFlag(flag, (val!=0));
		return EXECUTION_SUCCESS;
	}
	catch (const invalidParameterValue &)
	{
		return INVALID_PARAMETER_VALUE;
	}
	catch (const unrecognizedParameter &)
	{
		return UNKNOWN_PARAMETER;
	}
}

int gridDynEvent_setTarget(gridDynEvent evnt, gridDynObject obj)
{
	if (evnt == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto shr_event = reinterpret_cast<std::shared_ptr<Event> *>(evnt);
	auto comp = getComponentPointer(obj);
	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	try
	{
		shr_event->operator->()->updateObject(comp, object_update_mode::match);
		return EXECUTION_SUCCESS;
	}
	catch (...)
	{
		return ADD_FAILURE;
	}

}