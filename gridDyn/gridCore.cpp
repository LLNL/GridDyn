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

#include "gridCore.h"
#include "core/gridDynExceptions.h"  //This needs to be renamed

//set up the global object count

//start at 100 since there are some objects that use low numbers as a check for interface number and the id as secondary
std::atomic<count_t> gridCoreObject::s_obcnt(100);

gridCoreObject::gridCoreObject (const std::string &objName) : name (objName)
{

  m_oid = ++s_obcnt;
  //not using updateName since in many cases the id has not been set yet
  if (name.back () == '#')
    {
      name.pop_back ();
      name += std::to_string (m_oid);
    }
  id = m_oid;

}

gridCoreObject::~gridCoreObject ()
{
}

//inherited copy construction method
gridCoreObject *gridCoreObject::clone (gridCoreObject *obj) const
{
  if (obj == nullptr)
    {
      obj = new gridCoreObject (name);
    }
  obj->enabled = enabled;
  obj->updatePeriod = updatePeriod;
  obj->nextUpdateTime = nextUpdateTime;
  obj->lastUpdateTime = lastUpdateTime;
  obj->prevTime = prevTime;
  obj->description = description;
  obj->id = id;
  return obj;
}

void gridCoreObject::updateName ()
{
  if (name.back () == '$')
    {
      name.pop_back ();
      name += std::to_string (id);
    }
  else if (name.back () == '#')
    {
      name.pop_back ();
      name += std::to_string (m_oid);
    }
}

void gridCoreObject::add (gridCoreObject * /*obj*/)
{
	throw(objectAddFailure(this));
}

void gridCoreObject::remove (gridCoreObject * /*obj*/)
{
	throw(objectRemoveFailure(this));
}

void gridCoreObject::addsp (std::shared_ptr<gridCoreObject> /*obj*/)
{
	throw(objectAddFailure(this));
}

bool gridCoreObject::setOwner (gridCoreObject *currentOwner, gridCoreObject *newOwner)
{
  if (owner)
    {
      if (owner == currentOwner)
        {
          owner = newOwner;
          return true;
        }
    }
  else
    {
      owner = newOwner;
      return true;
    }
  return false;
}

static stringVec locNumStrings {
  "updateperiod", "updaterate","nextupdatetime","basepower","enabled","id"
};
static const stringVec locStrStrings {
  "name", "description"
};

void gridCoreObject::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
  switch (pstype)
    {
    case paramStringType::all:
      pstr.reserve (pstr.size () + locNumStrings.size () + locStrStrings.size () + 1);
      pstr.insert (pstr.end (), locNumStrings.begin (), locNumStrings.end ());
      pstr.push_back ("#");
      pstr.insert (pstr.end (), locStrStrings.begin (), locStrStrings.end ());
      break;
    case paramStringType::localnum:
      pstr = locNumStrings;
      break;
    case paramStringType::localstr:
      pstr = locStrStrings;
      break;
    case paramStringType::numeric:
      pstr.reserve (pstr.size () + locNumStrings.size ());
      pstr.insert (pstr.end (), locNumStrings.begin (), locNumStrings.end ());
      break;
    case paramStringType::string:
      pstr.reserve (pstr.size () + locStrStrings.size ());
      pstr.insert (pstr.end (), locStrStrings.begin (), locStrStrings.end ());
      break;
    case paramStringType::localflags:
      break;
    case paramStringType::flags:
      break;
    }
}


void gridCoreObject::setTime (double time)
{
  prevTime = time;
}

void gridCoreObject::set (const std::string &param,  const std::string &val)
{
 
  if ((param == "name") || (param == "rename")||(param == "id"))
    {
      setName (val);
    }
  else if (param == "description")
    {
      description = val;
    }
  else if (param[0] == '#')
    {

    }
  else
    {
      LOG_DEBUG ("parameter " + param + " not found");
	  throw(unrecognizedParameter());
    }

}

void gridCoreObject::setName (std::string newName)
{
  name = newName;
  if (parent)
    {
      alert (this, OBJECT_NAME_CHANGE);
    }
}


void gridCoreObject::setUserID (index_t newUserID)
{
  id = newUserID;
  if (parent)
    {
      parent->alert (this, OBJECT_ID_CHANGE);
    }
}

void gridCoreObject::setUpdateTime (double newUpdateTime)
{
  nextUpdateTime = newUpdateTime;
}

void gridCoreObject::setParent (gridCoreObject *parentObj)
{
  parent = parentObj;
}

void gridCoreObject::setFlag (const std::string &flag, bool val)
{
 
  if ((flag == "enable")||(flag == "status")||(flag=="enabled"))
    {
      if (enabled != val)
        {
          if (val)
            {
              enable ();
            }
          else
            {
              disable ();
            }
        }
    }
  else if (flag == "disabled")
    {
      if (enabled == val)            //looking for opposites to trigger
        {
          if (val)
            {
              disable ();
            }
          else
            {
              enable ();
            }
        }
    }
  else if (flag == "searchable")
    {
      alert (this, OBJECT_IS_SEARCHABLE);
    }
  else
    {
	  throw(unrecognizedParameter());
    }

}

bool gridCoreObject::getFlag (const std::string &param) const
{
  bool ret = false;
  if (param == "enabled")
    {
      ret = enabled;
    }
  return ret;
}


