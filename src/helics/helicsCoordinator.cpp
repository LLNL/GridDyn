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

#include "helicsCoordinator.h"
#include "helics/helics.hpp"

#include "utilities/stringConversion.h"
#include <algorithm>
#include <utilities/mapOps.hpp>

namespace griddyn
{
namespace helicsLib
{
helicsCoordinator::helicsCoordinator (const std::string &fedName) : coreObject ("helics"), info_ (fedName) {}

std::shared_ptr<helics::Federate> helicsCoordinator::RegisterAsFederate ()
{
    if (info_.name.empty ())
    {
        info_.name = getParent ()->getName ();
    }

    vFed_ = std::make_shared<helics::ValueFederate> (info_);

    int ii = 0;
    pubs_.resize(pubI.size());
    for (auto &p : pubI)
    {
        if (p.unitType != gridUnits::defUnit)
        {
            pubs_[ii] = helics::Publication(helics::GLOBAL,vFed_.get(), p.name, p.type, gridUnits::to_string(p.unitType));
        }
        else
        {
            pubs_[ii] = helics::Publication(helics::GLOBAL, vFed_.get(), p.name, p.type);
        }
        ++ii;
    }
    ii = 0;
    subs_.resize(subI.size());
    for (auto &s : subI)
    {
        if (s.unitType != gridUnits::defUnit)
        {
            subs_[ii] = helics::Subscription(vFed_.get(), s.name, gridUnits::to_string(s.unitType));
        }
        else
        {
            subs_[ii] = helics::Subscription(vFed_.get(), s.name);
        }
        ++ii;
    }
    vFed_->enterInitializationState ();
    return vFed_;
}

void helicsCoordinator::setFlag (const std::string &flag, bool val)
{
    if (flag == "source_only")
    {
        info_.source_only = val;
    }
    else if (flag == "uninterruptible")
    {
        info_.uninterruptible = val;
    }
    else if (flag == "observer")
    {
        info_.observer = val;
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
        info_.coreType = helics::coreTypeFromString(val);
    }
    else if ((param == "name") || (param == "fed_name"))
    {
        info_.name = val;
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
    if (param == "timedelta")
    {
        info_.timeDelta = val;
    }
    else if (param == "lookahead")
    {
        info_.lookAhead = val;
    }
    else if (param == "impactwindow")
    {
        info_.impactWindow = val;
    }
    else if (param == "period")
    {
        info_.period = val;
    }
    else if (param == "poffset")
    {
        info_.offset = val;
    }
    else
    {
        coreObject::set (param, val, unitType);
    }
}


bool helicsCoordinator::isUpdated (int32_t index) const
{
    if (isValidIndex(index,subs_))
    {
        subs_[index].isUpdated();
    }
    return false;
}


int32_t
helicsCoordinator::addPublication (const std::string &pubName, helics::helicsType_t  type, gridUnits::units_t unitType)
{
    PubInfo p;
    p.name = pubName;
   // p.type = type;
    p.unitType = unitType;
    pubI.push_back (p);
    auto ind = static_cast<int32_t> (pubs_.size ()) - 1;
    pubMap_.emplace (pubName, ind);
    return ind;
}

int32_t
helicsCoordinator::addSubscription (const std::string &subName, gridUnits::units_t unitType)
{
    SubInfo s;
    s.name = subName;
    s.unitType = unitType;
    subI.push_back (s);
    auto ind = static_cast<int32_t> (subs_.size ()) - 1;
    subMap_.emplace (subName, ind);
    return ind;
}

void helicsCoordinator::updatePublication (int32_t index,
                                           const std::string &pubName,
                                            helics::helicsType_t  type,
                                           gridUnits::units_t unitType)
{
    if (isValidIndex(index,pubI))
    {
        if (!pubName.empty ())
        {
            pubI[index].name = pubName;
        }
        pubI[index].type = type;
        if (unitType != gridUnits::defUnit)
        {
            pubI[index].unitType = unitType;
        }
    }
}

void helicsCoordinator::updateSubscription (int32_t index,
                                            const std::string &subName,
                                            gridUnits::units_t unitType)
{
     if (isValidIndex(index,subI))
    {
        if (!subName.empty ())
        {
            subI[index].name = subName;
        }
        if (unitType != gridUnits::defUnit)
        {
            subI[index].unitType = unitType;
        }
    }
}

void helicsCoordinator::addHelper (std::shared_ptr<helperObject> /*ho*/)
{
    // std::lock_guard<std::mutex> hLock(helperProtector);
    // helpers.push_back(std::move(ho));
}

int32_t helicsCoordinator::getSubscriptionIndex (const std::string &subName) const
{
    return mapFind (subMap_, subName, -1);
}

int32_t helicsCoordinator::getPublicationIndex (const std::string &pubName) const
{
    return mapFind (pubMap_, pubName, -1);
}


void helicsCoordinator::finalize ()
{
    if (vFed_)
    {
        vFed_->finalize ();
    }
    else if (mFed_)
    {
        mFed_->finalize ();
    }

}
}  // namespace helics
}  // namespace griddyn