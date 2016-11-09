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

#include "gridCore.h"
#include "core/helperTemplates.h"

#include "eventInterface.h"
#include "core/objectOperatorInterface.h"
#include "objectInterpreter.h"
#include <functional>
#include <memory>
#include <algorithm>
#include <algorithm>


/** @brief class for managing events of many types
 class is a wrapper around a number of different kinds of discrete events
*/
class eventAdapter
{
public:
  index_t eventID;                 //!< eventID for searching
  bool m_remove_event = false;              //!<flag to remove the event after execution
  bool two_part_execute = false;              //!< flag if the event has two parts
  bool partB_turn = false;                      //!< if we need to execute the second part only
  bool partB_only = false;                 //!<flag indicating we only run when partB runs
  double m_period;                 //!< the period of the event;
  double m_nextTime;              //!< the next time the event is scheduled to execute
  double partBdelay = 0;              //!<the delay between the first and second parts
private:
  static std::atomic<count_t> eventCounter;  //!< counter for generating the unique event ID
public:
  /** @brief constructor
  @param[in] nextTime the trigger time for the event
  @param[in] period the period of the event  (the event will trigger once per period starting at nextTime
  */
  eventAdapter (double nextTime = kBigNum, double period = 0);

  /** @brief destructor*/
  virtual ~eventAdapter ();

  /** make a copy of the eventAdapter 
  @param[in] eA  the pointer to copy the event adapter information to
  */
  virtual std::shared_ptr<eventAdapter> clone(std::shared_ptr<eventAdapter> eA = nullptr) const;

  /** Execute the pre-event portion of the event for two part execution events
  @param[in] cTime the current execution time
  */
  virtual void executeA (double cTime);

  /** Execute the event or partB of the event
  @param[in] cTime the current execution time
  */
  virtual change_code execute (double cTime);

  /** @brief update the next event time*/
  virtual void updateTime ();
  /** @brief update the target coreObject
  @param[in] newObject the new object
  @param[in] mode update_mode direct or match
  */
  virtual void updateObject(gridCoreObject *newObject, object_update_mode mode = object_update_mode::direct);

};

bool compareEventAdapters (const std::shared_ptr<eventAdapter> e1, const std::shared_ptr<eventAdapter> e2);



template<class Y>
class eventTypeAdapter : public eventAdapter
{
  static_assert (std::is_base_of<eventInterface, Y>::value, "classes must be inherited from eventInterface");
public:
  Y *m_eventObj;

  eventTypeAdapter()
  {

  }

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
	  default:
		  break;
      }
  }

  virtual std::shared_ptr<eventAdapter> clone(std::shared_ptr<eventAdapter> eA = nullptr) const override
  {
	  auto newAdapter = cloneBase<eventTypeAdapter<Y>, eventAdapter>(this, eA);
	  if (!newAdapter)
	  {
		  return eA;
	  }
	  newAdapter->m_eventObj = m_eventObj->clone();
	  return newAdapter;
  }

  virtual void updateObject(gridCoreObject * newObject, object_update_mode mode) override
  {
	  m_eventObj->updateObject(newObject, mode);
  }

  change_code execute (double cTime) override
  {
    change_code retval = change_code::not_triggered;              //EVENT_NOT_TRIGGERED
    while (m_nextTime <= cTime)
      {
        auto ret = m_eventObj->trigger (cTime);
        retval = (std::max)(ret, retval);
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


template<class Y>
class eventTypeAdapter<std::shared_ptr<Y> > : public eventAdapter
{
  static_assert (std::is_base_of<eventInterface, Y>::value, "classes must be inherited from eventInterface");
public:
  std::shared_ptr<Y> m_eventObj;
  eventTypeAdapter()
  {

  }
  explicit eventTypeAdapter (std::shared_ptr<Y> ge) : m_eventObj (ge)
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
	  default:
		  break;
      }
  }

  std::shared_ptr<eventAdapter> clone(std::shared_ptr<eventAdapter> eA = nullptr) const override
  {
	  auto newAdapter = cloneBase<eventTypeAdapter<std::shared_ptr<Y> >, eventAdapter>(this, eA);
	  if (!newAdapter)
	  {
		  return eA;
	  }
	  newAdapter->m_eventObj = std::static_pointer_cast<Y>(m_eventObj->clone());
	  return newAdapter;
  }

  virtual void updateObject(gridCoreObject * newObject, object_update_mode mode) override
  {
	  m_eventObj->updateObject(newObject, mode);
  }

  change_code execute (double cTime) override
  {
    change_code retval = change_code::not_triggered;                          //EVENT_NOT_TRIGGERED
    while (m_nextTime <= cTime)
      {
        auto ret = m_eventObj->trigger (cTime);
        retval = (std::max) (ret, retval);
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
private:
	gridCoreObject *targetObject;
public:
  eventTypeAdapter (gridCoreObject *gco=nullptr):targetObject(gco)
  {
    m_nextTime = targetObject->getNextUpdateTime ();
    two_part_execute = true;
  }
  ~eventTypeAdapter ()
  {
  }

  virtual void updateObject(gridCoreObject *newObject, object_update_mode mode) override
  {
	  if (mode == object_update_mode::direct)
	  {
		  targetObject = newObject;
		  
	  }
	  else if (mode==object_update_mode::match)
	  {
		  auto searchRes = findMatchingObject(targetObject, newObject);
		  if (searchRes)
		  {
			  targetObject = searchRes;
		  }
		  else
		  {
			  throw(objectUpdateFailException());
		  }
	  }
	  m_nextTime = (targetObject) ? targetObject->getNextUpdateTime() : kBigNum;
  }

  virtual std::shared_ptr<eventAdapter> clone(std::shared_ptr<eventAdapter> eA = nullptr) const override
  {
	  auto newAdapter = cloneBase<eventTypeAdapter<gridCoreObject>, eventAdapter>(this, eA);
	  if (!newAdapter)
	  {
		  return eA;
	  }
	  newAdapter->targetObject = targetObject;
	  return newAdapter;
  }

  virtual void executeA (double cTime) override
  {
    targetObject->updateA (cTime);
  }

  virtual change_code execute (double cTime) override
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

  virtual void updateTime () override
  {
    m_nextTime = targetObject->getNextUpdateTime ();
  }


  gridCoreObject* getTarget() const
  {
	  return targetObject;
  }
};
/** eventAdapter with a custom function call
 */
class functionEventAdapter : public eventAdapter
{
private:
  std::function<change_code ()> fptr;            //!< the function to execute
public:
	functionEventAdapter()
	{

	}
  functionEventAdapter (std::function<change_code ()> fcal) : fptr (fcal)
  {
  }
  functionEventAdapter (std::function<change_code ()> fcal, double triggerTime, double period = 0.0) : eventAdapter (triggerTime, period), fptr (fcal)
  {
  }

  std::shared_ptr<eventAdapter> clone(std::shared_ptr<eventAdapter> eA = nullptr) const override
  {
	  auto newAdapter = cloneBase<functionEventAdapter, eventAdapter>(this, eA);
	  if (!newAdapter)
	  {
		  return eA;
	  }
	  //functions may not be amenable to cloning or duplication so we can't really clone here.  
	  return newAdapter;
  }
  virtual change_code execute (double cTime) override;

/** @brief set the function of the event adapter
 *@param[in] nfptr  a std::function which returns a change code and takes 0 arguments*/
  void setfunction (std::function<change_code ()> nfptr)
  {
    fptr = nfptr;
  }
};


#endif

