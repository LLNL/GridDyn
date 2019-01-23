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

#include "helicsCoordinator.h"
#include "helics/core/core-exceptions.hpp"
#include "helics/helics.hpp"
#include "helics/helicsEvent.h"

#include "utilities/stringConversion.h"
#include <algorithm>
#include <utilities/mapOps.hpp>

namespace griddyn
{
namespace helicsLib
{
std::unordered_map<std::string, helicsCoordinator *> helicsCoordinator::registry;
std::mutex helicsCoordinator::registry_protection;

void helicsCoordinator::registerCoordinator (const std::string &coordinatorName, helicsCoordinator *ptr)
{
    std::lock_guard<std::mutex> lock (registry_protection);
    registry[coordinatorName] = ptr;
}

void helicsCoordinator::unregisterCoordinator (const std::string &coordinatorName)
{
    std::lock_guard<std::mutex> lock (registry_protection);
    auto fnd = registry.find (coordinatorName);
    if (fnd != registry.end ())
    {
        registry.erase (fnd);
    }
}

helicsCoordinator *helicsCoordinator::findCoordinator (const std::string &coordinatorName)
{
    std::lock_guard<std::mutex> lock (registry_protection);
    auto fnd = registry.find (coordinatorName);
    if (fnd != registry.end ())
    {
        return fnd->second;
    }
    return nullptr;
}

helicsCoordinator::helicsCoordinator (const std::string &fedName) : coreObject ("helics"), info_ (fedName)
{
    registerCoordinator (fedName, this);
}

std::shared_ptr<helics::Federate> helicsCoordinator::RegisterAsFederate ()
{
    if (info_.defName.empty ())
    {
        info_.defName = getParent ()->getName ();
    }
    std::shared_ptr<helics::Federate> cfed;
    try
    {
        cfed = std::make_shared<helics::CombinationFederate> (info_);
    }
    catch (const helics::RegistrationFailure &e)
    {
        LOG_WARNING ("failed to register as HELICS federate");
        return nullptr;
    }
    vFed_ = dynamic_cast<helics::ValueFederate *> (cfed.get ());
    mFed_ = dynamic_cast<helics::MessageFederate *> (cfed.get ());
    fed = std::move (cfed);

    int ii = 0;
    pubs_.resize (pubI.size ());
    for (auto &p : pubI)
    {
        if (p.unitType != gridUnits::defUnit)
        {
            pubs_[ii] =
              helics::Publication (helics::GLOBAL, vFed_, p.name, p.type, gridUnits::to_string (p.unitType));
        }
        else
        {
            pubs_[ii] = helics::Publication (helics::GLOBAL, vFed_, p.name, p.type);
        }
        ++ii;
    }
    ii = 0;
    subs_.resize (subI.size ());
    for (auto &s : subI)
    {
        if (s.unitType != gridUnits::defUnit)
        {
            subs_[ii] = helics::Input (helics::interface_availability::optional, vFed_, s.name,
                                       gridUnits::to_string (s.unitType));
        }
        else
        {
            subs_[ii] = helics::Input (helics::interface_availability::optional, vFed_, s.name);
        }
        ++ii;
    }

    ii = 0;
    epts_.resize (eptI.size ());
    for (auto &e : eptI)
    {
        epts_[ii] = helics::Endpoint (mFed_, e.name, e.type);
        if (!e.target.empty ())
        {
            epts_[ii].setDefaultDestination (e.target);
        }
        ++ii;
    }
    fed->enterInitializingMode ();
    LOG_SUMMARY ("entered HELICS initialization state");
    for (auto evnt : events)
    {
        if (evnt->initNeeded ())
        {
            evnt->initialize ();
        }
    }
    return fed;
}

void helicsCoordinator::setFlag (const std::string &flag, bool val)
{
    if (flag == "source_only")
    {
        info_.setFlagOption (helics::defs::source_only, val);
    }
    else if (flag == "uninterruptible")
    {
        info_.setFlagOption (helics::defs::uninterruptible, val);
        if (fed)
        {
            fed->setFlagOption (helics::defs::uninterruptible, val);
        }
    }
    else if (flag == "observer")
    {
        info_.setFlagOption (helics::defs::observer, val);
    }
    else if (flag == "only_update_on_change")
    {
        info_.setFlagOption (helics::defs::only_update_on_change, val);
        if (fed)
        {
            fed->setFlagOption (helics::defs::only_update_on_change, val);
        }
    }
    else if (flag == "only_transmit_on_change")
    {
        info_.setFlagOption (helics::defs::only_transmit_on_change, val);
        if (fed)
        {
            fed->setFlagOption (helics::defs::only_transmit_on_change, val);
        }
    }
    else
    {
        coreObject::setFlag (flag, val);
    }
}
void helicsCoordinator::set (const std::string &param, const std::string &val)
{
    if ((param == "corename") || (param == "core_name"))
    {
        info_.coreName = val;
    }
    else if ((param == "initstring") || (param == "init") || (param == "core_init"))
    {
        info_.coreInitString = val;
    }
    else if ((param == "coretype") || (param == "core_type"))
    {
        info_.coreType = helics::coreTypeFromString (val);
    }
    else if ((param == "name") || (param == "fed_name"))
    {
        if (val != info_.defName)
        {
            unregisterCoordinator (info_.defName);
            registerCoordinator (val, this);
            info_.defName = val;
        }
    }
    else if ((param == "broker") || (param == "connection_info"))
    {
        connectionInfo = val;
    }
    else
    {
        coreObject::set (param, val);
    }
}

void helicsCoordinator::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "timedelta") || (param == "mintimedelta"))
    {
        info_.setProperty (helics::defs::time_delta, val);
        if (fed)
        {
            fed->setProperty (helics::defs::time_delta, val);
        }
    }
    else if ((param == "outputdelay") || (param == "delay"))
    {
        info_.setProperty (helics::defs::output_delay, val);
        if (fed)
        {
            fed->setProperty (helics::defs::output_delay, val);
        }
    }
    else if (param == "inputdelay")
    {
        info_.setProperty (helics::defs::input_delay, val);
        if (fed)
        {
            fed->setProperty (helics::defs::input_delay, val);
        }
    }
    else if (param == "period")
    {
        info_.setProperty (helics::defs::period, val);
        if (fed)
        {
            fed->setProperty (helics::defs::period, val);
        }
    }
    else if ((param == "poffset") || (param == "offset"))
    {
        info_.setProperty (helics::defs::offset, val);
        if (fed)
        {
            fed->setProperty (helics::defs::offset, val);
        }
    }
    else
    {
        coreObject::set (param, val, unitType);
    }
}

