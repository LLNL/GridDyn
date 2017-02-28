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

// headers
#include "gridArea.h"
#include "gridDyn.h"
#include "vectorOps.hpp"
#include "core/objectFactoryTemplates.h"
#include "relays/gridRelay.h"
#include "linkModels/gridLink.h"
#include "gridBus.h"
#include "core/coreObjectTemplates.h"
#include "core/coreObjectList.h"
#include "core/objectInterpreter.h"
#include "core/coreExceptions.h"
#include "listMaintainer.h"

using namespace gridUnits;

std::atomic<count_t> gridArea::areaCounter(0);

static typeFactory<gridArea> gf ("area", stringVec { "basic","simple"}, "basic");


gridArea::gridArea (const std::string &objName) : gridPrimary (objName)
{
  // default values
  setUserID(++areaCounter);
  updateName ();
  opFlags.set (multipart_calculation_capable);
  obList = std::make_unique<coreObjectList>();
  opObjectLists = std::make_unique<listMaintainer>();
}

coreObject *gridArea::clone (coreObject *obj) const
{
  gridArea *area = cloneBase<gridArea, gridPrimary> (this, obj);
  if (area == nullptr)
    {
      return obj;
    }

  area->masterBus = masterBus;
  area->fTarget = fTarget;
  //clone all the areas
  for (size_t kk = 0; kk < m_Areas.size (); kk++)
    {
      if (kk >= area->m_Areas.size ())
        {
          gridArea *gA = static_cast<gridArea *> (m_Areas[kk]->clone ());
          area->add (gA);
        }
      else
        {
          m_Areas[kk]->clone (area->m_Areas[kk]);
        }
    }
  //clone all the buses
  for (size_t kk = 0; kk < m_Buses.size (); kk++)
    {
      if (kk >= area->m_Buses.size ())
        {
          gridBus *bus = static_cast<gridBus *> (m_Buses[kk]->clone ());
          area->add (bus);
		 
        }
      else
        {
          m_Buses[kk]->clone (area->m_Buses[kk]);
        }
    }

  //clone all the relays
  for (size_t kk = 0; kk < m_Relays.size (); kk++)
    {
      if (kk >= area->m_Relays.size ())
        {
          gridRelay *relay = static_cast<gridRelay *> (m_Relays[kk]->clone ());
          area->add (relay);
        }
      else
        {
          m_Relays[kk]->clone (area->m_Relays[kk]);
        }
        
    }

  //clone all the links
  for (size_t kk = 0; kk < m_Links.size (); kk++)
    {
      if (kk >= area->m_Links.size ())
        {
          gridLink *lnk = static_cast<gridLink *> (m_Links[kk]->clone ());
          //now we need to make sure the links are mapped properly
          for (index_t tt = 0; tt < lnk->terminalCount(); ++tt)
          {
              gridBus *bus=static_cast<gridBus *>(findMatchingObject(m_Links[kk]->getBus(tt + 1), area));
            
              assert(bus != nullptr);
              lnk->updateBus(bus, tt+1);
          }
          area->add (lnk);
        }
      else
        {
          m_Links[kk]->clone (area->m_Links[kk]);
          for (index_t tt = 0; tt < area->m_Links[kk]->terminalCount(); ++tt)
          {
              gridBus *bus = static_cast<gridBus *>(findMatchingObject(m_Links[kk]->getBus(tt + 1), area));
              area->m_Links[kk]->updateBus(bus, tt + 1);
          }
        }
    }

  if ((isRoot()) && (obj == nullptr))
  { //Now make sure to update all the objects linkages in the different objects
	  area->updateObjectLinkages(area);
  }
   
  for (auto &rel:area->m_Relays)
  {
      rel->updateObject(area, object_update_mode::match);
  }
  return area;
}

void gridArea::updateObjectLinkages(coreObject *newRoot)
{
	for (auto obj : primaryObjects)
	{
		obj->updateObjectLinkages(newRoot);
	}
}

// destructor
gridArea::~gridArea ()
{
  for (auto obj:primaryObjects)
    {
      removeReference(obj, this);
    }
}

void gridArea::add (coreObject *obj)
{
  if (dynamic_cast<gridBus *> (obj))
    {
      return add (static_cast<gridBus *> (obj));
    }
  else if (dynamic_cast<gridLink *> (obj))
    {
      return add (static_cast<gridLink *> (obj));
    }
  else if (dynamic_cast<gridArea *> (obj))
    {
      return add (static_cast<gridArea *> (obj));
    }
  else if (dynamic_cast<gridRelay *> (obj))
    {
      return add (static_cast<gridRelay *> (obj));
    }
  throw (unrecognizedObjectException(this));
}

template<class X>
void addObject (gridArea *area, X* obj, std::vector<X *> &objVector)
{
  if (!area->isMember (obj))
    {
	  auto insertRes=area->obList->insert(obj);
	  if (insertRes==false)
	  {
		  throw(objectAddFailure(area));
	  }
      objVector.push_back (obj);
      obj->setParent (area);
	  obj->addOwningReference();
      obj->locIndex = static_cast<index_t> (objVector.size ()) - 1;
      
      obj->set ("basepower", area->systemBasePower);
      obj->set ("basefreq", area->m_baseFreq);
      area->primaryObjects.push_back (obj);
      obj->locIndex2 = static_cast<index_t> (area->primaryObjects.size ()) - 1;
      if (area->checkFlag (pFlow_initialized))
        {
          area->alert (area, OBJECT_COUNT_INCREASE);
        }
    }
}

void gridArea::add (gridBus *bus)
{
  addObject (this,bus, m_Buses);
}

void gridArea::add (gridArea *area)
{
  addObject (this, area, m_Areas);
}

// add link
void gridArea::add (gridLink *lnk)
{
  addObject (this, lnk, m_Links);
}


// add link
void gridArea::add (gridRelay *relay)
{
  addObject (this, relay, m_Relays);
}


