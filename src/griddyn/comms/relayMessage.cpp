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
#include "comms/relayMessage.h"

#include "utilities/stringOps.h"
#include <map>

namespace griddyn
{
namespace comms
{
static dMessageFactory<relayMessage, BASE_RELAY_MESSAGE_NUMBER, BASE_RELAY_MESSAGE_NUMBER + 12> dmf ("relay");

std::string relayMessage::to_string (int modifiers) const
{
    std::string typeString = (modifiers == comm_modifiers::with_type) ? encodeTypeInString () : "";

    switch (getMessageType ())
    {
    case NO_EVENT:
        return typeString + "NO EVENT";
        break;
    case LOCAL_FAULT_EVENT:
        return typeString + "LOCAL FAULT:" + std::to_string (m_code);
        break;
    case REMOTE_FAULT_EVENT:
        return typeString + "REMOTE FAULT:" + std::to_string (m_code);
        break;
    case BREAKER_TRIP_EVENT:
        return typeString + "BREAKER TRIP:" + std::to_string (m_code);
        break;
    case BREAKER_CLOSE_EVENT:
        return typeString + "BREAKER CLOSE:" + std::to_string (m_code);
        break;
    case LOCAL_FAULT_CLEARED:
        return typeString + "LOCAL FAULT CLEARED:" + std::to_string (m_code);
        break;
    case REMOTE_FAULT_CLEARED:
        return typeString + "REMOTE FAULT CLEARED:" + std::to_string (m_code);
        break;
    case BREAKER_TRIP_COMMAND:
        return typeString + "TRIP BREAKER:" + std::to_string (m_code);
        break;
    case BREAKER_CLOSE_COMMAND:
        return typeString + "CLOSE BREAKER:" + std::to_string (m_code);
        break;
    case BREAKER_OOS_COMMAND:
        return typeString + "BREAKER OOS:" + std::to_string (m_code);
        break;
    case ALARM_TRIGGER_EVENT:
        return typeString + "ALARM TRIGGER:" + std::to_string (m_code);
        break;
    case ALARM_CLEARED_EVENT:
        return typeString + "ALARM CLEARED:" + std::to_string (m_code);
        break;
    }
    return "<UNKNOWN>";
}

void relayMessage::loadString (const std::string &fromString)
{
    auto lstr = convertToUpperCase (fromString);
    auto cc = fromString.find_first_of (':');
    if (cc != std::string::npos)
    {
        lstr = lstr.substr (1, cc - 1);
        m_code = std::stoi (fromString.substr (cc + 1));
    }
    else
    {
        m_code = 0;
    }

    if (lstr == "NO EVENT")
    {
        setMessageType (NO_EVENT);
    }
    else if (lstr == "LOCAL FAULT")
    {
        setMessageType (LOCAL_FAULT_EVENT);
    }
    else if (lstr == "REMOTE FAULT")
    {
        setMessageType (REMOTE_FAULT_EVENT);
    }
    else if (lstr == "BREAKER TRIP")
    {
        setMessageType (BREAKER_TRIP_EVENT);
    }
    else if (lstr == "BREAKER CLOSE")
    {
        setMessageType (BREAKER_CLOSE_EVENT);
    }
    else if (lstr == "LOCAL FAULT CLEARED")
    {
        setMessageType (LOCAL_FAULT_CLEARED);
    }
    else if (lstr == "REMOTE FAULT CLEARED")
    {
        setMessageType (REMOTE_FAULT_CLEARED);
    }
    else if (lstr == "TRIP BREAKER")
    {
        setMessageType (BREAKER_TRIP_COMMAND);
    }
    else if (lstr == "CLOSE BREAKER")
    {
        setMessageType (BREAKER_CLOSE_COMMAND);
    }
    else if (lstr == "BREAKER OOS ")
    {
        setMessageType (BREAKER_OOS_COMMAND);
    }
    else if (lstr == " ALARM TRIGGER")
    {
        setMessageType (ALARM_TRIGGER_EVENT);
    }
    else if (lstr == "ALARM CLEARED")
    {
        setMessageType (ALARM_CLEARED_EVENT);
    }
}

static std::map<std::string, std::uint32_t> alarmCodeMap{
  {"overcurrent", OVERCURRENT_ALARM},         {"undercurrent", UNDERCURRENT_ALARM},
  {"overvoltage", OVERVOLTAGE_ALARM},         {"undervoltage", UNDERVOLTAGE_ALARM},
  {"temperature_alarm1", TEMPERATURE_ALARM1}, {"temperature", TEMPERATURE_ALARM1},
  {"temperature_alarm2", TEMPERATURE_ALARM2}, {"temperature2", TEMPERATURE_ALARM2},
};

std::uint32_t getAlarmCode (const std::string &alarmStr)
{
    auto fnd = alarmCodeMap.find (alarmStr);
    if (fnd != alarmCodeMap.end ())
    {
        return fnd->second;
    }
    return 0;
}

}  // namespace comms
}  // namespace griddyn