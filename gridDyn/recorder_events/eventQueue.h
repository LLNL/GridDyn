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

#ifndef EVENTQUEUE_H_
#define EVENTQUEUE_H_

#include "eventAdapters.h"

#include <list>
#include <cstdint>

class gridCoreObject;

/** @brief class implementing a discrete event queue for a continuous time simulation
 the time check on events includes a tolerance to allow for numerical error in the execution of events
the event queue works with event adapters which allow for two part execution of some events including a potential delay between
parts A and B
the class also includes a null event which does nothing but can be called periodically.
*/
class eventQueue
{
protected:
  gridDyn_time timeTols = kSmallTime;  //!< the temporal tolerance on events
  std::list<std::shared_ptr<eventAdapter>> events; //!< storage location for events
  std::vector <std::shared_ptr<eventAdapter>> partB_list;  //!< container for immediate events awaiting part B execution
  std::shared_ptr<eventAdapter> nullEvent; //!< nullEvent operation for scheduling of the null event
public:
  /** @brief constructor*/
  eventQueue ();

  /** @brief virtual destructor*/
  virtual ~eventQueue ()
  {
  }
  /** @brief insert an event into the queue
   take as an input some object that implements an event interface and makes an eventAdapter out of it
  @tparam X X is a subclass of an eventInterface object
  @arg newEventObject  a pointer to the event object being inserted
  @return the event ID of the newly created event adapter
  */
  template< class X>
  index_t insert (X *newEventObject)
  {
    auto ev = std::make_shared < eventTypeAdapter < X >> (newEventObject);
    events.push_back (ev);
    events.sort (compareEventAdapters);
    checkDuplicates ();
    return ev->eventID;
  }

  /** @brief insert an event into the queue
   take as an input a shared pointer to an object that implements an event interface and makes an eventAdapter out of it
  @tparam X X is a subclass of an eventInterface object
  @arg newEventObject  a pointer to the event object being inserted
  @return the event ID of the newly created event adapter
  */
  template< class X>
  index_t insert (std::shared_ptr<X> newEventObject)
  {
    auto ev = std::make_shared < eventTypeAdapter < std::shared_ptr<X> >> (newEventObject);
    events.push_back (ev);
    events.sort (compareEventAdapters);
    checkDuplicates ();
    return ev->eventID;
  }

  /** @brief insert an eventAdapter into the queue
   take as an input a shared pointer to an object that implements an event interface and makes an eventAdapter out of it
  @arg newEvent  a shared pointer to the eventAdapter object
  @return the event ID of the event adapter
  */
  index_t insert (std::shared_ptr<eventAdapter> newEvent)
  {
    events.push_back (newEvent);
    events.sort (compareEventAdapters);
    checkDuplicates ();
    return newEvent->eventID;
  }

  /** @brief get the next event time
        @return the next Event time
        */
  virtual gridDyn_time getNextTime () const;

  /** @brief clone the entire queue to a new queue
  @param[in] eQ the eventQueue to clone to
  @return a shared_ptr to the updated Queue
  */
  virtual std::shared_ptr<eventQueue> clone(std::shared_ptr<eventQueue> eQ = nullptr) const;
  /** @brief map all objects used in the events to a new root object
 */
  virtual void mapObjectsOnto(gridCoreObject *newRootObject);
  /** @brief Execute the events up to the given time
  @param[in] cTime the current Time
  @return code describing the effect of the executed events
  */
  virtual change_code executeEvents (gridDyn_time cTime);

  /** @brief Execute the first part of the events only
  @param[in] cTime the current Time
  @return code describing the effect of the executed events
  */
  virtual change_code executeEventsAonly (gridDyn_time cTime);

  /** @brief Execute second portion of any events where the A portion (could be skipped) was executed by a call to
  execute A Events only
  @return code describing the effect of the executed events
  */
  virtual change_code executeEventsBonly ();

  /** @brief sort the event Queue by time */
  virtual void sort ();

  /** @brief remove an event
  @param[in] eventID the id of the event to remove
  @return OBJECT_REMOVE_SUCCESS if the event is successfully removed
  */
  virtual void remove (std::uint64_t eventID);

  /** @brief recheck all the time of the events for events that may have changed times and resort if required*/
  virtual void recheck ();

  /** @brief check for duplicate events and remove the duplicate
   this is important for removing duplicate gridCoreObject events so we don't have two of those being executed
  which could cause all sorts of issues with the simulation
  */
  virtual void checkDuplicates ();

  /** @brief set parameters for the queue
  @param[in] param a string representing the parameter to set
  @param[in] val the value to set the parameter to
  @return PARAMETER_FOUND if the given parameter was set valid
  */
  virtual void set (const std::string &param, double val);

  /** @brief set the null event time
   the null event is an event that does nothing setting this time is a way to mark events that can't be described by an event Adapter
  @param[in] time the time for the null event
  @param[in] period the period of the null event
  */
  void nullEventTime (gridDyn_time time, gridDyn_time period = negTime);

  /** @brief get the time for the next Null Event*/
  gridDyn_time getNullEventTime () const;
};



#endif
