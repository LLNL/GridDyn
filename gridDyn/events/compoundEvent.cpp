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

#include "gridEvent.h"
#include "gridDyn.h"
#include "utilities/units.h"
#include "core/objectInterpreter.h"
#include "utilities/stringOps.h"
#include "core/helperTemplates.h"
#include "core/coreExceptions.h"
#include <sstream>

#include <string>


compoundEvent::compoundEvent(coreTime time0) : gridEvent(time0)
{

}


compoundEvent::compoundEvent(const std::string &eventName) : gridEvent(eventName)
{

}

compoundEvent::compoundEvent(gridEventInfo &gdEI, coreObject *rootObject) : gridEvent(gdEI, rootObject)
{

}

std::shared_ptr<gridEvent> compoundEvent::clone(std::shared_ptr<gridEvent> gE) const
{
	auto nE = cloneBase<compoundEvent, gridEvent>(this, gE);
	if (!nE)
	{
		return gE;
	}

	nE->fields = fields;
	nE->values = values;
	nE->units = units;
	nE->targetObjects = targetObjects;

	return nE;
}

void compoundEvent::updateObject(coreObject *gco, object_update_mode mode)
{

	//TODO:  more thinking on exception safety
	if (mode == object_update_mode::direct)
	{
		setTarget(gco);
	}
	else if(mode == object_update_mode::match)
	{
		for (auto &obj : targetObjects)
		{
			if (obj)
			{
				auto tempobj = findMatchingObject(obj, gco);
				if (!tempobj)
				{
					throw(objectUpdateFailException());
				}
				obj = tempobj;
			}
		}
	}
}

coreObject * compoundEvent::getObject() const
{
	return targetObjects[0];
}

void compoundEvent::getObjects(std::vector<coreObject *> &objects) const
{
	for (auto &obj : targetObjects)
	{
		objects.push_back(obj);
	}
}

void compoundEvent::setValue(double val, gridUnits::units_t newUnit)
{
	for (auto &vv : values)
	{
		vv = val;
	}
	for (auto &uu : units)
	{
		uu = newUnit;
	}
}


void compoundEvent::set(const std::string &param, double val)
{
	if (param[0] == '#')
	{
		
	}
	else
	{
		gridEvent::set(param, val);
	}
}

void compoundEvent::set(const std::string &param, const std::string &val)
{
	if (param[0] == '#')
	{
		
	}
	else
	{
		gridEvent::set(param, val);
	}
}


void compoundEvent::setValue(const std::vector<double> &val)
{
	values = val;

}

std::string compoundEvent::toString()
{
	// @time1[,time2,time3,... |+ period] >[rootobj::obj:]field(units) = val1,[val2,val3,...]
	std::stringstream ss;
		ss << '@' << triggerTime << " | ";
		
		ss << fullObjectName(targetObjects[0]) << ':' << fields[0];
		if (units[0] != gridUnits::defUnit)
		{
			ss << '(' << gridUnits::to_string(units[0]) << ')';
		}
		ss << " = " << values[0];
		for (index_t kk = 1; kk < values.size(); ++kk)
		{
			ss << "; "<<fullObjectName(targetObjects[kk]) << ':' << fields[kk];
			if (units[kk] != gridUnits::defUnit)
			{
				ss << '(' << gridUnits::to_string(units[kk]) << ')';
			}
			ss << " = " << values[kk];
		}
	
	return ss.str();
}
change_code compoundEvent::trigger()
{
	try
	{
		m_obj->set(field, value, unitType);
		return change_code::parameter_change;
	}
	catch (const gridDynException &)
	{
		return change_code::execution_failure;
	}
}

change_code compoundEvent::trigger(coreTime time)
{
	change_code ret = change_code::not_triggered;
	if (time >= triggerTime)
	{
		try
		{
			m_obj->set(field, value, unitType);
			ret = change_code::parameter_change;
		}
		catch (const gridDynException &)
		{
			ret = change_code::execution_failure;
		}
	}
	return ret;
}

bool compoundEvent::setTarget(coreObject *gdo, const std::string &var)
{
	if (!var.empty())
	{
		field = var;
	}
	if (gdo)
	{
		m_obj = gdo;
	}
	

	if (m_obj)
	{
		setName(m_obj->getName());
		armed = true;
	}
	else
	{
		armed = false;
	}
	return armed;
}
