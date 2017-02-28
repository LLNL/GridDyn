/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include "stringConversion.h"
#include "comms/schedulerMessage.h"
#include <cstring>

static dMessageFactory<schedulerMessage, BASE_SCHEDULER_MESSAGE_NUMBER, BASE_SCHEDULER_MESSAGE_NUMBER + 16> dmf ("scheduler");

schedulerMessage::schedulerMessage (std::uint32_t type, std::vector<double> time, std::vector<double> target) : commMessage (type)
{
  m_time = time;
  m_target = target;
}
void schedulerMessage::loadMessage (std::uint32_t nType, std::vector<double> time, std::vector<double> target)
{
  setMessageType (nType);
  m_time = time;
  m_target = target;
}



std::string schedulerMessage::toString (int modifiers) const
{
	std::string typeString = (modifiers == comm_modifiers::with_type) ? encodeTypeInString() : "";
  auto tsize = m_time.size ();

  switch (getMessageType ())
    {
    case CLEAR_TARGETS:
      return typeString+"CLEAR TARGETS";
    case SHUTDOWN:
      return typeString + "SHUTDOWN @" + std::to_string (m_time[0]);
    case STARTUP:
      return typeString + "STARTUP @:" + std::to_string (m_time[0]);
    case ADD_TARGETS:
      return typeString + "ADD TARGETS:" + makeTargetString (tsize);
    case UPDATE_TARGETS:
      return typeString + "UPDATE TARGETS:" + makeTargetString (tsize);
    case UPDATE_RESERVES:
      return typeString + "UPDATE RESERVES:" + makeTargetString (tsize);
    case UPDATE_REGULATION_RESERVE:
      return typeString + "UPDATE REGULATION RESERVE:" + makeTargetString (tsize);
    case USE_RESERVE:
      return typeString + "USE RESERVE:" + makeTargetString (tsize);
    case UPDATE_REGULATION_TARGET:
      return typeString + "UPDATE REGULATION TARGET:" + makeTargetString (tsize);
    }
  return typeString + "<UNKNOWN>";
}

void schedulerMessage::loadString (const std::string &fromString)
{
  auto strV = stringOps::splitline (fromString, ':');
  auto lstr = convertToUpperCase (strV[0]);

  std::vector<double> targets;
  if (strV.size () > 1)
    {
      targets = str2vector (strV[1], kNullVal, "@ ");
    }

  auto loadtargets = [this](std::vector<double> newTargets)
    {
      auto dvs = newTargets.size () / 2;
      m_time.resize (dvs);
      m_target.resize (dvs);
      for (size_t kk = 0; kk < newTargets.size () - 1; kk += 2)
        {
          m_target[kk >> 1] = newTargets[kk];              //divide by 2
          m_time[(kk >> 1) + 1] = newTargets[kk + 1];
        }
    };

  if (lstr == "CLEAR TARGETS")
    {
      setMessageType (CLEAR_TARGETS);
    }
  else if (lstr == "SHUTDOWN")
    {
      setMessageType (SHUTDOWN);
      m_time.resize (1);
      m_time[0] = targets[0];
    }
  else if (lstr == "gridState_t::STARTUP")
    {
      setMessageType (STARTUP);
      m_time.resize (1);
      m_time[0] = targets[0];
    }
  else if (lstr == "ADD TARGETS")
    {
      setMessageType (ADD_TARGETS);
      loadtargets (targets);
    }
  else if (lstr == "UPDATE TARGETS")
    {
      setMessageType (UPDATE_TARGETS);
      loadtargets (targets);
    }
  else if (lstr == "UPDATE RESERVES")
    {
      setMessageType (UPDATE_RESERVES);
      loadtargets (targets);
    }
  else if (lstr == "UPDATE REGULATION RESERVE")
    {
      setMessageType (UPDATE_REGULATION_RESERVE);
      loadtargets (targets);
    }
  else if (lstr == "USE RESERVE")
    {
      setMessageType (USE_RESERVE);
      loadtargets (targets);
    }
  else if (lstr == "UPDATE REGULATION TARGET")
    {
      setMessageType (UPDATE_REGULATION_TARGET);
      loadtargets (targets);
    }

}

std::string schedulerMessage::makeTargetString(size_t cnt) const
{
	std::string targetString;
	for (size_t kk = 0; kk < cnt; ++kk)
	{
		targetString += ((kk == 0) ? "" : " ") + std::to_string(m_target[kk]) + '@' + std::to_string(m_time[kk]);
	}
	return targetString;
}