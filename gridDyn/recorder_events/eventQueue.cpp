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
#include "gridRecorder.h"

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

void eventQueue::nullEventTime (double time, double period)
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
                  events.erase (currentEvent);
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
              events.erase (currentEvent);
            }
        }


      if (nextEvent == events.end ())
        {
          break;
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

int eventQueue::remove (std::uint64_t eventID)
{
  auto nextEvent = events.begin ();
  auto endEvent = events.end ();
  while (nextEvent != endEvent)
    {
      if (eventID == (*nextEvent)->eventID)
        {
          events.erase (nextEvent);
          return OBJECT_REMOVE_SUCCESS;
        }
      ++nextEvent;
    }
  return OBJECT_REMOVE_FAILURE;
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
                      if (ap->targetObject->getID () == bp->targetObject->getID ())
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



int eventQueue::set (const std::string &param, double val)
{
  int out = PARAMETER_FOUND;
  if (param == "timetol")
    {
      if (val > 0)
        {
          timeTols = val;
        }
      else
        {
          out = INVALID_PARAMETER_VALUE;
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
      out = PARAMETER_NOT_FOUND;
    }
  return out;
}




