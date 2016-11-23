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

#include "eventQueue.h"

#include "gridEvent.h"
#include "relays/gridRelay.h"
#include "gridCore.h"
#include "collector.h"
#include "core/gridDynExceptions.h"

#include <typeinfo>

eventQueue::eventQueue ()
{
  nullEvent = std::make_shared<eventAdapter> ();
  insert (nullEvent);
}

double eventQueue::getNextTime () const
{
  return events.front ()->m_nextTime;

}

void eventQueue::nullEventTime (gridDyn_time time, double period)
{

  nullEvent->m_nextTime = time;
  if (period != kNullVal)
    {
      nullEvent->m_period = period;
    }
  events.sort (compareEventAdapters);
}

double eventQueue::getNullEventTime () const
{
  return nullEvent->m_nextTime;
}


std::shared_ptr<eventQueue> eventQueue::clone(std::shared_ptr<eventQueue> eQ) const
{
	auto newQueue = eQ;
	if (!newQueue)
	{
		newQueue = std::make_shared<eventQueue>();
	}
	nullEvent->clone(newQueue->nullEvent);
	for (auto ev : events)
	{
		if (ev == nullEvent) //we dealt with the nullEvent separately
		{
			continue;
		}
		newQueue->insert(ev->clone());
	}
	return newQueue;
}

void eventQueue::mapObjectsOnto(gridCoreObject *newRootObject)
{
	for (auto ev : events)
	{
		ev->updateObject(newRootObject, object_update_mode::match);
	}
}

change_code eventQueue::executeEvents (double cTime)
{
  if (events.front ()->m_nextTime > cTime + timeTols)
    {
      return change_code::no_change;
    }
  auto ret = change_code::no_change;
  auto eret = change_code::no_change;
  if (!partB_list.empty ())
    {
      ret = executeEventsBonly (cTime);
    }
  eret = executeEventsAonly (cTime);
  if (eret > ret)
    {
      ret = eret;
    }
  eret = executeEventsBonly (cTime);
  if (eret > ret)
    {
      ret = eret;
    }

  return ret;
}

change_code eventQueue::executeEventsAonly (double cTime)
{
  if (events.front ()->m_nextTime > cTime + timeTols)
    {
      return change_code::no_change;
    }
  auto nextEvent = events.begin ();
  auto currentEvent = nextEvent;
  auto ret = change_code::no_change;
  auto eret = change_code::no_change;

  bool remove_events = false;
  while ((*nextEvent)->m_nextTime <= cTime + timeTols)
    {
      currentEvent = nextEvent;
      ++nextEvent;
      if ((*currentEvent)->two_part_execute)
        {
          if ((*currentEvent)->partB_turn)
            {
              eret = (*currentEvent)->execute (cTime + timeTols);
              if (eret > ret)
                {
                  ret = eret;
                }
              if ((*currentEvent)->m_remove_event)
                {
				  remove_events = true;
                }
              (*currentEvent)->partB_turn = false;
            }
          else
            {
              if ((*currentEvent)->partB_only)
                {
                  partB_list.push_back (*currentEvent);
                }
              else
                {
                  (*currentEvent)->executeA (cTime);
                  if ((*currentEvent)->partBdelay > 0)
                    {
                      (*currentEvent)->m_nextTime = cTime + (*currentEvent)->partBdelay;
                      (*currentEvent)->partB_turn = true;
                    }
                  else
                    {
                      partB_list.push_back (*currentEvent);
                    }
                }

            }
        }
      else
        {
          eret = (*currentEvent)->execute (cTime + timeTols);
          if (eret > ret)
            {
              ret = eret;
            }
          if ((*currentEvent)->m_remove_event)
            {
			  remove_events = true;
              
            }
        }


      if (nextEvent == events.end ())
        {
          break;
        }
    }
  if (remove_events)
  {
	  nextEvent = events.begin();
	  while ((*nextEvent)->m_nextTime <= cTime + timeTols)
	  {
		  if (((*nextEvent)->m_remove_event))
		  {
			  nextEvent = events.erase(nextEvent);
		  }
		  else
		  {
			  ++nextEvent;
		  }
		  if (nextEvent == events.end())
		  {
			  break;
		  }
		  
	  }
  }
  return ret;
}


change_code eventQueue::executeEventsBonly (double cTime)
{

  auto ret = change_code::no_change;
  auto eret = change_code::no_change;

  for (auto &currentEvent : partB_list)
    {
      eret = currentEvent->execute (cTime + timeTols);
      if (eret > ret)
        {
          ret = eret;
        }
    }
  partB_list.clear ();
  events.sort (compareEventAdapters);
  return ret;
}

void eventQueue::recheck ()
{
  for (auto &ev : events)
    {
      ev->updateTime ();
    }
  events.sort (compareEventAdapters);
}

void eventQueue::remove (std::uint64_t eventID)
{
  auto nextEvent = events.begin ();
  auto endEvent = events.end ();
  while (nextEvent != endEvent)
    {
      if (eventID == (*nextEvent)->eventID)
        {
          events.erase (nextEvent);
          return;
        }
      ++nextEvent;
    }
}

void eventQueue::sort ()
{
  events.sort (compareEventAdapters);
}

void eventQueue::checkDuplicates ()
{ //checking for duplicated gridCoreObject updates which could potentially be bad

  auto currentEvent = events.begin ();
  auto nextEvent = currentEvent;
  ++nextEvent;  //for some reason C++ doesn't let me add 1 to currentEvent
  while (nextEvent != events.end ())
    {
      if ((*currentEvent)->m_nextTime == (*nextEvent)->m_nextTime)
        {
          if (typeid(*currentEvent) == typeid(*nextEvent))
            {
              auto ap = dynamic_cast<eventTypeAdapter<gridCoreObject> *> ((*currentEvent).get ());
              if (ap)
                {
                  auto bp = dynamic_cast<eventTypeAdapter<gridCoreObject> *> ((*nextEvent).get ());
                  if (bp)
                    {
                      if (ap->getTarget()->getID () == bp->getTarget()->getID ())
                        {
                          events.erase (nextEvent);
                          break;
                        }
                    }
                }

            }
        }
      currentEvent = nextEvent;
      ++nextEvent;

    }
}



void eventQueue::set (const std::string &param, double val)
{
  if (param == "timetol")
    {
      if (val > 0)
        {
          timeTols = val;
        }
      else
        {
		  throw(invalidParameterValue());
        }
    }
  else if (param == "nulleventperiod")
    {
      nullEvent->m_period = val;
    }
  else if (param == "nulleventtime")
    {
      nullEvent->m_nextTime = val;
      sort ();
    }
  else
    {
	  throw(unrecognizedParameter());
    }
}




