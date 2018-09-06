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
#include "griddyn/events/Event.h"
#include "griddyn/gridDynSimulation.h"
#include "griddyn_export.h"
#include "internal/griddyn_export_internal.h"
#include "runner/gridDynRunner.h"
#include <memory>
#include <vector>

using namespace griddyn;

gridDynEvent gridDynEvent_create (const char *eventString, gridDynObject *obj)
{
    auto evnt = new std::shared_ptr<Event> (make_event (eventString, getComponentPointer (obj)));
    if (evnt != nullptr)
    {
        return reinterpret_cast<void *> (evnt);
    }
    return nullptr;
}

void gridDynEvent_free (gridDynEvent evnt)
{
    if (evnt != nullptr)
    {
        auto shr_event = reinterpret_cast<std::shared_ptr<Event> *> (evnt);
        delete shr_event;
    }
}

griddyn_status gridDynEvent_trigger (gridDynEvent evnt)
{
    if (evnt == nullptr)
    {
        return griddyn_invalid_object;
    }
    auto shr_event = reinterpret_cast<std::shared_ptr<Event> *> (evnt);
    if (*shr_event)
    {
        if ((*shr_event)->trigger () >= change_code::no_change)
        {
            return griddyn_ok;
        }
    }
    return griddyn_function_failure;
}

griddyn_status gridDynEvent_schedule (gridDynEvent evnt, gridDynSimReference sim)
{
    if (evnt == nullptr)
    {
        return griddyn_invalid_object;
    }
    auto shr_event = reinterpret_cast<std::shared_ptr<Event> *> (evnt);

    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return griddyn_invalid_object;
    }
    try
    {
        runner->getSim ()->add (*shr_event);
        return griddyn_ok;
    }
    catch (...)
    {
        return griddyn_add_failure;
    }
}

griddyn_status gridDynEvent_setValue (gridDynEvent evnt, const char *parameter, double value)
{
    if (evnt == nullptr)
    {
        return griddyn_invalid_object;
    }
    auto shr_event = reinterpret_cast<std::shared_ptr<Event> *> (evnt);
    try
    {
        shr_event->operator-> ()->set (parameter, value);
        return griddyn_ok;
    }
    catch (const invalidParameterValue &)
    {
        return griddyn_invalid_parameter_value;
    }
    catch (const unrecognizedParameter &)
    {
        return griddyn_unknown_parameter;
    }
}

griddyn_status gridDynEvent_setString (gridDynEvent evnt, const char *parameter, const char *value)
{
    if (evnt == nullptr)
    {
        return griddyn_invalid_object;
    }
    auto shr_event = reinterpret_cast<std::shared_ptr<Event> *> (evnt);
    try
    {
        shr_event->operator-> ()->set (parameter, value);
        return griddyn_ok;
    }
    catch (const invalidParameterValue &)
    {
        return griddyn_invalid_parameter_value;
    }
    catch (const unrecognizedParameter &)
    {
        return griddyn_unknown_parameter;
    }
}

griddyn_status gridDynEvent_setFlag (gridDynEvent evnt, const char *flag, int val)
{
    if (evnt == nullptr)
    {
        return griddyn_invalid_object;
    }
    auto shr_event = reinterpret_cast<std::shared_ptr<Event> *> (evnt);
    try
    {
        shr_event->operator-> ()->setFlag (flag, (val != 0));
        return griddyn_ok;
    }
    catch (const invalidParameterValue &)
    {
        return griddyn_invalid_parameter_value;
    }
    catch (const unrecognizedParameter &)
    {
        return griddyn_unknown_parameter;
    }
}

griddyn_status gridDynEvent_setTarget (gridDynEvent evnt, gridDynObject *obj)
{
    if (evnt == nullptr)
    {
        return griddyn_invalid_object;
    }
    auto shr_event = reinterpret_cast<std::shared_ptr<Event> *> (evnt);
    auto comp = getComponentPointer (obj);
    if (comp == nullptr)
    {
        return griddyn_invalid_object;
    }
    try
    {
        shr_event->operator-> ()->updateObject (comp, object_update_mode::match);
        return griddyn_ok;
    }
    catch (...)
    {
        return griddyn_add_failure;
    }
}
