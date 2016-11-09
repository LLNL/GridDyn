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

#include "loadRelay.h"
#include "gridCondition.h"
#include "gridEvent.h"
#include "gridCoreTemplates.h"

#include <boost/format.hpp>

loadRelay::loadRelay (const std::string&objName) : gridRelay (objName)
{
  // opFlags.set(continuous_flag);
}

gridCoreObject *loadRelay::clone (gridCoreObject *obj) const
{
  loadRelay *nobj = cloneBase<loadRelay, gridRelay> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }

  nobj->cutoutVoltage = cutoutVoltage;
  nobj->cutoutFrequency = cutoutFrequency;
  return nobj;
}

void loadRelay::setFlag (const std::string &flag, bool val)
{

  if (flag == "nondirectional")
    {
      opFlags.set (nondirectional_flag,val);
    }
  else
    {
      gridRelay::setFlag (flag, val);
    }
}
/*
std::string commDestName;
std::uint64_t commDestId=0;
std::string commType;
*/
void loadRelay::set (const std::string &param,  const std::string &val)
{

  if (param[0] == '#')
    {

    }
  else
    {
      gridRelay::set (param, val);
    }

}

void loadRelay::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if ((param == "cutoutvoltage") || (param == "voltagelimit"))
    {
      cutoutVoltage = gridUnits::unitConversion (val, unitType, gridUnits::puV, systemBasePower);
    }
  else if ((param == "cutoutfrequency") || (param == "freqlimit"))
    {
      cutoutFrequency = gridUnits::unitConversion (val, unitType, gridUnits::puHz, m_baseFreq);
    }
  else if (param == "delay")
    {
      voltageDelay = val;
      frequencyDelay = val;
    }
  else if (param == "voltagedelay")
    {
      voltageDelay = val;
    }
  else if (param == "frequencydelay")
    {
      frequencyDelay = val;
    }
  else if (param == "offtime")
    {
      offTime = val;
    }
  else
    {
      gridRelay::set (param, val, unitType);
    }

}

void loadRelay::dynObjectInitializeA (double time0, unsigned long flags)
{

  auto ge = std::make_shared<gridEvent> ();

  ge->setTarget (m_sinkObject, "status");
  ge->setValue(0.0);

  add (ge);

  if (cutoutVoltage < 2.0)
    {
      add (make_condition ("voltage", "<", cutoutVoltage, m_sourceObject));
      setActionTrigger (0, 0, voltageDelay);
    }
  if (cutoutFrequency < 2.0)
    {
      add (make_condition ("frequency", "<", cutoutFrequency, m_sourceObject));
      setActionTrigger (1, 0, frequencyDelay);
    }

  gridRelay::dynObjectInitializeA (time0, flags);
}


void loadRelay::actionTaken (index_t ActionNum, index_t conditionNum, change_code /*actionReturn*/, double /*actionTime*/)
{
  LOG_NORMAL ((boost::format ("condition %d action %d") %  conditionNum % ActionNum).str ());
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
  setConditionState (kk, condition_states::disabled);
  }
  if (conditionNum < m_condition_level)
  {
  m_condition_level = conditionNum;
  }
  */
}

void loadRelay::conditionTriggered (index_t conditionNum, double /*triggerTime*/)
{
  LOG_NORMAL ((boost::format ("condition %d triggered") % conditionNum).str ());
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

void loadRelay::conditionCleared (index_t conditionNum, double /*triggerTime*/)
{
  LOG_NORMAL ((boost::format ("condition %d cleared") % conditionNum ).str ());
  /*for (size_t kk = 0; kk < m_zones; ++kk)
 {
 if (cStates[kk] == condition_states::active)
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



