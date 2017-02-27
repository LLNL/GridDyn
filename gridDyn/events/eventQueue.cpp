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

#include "eventQueue.h"

#include "gridEvent.h"

#include "core/coreObject.h"
#include "measurement/collector.h"
#include "core/coreExceptions.h"

#include <typeinfo>

eventQueue::eventQueue ()
{
  nullEvent = std::make_shared<eventAdapter> ();
  insert (nullEvent);
}
eventQueue::~eventQueue() = default;

coreTime eventQueue::getNextTime () const
{
  return events.front ()->m_nextTime;

}

void eventQueue::nullEventTime (coreTime time, coreTime period)
{

  nullEvent->m_nextTime = time;
  if (period != kNullVal)
    {
      nullEvent->m_period = period;
    }
  sort();
}

coreTime eventQueue::getNullEventTime () const
{
  return nullEvent->m_nextTime;
}


std::unique_ptr<eventQueue> eventQueue::clone(eventQueue *eQ) const
{
	std::unique_ptr<eventQueue> newQueue = (eQ) ? nullptr : std::make_unique<eventQueue>();
	auto cloneQ = (eQ) ? eQ : newQueue.get();
	
	nullEvent->clone(cloneQ->nullEvent);
	for (auto ev : events)
	{
		if (ev == nullEvent) //we dealt with the nullEvent separately
		{
			continue;
		}
		cloneQ->insert(ev->clone());
	}
	cloneQ->timeTols = timeTols;

	return newQueue;
}

void eventQueue::mapObjectsOnto(coreObject *newRootObject)
{
	for (auto ev : events)
	{
		ev->updateObject(newRootObject, object_update_mode::match);
	}
}

change_code eventQueue::executeEvents (coreTime cTime)
{
  if (events.front ()->m_nextTime > cTime + timeTols)
    {
      return change_code::no_change;
    }
  auto ret = change_code::no_change;
  auto eret = change_code::no_change;
  if (!partB_list.empty ())
    {
      ret = executeEventsBonly ();
    }
  eret = executeEventsAonly (cTime);
  if (eret > ret)
    {
      ret = eret;
    }
  eret = executeEventsBonly ();
  if (eret > ret)
    {
      ret = eret;
    }

  return ret;
}

change_code eventQueue::executeEventsAonly (coreTime cTime)
{
  if (events.front ()->m_nextTime > cTime + timeTols)
    {
      return change_code::no_change;
    }
  auto ret = change_code::no_change;
  auto eret = change_code::no_change;

  bool remove_events = false;

  std::lock_guard<std::mutex> lock(queuelock);

  auto nextEvent = events.begin ();
  auto currentEvent = nextEvent;
  
  while ((*nextEvent)->m_nextTime <= cTime + timeTols)
    {
      currentEvent = nextEvent;
      ++nextEvent;
      if ((*currentEvent)->two_part_execute)
        {
          if ((*currentEvent)->partB_turn)
            {
              eret = (*currentEvent)->execute ((*currentEvent)->m_nextTime);
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
                  (*currentEvent)->executeA ((*currentEvent)->m_nextTime);
                  if ((*currentEvent)->partBdelay > timeZero)
                    {
                      (*currentEvent)->m_nextTime += (*currentEvent)->partBdelay;
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
          eret = (*currentEvent)->execute ((*currentEvent)->m_nextTime);
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
	  auto it=std::remove_if(events.begin(), nextEvent, [](auto &evnt) {return evnt->m_remove_event; });
	  if (it != nextEvent)
	  {
		  events.erase(it, nextEvent);
	  }
	  
  }
  return ret;
}


change_code eventQueue::executeEventsBonly ()
{

  auto ret = change_code::no_change;
  auto eret = change_code::no_change;
  std::lock_guard<std::mutex> lock(queuelock);
  for (auto &currentEvent : partB_list)
    {
      eret = currentEvent->execute (currentEvent->m_nextTime);
      if (eret > ret)
        {
          ret = eret;
        }
    }
  partB_list.clear ();
  std::sort (events.begin(),events.end(),compareEventAdapters);
  return ret;
}

void eventQueue::recheck ()
{
	std::lock_guard<std::mutex> lock(queuelock);
  for (auto &ev : events)
    {
      ev->updateTime ();
    }
  std::sort(events.begin(),events.end(),compareEventAdapters);
}

void eventQueue::remove (std::uint64_t eventID)
{
	std::lock_guard<std::mutex> lock(queuelock);
	auto rm=std::remove_if(events.begin(), events.end(), [eventID](const auto &evnt) {return (eventID == evnt->eventID); });
	events.erase(rm, events.end());
}

count_t eventQueue::size() const
{
	return static_cast<count_t>(events.size());
}

void eventQueue::sort ()
{
	std::lock_guard<std::mutex> lock(queuelock);
  std::sort (events.begin(),events.end(),compareEventAdapters);
}

void eventQueue::checkDuplicates ()
{ //checking for duplicated coreObject updates which could potentially be bad
	//this function is a private function and should only be called from inside a locked scope
	auto pred = [](const auto &a, const auto &b)->bool
	{
		if (typeid(*a) == typeid(*b))
		{
			auto ap = dynamic_cast<eventTypeAdapter<coreObject> *> (a.get());
			if (ap)
			{
				auto bp = static_cast<eventTypeAdapter<coreObject> *> (b.get());
				if (isSameObject(ap->getTarget(),bp->getTarget()))
				{
					return true;
				}
			}
		}
		return false;
	};
	auto lastEvent=std::unique(events.begin(), events.end(), pred);
	if (lastEvent != events.end())
	{
		events.erase(lastEvent, events.end());
	}
}

void eventQueue::getEventObjects(std::vector<coreObject *> &objV) const
{
	for (auto &ev : events)
	{
		ev->getObjects(objV);
	}
	std::sort(objV.begin(), objV.end());
	auto eq = std::unique(objV.begin(), objV.end());
	objV.erase(eq, objV.end());

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




