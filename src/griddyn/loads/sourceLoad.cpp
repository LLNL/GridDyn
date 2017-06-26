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
#include "loads/sourceLoad.h"
#include "Source.h"
#include "sources/sourceTypes.h"

#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include <cmath>
#include <map>

namespace griddyn
{
namespace loads
{
sourceLoad::sourceLoad (const std::string &objName) : zipLoad (objName) { sourceLink.fill (-1); }
sourceLoad::sourceLoad (sourceType type, const std::string &objName) : sourceLoad (objName)
{
    sType = type;
    // add the sources for P and Q
    sourceLoad::add (makeSource (p_source));
    sourceLoad::add (makeSource (q_source));
}

coreObject *sourceLoad::clone (coreObject *obj) const
{
    auto nobj = cloneBase<sourceLoad, zipLoad> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    nobj->sourceLink = sourceLink;
    for (auto &src : sources)
    {
        if (src != nullptr)
        {
            auto newsrc = static_cast<Source *> (src->clone ());
            newsrc->locIndex = src->locIndex;
            nobj->add (newsrc);
        }
    }
    return nobj;
}

static const std::map<std::string, int> source_lookup{
  {"source", sourceLoad::p_source},     {"psource", sourceLoad::p_source},    {"p_source", sourceLoad::p_source},
  {"qsource", sourceLoad::q_source},    {"q_source", sourceLoad::q_source},   {"rsource", sourceLoad::r_source},
  {"r_source", sourceLoad::r_source},   {"xsource", sourceLoad::x_source},    {"x_source", sourceLoad::x_source},
  {"ypsource", sourceLoad::yp_source},  {"yp_source", sourceLoad::yp_source}, {"yqsource", sourceLoad::yq_source},
  {"yq_source", sourceLoad::yq_source}, {"ipsource", sourceLoad::ip_source},  {"ip_source", sourceLoad::ip_source},
  {"iqsource", sourceLoad::iq_source},  {"iq_source", sourceLoad::iq_source},
};

static const std::map<std::string, int> sourcekey_lookup{
  {"p", sourceLoad::p_source},   {"q", sourceLoad::q_source},   {"r", sourceLoad::r_source},
  {"x", sourceLoad::x_source},   {"yp", sourceLoad::yp_source}, {"zp", sourceLoad::yp_source},
  {"zr", sourceLoad::yp_source}, {"yq", sourceLoad::yq_source}, {"zq", sourceLoad::yq_source},
  {"ip", sourceLoad::ip_source}, {"iq", sourceLoad::iq_source},
};

static const std::map<std::string, int> source_match{
  {"source", sourceLoad::p_source},     {"psource", sourceLoad::p_source},    {"p_source", sourceLoad::p_source},
  {"qsource", sourceLoad::q_source},    {"q_source", sourceLoad::q_source},   {"rsource", sourceLoad::r_source},
  {"r_source", sourceLoad::r_source},   {"xsource", sourceLoad::x_source},    {"x_source", sourceLoad::x_source},
  {"ypsource", sourceLoad::yp_source},  {"yp_source", sourceLoad::yp_source}, {"yqsource", sourceLoad::yq_source},
  {"yq_source", sourceLoad::yq_source}, {"ipsource", sourceLoad::ip_source},  {"ip_source", sourceLoad::ip_source},
  {"iqsource", sourceLoad::iq_source},  {"iq_source", sourceLoad::iq_source}, {"p", sourceLoad::p_source},
  {"q", sourceLoad::q_source},          {"r", sourceLoad::r_source},          {"x", sourceLoad::x_source},
  {"yp", sourceLoad::yp_source},        {"zp", sourceLoad::yp_source},        {"zr", sourceLoad::yp_source},
  {"yq", sourceLoad::yq_source},        {"zq", sourceLoad::yq_source},        {"ip", sourceLoad::ip_source},
  {"iq", sourceLoad::iq_source},
};

void sourceLoad::add (coreObject *obj)
{
    if (dynamic_cast<Source *> (obj) != nullptr)
    {
        add (static_cast<Source *> (obj));
    }
}

void sourceLoad::add (Source *src)
{
    src->setParent (this);
    src->setFlag ("pflow_init_required", true);
    if (src->locIndex != kNullLocation)
    {
    }
    else if (!src->m_purpose.empty ())
    {
        auto ind = source_match.find (src->m_purpose);
        if (ind != source_match.end ())
        {
            src->locIndex = ind->second;
        }
        else
        {
            src->locIndex = static_cast<int> (sources.size ());
        }
    }
    else
    {
        src->locIndex = static_cast<int> (sources.size ());
    }

    if (static_cast<index_t> (sources.size ()) <= src->locIndex)
    {
        sources.resize (src->locIndex + 1, nullptr);
    }
    if (sources[src->locIndex] != nullptr)
    {
        remove (sources[src->locIndex]);
    }
    sources[src->locIndex] = src;
    if (src->locIndex < 8)
    {
        if (sourceLink[src->locIndex] < 0)
        {
            sourceLink[src->locIndex] = static_cast<int> (src->locIndex);
        }
    }
    // now add to the subObjectList
    addSubObject (src);
}

void sourceLoad::remove (coreObject *obj)
{
    if (dynamic_cast<Source *> (obj) != nullptr)
    {
        remove (static_cast<Source *> (obj));
    }
    else
    {
        gridSecondary::remove (obj);
    }
}

void sourceLoad::remove (Source *src)
{
    if ((src->locIndex != kNullLocation) && (src->locIndex < static_cast<index_t> (sources.size ())))
    {
        if (isSameObject (sources[src->locIndex], src))
        {
            sources[src->locIndex] = nullptr;
            src->setParent (nullptr);
            for (auto &lnk : sourceLink)
            {
                if (lnk == static_cast<int> (src->locIndex))
                {
                    lnk = -1;
                }
            }
            gridSecondary::remove (src);
        }
    }
}

Source *sourceLoad::findSource (const std::string &srcname)
{
    auto ind = source_match.find (srcname);
    if (ind != source_match.end ())
    {
        int index = sourceLink[ind->second];
        if (index < 0)
        {
            add (makeSource (static_cast<sourceLoad::sourceLoc> (ind->second)));
        }
        else if ((static_cast<int> (sources.size ()) <= index) || (sources[index] == nullptr))
        {
            // this may not actually do anything is the sType is set to other
            add (makeSource (static_cast<sourceLoad::sourceLoc> (ind->second)));
        }
        return sources[index];
    }
    return nullptr;
}

Source *sourceLoad::findSource (const std::string &srcname) const
{
    auto ind = source_match.find (srcname);
    if (ind != source_match.end ())
    {
        int index = sourceLink[ind->second];
        if ((index < 0) || (static_cast<int> (sources.size ()) <= index))
        {
            return nullptr;
        }
        return sources[index];
    }
    return nullptr;
}

void sourceLoad::setFlag (const std::string &flag, bool val)
{
    auto sfnd = flag.find_last_of (":?");
    if (sfnd != std::string::npos)
    {
        auto src = findSource (flag.substr (0, sfnd));
        if (src != nullptr)
        {
            src->setFlag (flag.substr (sfnd + 1, std::string::npos), val);
        }
        else
        {
            throw (unrecognizedParameter (flag));
        }
    }
    else
    {
        zipLoad::setFlag (flag, val);
    }
}

void sourceLoad::set (const std::string &param, const std::string &val)
{
    auto sfnd = param.find_last_of (":?");
    if (sfnd != std::string::npos)
    {
        auto src = findSource (param.substr (0, sfnd));
        if (src != nullptr)
        {
            src->set (param.substr (sfnd + 1, std::string::npos), val);
        }
        else
        {
            throw (unrecognizedParameter (param));
        }
    }
    else
    {
        zipLoad::set (param, val);
    }
}

void sourceLoad::timestep (coreTime time, const IOdata &inputs, const solverMode &sMode)
{
    for (auto &src : getSubObjects ())
    {
        static_cast<Source *> (src)->timestep (time, noInputs, sMode);
    }
    getSourceLoads ();
    prevTime = time;
    zipLoad::timestep (time, inputs, sMode);
}

void sourceLoad::setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
    for (auto &src : getSubObjects ())
    {
        src->setState (time, state, dstate_dt, sMode);
    }
    getSourceLoads ();
    prevTime = time;
}

void sourceLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    auto sfnd = param.find_last_of (":?");
    if (sfnd != std::string::npos)
    {
        auto src = findSource (param.substr (0, sfnd));
        if (src != nullptr)
        {
            src->set (param.substr (sfnd + 1, std::string::npos), val, unitType);
        }
        else
        {
            throw (unrecognizedParameter (param));
        }
    }
    else
    {
        auto ind = source_lookup.find (param.substr (0, sfnd));
        if (ind != source_lookup.end ())
        {
            if ((static_cast<int> (sources.size ()) < ind->second) && (sources[ind->second] != nullptr))
            {
                sourceLink[ind->second] = static_cast<int> (val);
            }
            else if (!opFlags[pFlow_initialized])
            {
                sourceLink[ind->second] = static_cast<int> (val);
            }
            else
            {
                throw (unrecognizedParameter (param));
            }
            return;
        }

        auto keyind = sourcekey_lookup.find (param.substr (0, sfnd));

        if (keyind != sourcekey_lookup.end ())
        {
            if ((static_cast<int> (sources.size ()) > keyind->second) && (sources[keyind->second] != nullptr))
            {
                sources[keyind->second]->set ("level", gridUnits::unitConversion (val, unitType, gridUnits::puMW,
                                                                                  systemBasePower));
                return;
            }
        }
        zipLoad::set (param, val, unitType);
    }
}