void gridArea::addsp(std::shared_ptr<coreObject> obj)
{
	coreObject *gco = obj.get();
	obj->addOwningReference();
	if (std::dynamic_pointer_cast<gridPrimary>(obj))
	{
		try
		{
			gridArea::add(gco);       //add the object to the regular system
			objectHolder.push_back(obj);
		}
		catch (const unrecognizedObjectException &)
		{
			objectHolder.push_back(obj);
			obj->locIndex = static_cast<index_t> (objectHolder.size()) - 1;
			obj->setParent(this);
			obList->insert(gco);
			if (obj->getNextUpdateTime() < kHalfBigNum)               //check if the object has updates
			{
				alert(gco, UPDATE_REQUIRED);
			}
		}
	}
	else
	{
		objectHolder.push_back(obj);
		obj->locIndex = static_cast<index_t> (objectHolder.size()) - 1;
		obj->setParent(this);
		obList->insert(gco);
		if (obj->getNextUpdateTime() < kHalfBigNum)               //check if the object has updates
		{
			alert(gco, UPDATE_REQUIRED);
		}
	}


}
// --------------- remove components ---------------
void gridArea::remove (coreObject *obj)
{
  if (dynamic_cast<gridBus *> (obj))
    {
      return remove (static_cast<gridBus *> (obj));
    }
  else if (dynamic_cast<gridLink *> (obj))
    {
      return remove (static_cast<gridLink *> (obj));
    }
  else if (dynamic_cast<gridArea *> (obj))
    {
      return remove (static_cast<gridArea *> (obj));
    }
  else if (dynamic_cast<gridRelay *> (obj))
    {
      return remove (static_cast<gridRelay *> (obj));
    }
  throw(unrecognizedObjectException(this));
}

template<class X>
void removeObject (gridArea *area, X* obj, std::vector<X *> &objVector)
{
  if ((obj->locIndex >= objVector.size ())||(objVector[obj->locIndex]->getID () != obj->getID ()))
    {
	  throw(objectRemoveFailure(area));
    }

  objVector[obj->locIndex]->setParent (nullptr);
  if (area->opFlags[being_deleted])
    {
      objVector[obj->locIndex] = nullptr;
    }
  else
    {
      if (area->checkFlag (pFlow_initialized))
        {
          area->alert (area, OBJECT_COUNT_DECREASE);
        }
      objVector.erase (objVector.begin () + obj->locIndex);
      //now shift the indices
      for (auto kk = obj->locIndex; kk < objVector.size (); ++kk)
        {
          objVector[kk]->locIndex = kk;
        }
      area->primaryObjects.erase (area->primaryObjects.begin () + obj->locIndex2);
      for (auto kk = obj->locIndex2; kk < area->primaryObjects.size (); ++kk)
        {
          objVector[kk]->locIndex2 = kk;
        }
      area->obList->remove (obj);

    }
}

// remove bus
void gridArea::remove (gridBus *bus)
{
  removeObject (this, bus, m_Buses);
}

// remove link
void gridArea::remove (gridLink *lnk)
{
  removeObject (this, lnk, m_Links);
}

// remove area
void gridArea::remove (gridArea *area)
{
  removeObject (this, area, m_Areas);
}

// remove area
void gridArea::remove (gridRelay *relay)
{
  removeObject (this, relay, m_Relays);
}


void gridArea::alert (coreObject *obj, int code)
{
  switch (code)
    {
    case OBJECT_NAME_CHANGE:
    case OBJECT_ID_CHANGE:
      obList->updateObject (obj);
      break;
    case OBJECT_IS_SEARCHABLE:
      if (isRoot())
        {
		  obList->insert(obj);
        }
      else
        {
		  getParent()->alert(obj, code);
        }
      break;
    default:
      gridPrimary::alert (obj, code);
    }
}

gridBus *gridArea::getBus (index_t x) const
{
  return (x < m_Buses.size ()) ? m_Buses[x] : nullptr;
}

gridLink *gridArea::getLink (index_t x) const
{
  return (x < m_Links.size ()) ? m_Links[x] : nullptr;
}

gridArea *gridArea::getArea (index_t x) const
{
  return (x < m_Areas.size ()) ? m_Areas[x] : nullptr;
}

gridRelay *gridArea::getRelay (index_t x) const
{
  return (x < m_Relays.size ()) ? m_Relays[x] : nullptr;
}

gridDynGenerator *gridArea::getGen (index_t x)
{
  for (auto a : m_Areas)
    {
      count_t tcnt = static_cast<count_t> (a->get ("gencount"));
      if (x < tcnt)
        {
          return (a->getGen (x));
        }
      else
        {
          x = x - tcnt;
        }
    }
  for (auto b : m_Buses)
    {
      count_t tcnt = b->get ("gencount");
      if (x < tcnt)
        {
          return b->getGen (x);
        }
      else
        {
          x = x - tcnt;
        }
    }
  return nullptr;
}

coreObject *gridArea::find (const std::string &objname) const
{
  
  
  auto rlc = objname.find_first_of (":/?");
  coreObject *obj = (rlc != std::string::npos) ? locateObject(objname, this, false) : obList->find(objname);

  if (obj == nullptr)
    {
      //try searching the subareas
      for (const auto &area : m_Areas)
        {
          obj = area->find (objname);
          if (obj)
            {
              break;
            }
        }
    }
  return obj;
}

coreObject *gridArea::getSubObject (const std::string &typeName, index_t num) const
{
  if (typeName == "bus")
    {
      return getBus (num);
    }
  if (typeName == "link")
    {
      return getLink (num);
    }
  if (typeName == "area")
    {
      return getArea (num);
    }
  if (typeName == "relay")
    {
      return getRelay (num);
    }
  if ((typeName == "object")||(typeName=="subobject"))
  {
	  return (num < primaryObjects.size()) ? primaryObjects[num] : nullptr;
  }
  return nullptr;
}

void gridArea::setAll (const std::string &type, std::string param, double val, gridUnits::units_t unitType)
{
	if (type == "all")
	{
		set(param, val, unitType);
		for (auto &area : m_Areas)
		{
			area->setAll(type, param, val, unitType);
		}
		for (auto &obj : primaryObjects)
		{
			obj->set(param, val, unitType);
		}
	}
  if (type == "area")
    {
      set (param, val, unitType);
      for (auto &area : m_Areas)
        {
          area->setAll (type, param, val, unitType);
        }
    }
  else if (type == "bus")
    {
      for (auto &bus : m_Buses)
        {
          bus->set (param, val, unitType);
        }
    }
  else if (type == "link")
    {
      for (auto &lnk : m_Links)
        {
          lnk->set (param, val, unitType);
        }
    }
  else if (type == "relay")
    {
      for (auto &rel : m_Relays)
        {
          rel->set (param, val, unitType);
        }
    }
  else if ((type == "gen") || (type == "load") || (type == "generator")||(type=="secondary"))
    {
      for (auto &bus : m_Buses)
        {
          bus->setAll (type,param, val,unitType);
        }
      for (auto &area : m_Areas)
        {
          area->setAll (type, param, val, unitType);
        }
    }

}

