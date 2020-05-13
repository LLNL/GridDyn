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

#include "core/coreExceptions.h"
#include "core/coreOwningPtr.hpp"
#include "core/objectFactory.hpp"
#include "griddyn/Area.h"
#include "griddyn/Block.h"
#include "griddyn/Exciter.h"
#include "griddyn/GenModel.h"
#include "griddyn/Generator.h"
#include "griddyn/Governor.h"
#include "griddyn/Link.h"
#include "griddyn/Load.h"
#include "griddyn/Source.h"
#include "griddyn/controllers/scheduler.h"
#include "griddyn/gridBus.h"
#include "griddyn/gridDynSimulation.h"
#include "griddyn/relays/sensor.h"
#include "griddyn_export.h"
#include "internal/griddyn_export_internal.h"
#include <cstring>
#include <map>
using namespace griddyn;

GridDynObject creategridDynObject(gridComponent* comp)
{
    if (comp == nullptr) {
        return nullptr;
    }
    auto ptr = new coreOwningPtr<gridComponent>(comp);
    return ptr;
}

gridComponent* getComponentPointer(GridDynObject obj)
{
    if (obj != nullptr) {
        auto cptr = reinterpret_cast<coreOwningPtr<gridComponent>*>(obj);
        return cptr->get();
    }
    return nullptr;
}

const gridComponent* getConstComponentPointer(GridDynObject obj)
{
    if (obj != nullptr) {
        auto cptr = reinterpret_cast<coreOwningPtr<gridComponent> const*>(obj);
        return cptr->get();
    }
    return nullptr;
}

GridDynObject
    gridDynObjectCreate(const char* componentType, const char* objectType, GridDynError* err)
{
    auto newObject = coreObjectFactory::instance()->createObject(componentType, objectType);
    if (newObject == nullptr) {
        return nullptr;
    }
    auto* comp = dynamic_cast<gridComponent*>(newObject);
    if (comp == nullptr) {
        return nullptr;
    }
    auto ptr = new coreOwningPtr<gridComponent>(comp);
    return ptr;
}

GridDynObject gridDynObjectClone(GridDynObject obj, GridDynError* err)
{
    auto comp = getConstComponentPointer(obj);

    if (comp == nullptr) {
        return nullptr;
    }
    auto newObject = comp->clone();
    auto* comp_clone = dynamic_cast<gridComponent*>(newObject);
    if (comp_clone == nullptr) {
        return nullptr;
    }
    auto ptr = new coreOwningPtr<gridComponent>(comp_clone);
    return ptr;
}

void gridDynObjectFree(GridDynObject obj)
{
    if (obj != nullptr) {
        auto cptr = reinterpret_cast<coreOwningPtr<gridComponent>*>(obj);
        delete cptr;
    }
}

void gridDynObjectAdd(GridDynObject parentObject, GridDynObject objectToAdd, GridDynError* err)
{
    gridComponent* parent = getComponentPointer(parentObject);
    coreObject* child = getComponentPointer(objectToAdd);

    try {
        parent->add(child);
        return griddyn_ok;
    }
    catch (const objectAddFailure&) {
        return griddyn_add_failure;
    }
}

void gridDynObjectRemove(GridDynObject parentObject,
                          GridDynObject objectToRemove,
                          GridDynError* err)
{
    gridComponent* parent = getComponentPointer(parentObject);
    coreObject* child = getComponentPointer(objectToRemove);

    if ((parent == nullptr) || (child == nullptr)) {
        return griddyn_invalid_object;
    }
    try {
        parent->remove(child);
        return griddyn_ok;
    }
    catch (const objectRemoveFailure&) {
        return griddyn_remove_failure;
    }
}

void gridDynObjectSetString(GridDynObject obj,
                             const char* parameter,
                             const char* value,
                             GridDynError* err)
{
    gridComponent* comp = getComponentPointer(obj);

    if (comp == nullptr) {
        return griddyn_invalid_object;
    }
    try {
        comp->set(parameter, value);
        return griddyn_ok;
    }
    catch (const invalidParameterValue&) {
        return griddyn_invalid_parameter_value;
    }
    catch (const unrecognizedParameter&) {
        return griddyn_unknown_parameter;
    }
}