void sourceLoad::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    // Do a check on the sources;
    int fnd = 0;
    for (auto &sL : sourceLink)
    {
        if (sL < 0)
        {
            continue;
        }
        ++fnd;
        if (sL >= static_cast<int> (sources.size ()))
        {
            LOG_WARNING ("no source given at called index");
        }
        else if (sources[sL] == nullptr)
        {
            LOG_WARNING ("no source given at called index");
        }
    }
    gridSecondary::pFlowObjectInitializeA (time0, flags);  // to initialize the submodels

    getSourceLoads ();
}

void sourceLoad::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    gridSecondary::dynObjectInitializeA (time0, flags);
    getSourceLoads ();
}

void sourceLoad::updateLocalCache (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode)
{
    for (auto &src : sources)
    {
        src->updateLocalCache (noInputs, sD, sMode);
    }
    getSourceLoads ();
}

void sourceLoad::getSourceLoads ()
{
    if (sourceLink[p_source] >= 0)
    {
        setP (sources[sourceLink[p_source]]->getOutput ());
    }
    if (sourceLink[q_source] >= 0)
    {
        setQ (sources[sourceLink[q_source]]->getOutput ());
    }
    if (sourceLink[yp_source] >= 0)
    {
        setYp (sources[sourceLink[yp_source]]->getOutput ());
    }
    if (sourceLink[yq_source] >= 0)
    {
        setYq (sources[sourceLink[yq_source]]->getOutput ());
    }
    if (sourceLink[ip_source] >= 0)
    {
        setIp (sources[sourceLink[ip_source]]->getOutput ());
    }
    if (sourceLink[iq_source] >= 0)
    {
        setIq (sources[sourceLink[iq_source]]->getOutput ());
    }
    if (sourceLink[r_source] >= 0)
    {
        setr (sources[sourceLink[r_source]]->getOutput ());
    }
    if (sourceLink[x_source] >= 0)
    {
        setx (sources[sourceLink[x_source]]->getOutput ());
    }
}

Source *sourceLoad::makeSource (sourceLoc loc)
{
    Source *src = nullptr;
    switch (sType)
    {
    case sourceType::pulse:
        src = new sources::pulseSource ();
        break;
    case sourceType::random:
        src = new sources::randomSource ();
        break;
    case sourceType::sine:
        src = new sources::sineSource ();
        break;
    case sourceType::other:
    default:
        return nullptr;
    }
    src->locIndex = static_cast<index_t> (loc);
    return src;
}

coreObject *sourceLoad::find (const std::string &obj) const
{
    auto src = findSource (obj);
    if (src == nullptr)
    {
        return gridComponent::find (obj);
    }
    return src;
}
}  // namespace loads
}  // namespace griddyn