coreObject *gridArea::findByUserID (const std::string &typeName, index_t searchID) const
{

  if ((typeName == "area")&&(searchID == getUserID()))
    {
      return const_cast<gridArea *> (this);
    }
  if ((typeName == "gen") || (typeName == "load") || (typeName == "generator"))
    {
      //this is potentially computationally expensive, wouldn't recommend doing this search in a big system
      for (auto &bus : m_Buses)
        {
          coreObject *obj = bus->findByUserID (typeName, searchID);
          if (obj)
            {
              return obj;
            }
        }
      for (auto &area : m_Areas)
        {
          coreObject *obj = area->findByUserID (typeName, searchID);
          if (obj)
            {
              return obj;
            }
        }
      return nullptr;
    }
  auto possObjs = obList->find (searchID);
  if (possObjs.empty ())
    {
      for (auto &area : m_Areas)
        {
          coreObject *obj = area->findByUserID (typeName, searchID);
          if (obj)
            {
              return obj;
            }
        }
      return nullptr;
    }
  if (typeName == "bus")
    {
      for (auto po : possObjs)
        {
          if (po->locIndex < m_Buses.size ())
            {
              if (po->getID () == m_Buses[po->locIndex]->getID ())
                {
                  return po;
                }
            }
        }
    }
  else if (typeName == "link")
    {
      for (auto po : possObjs)
        {
          if (po->locIndex < m_Links.size ())
            {
              if (po->getID () == m_Links[po->locIndex]->getID ())
                {
                  return po;
                }
            }
        }
    }
  else if (typeName == "area")
    {
      for (auto po : possObjs)
        {
          if (po->locIndex < m_Areas.size ())
            {
              if (po->getID () == m_Areas[po->locIndex]->getID ())
                {
                  return po;
                }
            }
        }
    }
  else if (typeName == "relay")
    {
      for (auto po : possObjs)
        {
          if (po->locIndex < m_Relays.size ())
            {
              if (po->getID () == m_Relays[po->locIndex]->getID ())
                {
                  return po;
                }
            }
        }
    }
  //if we haven't found something try the subareas
  for (auto &area : m_Areas)
    {
      coreObject *obj = area->findByUserID (typeName, searchID);
      if (obj)
        {
          return obj;
        }
    }
  return nullptr;
}


// check bus members
bool gridArea::isMember (coreObject *object) const
{
  return obList->isMember (object);
}


// reset the bus parameters
void gridArea::reset (reset_levels level)
{
  for (auto obj : primaryObjects)
    {
      obj->reset (level);
    }
}

// dynInitializeB states
void gridArea::pFlowObjectInitializeA (coreTime time0, unsigned long flags)
{
  for (auto obj : primaryObjects)
    {
      obj->pFlowInitializeA (time0,flags);
    }
}


void gridArea::pFlowObjectInitializeB ()
{
  std::vector<gridPrimary *> lateBObjects;

  //links need to be initialized first so the initial power flow can be computed through the buses
  for (auto &link : m_Links)
    {
      if (link->checkFlag (late_b_initialize))
        {
          lateBObjects.push_back (link);
        }
      else
        {
          link->pFlowInitializeB ();
        }
    }

  for (auto &area : m_Areas)
    {
      if (area->checkFlag (late_b_initialize))
        {
          lateBObjects.push_back (area);
        }
      else
        {
          area->pFlowInitializeB ();
        }
    }
  for (auto &bus : m_Buses)
    {
      if (bus->checkFlag (late_b_initialize))
        {
          lateBObjects.push_back (bus);
        }
      else
        {
          bus->pFlowInitializeB ();
        }
    }
  for (auto &rel : m_Relays)
    {
      if (rel->checkFlag (late_b_initialize))
        {
          lateBObjects.push_back (rel);
        }
      else
        {
          rel->pFlowInitializeB ();
        }
    }
  for (auto &obj : lateBObjects)
    {
      obj->pFlowInitializeB ();
    }

  opObjectLists->makePreList (primaryObjects);
}

void gridArea::updateLocalCache ()
{
  //links should come first
  for (auto &link : m_Links)
    {
      if (link->isEnabled())
        {
          link->updateLocalCache ();
        }
    }
  for (auto &area : m_Areas)
    {
      if (area->isEnabled())
        {
          area->updateLocalCache ();
        }
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isEnabled())
        {
          bus->updateLocalCache ();
        }
    }
  for (auto &rel : m_Relays)
    {
      if (rel->isEnabled())
        {
          rel->updateLocalCache ();
        }
    }

}


void gridArea::updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
  //links should come first
  for (auto &link : m_Links)
    {
      if (link->isEnabled())
        {
          link->updateLocalCache (inputs,sD,sMode);
        }
    }
  for (auto &area : m_Areas)
    {
      if (area->isEnabled())
        {
          area->updateLocalCache (inputs,sD, sMode);
        }
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isEnabled())
        {
          bus->updateLocalCache (inputs, sD,sMode);
        }
    }
  for (auto &rel : m_Relays)
    {
      if (rel->isEnabled())
        {
          rel->updateLocalCache (inputs, sD,sMode);
        }
    }
}

change_code gridArea::powerFlowAdjust (const IOdata &inputs, unsigned long flags, check_level_t level)
{
  auto ret = change_code::no_change;
  opFlags.set (disable_flag_updates);      //this is so the adjustment object list can't get reset in the middle of this computation
  if (level < check_level_t::low_voltage_check)
    {
      for (auto obj : pFlowAdjustObjects)
        {
          auto iret = obj->powerFlowAdjust (inputs, flags, level);
          if (iret > ret)
            {
              ret = iret;
            }
        }
    }
  else
    {
      for (auto &obj : primaryObjects)
        {
          if (obj->isEnabled())
            {
              auto iret = obj->powerFlowAdjust (inputs, flags, level);
              if (iret > ret)
                {
                  ret = iret;
                }
            }
        }
    }
  //unset the lock
  opFlags.reset (disable_flag_updates);
  if (opFlags[flag_update_required])
    {
      updateFlags ();
    }
  return ret;

}

void gridArea::pFlowCheck (std::vector<violation> &Violation_vector)
{

  for (auto obj : primaryObjects)
    {
      obj->pFlowCheck (Violation_vector);
    }
}

// dynInitializeB states for dynamic solution
void gridArea::dynObjectInitializeA (coreTime time0, unsigned long flags)
{

  for (auto obj : primaryObjects)
    {
      if (obj->isEnabled())
        {
          obj->dynInitializeA (time0,flags);
        }
    }
}

