/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2014, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "controlRelay.h"
#include "core/gridDynExceptions.h"
#include "timeSeries.h"
#include "comms/gridCommunicator.h"
#include "comms/controlMessage.h"
#include "simulation/gridSimulation.h"
#include "eventQueue.h"
#include "gridEvent.h"
#include "gridCoreTemplates.h"
#include "stringOps.h"

#include <boost/format.hpp>

controlRelay::controlRelay (const std::string &objName) : gridRelay (objName)
{

}

gridCoreObject *controlRelay::clone (gridCoreObject *obj) const
{
  controlRelay *nobj = cloneBase<controlRelay, gridRelay> (this, obj);
  if (!(nobj))
    {
      return obj;
    }

  nobj->autoName = autoName;
  nobj->actionDelay = actionDelay;
  nobj->measureDelay = measureDelay;
  nobj->m_terminal = m_terminal;
  return nobj;
}

void controlRelay::setFlag (const std::string &flag, bool val)
{

  if (flag == "noreply")
    {
      opFlags.set (no_message_reply, val);
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
void controlRelay::set (const std::string &param,  const std::string &val)
{

  if (param[0] == '#')
    {
    }
  else
    {
      gridRelay::set (param, val);
    }

}

void controlRelay::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "autoname")
    {
      autoName = static_cast<int> (val);
    }
  else if (param == "delay")
    {
      actionDelay = val;

    }
  else if (param == "terminal")
    {
      m_terminal = static_cast<index_t> (val);
      m_terminal_key = std::to_string (m_terminal);
    }
  else
    {
      gridRelay::set (param, val, unitType);
    }

}

void controlRelay::dynObjectInitializeA (double time0, unsigned long flags)
{
  if (autoName > 0)
    {
      std::string newName = generateAutoName (autoName);
      if (!newName.empty ())
        {
          if (newName != name)
            {
              name = newName;
              alert (this, OBJECT_NAME_CHANGE);
            }
        }
    }

  rootSim = dynamic_cast<gridSimulation *> (parent->find ("root"));


  gridRelay::dynObjectInitializeA (time0, flags);
  if (dynamic_cast<gridLink *> (m_sourceObject))
    {
      opFlags.set (link_type_source);
    }
  if (dynamic_cast<gridLink *> (m_sinkObject))
    {
      opFlags.set (link_type_sink);
    }
}


void controlRelay::actionTaken (index_t ActionNum, index_t conditionNum, change_code /*actionReturn*/, double /*actionTime*/)
{
  LOG_NORMAL ((boost::format ("condition %d action %d taken") % conditionNum % ActionNum ).str ());


}



void controlRelay::receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> msg)
{
  auto m = std::dynamic_pointer_cast<controlMessage> (msg);
  std::shared_ptr<controlMessage> reply;
  index_t actnum;
  ++instructionCounter;

  switch (m->getMessageType ())
    {
    case controlMessage::SET:
      if (m->m_time <= prevTime + kSmallTime)
        {
          if (actionDelay <= kSmallTime)
            {
              auto fea = generateSetEvent (prevTime, sourceID, m);
              fea->execute (prevTime);                  //just execute the event immediately
            }
          else
            {
              auto fea = generateSetEvent (prevTime + actionDelay, sourceID, m);
              rootSim->add (fea);
            }
        }
      else
        {
          auto gres = std::make_shared<controlMessage> (controlMessage::SET_SCHEDULED);
          gres->m_actionID = (m->m_actionID > 0) ? m->m_actionID : instructionCounter;
          commLink->transmit (sourceID, gres);
          //make the event
          auto fea = generateSetEvent (m->m_time, sourceID, m);
          rootSim->add (fea);

        }
      break;
    case controlMessage::GET:
      if (m->m_time <= prevTime + kSmallTime)
        {
          if (measureDelay <= kSmallTime)
            {
              //just generate the action and execute it
              auto fea = generateGetEvent (prevTime, sourceID, m);
              fea->execute (prevTime);                 //just execute the event immediately

            }
          else
            {
              auto fea = generateGetEvent (prevTime + measureDelay, sourceID, m);
              rootSim->add (fea);
            }
        }
      else
        {
          auto gres = std::make_shared<controlMessage> (controlMessage::GET_SCHEDULED);
          gres->m_actionID = (m->m_actionID > 0) ? m->m_actionID : instructionCounter;
          commLink->transmit (sourceID, gres);
          auto fea = generateGetEvent (m->m_time,sourceID,m);
          rootSim->add (fea);
        }
      break;
    case controlMessage::GET_MULTIPLE:
      break;
    case controlMessage::GET_PERIODIC:
      break;
    case controlMessage::GET_RESULT_MULTIPLE:
    case controlMessage::SET_SUCCESS:
    case controlMessage::SET_FAIL:
    case controlMessage::GET_RESULT:
    case controlMessage::SET_SCHEDULED:
    case controlMessage::GET_SCHEDULED:
    case controlMessage::CANCEL_FAIL:
    case controlMessage::CANCEL_SUCCESS:
      break;
    case controlMessage::CANCEL:
      actnum = findAction (m->m_actionID);

      if (actnum != kNullLocation)
        {
          if ((actions[actnum].executed == false) && (actions[actnum].triggerTime > actionDelay))
            {            //cannot cancel actions closer than the inherent actionDelay
              actions[actnum].executed = true;
              auto gres = std::make_shared<controlMessage> (controlMessage::CANCEL_SUCCESS);
              gres->m_actionID = m->m_actionID;
              commLink->transmit (sourceID, gres);
            }
          else
            {
              auto gres = std::make_shared<controlMessage> (controlMessage::CANCEL_FAIL);
              gres->m_actionID = m->m_actionID;
              commLink->transmit (sourceID, gres);
            }
        }
      else
        {
          auto gres = std::make_shared<controlMessage> (controlMessage::CANCEL_FAIL);
          gres->m_actionID = m->m_actionID;
          commLink->transmit (sourceID, gres);
        }
      break;
    }

}