void helicsCoordinator::sendMessage (int32_t index, const char *data, count_t size)
{
    if (isValidIndex (index, epts_))
    {
        epts_[index].send (data, size);
    }
}
void helicsCoordinator::sendMessage (int32_t index, const std::string &dest, const char *data, count_t size)
{
    if (isValidIndex (index, epts_))
    {
        epts_[index].send (dest, data, size);
    }
}

bool helicsCoordinator::isUpdated (int32_t index)
{
    if (isValidIndex (index, subs_))
    {
        return subs_[index].isUpdated ();
    }
    return false;
}

bool helicsCoordinator::hasMessage (int32_t index) const
{
    if (isValidIndex (index, epts_))
    {
        return epts_[index].hasMessage ();
    }
    return false;
}

helics::Publication *helicsCoordinator::getPublicationPointer (int32_t index)
{
    if (isValidIndex (index, pubs_))
    {
        return &pubs_[index];
    }
    return nullptr;
}

helics::Input *helicsCoordinator::getInputPointer (int32_t index)
{
    if (isValidIndex (index, subs_))
    {
        return &subs_[index];
    }
    return nullptr;
}

helics::Endpoint *helicsCoordinator::getEndpointPointer (int32_t index)
{
    if (isValidIndex (index, epts_))
    {
        return &epts_[index];
    }
    return nullptr;
}