// dynInitializeB states for dynamic solution part 2  //final clean up
void gridArea::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
  std::vector<gridPrimary *> lateBObjects;


  for (auto &link : m_Links)
    {
      if (link->isEnabled())
        {
          if (link->checkFlag (late_b_initialize))
            {
              lateBObjects.push_back (link);
            }
          else
            {
              link->dynInitializeB (inputs, desiredOutput,fieldSet);
            }
        }
    }
  for (auto &area : m_Areas)
    {
      if (area->isEnabled())
        {
          if (area->checkFlag (late_b_initialize))
            {
              lateBObjects.push_back (area);
            }
          else
            {
              area->dynInitializeB (inputs,desiredOutput, fieldSet);
            }
        }
    }
  double pmx = 0;
  for (auto &bus : m_Buses)
    {
      if (bus->isEnabled())
        {
          if (bus->checkFlag (late_b_initialize))
            {
              lateBObjects.push_back (bus);
            }
          else
            {
              bus->dynInitializeB (inputs,desiredOutput,fieldSet);
              double bmx = bus->getMaxGenReal ();
              if (bmx > pmx)
                {
                  pmx = bmx;
                  masterBus = bus->locIndex;
                }
            }
        }
    }
  for (auto &rel : m_Relays)
    {
      if (rel->isEnabled())
        {
          if (rel->checkFlag (late_b_initialize))
            {
              lateBObjects.push_back (rel);
            }
          else
            {
              rel->dynInitializeB (inputs, desiredOutput,fieldSet);
            }
        }
    }
  for (auto &obj : lateBObjects)
    {
      obj->dynInitializeB (inputs, desiredOutput,fieldSet);
    }

  opObjectLists->makePreList (primaryObjects);
}

//TODO:: PT make this do something or remove it
void gridArea::updateTheta (coreTime /*time*/)
{
}

void gridArea::converge (coreTime ttime, double state[], double dstate_dt[], const solverMode &sMode, converge_mode mode, double tol)
{

  if (opFlags[reverse_converge])
    {
      auto ra = opObjectLists->rbegin (sMode);
      auto rend = opObjectLists->rend (sMode);
      while (ra != rend)
        {
          (*ra)->converge (ttime, state, dstate_dt, sMode,mode,tol);
          ++ra;
        }
    }
  else
    {
      auto fa = opObjectLists->begin (sMode);
      auto fend = opObjectLists->end (sMode);
      while (fa != fend)
        {
          (*fa)->converge (ttime, state, dstate_dt, sMode, mode, tol);
          ++fa;
        }
    }
  //Toggle the reverse indicator every time
  if (opFlags[direction_oscillate])
    {
      opFlags.flip (reverse_converge);
    }

}

void gridArea::setFlag (const std::string &flag, bool val)
{
  if (flag == "reverse_converge")
    {
      opFlags.set (reverse_converge,val);
    }
  else if (flag == "direction_oscillate")
    {
      opFlags.set (direction_oscillate, val);
    }
  else
    {
      gridPrimary::setFlag (flag, val);
    }
}

// set properties
void gridArea::set (const std::string &param,  const std::string &val)

{

  gridPrimary::set (param, val);

}

static stringVec locNumStrings {};
static const stringVec locStrStrings {};
static const stringVec flagStrings {};

void gridArea::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
  getParamString<gridArea, gridObject> (this, pstr, locNumStrings, locStrStrings, flagStrings, pstype);
}

void gridArea::set (const std::string &param, double val, units_t unitType)
{
  if (param == "basepower")
    {
      systemBasePower = unitConversion (val,unitType,MW);
      for (auto &obj : primaryObjects)
        {
          obj->set (param, val);
        }
    }
  else if ((param == "basefrequency") || (param == "basefreq"))
    {
      //the default unit in this case should be Hz since that is what everyone assumes but we
      //need to store it in rps NOTE: we only do this assumed conversion for an area/simulation

      m_baseFreq = unitConversionFreq (val, (unitType == defUnit)?Hz:unitType, rps);

      for (auto obj : primaryObjects)
        {
          obj->set (param, m_baseFreq,rps);
        }
    }
  else
    {
      gridPrimary::set (param, val, unitType);
    }
}

double gridArea::get (const std::string &param, units_t unitType) const
{
  double val;
  if (param == "buscount")
    {
      val = m_Buses.size ();
    }
  else if (param == "linkcount")
    {
      val = m_Links.size ();
    }
  else if (param == "areacount")
    {
      val = m_Areas.size ();
    }
  else if (param == "relaycount")
    {
      val = m_Relays.size ();
    }
  else if (param == "totalbuscount")
    {
      val = 0;
      for (auto gA : m_Areas)
        {
          val += gA->get (param);
        }
      for (auto gA : m_Links)
        {
          val += gA->get ("buscount");
        }
      val += m_Buses.size ();
    }
  else if (param == "totallinkcount")
    {
      val = 0;
      for (auto gA : m_Areas)
        {
          val += gA->get (param);
        }
      for (auto gA : m_Links)
        {
          val += gA->get ("linkcount");
        }
      //links should return 1 from getting link count so don't need to add the links size again.
    }
  else if (param == "totalareacount")
    {
      val = 0;
      for (auto gA : m_Areas)
        {
          val += gA->get (param);
        }
      val += m_Areas.size ();
    }
  else if (param == "totalrelaycount")
    {
      val = 0;
      for (auto gA : m_Areas)
        {
          val += gA->get (param);
        }
      for (auto gA : m_Links)
        {
          val += gA->get ("relaycount");
        }
      val += m_Relays.size ();
    }
  else if ((param == "gencount")|| (param == "loadcount"))
    {
      val = 0;
      for (auto obj : primaryObjects)
        {
          val += obj->get (param);
        }
    }
  else if (param == "subobjectcount")
  {
	  val=primaryObjects.size();
  }
  else
    {
      return gridPrimary::get (param, unitType);
    }
  return val;
}



void gridArea::timestep (coreTime ttime, const IOdata &inputs, const solverMode &sMode)
{

  //update the tie lines first
  for (auto gL : m_Links)
    {
      if (gL->isEnabled())
        {
          gL->timestep (ttime, inputs, sMode);
        }

    }
  for (auto gA : m_Areas)
    {
      if (gA->isEnabled())
        {
          gA->timestep (ttime,inputs, sMode);
        }

    }
  for (auto bus : m_Buses)
    {
      if (bus->isEnabled())
        {
          bus->timestep (ttime,inputs, sMode);
        }

    }
  for (auto rel : m_Relays)
    {
      if (rel->isEnabled())
        {
          rel->timestep (ttime, inputs, sMode);
        }

    }
}

