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

#include "loadRelay.h"

#include "../events/Event.h"
#include "../measurement/Condition.h"
#include "core/coreObjectTemplates.hpp"

#include <boost/format.hpp>

namespace griddyn {
namespace relays {
    loadRelay::loadRelay(const std::string& objName): Relay(objName)
    {
        // opFlags.set(continuous_flag);
    }

    coreObject* loadRelay::clone(coreObject* obj) const
    {
        auto nobj = cloneBase<loadRelay, Relay>(this, obj);
        if (nobj == nullptr) {
            return obj;
        }

        nobj->cutoutVoltage = cutoutVoltage;
        nobj->cutoutFrequency = cutoutFrequency;
        nobj->voltageDelay = voltageDelay;
        nobj->frequencyDelay = frequencyDelay;
        nobj->offTime = offTime;
        return nobj;
    }

    void loadRelay::setFlag(const std::string& flag, bool val)
    {
        if (flag == "nondirectional") {
            opFlags.set(nondirectional_flag, val);
        } else {
            Relay::setFlag(flag, val);
        }
    }
    /*
std::string commDestName;
std::uint64_t commDestId=0;
std::string commType;
*/
    void loadRelay::set(const std::string& param, const std::string& val)
    {
        if (param[0] == '#') {
        } else {
            Relay::set(param, val);
        }
    }

    void loadRelay::set(const std::string& param, double val, units::unit unitType)
    {
        if ((param == "cutoutvoltage") || (param == "voltagelimit")) {
            cutoutVoltage =
                units::convert(val, unitType, units::puV, systemBasePower, baseVoltage());
        } else if ((param == "cutoutfrequency") || (param == "freqlimit")) {
            cutoutFrequency = units::convert(val, unitType, units::puHz, systemBaseFrequency);
        } else if (param == "delay") {
            voltageDelay = val;
            frequencyDelay = val;
        } else if (param == "voltagedelay") {
            voltageDelay = val;
        } else if (param == "frequencydelay") {
            frequencyDelay = val;
        } else if (param == "offtime") {
            offTime = val;
        } else {
            Relay::set(param, val, unitType);
        }
    }

    void loadRelay::dynObjectInitializeA(coreTime time0, std::uint32_t flags)
    {
        auto ge = std::make_shared<Event>();

        ge->setTarget(m_sinkObject, "status");
        ge->setValue(0.0);

        add(std::move(ge));
        add(std::shared_ptr<Condition>(
            make_condition("voltage", "<", cutoutVoltage, m_sourceObject)));
        add(std::shared_ptr<Condition>(
            make_condition("frequency", "<", cutoutFrequency, m_sourceObject)));
        if (cutoutVoltage < 2.0) {
            setActionTrigger(0, 0, voltageDelay);
        } else {
            setConditionStatus(0, condition_status_t::disabled);
        }
        if (cutoutFrequency < 2.0) {
            setActionTrigger(0, 1, frequencyDelay);
        } else {
            setConditionStatus(1, condition_status_t::disabled);
        }

        Relay::dynObjectInitializeA(time0, flags);
    }

    void loadRelay::actionTaken(index_t ActionNum,
                                index_t conditionNum,
                                change_code /*actionReturn*/,
                                coreTime /*actionTime*/)
    {
        LOG_NORMAL((boost::format("condition %d action %d") % conditionNum % ActionNum).str());
        ((void)(ActionNum));
        ((void)(conditionNum));
        /*
    if (opFlags.test (use_commLink))
    {
    relayMessage P;
    if (ActionNum == 0)
    {
    P.setMessageType (relayMessage::MESSAGE_TYPE::BREAKER_TRIP_EVENT);
    if (commDestName.empty ())
    {
    auto b = P.buffer ();
    commLink->transmit (commDestId, static_cast<int> (P.GetMessageType ()), P.size (), b);
    }
    else
    {
    auto b = P.buffer ();
    commLink->transmit (commDestName, static_cast<int> (P.GetMessageType ()), P.size (), b);
    }
    }
    }
    for (size_t kk = conditionNum + 1; kk < m_zones; ++kk)
    {
    setConditionStatus (kk, condition_status_t::disabled);
    }
    if (conditionNum < m_condition_level)
    {
    m_condition_level = conditionNum;
    }
    */
    }

    void loadRelay::conditionTriggered(index_t conditionNum, coreTime /*triggerTime*/)
    {
        LOG_NORMAL((boost::format("condition %d triggered") % conditionNum).str());
        ((void)(conditionNum));
        /*
    if (conditionNum < m_condition_level)
    {
    m_condition_level = conditionNum;
    }
    if (opFlags.test (use_commLink))
    {
    if (conditionNum > m_condition_level)
    {
    return;
    }
    relayMessage P;
    //std::cout << "GridDyn conditionTriggered(), conditionNum = " << conditionNum << '\n';
    if (conditionNum == 0)
    {
    //std::cout << "GridDyn setting relay message type to LOCAL_FAULT_EVENT" << '\n';
    P.setMessageType (relayMessage::MESSAGE_TYPE::LOCAL_FAULT_EVENT);
    }
    else
    {
    //std::cout << "GridDyn setting relay message type to REMOTE_FAULT_EVENT" << '\n';
    P.setMessageType (relayMessage::MESSAGE_TYPE::REMOTE_FAULT_EVENT);
    }
    if (commDestName.empty ())
    {
    auto b = P.buffer ();
    commLink->transmit (commDestId, static_cast<int> (P.GetMessageType ()), P.size (), b);
    }
    else
    {
    auto b = P.buffer ();
    commLink->transmit (commDestName, static_cast<int> (P.GetMessageType ()), P.size (), b);
    }
    }
    */
    }

    void loadRelay::conditionCleared(index_t conditionNum, coreTime /*triggerTime*/)
    {
        LOG_NORMAL((boost::format("condition %d cleared") % conditionNum).str());
        ((void)(conditionNum));
        /*for (size_t kk = 0; kk < m_zones; ++kk)
   {
   if (cStates[kk] == condition_status_t::active)
   {
   m_condition_level = kk + 1;
   }
   else
   {
   return;
   }
   }
   if (opFlags.test (use_commLink))
   {
   relayMessage P;
   if (conditionNum == 0)
   {
   P.setMessageType (relayMessage::MESSAGE_TYPE::LOCAL_FAULT_CLEARED);
   }
   else
   {
   P.setMessageType (relayMessage::MESSAGE_TYPE::REMOTE_FAULT_CLEARED);
   }
   if (commDestName.empty ())
   {
   auto b = P.buffer ();
   commLink->transmit (commDestId, static_cast<int> (P.GetMessageType ()), P.size (), b);
   }
   else
   {
   auto b = P.buffer ();
   commLink->transmit (commDestName, static_cast<int> (P.GetMessageType ()), P.size (), b);
   }
   }
   */
    }
}  // namespace relays
}  // namespace griddyn