void gridDynObjectSetValue(GridDynObject obj,
                            const char* parameter,
                            double value,
                            GridDynError* err)
{
    gridComponent* comp = getComponentPointer(obj);

    if (comp == nullptr) {
        return griddyn_invalid_object;
    }
    try {
        comp->set(parameter, value);
        return griddyn_ok;
    }
    catch (const invalidParameterValue&) {
        return griddyn_invalid_parameter_value;
    }
    catch (const unrecognizedParameter&) {
        return griddyn_unknown_parameter;
    }
}

void gridDynObjectSetValueUnits(GridDynObject obj,
                                           const char* parameter,
                                           double value,
                                 const char* units,
                                 GridDynError* err)
{
    gridComponent* comp = getComponentPointer(obj);

    if (comp == nullptr) {
        return griddyn_invalid_object;
    }
    auto unitType = (units == nullptr) ? units::defunit : units::unit_cast_from_string(units);
    try {
        comp->set(parameter, value, unitType);
        return griddyn_ok;
    }
    catch (const invalidParameterValue&) {
        return griddyn_invalid_parameter_value;
    }
    catch (const unrecognizedParameter&) {
        return griddyn_unknown_parameter;
    }
}

void gridDynObjectSetFlag(GridDynObject obj, const char* flag, int val, GridDynError* err)
{
    gridComponent* comp = getComponentPointer(obj);

    if (comp == nullptr) {
        return griddyn_invalid_object;
    }
    try {
        // ???
        comp->set(flag, static_cast<double>(val != 0));
        return griddyn_ok;
    }
    catch (const unrecognizedParameter&) {
        return griddyn_unknown_parameter;
    }
}

void gridDynObjectGetString(GridDynObject obj,
                             const char* parameter,
                             char* value,
                             int N,
                             GridDynError* err)
{
    auto comp = getConstComponentPointer(obj);

    if (comp == nullptr) {
        return griddyn_invalid_object;
    }
    auto s = comp->getString(parameter);
    strncpy(value, s.c_str(), N);
    return static_cast<int>(s.size());
}

void gridDynObjectGetValue(GridDynObject obj,
                            const char* parameter,
                            double* result,
                            GridDynError* err)
{
    auto comp = getConstComponentPointer(obj);

    if (comp == nullptr) {
        return griddyn_invalid_object;
    }
    try {
        *result = comp->get(parameter);
        if (*result == kNullVal) {
            return griddyn_unknown_parameter;
        }
        return griddyn_ok;
    }
    catch (const unrecognizedParameter&) {
        return griddyn_unknown_parameter;
    }
}

void gridDynObjectGetValueUnits(GridDynObject obj,
                                           const char* parameter,
                                           const char* units,
                                 double* result,
                                 GridDynError* err)
{
    auto comp = getConstComponentPointer(obj);

    if (comp == nullptr) {
        return griddyn_invalid_object;
    }

    auto unitType = (units == nullptr) ? units::defunit : units::unit_cast_from_string(units);
    try {
        *result = comp->get(parameter, unitType);
        if (*result == kNullVal) {
            return griddyn_unknown_parameter;
        }
        return griddyn_ok;
    }
    catch (const unrecognizedParameter&) {
        return griddyn_unknown_parameter;
    }
}

void gridDynObjectGetFlag(GridDynObject obj, const char* flag, int* result, GridDynError* err)
{
    auto comp = getConstComponentPointer(obj);

    if (comp == nullptr) {
        return griddyn_invalid_object;
    }
    try {
        auto res = comp->getFlag(flag);
        *result = (res) ? 1 : 0;
        return griddyn_ok;
    }
    catch (const unrecognizedParameter&) {
        return griddyn_unknown_parameter;
    }
}

GridDynObject gridDynObjectFind(GridDynObject obj, const char* objectToFind, GridDynError* err)
{
    auto comp = getConstComponentPointer(obj);

    if (comp == nullptr) {
        return nullptr;
    }
    auto res = comp->find(objectToFind);
    if (res == nullptr) {
        return nullptr;
    }
    auto compNew = dynamic_cast<gridComponent*>(res);
    if (compNew == nullptr) {
        return nullptr;
    }
    return creategridDynObject(compNew);
}

