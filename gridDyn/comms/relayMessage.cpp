/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#include "comms/relayMessage.h"

#include "stringOps.h"
#include <map>

static dMessageFactory<relayMessage, BASE_RELAY_MESSAGE_NUMBER, BASE_RELAY_MESSAGE_NUMBER + 12> dmf ("relay");

std::string relayMessage::toString ()
{
  switch (getMessageType ())
    {
    case NO_EVENT:
      return "NO EVENT";
      break;
    case LOCAL_FAULT_EVENT:
      return "LOCAL FAULT:" + std::to_string (m_code);
      break;
    case REMOTE_FAULT_EVENT:
      return "REMOTE FAULT:" + std::to_string (m_code);
      break;
    case BREAKER_TRIP_EVENT:
      return "BREAKER TRIP:" + std::to_string (m_code);
      break;
    case BREAKER_CLOSE_EVENT:
      return "BREAKER CLOSE:" + std::to_string (m_code);
      break;
    case LOCAL_FAULT_CLEARED:
      return "LOCAL FAULT CLEARED:" + std::to_string (m_code);
      break;
    case REMOTE_FAULT_CLEARED:
      return "REMOTE FAULT CLEARED:" + std::to_string (m_code);
      break;
    case BREAKER_TRIP_COMMAND:
      return "TRIP BREAKER:" + std::to_string (m_code);
      break;
    case BREAKER_CLOSE_COMMAND:
      return "CLOSE BREAKER:" + std::to_string (m_code);
      break;
    case BREAKER_OOS_COMMAND:
      return "BREAKER OOS:" + std::to_string (m_code);
      break;
    case ALARM_TRIGGER_EVENT:
      return "ALARM TRIGGER:" + std::to_string (m_code);
      break;
    case ALARM_CLEARED_EVENT:
      return "ALARM CLEARED:" + std::to_string (m_code);
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
	  setMessageType(ALARM_TRIGGER_EVENT);
  }
  else if (lstr == "ALARM CLEARED")
  {
	  setMessageType(ALARM_CLEARED_EVENT);
  }
}



static std::map<std::string, std::uint32_t> alarmCodeMap
{
	{"overcurrent",OVERCURRENT_ALARM},
	{"undercurrent",UNDERCURRENT_ALARM},
	{"overvoltage",OVERVOLTAGE_ALARM},
	{"undervoltage",UNDERVOLTAGE_ALARM},
	{"temperature_alarm1",TEMPERATURE_ALARM1},
	{"temperature",TEMPERATURE_ALARM1},
	{"temperature_alarm2",TEMPERATURE_ALARM2},
	{"temperature2",TEMPERATURE_ALARM2},
};

std::uint32_t getAlarmCode(const std::string &alarmStr)
{
	auto fnd = alarmCodeMap.find(alarmStr);
	if (fnd != alarmCodeMap.end())
	{
		return fnd->second;
	}
	return 0;
}