std::string controlRelay::generateAutoName (int code)
{
  std::string autoname = "";


  switch (code)
    {
    case 1:
      autoname = m_sinkObject->getName ();
      break;
    case 2:
      autoname = m_sourceObject->getName ();
      break;
    default:
      ;
      //do nothing
    }

  return autoname;
}


change_code controlRelay::executeAction (index_t index)
{
  if (index > actions.size ())
    {
      return change_code::not_triggered;
    }
  auto cact = actions[index];
  if (cact.executed == false)
    {
      cact.executed = true;

      if (cact.measureAction)
        {
          double val = m_sourceObject->get (cact.field, cact.unitType);
          auto gres = std::make_shared<controlMessage> (controlMessage::GET_RESULT);
          gres->m_field = cact.field;
          gres->m_value = val;
          gres->m_time = prevTime;
          commLink->transmit (cact.sourceID, gres);
          return change_code::no_change;
        }
      else
        {
		  try
		  {
			  m_sinkObject->set(cact.field, cact.val, cact.unitType);
			  if (!opFlags.test(no_message_reply))               //unless told not to respond return with the
			  {
				  auto gres = std::make_shared<controlMessage>(controlMessage::SET_SUCCESS);
				  gres->m_actionID = cact.actionID;
				  commLink->transmit(cact.sourceID, gres);
			  }
			  return change_code::parameter_change;
		  }
		  catch (const gridDynException &)
		  {
			  if (!opFlags.test(no_message_reply))               //unless told not to respond return with the
			  {
				  auto gres = std::make_shared<controlMessage>(controlMessage::SET_FAIL);
				  gres->m_actionID = cact.actionID;
				  commLink->transmit(cact.sourceID, gres);
			  }
			  return change_code::execution_failure;
		 }
        
        }
    }
  return change_code::not_triggered;
}


std::shared_ptr<functionEventAdapter> controlRelay::generateGetEvent (double eventTime, std::uint64_t sourceID,std::shared_ptr<controlMessage> m)
{
  
  auto act = getFreeAction ();
  actions[act].actionID = (m->m_actionID > 0) ? m->m_actionID : instructionCounter;
  actions[act].executed = false;
  actions[act].measureAction = true;
  actions[act].sourceID = sourceID;
  actions[act].triggerTime = eventTime;
  makeLowerCase(m->m_field);
  if (opFlags[link_type_source])
  {

	  actions[act].field = m->m_field + m_terminal_key;

  }
  else
  {
	  actions[act].field = m->m_field;
  }
  if (!(m->m_units.empty()))
  {
	  actions[act].unitType = gridUnits::getUnits(m->m_units);
  }
  auto fea = std::make_shared<functionEventAdapter> ([act, this]() {
    return executeAction (act);
  },eventTime);
  return fea;
}


std::shared_ptr<functionEventAdapter> controlRelay::generateSetEvent (double eventTime, std::uint64_t sourceID, std::shared_ptr<controlMessage> m)
{
	auto act = getFreeAction();
	actions[act].actionID = (m->m_actionID > 0) ? m->m_actionID : instructionCounter;
	actions[act].executed = false;
	actions[act].measureAction = false;
	actions[act].sourceID = sourceID;
	actions[act].triggerTime = eventTime;
  makeLowerCase (m->m_field);
  if (opFlags[link_type_sink])
    {

      if ((m->m_field == "breaker") || (m->m_field == "switch")||(m->m_field == "breaker_open"))
        {
		  actions[act].field = m->m_field + m_terminal_key;
        }
      else
        {
		  actions[act].field = m->m_field;
        }

    }
  else
    {
	  actions[act].field = m->m_field;
    }
  actions[act].val = m->m_value;
 
  if (!m->m_units.empty())
  {
	  actions[act].unitType = gridUnits::getUnits(m->m_units);
  }
 
  auto fea = std::make_shared<functionEventAdapter> ([act, this]() {
    return executeAction (act);
  },eventTime);
  return fea;
}

index_t controlRelay::findAction (std::uint64_t actionID)
{
  for (index_t ii = 0; ii < actions.size (); ++ii)
    {
      if (actions[ii].actionID == actionID)
        {
          return ii;
        }
    }
  return kNullLocation;
}

index_t controlRelay::getFreeAction ()
{
  index_t act;
  for (act = 0; act < actions.size (); ++act)
    {
      if (actions[act].executed == false)
        {
          return act;
        }
    }
  //if we didn't find an open one,  make the actions vector longer and return the new index
  act = static_cast<index_t> (actions.size ());
  actions.resize ((act + 1) * 2);  //double the size
  return act;
}
