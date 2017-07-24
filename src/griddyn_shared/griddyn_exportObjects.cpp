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
#include "griddyn_export_internal.h"

#include "core/coreOwningPtr.hpp"
#include "core/objectFactory.hpp"
#include "core/coreExceptions.h"

#include "gridBus.h"
#include "Area.h"
#include "Load.h"
#include "Exciter.h"
#include "Governor.h"
#include "GenModel.h"
#include "Generator.h"
#include "Link.h"
#include "Source.h"
#include "Block.h"
#include "relays/Sensor.h"
#include "gridDyn.h"

#include <cstring>
#include <map>
using namespace griddyn;


gridDynObject creategridDynObject(griddyn::gridComponent *comp)
{
	if (comp == nullptr)
	{
		return nullptr;
	}
	auto ptr = new coreOwningPtr<gridComponent>(comp);
	return reinterpret_cast<void *>(ptr);
}

griddyn::gridComponent *getComponentPointer(gridDynObject obj)
{
	if (obj != nullptr)
	{
		auto cptr = reinterpret_cast<coreOwningPtr<gridComponent> *>(obj);
		return cptr->get();
	}
	return nullptr;
}

gridDynObject griddynObject_create(const char *componentType, const char *objectType)
{
	auto newObject = coreObjectFactory::instance()->createObject(componentType, objectType);
	if (newObject == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	gridComponent *comp = dynamic_cast<gridComponent *>(newObject);
	if (comp == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	auto ptr = new coreOwningPtr<gridComponent>(comp);
	return reinterpret_cast<void *>(ptr);
}

void griddynObject_free(gridDynObject obj)
{
	if (obj != nullptr)
	{
		auto cptr = reinterpret_cast<coreOwningPtr<gridComponent> *>(obj);
		delete cptr;
	}
}

int griddynObject_add(gridDynObject parentObject, gridDynObject objectToAdd)
{
	gridComponent *parent = getComponentPointer(parentObject);
	coreObject *child = getComponentPointer(objectToAdd);

	try
	{
		parent->add(child);
		return FUNCTION_EXECUTION_SUCCESS;
	}
	catch (const objectAddFailure &)
	{
		return ADD_FAILURE;
	}
	
}

int griddynObject_remove(gridDynObject parentObject, gridDynObject objectToRemove)
{
	gridComponent *parent = getComponentPointer(parentObject);
	coreObject *child = getComponentPointer(objectToRemove);

	if ((parent == nullptr) || (child == nullptr))
	{
		return INVALID_OBJECT;
	}
	try
	{
		parent->remove(child);
		return FUNCTION_EXECUTION_SUCCESS;
	}
	catch (const objectRemoveFailure &)
	{
		return REMOVE_FAILURE;
	}
}

int griddynObject_setString(gridDynObject obj, const char *parameter, const char *value)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	try
	{
		comp->set(parameter, value);
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

int griddynObject_setValue(gridDynObject obj, const char *parameter, double value, const char *units)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto unitType=gridUnits::getUnits(units);
	try
	{
		comp->set(parameter, value,unitType);
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

int griddynObject_setFlag(gridDynObject obj, const char *flag, bool val)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	try
	{
		comp->set(flag, val);
		return EXECUTION_SUCCESS;
	}
	catch (const unrecognizedParameter &)
	{
		return UNKNOWN_PARAMETER;
	}
}

int griddynObject_getString(gridDynObject obj, const char *parameter, char *value, int N)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto s = comp->getString(parameter);
	strncpy(value, s.c_str(), N);
	return static_cast<int>(s.size());
	
}

int griddynObject_getValue(gridDynObject obj, const char *parameter, const char *units, double *result)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto unitType = gridUnits::getUnits(units);
	try
	{
		*result = comp->get(parameter, unitType);
		if (*result == kNullVal)
		{
			return UNKNOWN_PARAMETER;
		}
		return EXECUTION_SUCCESS;
	}
	catch (const unrecognizedParameter &)
	{
		return UNKNOWN_PARAMETER;
	}
	
}

int griddynObject_getFlag(gridDynObject obj, const char *flag, bool *result)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	try
	{
		*result=comp->getFlag(flag);
		return EXECUTION_SUCCESS;
	}
	catch (const unrecognizedParameter)
	{
		return UNKNOWN_PARAMETER;
	}
}