count_t gridArea::getBusVector (std::vector<gridBus *> &busVector, index_t start) const
{
  count_t cnt = static_cast<count_t> (m_Buses.size ());
  if (cnt > 0)
    {
      if (busVector.size () < start + cnt)
        {
          busVector.resize (start + cnt);
        }

      std::copy (m_Buses.begin (), m_Buses.end (), busVector.begin () + start);
    }
  for (auto &area : m_Areas)
    {
      cnt += area->getBusVector (busVector, start + cnt);
    }
  return cnt;
}

count_t gridArea::getLinkVector(std::vector<gridLink *> &linkVector, index_t start) const
{
    count_t cnt = static_cast<count_t> (m_Links.size());
    if (cnt > 0)
    {
        if (linkVector.size() < start + cnt)
        {
            linkVector.resize(start + cnt);
        }

        std::copy(m_Links.begin(), m_Links.end(), linkVector.begin() + start);
    }
    for (auto &area : m_Areas)
    {
        cnt += area->getLinkVector(linkVector, start + cnt);
    }
    return cnt;
}

count_t gridArea::getVoltage (std::vector<double> &V, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getVoltage (V, start + cnt);
    }
  if (V.size () < start + m_Buses.size ())
    {
      V.resize (start + m_Buses.size ());
    }
  for (size_t kk = 0; kk < m_Buses.size (); ++kk)
    {
      V[start + cnt + kk] = m_Buses[kk]->getVoltage ();
    }
  cnt += static_cast<count_t> (m_Buses.size ());
  return cnt;
}

count_t gridArea::getVoltage (std::vector<double> &V, const double state[], const solverMode &sMode, index_t start) const
{
  count_t cnt = 0;

  for (auto &area : m_Areas)
    {
      cnt += area->getVoltage (V, state, sMode, start + cnt);
    }
  if (V.size () < start + m_Buses.size ())
    {
      V.resize (start + m_Buses.size ());
    }
  for (size_t kk = 0; kk < m_Buses.size (); ++kk)
    {
      V[start + cnt + kk] = m_Buses[kk]->getVoltage (state,sMode);
    }
  cnt += static_cast<count_t> (m_Buses.size ());
  return cnt;
}


count_t gridArea::getAngle (std::vector<double> &A, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getAngle (A, start + cnt);
    }
  if (A.size () < start + m_Buses.size ())
    {
      A.resize (start + m_Buses.size ());
    }
  for (size_t kk = 0; kk < m_Buses.size (); ++kk)
    {
      A[start + cnt + kk] = m_Buses[kk]->getAngle ();
    }
  cnt += static_cast<count_t> (m_Buses.size ());
  return cnt;
}

count_t gridArea::getAngle (std::vector<double> &V, const double state[], const solverMode &sMode, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getAngle (V, state, sMode, start + cnt);
    }
  if (V.size () < start + m_Buses.size ())
    {
      V.resize (start + m_Buses.size ());
    }
  for (size_t kk = 0; kk < m_Buses.size (); ++kk)
    {
      V[start + cnt + kk] = m_Buses[kk]->getAngle (state, sMode);
    }
  cnt += static_cast<count_t> (m_Buses.size ());
  return cnt;
}

count_t gridArea::getFreq(std::vector<double> &F, index_t start) const
{
    count_t cnt = 0;
    for (auto &area : m_Areas)
    {
        cnt += area->getFreq(F, start + cnt);
    }
    if (F.size() < start + m_Buses.size())
    {
        F.resize(start + m_Buses.size());
    }
    for (size_t kk = 0; kk < m_Buses.size(); ++kk)
    {
        F[start + cnt + kk] = m_Buses[kk]->getFreq();
    }
    cnt += static_cast<count_t> (m_Buses.size());
    return cnt;
}

/*
count_t gridArea::getFreq(std::vector<double> &F, const double state[], const solverMode &sMode, index_t start) const
{
    count_t cnt = 0;
    for (auto &area : m_Areas)
    {
        cnt += area->getFreq(F, state, sMode, start + cnt);
    }
    if (F.size() < start + m_Buses.size())
    {
        F.resize(start + m_Buses.size());
    }
    for (size_t kk = 0; kk < m_Buses.size(); ++kk)
    {
        F[start + cnt + kk] = m_Buses[kk]->getFreq(state, sMode);
    }
    cnt += static_cast<count_t> (m_Buses.size());
    return cnt;
}
*/

count_t gridArea::getLinkRealPower (std::vector<double> &A, index_t start, int bus) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getLinkRealPower (A, start + cnt, bus);
    }
  if (A.size () < start + m_Links.size ())
    {
      A.resize (start + m_Links.size ());
    }
  for (size_t kk = 0; kk < m_Links.size (); ++kk)
    {
      A[start + cnt + kk] = m_Links[kk]->getRealPower (bus);
    }
  cnt += static_cast<count_t> (m_Links.size ());
  return cnt;
}

count_t gridArea::getLinkReactivePower (std::vector <double> &A, index_t start, int bus) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getLinkReactivePower (A, start + cnt, bus);
    }
  if (A.size () < start + m_Links.size ())
    {
      A.resize (start + m_Links.size ());
    }
  for (size_t kk = 0; kk < m_Links.size (); ++kk)
    {
      A[start + cnt + kk] = m_Links[kk]->getReactivePower (bus);
    }
  cnt += static_cast<count_t> (m_Links.size ());
  return cnt;
}

count_t gridArea::getBusGenerationReal (std::vector<double> &A, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getBusGenerationReal (A, start + cnt);
    }
  if (A.size () < start + m_Buses.size ())
    {
      A.resize (start + m_Buses.size ());
    }
  for (size_t kk = 0; kk < m_Buses.size (); ++kk)
    {
      A[start + cnt + kk] = m_Buses[kk]->getGenerationReal ();
    }
  cnt += static_cast<count_t> (m_Buses.size ());
  return cnt;
}

count_t gridArea::getBusGenerationReactive (std::vector <double> &A, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getBusGenerationReactive (A, start + cnt);
    }
  if (A.size () < start + m_Buses.size ())
    {
      A.resize (start + m_Buses.size ());
    }
  for (size_t kk = 0; kk < m_Buses.size (); ++kk)
    {
      A[start + cnt + kk] = m_Buses[kk]->getGenerationReactive ();
    }
  cnt += static_cast<count_t> (m_Buses.size ());
  return cnt;
}

