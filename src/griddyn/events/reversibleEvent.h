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
class reversibleEvent :public Event
{
protected:
	double undoValue = kNullVal;
	std::unique_ptr<gridGrabber> ggrab;
	bool canUndo = false;
	bool hasUndo = false;
public:
	explicit reversibleEvent(const std::string &eventName);
	explicit reversibleEvent(coreTime time0 = 0.0);
	reversibleEvent(EventInfo &gdEI, coreObject *rootObject);
	virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject) override;
	virtual ~reversibleEvent();
	virtual std::shared_ptr<Event> clone(std::shared_ptr<Event> gE = nullptr) const override;

	//virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject) override;
	virtual change_code trigger() override;
	virtual change_code trigger(coreTime time) override;

	virtual bool setTarget(coreObject *gdo, const std::string &var = "") override;

	virtual void updateObject(coreObject *gco, object_update_mode mode = object_update_mode::direct) override;
	virtual change_code undo();
	virtual double query();

};
}//namespace events
}//namespace griddyn
#endif