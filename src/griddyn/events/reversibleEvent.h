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

#ifndef REVERSIBLE_EVENT_H_
#define REVERSIBLE_EVENT_H_
#pragma once

#include "Event.h"

namespace griddyn
{
class gridGrabber;
namespace events
{
/** an event that allows undoing,  it is a grabber and event rolled into one */
class reversibleEvent : public Event
{
  protected:
    bool canUndo = false;  //!< flag indicating that the event is capable of undoing
    bool hasUndo = false;  //!< flag indicating that the event has an undo value
    bool stringEvent = false;  //!< flag indicating that the event corresponds to a string set
    double undoValue = kNullVal;  //!< the value before the event took place
    std::unique_ptr<gridGrabber> ggrab;  //!< a grabber to get the values
    std::string undoString;  //!< the previous value for a string
    std::string newStringValue;  //!< the new value to set for a string
  public:
    explicit reversibleEvent (const std::string &eventName);
    explicit reversibleEvent (coreTime time0 = 0.0);
    reversibleEvent (const EventInfo &gdEI, coreObject *rootObject);
    virtual void updateEvent (const EventInfo &gdEI, coreObject *rootObject) override;
    virtual ~reversibleEvent ();
    virtual std::unique_ptr<Event> clone () const override;

    virtual void cloneTo (Event *evnt) const override;

    // virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject) override;
    virtual change_code trigger () override;
    virtual change_code trigger (coreTime time) override;

    virtual bool setTarget (coreObject *gdo, const std::string &var = "") override;

    virtual void updateStringValue (const std::string &newStr);

    virtual void updateObject (coreObject *gco, object_update_mode mode = object_update_mode::direct) override;
    virtual change_code undo ();
    virtual double query ();
};
}  // namespace events
}  // namespace griddyn
#endif