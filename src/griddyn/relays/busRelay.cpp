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

#include "busRelay.h"
#include "core/coreObjectTemplates.hpp"
#include "../events/Event.h"
#include "../events/eventQueue.h"
#include "../measurement/Condition.h"

#include <boost/format.hpp>

namespace griddyn
{
namespace relays
{
busRelay::busRelay (const std::string &objName) : Relay (objName)
{
    opFlags.set (continuous_flag);
    opFlags.set (power_flow_checks_flag);  // enable power flow checks for busRelay
}

coreObject *busRelay::clone (coreObject *obj) const
{
    auto nobj = cloneBase<busRelay, Relay> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    nobj->cutoutVoltage = cutoutVoltage;
    nobj->cutoutFrequency = cutoutFrequency;
    nobj->voltageDelay = voltageDelay;
    nobj->frequencyDelay = frequencyDelay;
    return nobj;
}

void busRelay::setFlag (const std::string &flag, bool val)
{
    if (flag.empty())
    {
    }
    else
    {
        Relay::setFlag (flag, val);
    }
}
/*
std::string commDestName;
std::uint64_t commDestId=0;
std::string commType;
*/
void busRelay::set (const std::string &param, const std::string &val)
{
    if (param.empty())
    {
    }
    else
    {
        Relay::set (param, val);
    }
}

void busRelay::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "cutoutvoltage") || (param == "voltagelimit"))
    {
        cutoutVoltage = gridUnits::unitConversion (val, unitType, gridUnits::puV, systemBasePower);
        if (opFlags[dyn_initialized])
        {
            setConditionLevel (0, cutoutVoltage);
        }
    }
    else if ((param == "cutoutfrequency") || (param == "freqlimit"))
    {
        cutoutFrequency = gridUnits::unitConversion (val, unitType, gridUnits::puHz, systemBaseFrequency);
        if (opFlags[dyn_initialized])
        {
            setConditionLevel (1, cutoutFrequency);
        }
    }
    else if (param == "delay")
    {
        voltageDelay = val;
        frequencyDelay = val;
        if (opFlags[dyn_initialized])
        {
            setActionTrigger (0, 0, voltageDelay);
            setActionTrigger (0, 1, frequencyDelay);
        }
    }
    else if (param == "voltagedelay")
    {
        voltageDelay = val;
        if (opFlags[dyn_initialized])
        {
            setActionTrigger (0, 0, voltageDelay);
        }
    }
    else if (param == "frequencydelay")
    {
        frequencyDelay = val;
        if (opFlags[dyn_initialized])
        {
            setActionTrigger (0, 1, frequencyDelay);
        }
    }
    else
    {
        Relay::set (param, val, unitType);
    }
}

void busRelay::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    auto ge = std::make_unique<Event> (0.0);

    ge->setValue (0.0);
    ge->setTarget (m_sinkObject, "status");

    add (std::shared_ptr<Event> (std::move (ge)));

    add (std::shared_ptr<Condition> (make_condition ("voltage", "<", cutoutVoltage, m_sourceObject)));
    setActionTrigger (0, 0, voltageDelay);
    if ((cutoutVoltage > 2.0) || (cutoutVoltage <= 0))
    {
        setConditionStatus (0, condition_status_t::disabled);
    }
    add (std::shared_ptr<Condition> (make_condition ("frequency", "<", cutoutFrequency, m_sourceObject)));
    setActionTrigger (0, 1, frequencyDelay);
    if ((cutoutFrequency > 2.0) || (cutoutFrequency <= 0))
    {
        setConditionStatus (1, condition_status_t::disabled);
    }

    Relay::pFlowObjectInitializeA (time0, flags);
}

void busRelay::actionTaken (index_t /*actionNum*/,
                            index_t conditionNum,
                            change_code /*actionReturn*/,
                            coreTime /*actionTime*/)
{
    if (conditionNum == 0)
    {
        alert (m_sourceObject, BUS_UNDER_VOLTAGE);
    }
    else if (conditionNum == 1)
    {
        alert (m_sourceObject, BUS_UNDER_FREQUENCY);
    }
}
}  // namespace relays
}  // namespace griddyn
