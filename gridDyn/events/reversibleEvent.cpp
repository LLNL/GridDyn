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

#include "reversibleEvent.h"
#include "measurement/gridGrabbers.h"
#include "core/helperTemplates.h"

reversibleEvent::reversibleEvent(const std::string &newName):gridEvent(newName)
{

}
reversibleEvent::reversibleEvent(coreTime time0):gridEvent(time0)
{

}
reversibleEvent::reversibleEvent(gridEventInfo &gdEI, coreObject *rootObject):gridEvent(gdEI,rootObject)
{
	ggrab = createGrabber(field, m_obj);
	ggrab->outputUnits = unitType;
	canUndo = ggrab->loaded;

	
}

void reversibleEvent::updateEvent(gridEventInfo &gdEI, coreObject *rootObject)
{
	gridEvent::updateEvent(gdEI, rootObject);
	ggrab = createGrabber(field, m_obj);
	ggrab->outputUnits = unitType;
	canUndo = ggrab->loaded;
}

reversibleEvent::~reversibleEvent() = default;

std::shared_ptr<gridEvent> reversibleEvent::clone(std::shared_ptr<gridEvent> gE) const
{
	auto re = cloneBase<reversibleEvent, gridEvent>(this, gE);
	if (!re)
	{
		return gE;
	}
	re->ggrab = createGrabber(field, m_obj);
	re->ggrab->outputUnits = ggrab->outputUnits;
	re->canUndo = canUndo;
	return gE;
}

//virtual void updateEvent(gridEventInfo &gdEI, coreObject *rootObject) override;
change_code reversibleEvent::trigger()
{
	if (canUndo)
	{
		undoValue= ggrab->grabData();
		hasUndo = true;
	}
	return gridEvent::trigger();
}

change_code reversibleEvent::trigger(coreTime time)
{
	if (canUndo)
	{
		undoValue = ggrab->grabData();
		hasUndo = true;
	}
	return gridEvent::trigger(time);
}

bool reversibleEvent::setTarget(coreObject *gdo, const std::string &var)
{
	auto res=gridEvent::setTarget(gdo, var);
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
	gridEvent::updateObject(gco, mode);
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
		return gridEvent::trigger();
		
	}
	return change_code::not_triggered;
}

double reversibleEvent::query()
{
	return (ggrab)?(ggrab->grabData()):kNullVal;
}