int32_t helicsCoordinator::addPublication (const std::string &pubName, helics::data_type type, gridUnits::units_t unitType)
{
    PubInfo p;
    p.name = pubName;
    p.type = type;
    p.unitType = unitType;
    pubI.push_back (p);
    auto ind = static_cast<int32_t> (pubI.size ()) - 1;
    pubMap_.emplace (pubName, ind);
    return ind;
}

int32_t helicsCoordinator::addSubscription (const std::string &subName, gridUnits::units_t unitType)
{
    SubInfo s;
    s.name = subName;
    s.unitType = unitType;
    subI.push_back (s);
    auto ind = static_cast<int32_t> (subI.size ()) - 1;
    subMap_.emplace (subName, ind);
    return ind;
}

void helicsCoordinator::updatePublication (int32_t index,
                                           const std::string &pubName,
                                           helics::data_type type,
                                           gridUnits::units_t unitType)
{
    if (isValidIndex (index, pubI))
    {
        if (!pubName.empty ())
        {
            pubI[index].name = pubName;
            pubMap_[pubName] = index;
        }
        if (type != helics::data_type::helics_any)
        {
            pubI[index].type = type;
        }
        if (unitType != gridUnits::defUnit)
        {
            pubI[index].unitType = unitType;
        }
    }
}

void helicsCoordinator::updateSubscription (int32_t index, const std::string &subName, gridUnits::units_t unitType)
{
    if (isValidIndex (index, subI))
    {
        if (!subName.empty ())
        {
            subI[index].name = subName;
            subMap_[subName] = index;
        }
        if (unitType != gridUnits::defUnit)
        {
            subI[index].unitType = unitType;
        }
    }
}

int32_t helicsCoordinator::addEndpoint (const std::string &eptName, const std::string &type, const std::string &target)
{
    EptInfo e;
    e.name = eptName;
    e.type = type;
    e.target = target;
    eptI.push_back (e);
    auto ind = static_cast<int32_t> (eptI.size ()) - 1;
    eptMap_.emplace (eptName, ind);
    return ind;
}

void helicsCoordinator::updateEndpoint (int32_t index, const std::string &eptName, const std::string &type)
{
    if (isValidIndex (index, eptI))
    {
        if (!eptName.empty ())
        {
            eptI[index].name = eptName;
            eptMap_[eptName] = index;
        }
        eptI[index].type = type;
    }
}

/** set the target destination for an endpoint
 */
void helicsCoordinator::setEndpointTarget (int32_t index, const std::string &target)
{
    if (isValidIndex (index, eptI))
    {
        eptI[index].target = target;
    }
    if (isValidIndex (index, epts_))
    {
        epts_[index].setDefaultDestination (target);
    }
}

void helicsCoordinator::addHelper (std::shared_ptr<helperObject> ho)
{
    std::lock_guard<std::mutex> hLock (helperProtector);
    helpers.push_back (std::move (ho));
}

void helicsCoordinator::addEvent (helicsEvent *evnt) { events.push_back (evnt); }
void helicsCoordinator::addCollector (helicsCollector *col) { collectors.push_back (col); }

int32_t helicsCoordinator::getSubscriptionIndex (const std::string &subName) const
{
    return mapFind (subMap_, subName, -1);
}

int32_t helicsCoordinator::getPublicationIndex (const std::string &pubName) const
{
    return mapFind (pubMap_, pubName, -1);
}

int32_t helicsCoordinator::getEndpointIndex (const std::string &eptName) const
{
    return mapFind (eptMap_, eptName, -1);
}

void helicsCoordinator::finalize ()
{
    if (fed)
    {
        fed->finalize ();
    }
}
}  // namespace helicsLib
}  // namespace griddyn