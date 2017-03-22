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

#include "fmiEvent.h"
#include "fmiCoordinator.h"
#include "core/helperTemplates.h"

fmiEvent::fmiEvent(const std::string &newName, fmiEventType type):reversibleEvent(newName), eventType(type)
{

}

fmiEvent::fmiEvent(fmiEventType type): eventType(type)
{

}

fmiEvent::fmiEvent(gridEventInfo &gdEI, coreObject *rootObject):reversibleEvent(gdEI,rootObject)
{
	findCoordinator();
}

std::shared_ptr<gridEvent> fmiEvent::clone(std::shared_ptr<gridEvent> gE) const
{
	auto gp = cloneBaseStack<fmiEvent, reversibleEvent,gridEvent>(this, gE);
	if (!gp)
	{
		return gE;
	}
	//gp->valueref = valueref;
	return gp;
}

void fmiEvent::set(const std::string &param, double val)
{
	if ((param == "vr") || (param == "valuereference"))
	{
		//valueref = static_cast<unsigned int>(val);
	}
	else
	{
		gridEvent::set(param, val);
	}
}

void fmiEvent::set(const std::string &param, const std::string &val)
{
	reversibleEvent::set(param, val);
}

void fmiEvent::updateEvent(gridEventInfo &gdEI, coreObject *rootObject)
{
	reversibleEvent::updateEvent(gdEI, rootObject);
	findCoordinator();
}

bool fmiEvent::setTarget(coreObject *gdo, const std::string &var)
{
	auto ret=reversibleEvent::setTarget(gdo, var);
	if (ret)
	{
		findCoordinator();
	}
	return ret;
	
}


coreObject *fmiEvent::getOwner() const
{
	return coord;
}

void fmiEvent::updateObject(coreObject *gco, object_update_mode mode)
{
	reversibleEvent::updateObject(gco, mode);
	findCoordinator();
}

void fmiEvent::findCoordinator()
{
	if (m_obj)
	{
		auto rto = m_obj->getRoot();
		if (rto)
		{
			auto fmiCont = rto->find("fmiCoordinator");
			if (dynamic_cast<fmiCoordinator *>(fmiCont))
			{
				if (!isSameObject(fmiCont,coord))
				{
					coord = static_cast<fmiCoordinator *>(fmiCont);
					if (eventType == fmiEventType::input)
					{
						coord->registerInput(getName(), this);
					}
					else
					{
						coord->registerParameter(getName(), this);
					}
				}
			}

		}
		 
	}
	}