GridDynObject gridDynObjectGetSubObject(GridDynObject obj,
                                         const char* componentType,
                                         int N,
                                         GridDynError* err)
{
    auto comp = getConstComponentPointer(obj);
    if (comp == nullptr) {
        return nullptr;
    }

    auto res = comp->getSubObject(componentType, static_cast<index_t>(N));
    if (res == nullptr) {
        return nullptr;
    }
    auto compNew = dynamic_cast<gridComponent*>(res);
    if (compNew == nullptr) {
        return nullptr;
    }
    return creategridDynObject(compNew);
}

GridDynObject gridDynObjectFindByUserId(GridDynObject obj,
                                         const char* componentType,
                                         int ID,
                                         GridDynError* err)
{
    auto comp = getConstComponentPointer(obj);

    if (comp == nullptr) {
        return nullptr;
    }
    auto res = comp->findByUserID(componentType, static_cast<index_t>(ID));
    if (res == nullptr) {
        return nullptr;
    }
    auto compNew = dynamic_cast<gridComponent*>(res);
    if (compNew == nullptr) {
        return nullptr;
    }
    return creategridDynObject(compNew);
}

GridDynObject gridDynObjectGetParent(GridDynObject obj, GridDynError* err)
{
    auto comp = getConstComponentPointer(obj);
    if (comp == nullptr) {
        return nullptr;
    }
    auto compNew = dynamic_cast<gridComponent*>(comp->getParent());
    if (compNew == nullptr) {
        return nullptr;
    }
    return creategridDynObject(compNew);
}
static const char* invalid_str = "invalid";
static const char* bus_str = "bus";
static const char* area_str = "area";
static const char* link_str = "link";
static const char* load_str = "load";
static const char* generator_str = "generator";

static const char* sim_str = "simulation";
static const char* exciter_str = "exciter";
static const char* scheduler_str = "scheduler";
static const char* governor_str = "governor";
static const char* genModel_str = "genModel";
static const char* block_str = "block";
static const char* source_str = "source";
static const char* relay_str = "relay";
static const char* sensor_str = "sensor";
static const char* submodel_str = "submodel";
static const char* unknown_str = "unknown";

const char* gridDynObjectGetType(GridDynObject obj)
{
    auto comp = getConstComponentPointer(obj);

    if (comp == nullptr) {
        return invalid_str;
    }
    if (dynamic_cast<const gridBus*>(comp) != nullptr) {
        return bus_str;
    }
    if (dynamic_cast<const Link*>(comp) != nullptr) {
        return link_str;
    }
    if (dynamic_cast<const gridDynSimulation*>(comp) != nullptr) {
        return sim_str;
    }
    if (dynamic_cast<const Area*>(comp) != nullptr) {
        return area_str;
    }
    if (dynamic_cast<const Load*>(comp) != nullptr) {
        return load_str;
    }
    if (dynamic_cast<const Generator*>(comp) != nullptr) {
        return generator_str;
    }
    if (dynamic_cast<const Governor*>(comp) != nullptr) {
        return governor_str;
    }
    if (dynamic_cast<const Exciter*>(comp) != nullptr) {
        return exciter_str;
    }
    if (dynamic_cast<const GenModel*>(comp) != nullptr) {
        return genModel_str;
    }
    if (dynamic_cast<const scheduler*>(comp) != nullptr) {
        return scheduler_str;
    }
    if (dynamic_cast<const Source*>(comp) != nullptr) {
        return source_str;
    }
    if (dynamic_cast<const Block*>(comp) != nullptr) {
        return block_str;
    }
    if (dynamic_cast<const sensor*>(comp) != nullptr) {
        return sensor_str;
    }
    if (dynamic_cast<const Relay*>(comp) != nullptr) {
        return relay_str;
    }

    if (dynamic_cast<const gridSubModel*>(comp) != nullptr) {
        return submodel_str;
    }
    return unknown_str;
}
