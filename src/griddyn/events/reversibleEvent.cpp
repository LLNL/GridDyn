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

#include "reversibleEvent.h"
#include "measurement/gridGrabbers.h"
#include "core/helperTemplates.hpp"

namespace griddyn
{
namespace events
{
reversibleEvent::reversibleEvent(const std::string &eventName):Event(eventName)
{

}
reversibleEvent::reversibleEvent(coreTime time0):Event(time0)
{

}
reversibleEvent::reversibleEvent(EventInfo &gdEI, coreObject *rootObject):Event(gdEI,rootObject)
{
	ggrab = createGrabber(field, m_obj);
	ggrab->outputUnits = unitType;
	canUndo = ggrab->loaded;

	
}

void reversibleEvent::updateEvent(EventInfo &gdEI, coreObject *rootObject)
{
	Event::updateEvent(gdEI, rootObject);
	ggrab = createGrabber(field, m_obj);
	ggrab->outputUnits = unitType;
	canUndo = ggrab->loaded;
}

reversibleEvent::~reversibleEvent() = default;

std::shared_ptr<Event> reversibleEvent::clone(std::shared_ptr<Event> gE) const
{
	auto re = cloneBase<reversibleEvent, Event>(this, gE);
	if (!re)
	{
		return gE;
	}
	re->ggrab = createGrabber(field, m_obj);
	re->ggrab->outputUnits = ggrab->outputUnits;
	re->canUndo = canUndo;
	return gE;
}

//virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject) override;
change_code reversibleEvent::trigger()
{
	if (canUndo)
	{
		undoValue= ggrab->grabData();
		hasUndo = true;
	}
	return Event::trigger();
}

change_code reversibleEvent::trigger(coreTime time)
{
	if (canUndo)
	{
		undoValue = ggrab->grabData();
		hasUndo = true;
	}
	return Event::trigger(time);
}

bool reversibleEvent::setTarget(coreObject *gdo, const std::string &var)
{
	auto res=Event::setTarget(gdo, var);
	if (ggrab)
	{
		ggrab->updateObject(m_obj);
		ggrab->updateField(field);
		canUndo = ggrab->loaded;
	}
	else
	{
		ggrab = createGrabber(field, m_obj);
		ggrab->outputUnits = unitType;
		canUndo = ggrab->loaded;
	}
	return res;
}

void reversibleEvent::updateObject(coreObject *gco, object_update_mode mode)
{
	Event::updateObject(gco, mode);
	if (ggrab)
	{
		ggrab->updateObject(gco, mode);
	}
}

change_code reversibleEvent::undo()
{
	if (hasUndo)
	{
		setValue(undoValue);
		hasUndo = false;
		return Event::trigger();
		
	}
	return change_code::not_triggered;
}

double reversibleEvent::query()
{
	return (ggrab)?(ggrab->grabData()):kNullVal;
}

}//namespace events
}//namespace griddyn