count_t gridArea::getBusLoadReal (std::vector<double> &A, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getBusLoadReal (A, start + cnt);
    }
  if (A.size () < start + m_Buses.size ())
    {
      A.resize (start + m_Buses.size ());
    }
  for (size_t kk = 0; kk < m_Buses.size (); ++kk)
    {
      A[start + cnt + kk] = m_Buses[kk]->getLoadReal ();
    }
  cnt += static_cast<count_t> (m_Buses.size ());
  return cnt;
}

count_t gridArea::getBusLoadReactive (std::vector <double> &A, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getBusLoadReactive (A, start + cnt);
    }
  if (A.size () < start + m_Buses.size ())
    {
      A.resize (start + m_Buses.size ());
    }
  for (size_t kk = 0; kk < m_Buses.size (); ++kk)
    {
      A[start + cnt + kk] = m_Buses[kk]->getLoadReactive ();
    }
  cnt += static_cast<count_t> (m_Buses.size ());
  return cnt;
}

count_t gridArea::getLinkLoss (std::vector<double> &L, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      if (area->isEnabled())
        {
          cnt += area->getLinkLoss (L, start + cnt);
        }
    }
  if (L.size () < start + m_Links.size ())
    {
      L.resize (start + m_Links.size ());
    }
  for (size_t kk = 0; kk < m_Links.size (); ++kk)
    {

      L[cnt + kk] = m_Links[kk]->getLoss ();
    }
  return cnt + static_cast<count_t> (m_Links.size ());
}

count_t gridArea::getBusName (stringVec &nm, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getBusName (nm, start + cnt);
    }
  if (nm.size () < start + m_Buses.size ())
    {
      nm.resize (start + m_Buses.size ());
    }
  auto nmloc = nm.begin () + start + cnt;
  for (auto &bus : m_Buses)
    {
      *nmloc = bus->getName ();
      ++nmloc;
    }
  cnt += static_cast<count_t> (m_Buses.size ());
  return cnt;
}

count_t gridArea::getLinkName (stringVec &nm, index_t start) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getLinkName (nm, start + cnt);
    }
  if (nm.size () < start + m_Links.size ())
    {
      nm.resize (start + m_Links.size ());
    }
  auto nmloc = nm.begin () + start + cnt;
  for (auto &link : m_Links)
    {
      *nmloc = link->getName ();
      ++nmloc;
    }
  cnt += static_cast<count_t> (m_Links.size ());
  return cnt;
}

count_t gridArea::getLinkBus (stringVec &nm, index_t start, int busNum) const
{
  count_t cnt = 0;
  for (auto &area : m_Areas)
    {
      cnt += area->getLinkBus (nm, start + cnt,busNum);
    }
  if (nm.size () < start + m_Links.size ())
    {
      nm.resize (start + m_Links.size ());
    }
  auto nmloc = nm.begin () + start + cnt;
  for (auto &link : m_Links)
    {
      *nmloc = link->getBus (busNum)->getName ();
      ++nmloc;
    }

  cnt += static_cast<count_t> (m_Links.size ());
  return cnt;
}


//single value return functions





double gridArea::getAdjustableCapacityUp (coreTime time) const
{
  double adjUp = 0.0;
  for (auto &area : m_Areas)
    {
      adjUp += area->getAdjustableCapacityUp (time);
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isConnected ())
        {
          adjUp += bus->getAdjustableCapacityUp (time);
        }
    }
  return adjUp;
}

double gridArea::getAdjustableCapacityDown (coreTime time) const
{
  double adjDown = 0.0;
  for (auto &area : m_Areas)
    {
      adjDown += area->getAdjustableCapacityDown (time);
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isConnected ())
        {
          adjDown += bus->getAdjustableCapacityDown (time);
        }
    }
  return adjDown;
}


double gridArea::getLoss () const
{
  double loss = 0.0;
  for (auto &area : m_Areas)
    {
      loss += area->getLoss ();
    }
  for (auto &link : m_Links)
    {
      if (link->isEnabled())
        {
          loss += link->getLoss ();
        }
    }
  for (auto &link : m_externalLinks)
    {
      if (link->isEnabled())
        {    //half of losses of the tie lines get attributed to the area
          loss += 0.5 * link->getLoss ();
        }
    }
  return loss;
}

double gridArea::getGenerationReal () const
{
  double genP = 0.0;
  for (auto &area : m_Areas)
    {
         genP += area->getGenerationReal ();
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isConnected ())
        {
          genP += bus->getGenerationReal ();
        }
    }
  return genP;
}

double gridArea::getGenerationReactive () const
{
  double genQ = 0.0;
  for (auto &area : m_Areas)
    {
         genQ += area->getGenerationReactive ();
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isConnected ())
        {
          genQ += bus->getGenerationReactive ();
        }
    }
  return genQ;
}

double gridArea::getLoadReal () const
{
  double loadP = 0.0;
  for (auto &area : m_Areas)
    {
         loadP += area->getLoadReal ();
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isConnected ())
        {
          loadP += bus->getLoadReal ();
        }
    }
  return loadP;
}

double gridArea::getLoadReactive () const
{
  double loadQ = 0.0;
  for (auto &area : m_Areas)
    {
         loadQ += area->getLoadReactive ();
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isConnected ())
        {
          loadQ += bus->getLoadReactive ();
        }
    }
  return loadQ;
}

double gridArea::getAvgAngle () const
{
  double a = 0.0;
  double cnt = 0.0;
  for (auto &bus : m_Buses)
    {
      if (bus->hasInertialAngle ())
        {
          a += bus->getAngle ();
          cnt += 1.0;
        }
    }
  return (a / cnt);
}

double gridArea::getAvgAngle (const stateData &sD, const solverMode &sMode) const
{
  double a = 0.0;
  double cnt = 0.0;
  for (auto &bus : m_Buses)
    {
      if (bus->hasInertialAngle ())
        {
          a += bus->getAngle (sD,sMode);
          cnt += 1.0;
        }
    }

  return (a / cnt);
}

double gridArea::getAvgFreq() const
{
    double a = 0.0;
    double cnt = 0.0;
    for (auto &bus : m_Buses)
    {
        if (bus->hasInertialAngle())
        {
            a += bus->getFreq();
            cnt += 1.0;
        }
    }
    return (a / cnt);
}

