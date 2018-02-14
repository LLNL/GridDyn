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

#include "reversibleEvent.h"
#include "measurement/gridGrabbers.h"


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
reversibleEvent::reversibleEvent(const EventInfo &gdEI, coreObject *rootObject):Event(gdEI,rootObject)
{
	ggrab = createGrabber(field, m_obj);
	ggrab->outputUnits = unitType;
	canUndo = ggrab->loaded;

	
}

void reversibleEvent::updateEvent(const EventInfo &gdEI, coreObject *rootObject)
{
	Event::updateEvent(gdEI, rootObject);
	ggrab = createGrabber(field, m_obj);
	ggrab->outputUnits = unitType;
	canUndo = ggrab->loaded;
}

reversibleEvent::~reversibleEvent() = default;

std::unique_ptr<Event> reversibleEvent::clone() const
{
	std::unique_ptr<Event> upE = std::make_unique<reversibleEvent>(getName());
	cloneTo(upE.get());
	return upE;
}

void reversibleEvent::cloneTo(Event *gE) const
{
	Event::cloneTo(gE);
	auto nE = dynamic_cast<reversibleEvent *>(gE);
	if (nE == nullptr)
	{
		return;
	}
	nE->ggrab = createGrabber(field, m_obj);
	nE->ggrab->outputUnits = ggrab->outputUnits;
	nE->canUndo = canUndo;
	
}


//virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject) override;
change_code reversibleEvent::trigger()
{
	if (canUndo)
	{
		undoValue= ggrab->grabData();
		hasUndo = true;
	}
    if (stringEvent)
    {
        if (m_obj == nullptr)
        {
            armed = false;
            return change_code::execution_failure;
        }
        try
        {
            m_obj->set(field, newStringValue);
            return change_code::parameter_change;
        }
        catch (const std::invalid_argument &)
        {
            return change_code::execution_failure;
        }

    }
    else
    {
        return Event::trigger();
    }
	
}

change_code reversibleEvent::trigger(coreTime time)
{
	if (canUndo)
	{
		undoValue = ggrab->grabData();
		hasUndo = true;
	}
    if (stringEvent)
    {
        change_code ret = change_code::not_triggered;
        if (time >= triggerTime)
        {
            if (m_obj == nullptr)
            {
                armed = false;
                return change_code::execution_failure;
            }
            try
            {
                m_obj->set(field, newStringValue);
                ret = change_code::parameter_change;
            }
            catch (const std::invalid_argument &)
            {
                ret = change_code::execution_failure;
            }
            armed = false;
        }
        return ret;
    }
    else
    {
        return Event::trigger(time);
    }
	
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


void reversibleEvent::updateStringValue(const std::string &newStr)
{
    newStringValue = newStr;
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
