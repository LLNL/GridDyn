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

#include "gridEvent.h"
#include "gridDyn.h"
#include "units.h"
#include "objectInterpreter.h"
#include "core/factoryTemplates.h"
#include "core/gridDynExceptions.h"
#include "gridDynTypes.h"
#include "stringOps.h"

#include <sstream>

#include <string>

static classFactory<gridEvent> evntFac(std::vector<std::string>{ "event", "simple","single" },"event");
static childClassFactory<gridEvent, gridPlayer> playerFac(std::vector<std::string>{ "player","timeseries","file" });

static childClassFactory<gridEvent, compoundEvent> cmpdEvnt(std::vector<std::string>{ "multi", "compound" });

static childClassFactory<gridEvent, compoundEventPlayer> cmpdEvntPlay(std::vector<std::string>{ "compoundplayer", "multifile", "multiplayer" });

std::atomic<count_t> gridEvent::eventCount(0);



gridEvent::gridEvent(const std::string &objName):name(objName),triggerTime(kBigNum)
{
	eventId=++eventCount;
}

gridEvent::gridEvent (double time0): triggerTime(time0)
{
	eventId=++eventCount;
}

gridEvent::gridEvent(gridEventInfo *gdEI, gridCoreObject *rootObject):description(gdEI->description), name(gdEI->name)
{
	if (!gdEI->value.empty())
	{
		value = gdEI->value[0];
	}
	gridCoreObject *searchObj = rootObject;
	
	if (!gdEI->targetObjs.empty())
	{
		searchObj = gdEI->targetObjs[0];
	}
	if (!gdEI->units.empty())
	{
		unitType = gdEI->units[0];
	}
	if (!gdEI->fieldList.empty())
	{
		loadField(searchObj, gdEI->fieldList[0]);
	}
	else
	{
		m_obj = searchObj;
	}
	triggerTime = gdEI->time[0];
	armed = true;
	
}

void gridEvent::loadField(gridCoreObject *searchObj, const std::string newfield)
{
	objInfo fdata(newfield, searchObj);
	field = fdata.m_field;
	if (fdata.m_unitType != gridUnits::units_t::defUnit)
	{
		unitType = fdata.m_unitType;
	}
	if (fdata.m_obj!=m_obj)
	{
		m_obj = fdata.m_obj;
		name = m_obj->getName();
	}
	
}

std::shared_ptr<gridEvent> gridEvent::clone(std::shared_ptr<gridEvent> ggb) const
{

	auto nE = ggb;
	if (!nE)
	{
		nE = std::make_shared<gridEvent>(triggerTime);
		
	}
  nE->name =  name;
  nE->value = value;
  nE->field = field;
  nE->description = description;
  nE->armed = armed;
  nE->m_obj = m_obj;
  nE->resettable = resettable;
  nE->reversable = reversable;
  nE->unitType = unitType;
 
  return nE;
}

gridEvent::~gridEvent ()
{
}


void gridEvent::set(const std::string &param, double val)
{
	if (param == "value")
	{
		value = val;
	}
	else if (param == "time")
	{
		triggerTime = val;
	}
	else if (param == "armed")
	{
		armed = (val > 0.1);
	}
	else
	{
		throw(unrecognizedParameter());
	}
}

void gridEvent::set(const std::string &param, const std::string &val)
{

	if (param == "field")
	{
		loadField(m_obj, val);
	}
	else if (param == "units")
	{
		auto newUnits = gridUnits::getUnits(val);
		if (newUnits == gridUnits::defUnit)
		{
			throw (invalidParameterValue());
		}
		else
		{
			unitType = newUnits;
		}
	}
	else if (param == "name")
	{
		name = val;
	}
	else if (param == "description")
	{
		description = val;
	}
	else
	{
		throw(unrecognizedParameter());
	}
}

void gridEvent::setTime (double time)
{
  triggerTime = time;
}

void gridEvent::setValue (double val, gridUnits::units_t newUnits)
{
  value = val;
  if (unitType != gridUnits::defUnit)
  {
	  unitType = newUnits;
  }
}