double gridCoreObject::get (const std::string & param, gridUnits::units_t unitType) const
{
  double val = kNullVal;
  if (param == "period")
    {
      val = gridUnits::unitConversion (updatePeriod, gridUnits::sec, unitType);
    }
  else if (param == "time")
    {
      val = prevTime;
    }
  else if (param == "update")
    {
      val = nextUpdateTime;
    }
  return val;
}

void gridCoreObject::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if ((param == "updateperiod") || (param == "period"))
    {
      updatePeriod = gridUnits::unitConversion (val, unitType, gridUnits::sec);
    }
  else if ((param == "updaterate") || (param == "rate"))
    {
      double rt = gridUnits::unitConversion (val, unitType, gridUnits::Hz);
      if (rt <= 0.0)
        {
          updatePeriod = kBigNum;
        }
      else
        {
          updatePeriod = 1.0 / rt;
        }
    }
  else if (param == "nextupdatetime")
    {
      nextUpdateTime = gridUnits::unitConversion (val, unitType, gridUnits::sec);
    }
  else if ((param == "number") || (param == "renumber") || (param == "id"))
    {
      setUserID (static_cast<index_t> (val));
    }
  else if (param[0] == '#') //the comment parameter
    {

    }
  else
    {
	  setFlag(param, (val > 0.1));
	 
    }

}


std::string gridCoreObject::getString (const std::string &param) const
{
  std::string out ("NA");
  if (param == "name")
    {
      out = name;
    }
  else if (param == "description")
    {
      out = description;
    }
  else if (param == "parent")
    {
      if (parent)
        {
          out = parent->getName ();
        }
    }
  return out;
}

gridCoreObject* gridCoreObject::getSubObject (const std::string & /*typeName*/, index_t /*num*/) const
{
  return nullptr;
}

gridCoreObject * gridCoreObject::findByUserID (const std::string & /*typeName*/, index_t searchID) const
{
  if (searchID == id)
    {
      return const_cast<gridCoreObject *> (this);
    }
  else
    {
      return nullptr;
    }
}

void gridCoreObject::updateA (double time)
{
	prevTime = time;
}

double gridCoreObject::updateB ()
{
	if (nextUpdateTime != kNullVal)
	{
		while (prevTime >= nextUpdateTime)
		{
			nextUpdateTime += updatePeriod;
		}
	}
  return nextUpdateTime;
}

void gridCoreObject::enable ()
{
  enabled = true;
}

void gridCoreObject::disable ()
{
  enabled = false;
}


void gridCoreObject::alert (gridCoreObject *object, int code)
{
  if (parent)
    {
      parent->alert (object,code);
    }
}

void gridCoreObject::log (gridCoreObject *object, print_level level, const std::string &message)
{
  if (parent)
    {
      parent->log (object,level,message);
    }
}


void gridCoreObject::makeNewOID ()
{
  m_oid = ++s_obcnt;
}
//NOTE: there is some potential for recursion here if the parent object searches in lower objects
//But in some cases you search up, and others you want to search down so we will rely on intelligence on the part of the implementer
gridCoreObject* gridCoreObject::find (const std::string &object) const
{
  if (parent)
    {
      return (parent->find (object));
    }
  else
    {
      return nullptr;
    }

}

int gridCoreObject::getInt (const std::string &param) const
{
  return static_cast<int> (get (param));
}


void gridCoreObject::loadPosition (std::shared_ptr<gridPositionInfo> npos)
{
  pos = npos;
}


std::string fullObjectName (gridCoreObject *obj)
{
  if (obj->parent)
    {
      if (obj->parent->parent)
        {
          return fullObjectName (obj->parent) + "::" + obj->getName ();               //yay recursion
        }
      else
        {
          return obj->getName ();               //the objective is to be searchable from the root object so don't need to list the root object name
        }
    }
  else
    {
      return obj->getName ();
    }

}

bool compareUpdates (gridCoreObject *o1, gridCoreObject *o2)
{
  return ((o1->nextUpdateTime < o2->nextUpdateTime) ? true : false);
}

bool compareNames (gridCoreObject *o1, gridCoreObject *o2)
{
  return ((o1->name < o2->name) ? true : false);
}

void condDelete (gridCoreObject *obj, gridCoreObject *Pobject)
{
  if (obj)
    {
      if (obj->owner)
        {
          if (obj->owner->m_oid == Pobject->m_oid)
            {
              if (obj->parent)
                {
                  obj->parent->remove (obj);
                }
              delete obj;
            }
          else if (obj->parent->m_oid == Pobject->m_oid)
            {
              obj->parent = nullptr;
            }
        }
      else if (obj->parent)
        {
          if (obj->parent->m_oid == Pobject->m_oid)
            {
              delete obj;
            }
        }
      else
        {
          delete obj;
        }
    }
}

#include <map>

static const std::map<std::string, print_level> printLevelsMap
{
	{"none",print_level::no_print},
	{ "error",print_level::error },
	{ "warning",print_level::warning },
	{ "normal",print_level::normal },
	{ "summary",print_level::summary },
	{ "debug",print_level::debug },
	{ "trace",print_level::trace },
	{ "no_print",print_level::no_print },
	{ "error_print",print_level::error },
	{ "warning_print",print_level::warning },
	{ "normal_print",print_level::normal },
	{ "summary_print",print_level::summary },
	{ "debug_print",print_level::debug },
	{ "trace_print",print_level::trace },
};

print_level stringToPrintLevel(const std::string &level)
{
	auto fnd = printLevelsMap.find(level);
	if (fnd != printLevelsMap.end())
	{
		return fnd->second;
	}
	else
	{
		throw(invalidParameterValue());
	}
}