// -------------------- Power Flow --------------------

//guess the solution
void gridArea::guess (coreTime ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  auto cobj = opObjectLists->begin (sMode);
  auto cend = opObjectLists->end (sMode);
  while (cobj != cend)
    {
      (*cobj)->guess (ttime, state, dstate_dt, sMode);
      ++cobj;
    }
  //next do any internal control elements

}

void gridArea::getVariableType (double sdata[], const solverMode &sMode)
{

  auto ra = opObjectLists->begin (sMode);
  auto rend = opObjectLists->end (sMode);
  while (ra != rend)
    {
      (*ra)->getVariableType (sdata, sMode);
      ++ra;
    }

  //next do any internal area states
}

void gridArea::getTols (double tols[], const solverMode &sMode)
{
  auto ra = opObjectLists->begin (sMode);
  auto rend = opObjectLists->end (sMode);
  while (ra != rend)
    {
      (*ra)->getTols (tols, sMode);
      ++ra;
    }
  //next do any internal area states
}

//#define DEBUG_PRINT
void gridArea::rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)
{
  for (auto ro : rootObjects)
    {
      ro->rootTest (inputs, sD, roots, sMode);
    }
#ifdef DEBUG_PRINT
  for (size_t kk = 0; kk < rootSize (sMode); ++kk)
    {
      printf ("t=%f root[%d]=%e\n", ttime, kk, roots[kk]);
    }
#endif
}

change_code gridArea::rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode,  check_level_t level)
{
  change_code ret = change_code::no_change;
  //root checks can trigger flag updates disable and just do the update once
  opFlags.set (disable_flag_updates);
  if (level >= check_level_t::low_voltage_check)
    {
      for (auto &obj : primaryObjects)
        {
          if (obj->isEnabled())
            {
              auto iret = obj->rootCheck (inputs, sD, sMode, level);
              if (iret > ret)
                {
                  ret = iret;
                }
            }
        }
    }
  else
    {
      for (auto &ro : rootObjects)
        {
          if (ro->checkFlag (has_alg_roots))
            {
              auto iret = ro->rootCheck (inputs, sD,sMode,level);
              if (iret > ret)
                {
                  ret = iret;
                }
            }
        }
    }
  opFlags.reset (disable_flag_updates);
  if (opFlags[flag_update_required])
    {
      updateFlags ();
    }
  return ret;
}


void gridArea::rootTrigger (coreTime ttime, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode)
{
  auto RF = vecFindne (rootMask, 0);
  size_t cloc = 0;
  size_t rs = rootSize (sMode);
  size_t rootOffset = offsets.getRootOffset (sMode);

  auto currentRootObject = rootObjects.begin ();
  auto obend = rootObjects.end ();
  auto ors = (*currentRootObject)->rootSize (sMode);
  opFlags.set (disable_flag_updates); //root triggers can cause a flag change and the flag update currently checks the root object
  //TODO::May be wise at some point to revisit the combination of the flags and root object checking
  for (auto rc : RF)
    {
      if (rc < rootOffset + cloc)
        {
          continue;
        }
      if (rc >= rootOffset + rs)
        {
          break;
        }
      while (rc >= rootOffset + cloc + ors)
        {
          cloc += ors;
          ++currentRootObject;
          ors = (*currentRootObject)->rootSize (sMode);
        }
      (*currentRootObject)->rootTrigger (ttime, inputs, rootMask, sMode);
      cloc += ors;
      if ((++currentRootObject) == obend)
        {
          break;
        }
      ors = (*currentRootObject)->rootSize (sMode);
    }
  opFlags.reset (disable_flag_updates);
  if (opFlags[flag_update_required])
    {
      updateFlags ();
      opFlags.reset (flag_update_required);
    }
}

// pass the solution
void gridArea::setState (coreTime ttime, const double state[],const double dstate_dt[], const solverMode &sMode)
{
  prevTime = ttime;

  //links come first
  for (auto &link : m_Links)
    {
      if (link->isEnabled())
        {
          link->setState (ttime, state, dstate_dt, sMode);
        }
    }
  for (auto &area : m_Areas)
    {
      if (area->isEnabled())
        {
          area->setState (ttime, state, dstate_dt, sMode);
        }
    }

  for (auto &bus : m_Buses)
    {
      if (bus->isEnabled())
        {
          bus->setState (ttime, state, dstate_dt, sMode);
        }
    }
  for (auto &rel : m_Relays)
    {
      if (rel->isEnabled())
        {
          rel->setState (ttime, state, dstate_dt, sMode);
        }
    }
  //next do any internal area states
}

void gridArea::getVoltageStates (double vStates[], const solverMode &sMode) const

{
  index_t Voffset;
  for (auto &area : m_Areas)
    {
      if (area->isEnabled())
        {
          area->getVoltageStates (vStates, sMode);
        }
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isEnabled())
        {
          Voffset = bus->getOutputLoc(sMode,voltageInLocation);
          if (Voffset != kNullLocation)
            {
              vStates[Voffset] = 2.0;
            }
        }
    }
  for (auto &link : m_Links)
    {
      if (link->isEnabled())
        {
          if (link->voltageStateCount (sMode) > 0)
            {
              auto linkOffsets = link->getOffsets(sMode);
              Voffset = linkOffsets->vOffset;
              for (size_t kk = 0; kk < link->voltageStateCount (sMode); kk++)
                {
                  vStates[Voffset + kk] = 2.0;
                }
            }

        }
    }
}

void gridArea::getAngleStates (double aStates[], const solverMode &sMode) const

{
  index_t Aoffset;
  for (auto &area : m_Areas)
    {
      if (area->isEnabled())
        {
          area->getAngleStates (aStates, sMode);
        }
    }
  for (auto &bus : m_Buses)
    {
      if (bus->isEnabled())
        {
          Aoffset = bus->getOutputLoc(sMode,angleInLocation);
          if (Aoffset != kNullLocation)
            {
              aStates[Aoffset] = 1.0;
            }
        }
    }
  for (auto &link : m_Links)
    {
      if (link->isEnabled())
        {
          if (link->angleStateCount (sMode) > 0)
            {
              auto linkOffsets = link->getOffsets(sMode);
              Aoffset = linkOffsets->aOffset;
              for (size_t kk = 0; kk < link->voltageStateCount (sMode); kk++)
                {
                  aStates[Aoffset + kk] = 1.0;
                }
            }

        }
    }
}

// residual

void gridArea::preEx (const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
  opObjectLists->preEx (inputs, sD, sMode);
}

