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

#include "pmu.h"
#include "core/coreObjectTemplates.hpp"

#include "Link.h"
#include "blocks/delayBlock.h"
#include "core/coreExceptions.h"
#include "events/Event.h"
#include "measurement/Condition.h"
#include "measurement/grabberSet.h"
#include "measurement/gridGrabbers.h"
#include <cmath>

namespace griddyn
{
namespace relays
{
pmu::pmu (const std::string &objName) : sensor (objName)
{
    outputNames = {"voltage", "angle", "frequency","rocof"};
}

coreObject *pmu::clone (coreObject *obj) const
{
    auto nobj = cloneBase<pmu, sensor> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }

    nobj->Tv = Tv;
    nobj->Ttheta = Ttheta;
    nobj->transmissionRate = transmissionRate;
    nobj->sampleRate = sampleRate;
    return nobj;
}

void pmu::setFlag (const std::string &flag, bool val)
{
    if ((flag == "transmit") || (flag == "transmitactive") || (flag == "transmit_active"))
    {
        opFlags.set (transmit_active, val);
    }
    else
    {
        sensor::setFlag (flag, val);
    }
}

void pmu::set (const std::string &param, const std::string &val)
{
    if (param.front() == '#')
    {
    }
    else
    {
        Relay::set (param, val);
    }
}

using namespace gridUnits;

void pmu::set (const std::string &param, double val, units_t unitType)
{
    if ((param == "tv") || (param == "voltagedelay"))
    {
        Tv = val;
    }
    else if ((param == "ttheta") || (param == "tangle") || (param == "angledelay"))
    {
        Ttheta = val;
    }
    else if ((param == "transmitrate") || (param == "rate"))
    {
        transmissionRate = val;
    }
    else if (param == "samplerate")
    {
        sampleRate = val;
    }
    else
    {
        sensor::set (param, val, unitType);
    }
}

double pmu::get (const std::string &param, gridUnits::units_t unitType) const
{
    if ((param == "tv") || (param == "voltagedelay"))
    {
        return Tv;
    }
    if ((param == "ttheta") || (param == "tangle") || (param == "angledelay"))
    {
        return Ttheta;
    }
    if ((param == "transmitrate") || (param == "rate"))
    {
        return transmissionRate;
    }
    if (param == "samplerate")
    {
        return sampleRate;
    }
    return sensor::get (param, unitType);
}

void pmu::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    if (m_sourceObject == nullptr)
    {
        return sensor::dynObjectInitializeA (time0, flags);
    }

    return sensor::dynObjectInitializeA (time0, flags);
}

void pmu::updateA (coreTime time)
{
    if (time == prevTime)
    {
        return;
    }

    Relay::updateA (time);
    prevTime = time;
}

}  // namespace relays
}  // namespace griddyn