gridDynObject griddynObject_find(gridDynObject obj, const char *objectToFind)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	auto res = comp->find(objectToFind);
	if (res == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	auto compNew = dynamic_cast<gridComponent *>(res);
	if (compNew == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	return creategridDynObject(compNew);
}

gridDynObject griddynObject_getSubObject(gridDynObject obj, const char *componentType, int N)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return EXECUTION_SUCCESS;
	}

	auto res = comp->getSubObject(componentType, static_cast<index_t>(N));
	if (res == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	auto compNew = dynamic_cast<gridComponent *>(res);
	if (compNew == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	return creategridDynObject(compNew);
}

gridDynObject griddynObject_findByUserId(gridDynObject obj, const char *componentType, int ID)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	auto res = comp->findByUserID(componentType, static_cast<index_t>(ID));
	if (res == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	auto compNew = dynamic_cast<gridComponent *>(res);
	if (compNew == nullptr)
	{
		return EXECUTION_SUCCESS;
	}
	return creategridDynObject(compNew);
}

gridDynObject griddynObject_getParent(gridDynObject obj)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return nullptr;
	}
	auto compNew = dynamic_cast<gridComponent *>(comp->getParent());
	if (compNew == nullptr)
	{
		return nullptr;
	}
	return creategridDynObject(compNew);
}
static const char * invalid_str = "invalid";
static const char * bus_str = "bus";
static const char * area_str = "area";
static const char * link_str = "link";
static const char * load_str = "load";
static const char * generator_str = "generator";

static const char * sim_str = "simulation";
static const char *exciter_str = "exciter";
static const char *scheduler_str = "scheduler";
static const char *governor_str = "governor";
static const char *genModel_str = "genModel";
static const char *block_str = "block";
static const char *source_str = "source";
static const char *relay_str = "relay";
static const char *sensor_str = "sensor";
static const char *submodel_str = "submodel";
static const char *unknown_str = "unknown";


const char *griddynObject_getType(gridDynObject obj)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return invalid_str;
	}
	if (dynamic_cast<gridBus *>(comp) != nullptr)
	{
		return bus_str;
	}
	if (dynamic_cast<Link *>(comp) != nullptr)
	{
		return link_str;
	}
	if (dynamic_cast<gridDynSimulation *>(comp) != nullptr)
	{
		return sim_str;
	}
	if (dynamic_cast<Area *>(comp) != nullptr)
	{
		return area_str;
	}
	if (dynamic_cast<Load *>(comp) != nullptr)
	{
		return load_str;
	}
	if (dynamic_cast<Generator *>(comp) != nullptr)
	{
		return generator_str;
	}
	if (dynamic_cast<Governor *>(comp) != nullptr)
	{
		return governor_str;
	}
	if (dynamic_cast<Exciter *>(comp) != nullptr)
	{
		return exciter_str;
	}
	if (dynamic_cast<GenModel *>(comp) != nullptr)
	{
		return genModel_str;
	}
	if (dynamic_cast<Source *>(comp) != nullptr)
	{
		return source_str;
	}
	if (dynamic_cast<Block *>(comp) != nullptr)
	{
		return block_str;
	}
	if (dynamic_cast<sensor *>(comp) != nullptr)
	{
		return sensor_str;
	}
	if (dynamic_cast<Relay *>(comp) != nullptr)
	{
		return relay_str;
	}

	if (dynamic_cast<gridSubModel *>(comp) != nullptr)
	{
		return submodel_str;
	}
	return unknown_str;
}