void gridArea::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  opObjectLists->residual (inputs, sD, resid, sMode);

  //next do any internal states
}

void gridArea::algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha)
{
  opObjectLists->algebraicUpdate (inputs, sD, update, sMode, alpha);

  //next do any internal states
}

void gridArea::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  std::string prefix2 = "";
  if (!isRoot())
    {
      prefix2 = prefix + getName() + "::";
    }
  else
    {
      if (stNames.size () < offsets.maxIndex (sMode))
        {
          stNames.resize (offsets.maxIndex (sMode));
        }
    }
  auto obeg = opObjectLists->cbegin (sMode);
  auto oend = opObjectLists->cend (sMode);
  while (obeg != oend)
    {
      (*obeg)->getStateName (stNames, sMode, prefix2);
      ++obeg;
    }
}


void gridArea::delayedResidual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  opObjectLists->delayedResidual (inputs, sD, resid, sMode);
}

void gridArea::delayedDerivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  opObjectLists->delayedDerivative (inputs, sD, deriv, sMode);
}

void gridArea::delayedJacobian (const IOdata &inputs, const stateData &sD, matrixData<double> &ad,const IOlocs &inputLocs, const solverMode &sMode)
{
  opObjectLists->delayedJacobian (inputs, sD, ad,inputLocs, sMode);
}

void gridArea::delayedAlgebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha)
{
  opObjectLists->delayedAlgebraicUpdate (inputs, sD, update, sMode, alpha);
}


void gridArea::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  opObjectLists->derivative (inputs, sD, deriv, sMode);
  //next do any internal states
}

// Jacobian
void gridArea::jacobianElements (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{
  opObjectLists->jacobianElements (inputs, sD, ad,inputLocs, sMode);
  //next do any internal control elements

}


void gridArea::updateFlags (bool /*dynOnly*/)
{

  rootObjects.clear ();
  pFlowAdjustObjects.clear ();
  opFlags &= (~flagMask);  //clear the cascading flags

  for (auto &obj : primaryObjects)
    {
      if (obj->isEnabled())
        {
          opFlags |= obj->cascadingFlags ();
          if (obj->checkFlag (has_roots))
            {
              rootObjects.push_back (obj);
            }
          if (obj->checkFlag (has_powerflow_adjustments))
            {
              pFlowAdjustObjects.push_back (obj);
            }
        }

    }

}

void gridArea::setOffsets (const solverOffsets &newOffsets, const solverMode &sMode)
{

  if (!(isLoaded (sMode,false)))
    {
      loadSizes (sMode,false);
    }
  offsets.setOffsets (newOffsets, sMode);
  solverOffsets no (newOffsets);
  no.localIncrement (offsets.getOffsets (sMode));

  for (auto &obj : primaryObjects)
    {
      obj->setOffsets (no, sMode);
      no.increment (obj->getOffsets (sMode));
    }
}


void gridArea::setOffset (index_t offset, const solverMode &sMode)
{
  if (!isEnabled())
    {
      return;
    }
  for (auto &obj : primaryObjects)
    {
      obj->setOffset (offset, sMode);
      offset += obj->stateSize (sMode);

    }
  offsets.setOffset (offset, sMode);

}


void gridArea::setRootOffset (index_t Roffset, const solverMode &sMode)
{
  offsets.setRootOffset (Roffset, sMode);
  auto so = offsets.getOffsets (sMode);
  auto nR = so->local.algRoots + so->local.diffRoots;
  for (auto &ro : rootObjects)
    {
      ro->setRootOffset (Roffset + nR, sMode);
      nR += ro->rootSize (sMode);
    }
}


double gridArea::getTieFlowReal () const
{
  return (getGenerationReal () - getLoadReal () - getLoss ());
}

double gridArea::getMasterAngle (const stateData &sD, const solverMode &sMode) const
{
  if (masterBus < 0)
    {
      if (!isRoot())
        {
          return static_cast<gridArea *> (getParent())->getMasterAngle (sD,sMode);
        }
      else if (!m_Buses.empty ())
        {
          return m_Buses[0]->getAngle (sD,sMode);
        }
      else
        {
          return 0;
        }
    }
  else
    {
      return m_Buses[masterBus]->getAngle (sD,sMode);
    }
}

void gridArea::loadSizes (const solverMode &sMode, bool dynOnly)
{

  if (isLoaded (sMode, dynOnly))
    {
      return;
    }
  solverOffsets *so = offsets.getOffsets (sMode);
  if (!isEnabled())
    {
      so->reset ();
      so->stateLoaded = true;
      so->rjLoaded = true;
      return;
    }

  if ((dynOnly)||(so->stateLoaded))
    {
      so->rootAndJacobianCountReset ();
      for (auto &obj : primaryObjects)
        {
          if (!(obj->isLoaded (sMode, dynOnly)))
            {
              obj->loadSizes (sMode, dynOnly);
            }
          so->addRootAndJacobianSizes (obj->getOffsets (sMode));
        }

      so->rjLoaded = true;
    }
  else
    {
      so->reset ();

      for (auto &obj : primaryObjects)
        {
          if (!(obj->isLoaded (sMode, dynOnly)))
            {
              obj->loadSizes (sMode, dynOnly);
            }
          so->addSizes (obj->getOffsets (sMode));
        }

      so->stateLoaded = true;
      so->rjLoaded = true;
      opObjectLists->makeList (sMode, primaryObjects);
    }


}

gridArea * getMatchingArea (gridArea *area, gridPrimary *src, gridPrimary *sec)
{
  
  if (area->isRoot())
    {
      return nullptr;
    }
 
  if (isSameObject(area->getParent (),src))    //if this is true then things are easy
    {
      return sec->getArea (area->locIndex);
    }
  else
    {
      std::vector<index_t> lkind;
      auto par = dynamic_cast<gridPrimary *> (area->getParent ());
      if (par == nullptr)
        {
          return nullptr;
        }
      lkind.push_back (area->locIndex);
      while (par->getID () != src->getID ())
        {
          lkind.push_back (par->locIndex);
          par = dynamic_cast<gridPrimary *> (par->getParent ());
          if (par == nullptr)
            {
              return nullptr;
            }
        }
      //now work our way backwards through the secondary
      par = sec;
      for (auto kk = lkind.size () - 1; kk > 0; --kk)
        {
          par = static_cast<gridPrimary *> (par->getArea (lkind[kk]));
        }
      return par->getArea (lkind[0]);

    }
}
