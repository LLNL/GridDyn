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

#include "zonalRelay.h"
#include "../comms/Communicator.h"
#include "../comms/relayMessage.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "../events/Event.h"
#include "../measurement/Condition.h"
#include "utilities/stringConversion.h"
#include "utilities/timeSeries.hpp"
#include <algorithm>

#include <boost/format.hpp>

namespace griddyn
{
namespace relays
{
zonalRelay::zonalRelay (const std::string &objName) : Relay (objName) { opFlags.set (continuous_flag); }

coreObject *zonalRelay::clone (coreObject *obj) const
{
    auto nobj = cloneBase<zonalRelay, Relay> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }

    nobj->m_zones = m_zones;
    nobj->m_terminal = m_terminal;
    nobj->m_zoneLevels = m_zoneLevels;
    nobj->m_zoneDelays = m_zoneDelays;
    nobj->m_resetMargin = m_resetMargin;
    nobj->autoName = autoName;
    nobj->m_condition_level = m_condition_level;
    return nobj;
}

void zonalRelay::setFlag (const std::string &flag, bool val)
{
    if (flag == "nondirectional")
    {
        opFlags.set (nondirectional_flag, val);
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
void zonalRelay::set (const std::string &param, const std::string &val)
{
    if (param == "levels")
    {
        auto dvals = str2vector<double> (val, kNullVal);
        // check to make sure all the levels are valid
        for (auto level : dvals)
        {
            if (level < -0.00001)
            {
                throw (invalidParameterValue (param));
            }
        }
        set ("zones", dvals.size ());
        m_zoneLevels = std::move (dvals);
    }
    else if (param == "delay")
    {
        auto dvals = str2vector<coreTime> (val, negTime);
        if (dvals.size () != m_zoneDelays.size ())
        {
            throw (invalidParameterValue (param));
        }
        // check to make sure all the values are valid
        for (auto ld : dvals)
        {
            if (ld < timeZero)
            {
                throw (invalidParameterValue (param));
            }
        }
        m_zoneDelays = std::move (dvals);
    }
    else
    {
        Relay::set (param, val);
    }
}

void zonalRelay::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    index_t zn;
    if (param == "zones")
    {
        m_zones = static_cast<count_t> (val);
        auto zlsize = static_cast<count_t> (m_zoneLevels.size ());
        if (m_zones > zlsize)
        {
            for (auto kk = zlsize; kk < m_zones; ++kk)
            {
                if (kk == 0)
                {
                    m_zoneLevels.push_back (0.8);
                    m_zoneDelays.push_back (timeZero);
                }
                else
                {
                    m_zoneLevels.push_back (m_zoneLevels[kk - 1] + 0.7);
                    m_zoneDelays.push_back (m_zoneDelays[kk - 1] + timeOneSecond);
                }
            }
        }
        else
        {
            m_zoneLevels.resize (m_zones);
            m_zoneDelays.resize (m_zones);
        }
    }
    else if ((param == "terminal") || (param == "side"))
    {
        m_terminal = static_cast<index_t> (val);
    }
    else if ((param == "resetmargin") || (param == "margin"))
    {
        m_resetMargin = val;
    }
    else if (param == "autoname")
    {
        autoName = static_cast<int> (val);
    }
    else if (param.compare (0, 5, "level") == 0)
    {
        zn = (param.size() == 6)?((isdigit (param[5])!=0) ? param[5] - '0' : 0):0;

        if (zn >= m_zones)
        {
            set ("zones", zn);
        }
		ensureSizeAtLeast(m_zoneLevels, zn + 1);
        m_zoneLevels[zn] = val;
    }
    else if (param.compare (0, 5, "delay") == 0)
    {
       zn = (param.size() == 6)?((isdigit (param[5])!=0) ? param[5] - '0' : 0):0;
        if (zn >= m_zones)
        {
            set ("zones", zn);
        }
		ensureSizeAtLeast(m_zoneDelays, zn + 1);
        m_zoneDelays[zn] = val;
    }
    else
    {
        Relay::set (param, val, unitType);
    }
}

double zonalRelay::get (const std::string &param, gridUnits::units_t unitType) const
{
    double val;
    if (param == "condition")
    {
        val = kNullVal;
    }
    else
    {
        val = Relay::get (param, unitType);
    }
    return val;
}

void zonalRelay::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    double baseImpedance = m_sourceObject->get ("impedance");
    for (index_t kk = 0; kk < m_zones; ++kk)
    {
        if (opFlags[nondirectional_flag])
        {
            add (std::shared_ptr<Condition> (make_condition ("abs(admittance" + std::to_string (m_terminal) + ")",
                                                             ">=", 1.0 / (m_zoneLevels[kk] * baseImpedance),
                                                             m_sourceObject)));
        }
        else
        {
            add (std::shared_ptr<Condition> (make_condition ("admittance" + std::to_string (m_terminal),
                                                             ">=", 1.0 / (m_zoneLevels[kk] * baseImpedance),
                                                             m_sourceObject)));
        }
        setResetMargin (kk, m_resetMargin * 1.0 / (m_zoneLevels[kk] * baseImpedance));
    }

    auto ge = std::make_unique<Event> ();
    ge->setTarget (m_sinkObject, "switch" + std::to_string (m_terminal));
    ge->setValue (1.0);

    add (std::shared_ptr<Event> (std::move (ge)));
    for (index_t kk = 0; kk < m_zones; ++kk)
    {
        setActionTrigger (0, kk, m_zoneDelays[kk]);
    }

    if (opFlags[use_commLink])
    {
        if (cManager.destName ().compare (0, 4, "auto") == 0)
        {
            if (cManager.destName ().length () == 6)
            {
                int code;
                try
                {
                    code = std::stoi (cManager.destName ().substr (5, 1));
                }
                catch (const std::invalid_argument &)
                {
                    code = 0;
                }

                std::string newName = generateAutoName (code);
                if (!newName.empty ())
                {
                    cManager.set ("commdestname", newName);
                }
            }
        }
    }
    return Relay::dynObjectInitializeA (time0, flags);
}

using namespace comms;

void zonalRelay::actionTaken (index_t ActionNum,
                              index_t conditionNum,
                              change_code /*actionReturn*/,
                              coreTime /*actionTime*/)
{
    LOG_NORMAL (
      (boost::format ("condition %d action %d taken terminal %d") % conditionNum % ActionNum % m_terminal).str ());

    if (opFlags[use_commLink])
    {
        if (ActionNum == 0)
        {
            auto P = std::make_shared<relayMessage>(relayMessage::BREAKER_TRIP_EVENT);
            cManager.send (P);
        }
    }
    for (index_t kk = conditionNum + 1; kk < m_zones; ++kk)
    {
        setConditionStatus (kk, condition_status_t::disabled);
    }
    if (conditionNum < m_condition_level)
    {
        m_condition_level = conditionNum;
    }
}

void zonalRelay::conditionTriggered (index_t conditionNum, coreTime /*triggerTime*/)
{
    LOG_NORMAL ((boost::format ("condition %d triggered terminal %d") % conditionNum % m_terminal).str ());
    if (conditionNum < m_condition_level)
    {
        m_condition_level = conditionNum;
    }
    if (opFlags[use_commLink])
    {
        if (conditionNum > m_condition_level)
        {
            return;
        }
        auto P = std::make_shared<commMessage> ();
        // std::cout << "GridDyn conditionTriggered(), conditionNum = " << conditionNum << '\n';
        if (conditionNum == 0)
        {
            // std::cout << "GridDyn setting relay message type to LOCAL_FAULT_EVENT" << '\n';
            P->setMessageType (commMessage::LOCAL_FAULT_EVENT);
        }
        else
        {
            // std::cout << "GridDyn setting relay message type to REMOTE_FAULT_EVENT" << '\n';
            P->setMessageType (commMessage::REMOTE_FAULT_EVENT);
        }
        cManager.send (P);
    }
}

void zonalRelay::conditionCleared (index_t conditionNum, coreTime /*triggerTime*/)
{
    LOG_NORMAL ((boost::format ("condition %d cleared terminal %d ") % conditionNum % m_terminal).str ());
    for (index_t kk = 0; kk < m_zones; ++kk)
    {
        if (getConditionStatus (kk) == condition_status_t::active)
        {
            m_condition_level = kk + 1;
        }
        else
        {
            return;
        }
    }
    if (opFlags[use_commLink])
    {
        auto P = std::make_shared<relayMessage> ();
        if (conditionNum == 0)
        {
            P->setMessageType (relayMessage::LOCAL_FAULT_CLEARED);
        }
        else
        {
            P->setMessageType (relayMessage::REMOTE_FAULT_CLEARED);
        }
        cManager.send (P);
    }
}

void zonalRelay::receiveMessage (std::uint64_t /*sourceID*/, std::shared_ptr<commMessage> message)
{
    switch (message->getMessageType ())
    {
    case relayMessage::BREAKER_TRIP_COMMAND:
        triggerAction (0);
        break;
    case relayMessage::BREAKER_CLOSE_COMMAND:
        if (m_sinkObject!=nullptr)
        {
            m_sinkObject->set ("switch" + std::to_string (m_terminal), 0);
        }
        break;
    case relayMessage::BREAKER_OOS_COMMAND:
        for (index_t kk = 0; kk < m_zones; ++kk)
        {
            setConditionStatus (kk, condition_status_t::disabled);
        }
        break;
    default:
    {
        // assert (false);
    }
    }
}

std::string zonalRelay::generateCommName ()
{
    if (autoName > 0)
    {
        std::string newName = generateAutoName (autoName);
        if (!newName.empty ())
        {
            if (newName != getName ())
            {
                setName (newName);
            }
            return newName;
        }
    }
    return getName ();
}

std::string zonalRelay::generateAutoName (int code)
{
    std::string autoname;
    auto b1 = m_sourceObject->getSubObject ("bus", 1);
    auto b2 = m_sourceObject->getSubObject ("bus", 2);

    switch (code)
    {
    case 1:
        if (m_terminal == 1)
        {
            autoname = b1->getName () + '_' + b2->getName ();
        }
        else
        {
            autoname = b2->getName () + '_' + b1->getName ();
        }
        break;
    case 2:
        if (m_terminal == 1)
        {
            autoname = std::to_string (b1->getUserID ()) + '_' + std::to_string (b2->getUserID ());
        }
        else
        {
            autoname = std::to_string (b2->getUserID ()) + '_' + std::to_string (b1->getUserID ());
        }
        break;
    default:;
        // do nothing
    }
    // check if there are multiple lines in parallel
    if (!autoname.empty ())
    {
        auto ri = m_sourceObject->getName ().rbegin ();
        if (*(ri + 1) == '_')
        {
            if ((*ri >= 'a') && (*ri <= 'z'))
            {
				autoname.push_back('_');
				autoname.push_back(*ri);
            }
        }
    }
    return autoname;
}
}  // namespace relays
}  // namespace griddyn
