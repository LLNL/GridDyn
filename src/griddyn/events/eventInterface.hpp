/*
* LLNS Copyright Start
* Copyright (c) 2014-2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef EVENTINTERFACE_H_
#define EVENTINTERFACE_H_
#pragma once

#include "griddyn/gridDynDefinitions.hpp"

namespace griddyn
{
/** class defining different ways an event can be run*/
enum class event_execution_mode
{
  normal = 0, //!< run immediately
  delayed = 1,	//!< delay the execution until after normal events have executed
  two_part_execution = 2,	//!< event has two parts one running with the normal events and one running with the delayed events
};

/** @brief defining a very basic virtual interface for all objects which work with events and triggers
*/
class eventInterface
{
public:
  virtual ~eventInterface() = default;
  /** get the next trigger time */
  virtual coreTime nextTriggerTime () const = 0;
  /** get the events execution mode
  normal mean it run immediately
  delayed means it should run after other normal events
  two_part_execution means it has two components to it
  */
  virtual event_execution_mode executionMode () const = 0;
  /** run the event if time >triggerTime
  @return a change code corresponding to what the event did
  */
  virtual change_code trigger (coreTime ctime) = 0;
  /** answer the question if the event is ready to triggered*/
  virtual bool isArmed () const
  {
    return true;
  }
  /** do any event initialization that may be required before an event is ready to execute*/
  virtual void initialize()
  {

  }
  /** get an event code refering to the type of event
  @details this is not connected to the typeid or class system but a subset of events
  primarily intended to inform of events that only impact a specific process possibly external
  @return the eventCode*/
  virtual int eventCode() const
  {
	  return 0;
  }
};

}//namespace griddyn
#endif