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

#include "differentialRelay.h"
#include "gridCondition.h"
#include "fileReaders.h"
#include "comms/gridCommunicator.h"
#include "comms/relayMessage.h"
#include "eventQueue.h"
#include "gridEvent.h"
#include "linkModels/gridLink.h"
#include "gridBus.h"
#include "gridCoreTemplates.h"


differentialRelay::differentialRelay (const std::string&objName) : gridRelay (objName)
{
  opFlags.set (continuous_flag);
}

gridCoreObject *differentialRelay::clone (gridCoreObject *obj) const
{
  differentialRelay *nobj = cloneBase<differentialRelay, gridRelay> (this, obj);
  if (!(nobj))
    {
      return obj;
    }
  nobj->m_max_differential = m_max_differential;
  nobj->m_delayTime = m_delayTime;
  nobj->m_resetMargin = m_resetMargin;
  nobj->m_minLevel = m_minLevel;
  return nobj;
}

int differentialRelay::setFlag (const std::string &flag, bool val)
{
  int out = PARAMETER_FOUND;

  if (flag == "relative")
    {
      opFlags.set (relative_differential_flag,val);
    }
  if (flag == "absolute")
    {
      opFlags.set (relative_differential_flag,!val);
    }
  else
    {
      out = gridRelay::setFlag (flag, val);
    }
  return out;
}

bool differentialRelay::getFlag (const std::string &param) const
{
  if (param == "relative")
    {
      return opFlags[relative_differential_flag];
    }
  else
    {
      return gridRelay::getFlag (param);
    }
}

int differentialRelay::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;

  if (param[0] == '#')
    {

    }
  else
    {
      out = gridRelay::set (param, val);
    }
  return out;
}


static const stringVec locNumStrings {
  "delay","max_difference","reset_margin","minlevel"
};
static const stringVec locStrStrings {
};

static const stringVec locFlagStrings {
  "relative"
};

void differentialRelay::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
  getParamString<differentialRelay, gridRelay> (this, pstr, locNumStrings, locStrStrings, {}, pstype);
}

int differentialRelay::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;

  if (param == "delay")
    {
      m_delayTime = val;
    }
  else if ((param == "level") || (param == "max_difference"))
    {
      m_max_differential = val;
    }
  else if (param == "reset_margin")
    {
      m_resetMargin = val;
    }
  else if (param == "minlevel")
    {
      m_minLevel = val;
    }
  else
    {
      out = gridRelay::set (param, val, unitType);
    }
  return out;
}

void differentialRelay::pFlowObjectInitializeA (double time0, unsigned long flags)
{
  //if the target object is a link of some kind
  if (dynamic_cast<gridLink *> (m_sourceObject))
    {
      double tap = m_sourceObject->get ("tap");
      if (opFlags[relative_differential_flag])
        {
          if (tap != 1.0)
            {
              std::string c1 = std::to_string (tap) + "*current1";
              add (make_condition ("abs(" + c1 + "-current2)/max(abs(" + c1 + "),abs(current2))", ">", m_max_differential, m_sourceObject));
              if (m_minLevel > 0)
                {
                  add (make_condition ("max(abs(" + c1 + "),abs(current2))", ">", m_minLevel, m_sourceObject));
                }
            }
          else
            {
              add (make_condition ("abs(current1-current2)/max(abs(current1),abs(current2))", ">", m_max_differential, m_sourceObject));
              if (m_minLevel > 0)
                {
                  add (make_condition ("max(abs(current1),abs(current2))", ">", m_minLevel, m_sourceObject));
                }
            }

        }
      else
        {
          if (tap != 1.0)
            {
              add (make_condition ("abs(" + std::to_string (tap) + "*current1-current2)", ">", m_max_differential, m_sourceObject));
            }
          else
            {
              add (make_condition ("abs(current1-current2)", ">", m_max_differential, m_sourceObject));
            }
        }
      opFlags.set (link_mode);
      opFlags.reset (bus_mode);
    }
  else if (dynamic_cast<gridBus *> (m_sourceObject))
    {
      add (make_condition ("abs(load)", "<=", m_max_differential, m_sourceObject));
      opFlags.set (bus_mode);
      opFlags.reset (link_mode);
    }


  auto ge = std::make_shared<gridEvent> ();
  ge->field = "connected";
  ge->value = 0;
  ge->setTarget (m_sinkObject);
  //action 2 to reenable object

  add (ge);
  if ((opFlags[relative_differential_flag]) && (opFlags[link_mode]) && (m_minLevel > 0))
    {
      setActionMultiTrigger ({ 0, 1 }, 0, m_delayTime);
    }
  else
    {
      setActionTrigger (0, 0, m_delayTime);
    }

  return gridRelay::pFlowObjectInitializeA (time0, flags);
}

void differentialRelay::actionTaken (index_t ActionNum, index_t /*conditionNum*/,  change_code /*actionReturn*/, double /*actionTime*/)
{
  LOG_NORMAL ("Relay Tripped");

  if (opFlags.test (use_commLink))
    {
      auto P = std::make_shared<relayMessage> (relayMessage::BREAKER_TRIP_EVENT);
      if (ActionNum == 0)
        {
          if (commDestName.empty ())
            {
              commLink->transmit (commDestId, P);
            }
          else
            {
              commLink->transmit (commDestName, P);
            }
        }
    }

}

void differentialRelay::conditionTriggered (index_t /*conditionNum*/, double /*triggerTime*/)
{
  LOG_NORMAL ("differential condition met");
  if (opFlags.test (use_commLink))
    {
      //std::cout << "GridDyn conditionTriggered(), conditionNum = " << conditionNum << '\n';
      auto P = std::make_shared<relayMessage> (relayMessage::LOCAL_FAULT_EVENT);

      if (commDestName.empty ())
        {
          commLink->transmit (commDestId, P);
        }
      else
        {
          commLink->transmit (commDestName, P);
        }
    }

}

void differentialRelay::conditionCleared (index_t /*conditionNum*/, double /*triggerTime*/)
{
  LOG_NORMAL ("differential condition cleared");

  if (opFlags.test (use_commLink))
    {
      auto P = std::make_shared<relayMessage> (relayMessage::LOCAL_FAULT_CLEARED);
      if (commDestName.empty ())
        {

          commLink->transmit (commDestId, P);
        }
      else
        {
          commLink->transmit (commDestName, P);
        }
    }
}


void differentialRelay::receiveMessage (std::uint64_t /*sourceID*/, std::shared_ptr<commMessage> message)
{
  switch (message->getMessageType ())
    {
    case relayMessage::BREAKER_TRIP_COMMAND:
      triggerAction (0);
      break;
    case relayMessage::BREAKER_CLOSE_COMMAND:
      if (m_sinkObject)
        {
          m_sinkObject->set ("enable", 1);
        }
      break;
    case relayMessage::BREAKER_OOS_COMMAND:

      setConditionState (0, condition_states::disabled);
      break;
    default:
      {
        assert (false);
      }
    }

}
