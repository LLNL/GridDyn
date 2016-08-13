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

#ifndef EVENTADAPTERS_H_
#define EVENTADAPTERS_H_

#include "basicDefs.h"
#include "gridCore.h"

#include "eventInterface.h"

#include <functional>
#include <memory>
#include <cstdint>

/** @brief class for managing events of many types
 class is a wrapper around a number of different kinds of discrete events
*/
class eventAdapter
{
public:
  index_t eventID;                 //!< eventID for searching
  double m_period;                 //!< the period of the event;
  double m_nextTime;              //!< the next time the event is scheduled to execute
  double partBdelay = 0;              //!<the delay between the first and second parts
  bool m_remove_event = false;              //!<flag to remove the event after execution
  bool two_part_execute = false;              //!< flag if the event has two parts
  bool partB_turn = false;                      //!< if we need to execute the second part only
  bool partB_only = false;                 //!<flag indicating we only run when partB runs
private:
  static count_t eventCounter;  //!< counter for generating the unique event ID
public:
  /** @brief constuctor
  @param[in] nextTime the trigger time for the event
  @param[in] the period of the event  (the event will trigger once per period starting at nextTime
  */
  eventAdapter (double nextTime = kBigNum, double period = 0);

  /** @brief destructor*/
  virtual ~eventAdapter ();

  /** Execute the pre event portion of the event for two part execution events
  @param[in] cTime the current execution time
  */
  virtual void executeA (double cTime);

  /** Execute the event or partB of the event
  @param[in] cTime the current execution time
  */
  virtual change_code execute (double cTime);

  /** @brief update the next event time*/
  virtual void updateTime ();

};

bool compareEventAdapters (const std::shared_ptr<eventAdapter> e1, const std::shared_ptr<eventAdapter> e2);



template<class Y>
class eventTypeAdapter : public eventAdapter
{
  static_assert (std::is_base_of<eventInterface, Y>::value, "classes must be inherited from eventInterface");
public:
  Y *m_eventObj;
  eventTypeAdapter (Y *ge) : m_eventObj (ge)
  {
    m_nextTime = ge->nextTriggerTime ();
    switch (ge->executionMode ())
      {
      case event_execution_mode::normal:
        break;
      case event_execution_mode::two_part_execution:
        two_part_execute = true;
        break;
      case event_execution_mode::delayed:
        two_part_execute = true;
        partB_only = true;
        break;
      }
  }

  change_code execute (double cTime)
  {
    change_code retval = change_code::not_triggered;              //EVENT_NOT_TRIGGERED
    while (m_nextTime <= cTime)
      {
        auto ret = m_eventObj->trigger (cTime);
        retval = std::max (ret, retval);
        if (!(m_eventObj->isArmed ()))
          {
            m_remove_event = true;
            break;
          }
        m_nextTime = m_eventObj->nextTriggerTime ();
      }
    return retval;
  }
  void updateTime ()
  {
    m_nextTime = m_eventObj->nextTriggerTime ();
  }
};


template<class Y>
class eventTypeAdapter<std::shared_ptr<Y> > : public eventAdapter
{
  static_assert (std::is_base_of<eventInterface, Y>::value, "classes must be inherited from eventInterface");
public:
  std::shared_ptr<Y> m_eventObj;
  eventTypeAdapter (std::shared_ptr<Y> ge) : m_eventObj (ge)
  {
    m_nextTime = ge->nextTriggerTime ();
    switch (ge->executionMode ())
      {
      case event_execution_mode::normal:
        break;
      case event_execution_mode::two_part_execution:
        two_part_execute = true;
        break;
      case event_execution_mode::delayed:
        two_part_execute = true;
        partB_only = true;
        break;
      }
  }

  change_code execute (double cTime) override
  {
    change_code retval = change_code::not_triggered;                          //EVENT_NOT_TRIGGERED
    while (m_nextTime <= cTime)
      {
        auto ret = m_eventObj->trigger (cTime);
        retval = std::max (ret, retval);
        if (!(m_eventObj->isArmed ()))
          {
            m_remove_event = true;
            break;
          }
        m_nextTime = m_eventObj->nextTriggerTime ();
      }
    return retval;
  }

  void updateTime () override
  {
    m_nextTime = m_eventObj->nextTriggerTime ();
  }
};

class gridCoreObject;

template<>
class eventTypeAdapter<gridCoreObject> : public eventAdapter
{
public:
  gridCoreObject *targetObject;
  eventTypeAdapter (gridCoreObject *gco)
  {
    targetObject = gco;
    m_nextTime = targetObject->getNextUpdateTime ();
    two_part_execute = true;
  }
  ~eventTypeAdapter ()
  {
  }
  void executeA (double cTime) override
  {
    targetObject->updateA (cTime);
  }

  change_code execute (double cTime) override
  {
    targetObject->updateB ();
    double ttime = targetObject->getNextUpdateTime ();
    if ((ttime <= cTime) && (ttime <= m_nextTime))
      {
        m_nextTime = kBigNum;
      }
    else
      {
        m_nextTime = ttime;
      }
    return change_code::parameter_change;
  }
  void updateTime () override
  {
    m_nextTime = targetObject->getNextUpdateTime ();
  }
};

class functionEventAdapter : public eventAdapter
{
private:
  std::function<change_code ()> fptr;            //!< the function to execute
public:
  functionEventAdapter (std::function<change_code ()> fcal) : fptr (fcal)
  {
  }
  functionEventAdapter (std::function<change_code ()> fcal, double period, double startTime = 0.0) : eventAdapter (startTime, period), fptr (fcal)
  {
  }
  change_code execute (double cTime);

  void setfunction (std::function<change_code ()> nfptr)
  {
    fptr = nfptr;
  }
};


#endif