std::string gridEvent::toString()
{
	// @time1 |[rootobj::obj:]field(units) = val1
	std::stringstream ss;
	if (triggerTime >(-kDayLength))
	{
		ss << '@' << triggerTime;
	ss << " | ";
	}
      ss << fullObjectName (m_obj) << ':' << field;
      if (unitType != gridUnits::defUnit)
        {
          ss << '(' << gridUnits::to_string (unitType) << ')';
        }
      ss << " = " << value;
	return ss.str ();
}

change_code gridEvent::trigger ()
{
	if (!m_obj)
	{
		armed = false;
		return change_code::execution_failure;
	}
	try
	{
		m_obj->set(field, value, unitType);
		return change_code::parameter_change;
	}
	catch(const gridDynException &)
	{
		return change_code::execution_failure;
	}

}

change_code gridEvent::trigger (double time)
{
  change_code ret = change_code::not_triggered;
  if (time >= triggerTime)
    {
	  if (!m_obj)
	  {
		  armed = false;
		  return change_code::execution_failure;
	  }
	  try
	  {
		  m_obj->set(field, value, unitType);
		  ret = change_code::parameter_change;
	  }
	  catch (gridDynException &)
	  {
		  ret = change_code::execution_failure;
	  }
	  armed = false;
    }
  return ret;
}

void gridEvent::updateObject(gridCoreObject *gco, object_update_mode mode)
{
	if (mode == object_update_mode::direct)
	{
		setTarget(gco);
	}
	else
	{
		if (m_obj)
		{
			auto newTarget = findMatchingObject(m_obj, gco);
			if (newTarget)
			{
				setTarget(newTarget);
			}
			else
			{
				throw(objectUpdateFailException());
			}		
			
		}
		else
		{
			setTarget(gco);
		}
		
	}
}

gridCoreObject * gridEvent::getObject() const
{
	return m_obj;
}

void gridEvent::getObjects(std::vector<gridCoreObject *> &objects) const
{
	objects.push_back(getObject());
}

bool gridEvent::setTarget ( gridCoreObject *gdo,const std::string &var)
{
	if (gdo)
	{
		m_obj = gdo;
		name = m_obj->getName();
	}
  if (!var.empty ())
    {
	  loadField(m_obj, var);
    }

  if (m_obj)
    {
      armed = true;
    }
  else
  {
	  armed = false;
  }
  return armed;
}

enum class event_types
{
	basic,
	compound,
	player,
	compoundplayer,
	toggle,
	interpolating,
	reversible,
};


event_types findEventType(gridEventInfo *gdEI)
{
	if (!gdEI->type.empty())
	{
		if ((gdEI->type == "basic") || (gdEI->type == "simple"))
		{
			return event_types::basic;
		}
		if (gdEI->type == "player")
		{
			return event_types::player;
		}
		if (gdEI->type == "compound")
		{
			return event_types::compound;
		}
		if (gdEI->type == "compoundplayer")
		{
			return event_types::compoundplayer;
		}
		if (gdEI->type == "toggle")
		{
			return event_types::toggle;
		}
		if (gdEI->type=="reversible")
		{
			return event_types::reversible;
		}
		if (gdEI->type=="interpolating")
		{
			return event_types::interpolating;
		}
	}
	if (!gdEI->file.empty())
	{
		return event_types::player;
	}
	if (gdEI->period > 0)
	{
		return event_types::player;
	}
	if (gdEI->time.size() > 1)
	{
		return event_types::player;
	}
	else if (gdEI->value.size() > 1)
	{
		return event_types::compound;
	}
	else if (gdEI->fieldList.size()>1)
	{
		return event_types::compound;
	}
	else
	{
		return event_types::basic;
	}
}


gridEventInfo::gridEventInfo(const std::string &eventString, gridCoreObject *rootObj)
{
	loadString(eventString, rootObj);
}

// @time1[,time2,time3,... + period] |[rootobj::obj1:]field(units) const = val1,[val2,val3,...];[rootobj::obj1:]field(units) const = val1,[val2,val3,...];  or
// [rootobj::obj:]field(units) = val1,[val2,val3,...] @time1[,time2,time3,...|+ period] or
void gridEventInfo::loadString(const std::string &eventString, gridCoreObject *rootObj)
{

  std::string objString;

  auto posA = eventString.find_first_of ("@");
  if (posA == std::string::npos)
    {
      objString = eventString;
    }
  else
    {
      auto posT = eventString.find_first_of ("|", posA + 2);
      std::string tstring = (posT != std::string::npos) ? eventString.substr (posA + 1, posT - posA - 1) : eventString.substr (posA + 1, std::string::npos);
      trimString (tstring);
      auto cstr = tstring.find_first_of (",");
      if (cstr == std::string::npos)
        {
          cstr = tstring.find_first_of ("+");
          if (cstr == std::string::npos)
            {

              time.push_back (std::stod (tstring));
            }
          else
            {
              time.push_back (std::stod (tstring.substr (0, cstr - 1)));
              period = std::stod (tstring.substr (cstr + 1, std::string::npos));
            }

        }
      else
        {
          time = str2vector (tstring,-1,",");
        }
	  objString = (posA > 2) ? eventString.substr(0, posA - 1) : eventString.substr(posT + 1, std::string::npos);

    }


  trimString (objString);
  auto posE = objString.find_first_of ('=');
  std::string vstring = objString.substr (posE + 1, std::string::npos);
  trimString (vstring);
  objString = objString.substr (0, posE);
  //break down the object specification
  objInfo fdata (objString, rootObj);

  targetObjs.push_back(fdata.m_obj);
  units.push_back(fdata.m_unitType);
  fieldList.push_back(fdata.m_field);

  auto posFile = vstring.find_first_of ('{');
  if (posFile != std::string::npos)
    { //now we get into file based event
	  auto posEndFile = vstring.find_first_of('}', posFile);
      file = vstring.substr (posE + 1,posEndFile-posFile-1);
      
      int col = trailingStringInt (file, file, 0);
	  columns.push_back(col);
	  auto posPlus= vstring.find_first_of('+',posEndFile);
	  if (posPlus != std::string::npos)
	  {
		  period = std::stod(vstring.substr(posPlus + 1, std::string::npos));
	  }
    }
  else
  {
	  auto cstr = vstring.find_first_of(',');
	  if (cstr == std::string::npos)
	  {
		  value.push_back(std::stod(vstring));
	  }
	  else
	  {
		  value = str2vector(vstring, -1, ",");
	  }
  }
}


std::shared_ptr<gridEvent> make_event (const std::string &field, double val, double eventTime, gridCoreObject *rootObject)
{
  auto ev = std::make_shared<gridEvent> (eventTime);
  objInfo fdata (field, rootObject);
  ev->setTarget (fdata.m_obj, fdata.m_field);
  ev->setValue(val, fdata.m_unitType);
  return ev;
}

std::shared_ptr<gridEvent> make_event(const std::string &eventString, gridCoreObject *rootObject)
{
	gridEventInfo gdEI(eventString, rootObject);
	return make_event(&gdEI, rootObject);
}

std::shared_ptr<gridEvent> make_event (gridEventInfo *gdEI, gridCoreObject *rootObject)
{
	std::shared_ptr<gridEvent> ev;
	if (!gdEI->type.empty())
	{
		ev = coreClassFactory<gridEvent>::instance()->createObject(gdEI->type);
		if (ev)
		{
			return ev;
		}
	}
	auto evType = findEventType(gdEI);
	
	switch (evType)
	{
	case event_types::basic:
		ev = std::make_shared<gridEvent>(gdEI, rootObject);
		break;
	case event_types::compound:
		ev = std::make_shared<compoundEvent>(gdEI, rootObject);
		break;
	case event_types::player:
		ev = std::make_shared<gridPlayer>(gdEI, rootObject);
		break;
	case event_types::compoundplayer:
		ev = std::make_shared<compoundEventPlayer>(gdEI, rootObject);
		break;
	case event_types::interpolating:
	case event_types::reversible:
	case event_types::toggle:
		break;
	default:
		break;
	}
 
  return ev;
}
