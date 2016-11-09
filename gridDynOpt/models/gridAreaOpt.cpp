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

// headers
#include "gridAreaOpt.h"
#include "gridBusOpt.h"
#include "gridLinkOpt.h"
#include "gridRelayOpt.h"
#include "gridArea.h"
#include "optObjectFactory.h"
#include "vectorOps.hpp"
#include "vectData.h"
#include "linkModels/gridLink.h"
#include "core/gridDynExceptions.h"
#include "gridBus.h"
#include "stringOps.h"


using namespace gridUnits;

static optObjectFactory<gridAreaOpt, gridArea> opa ("basic", "area");

gridAreaOpt::gridAreaOpt (const std::string &objName) : gridOptObject (objName)
{

}



gridAreaOpt::gridAreaOpt (gridCoreObject *obj, const std::string &objName) : gridOptObject (objName),area (dynamic_cast<gridArea *> (obj))
{
  if (area)
    {
      if (name.empty ())
        {
          name = area->getName ();
        }
      id = area->getUserID ();
    }
}

// destructor
gridAreaOpt::~gridAreaOpt ()
{
  for (auto &obj : objectList)
    {
      condDelete (obj, this);
    }

}

gridCoreObject *gridAreaOpt::clone (gridCoreObject *obj) const
{
  gridAreaOpt *nobj;
  if (obj == nullptr)
    {
      nobj = new gridAreaOpt ();
    }
  else
    {
      nobj = dynamic_cast<gridAreaOpt *> (obj);
      if (nobj == nullptr)
        {
          //if we can't cast the pointer clone at the next lower level
          gridOptObject::clone (obj);
          return obj;
        }
    }
  gridOptObject::clone (nobj);


  //now clone all the loads and generators
  //cloning the links from this component would be bad
  //clone the generators and loads


  for (size_t kk = 0; kk < busList.size (); ++kk)
    {
      if (kk >= nobj->busList.size ())
        {
          nobj->add (static_cast<gridBusOpt *> (busList[kk]->clone (nullptr)));
        }
      else
        {
          busList[kk]->clone (nobj->busList[kk]);
        }
    }
  for (size_t kk = 0; kk < areaList.size (); ++kk)
    {
      if (kk >= nobj->areaList.size ())
        {
          nobj->add (static_cast<gridAreaOpt *> (areaList[kk]->clone (nullptr)));
        }
      else
        {
          areaList[kk]->clone (nobj->areaList[kk]);
        }
    }
  for (size_t kk = 0; kk < linkList.size (); ++kk)
    {
      if (kk >= nobj->linkList.size ())
        {
          nobj->add (static_cast<gridLinkOpt *> (linkList[kk]->clone (nullptr)));
        }
      else
        {
          linkList[kk]->clone (nobj->linkList[kk]);
        }
    }
  for (size_t kk = 0; kk < relayList.size (); ++kk)
    {
      if (kk >= nobj->relayList.size ())
        {
          nobj->add (static_cast<gridRelayOpt *> (relayList[kk]->clone (nullptr)));
        }
      else
        {
          relayList[kk]->clone (nobj->relayList[kk]);
        }
    }
  return nobj;
}



void gridAreaOpt::objectInitializeA (unsigned long flags)
{
  //first do a check to make sure all gridDyn areas are represented by gridDynOpt Area
  gridArea *a;
  gridBus *b;
  gridLink *Lnk;
  auto coof = coreOptObjectFactory::instance ();
  bool found;
  std::vector<gridOptObject *> newObj;
  gridOptObject *oo;


  index_t kk = 0;

  //make sure all areas have an opt object
  a = area->getArea (kk);

  while (a)
    {
      found = false;
      for (auto oa:areaList)
        {
          if (a->getID () == oa->getID ())
            {
              found = true;
              break;
            }
        }
      if (!found)
        {
          oo = coof->createObject (a);
          newObj.push_back (oo);
        }
      ++kk;
      a = area->getArea (kk);
    }
  for (auto no:newObj)
    {
      add (no);
    }
  newObj.clear ();
  //make sure all buses have an opt object
  b = area->getBus (kk);

  while (b)
    {
      found = false;
      for (auto oa : busList)
        {
          if (b->getID () == oa->getID ())
            {
              found = true;
              break;
            }
        }
      if (!found)
        {
          oo = coof->createObject (b);
          newObj.push_back (oo);
        }
      ++kk;
      b = area->getBus (kk);
    }
  for (auto no : newObj)
    {
      add (no);
    }
  newObj.clear ();
  //make sure all links have an opt object
  Lnk = area->getLink (kk);

  while (Lnk)
    {
      found = false;
      for (auto oa : linkList)
        {
          if (Lnk->getID () == oa->getID ())
            {
              found = true;
              break;
            }
        }
      if (!found)
        {
          oo = coof->createObject (b);
          newObj.push_back (oo);
        }
      ++kk;
      Lnk = area->getLink (kk);
    }
  for (auto no : newObj)
    {
      add (no);
    }
  newObj.clear ();

  for (auto obj : objectList)
    {
      obj->objectInitializeA (flags);
    }

}

void gridAreaOpt::loadSizes (const optimMode &oMode)
{
  optimOffsets *oo = offsets.getOffsets (oMode);
  oo->reset ();
  switch (oMode.flowMode)
    {
    case flowModel_t::none:
	default:
		break;
    case flowModel_t::transport:
      break;
    case flowModel_t::dc:
    case flowModel_t::ac:
      break;
    }

  for (auto obj : objectList)
    {
      obj->loadSizes (oMode);
      oo->addSizes (obj->offsets.getOffsets (oMode));
    }

}


void gridAreaOpt::setValues (const optimData *oD, const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->setValues (oD, oMode);
    }

}
//for saving the state
void gridAreaOpt::guess (double ttime, double val[], const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->guess (ttime, val, oMode);
    }

}

void gridAreaOpt::getTols (double tols[], const optimMode &oMode)

{
  for (auto obj : objectList)
    {
      obj->getTols (tols, oMode);
    }

}

void gridAreaOpt::getVariableType (double sdata[], const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->getVariableType (sdata, oMode);
    }

}

void gridAreaOpt::valueBounds (double ttime, double upperLimit[], double lowerLimit[], const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->valueBounds (ttime, upperLimit, lowerLimit, oMode);
    }

}

void gridAreaOpt::linearObj (const optimData *oD, vectData<double> *linObj, const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->linearObj (oD, linObj, oMode);
    }

}
void gridAreaOpt::quadraticObj (const optimData *oD, vectData<double> *linObj, vectData<double> *quadObj, const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->quadraticObj (oD, linObj, quadObj, oMode);
    }

}

double gridAreaOpt::objValue (const optimData *oD, const optimMode &oMode)
{
  double cost = 0;
  for (auto obj : objectList)
    {
      cost += obj->objValue (oD, oMode);
    }

  return cost;
}

void gridAreaOpt::gradient (const optimData *oD, double deriv[], const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->gradient (oD, deriv, oMode);
    }

}
void gridAreaOpt::jacobianElements (const optimData *oD, matrixData<double> *ad, const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->jacobianElements (oD, ad, oMode);
    }

}
void gridAreaOpt::getConstraints (const optimData *oD, matrixData<double> *cons, double upperLimit[], double lowerLimit[], const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->getConstraints (oD, cons, upperLimit, lowerLimit,oMode);
    }

}

void gridAreaOpt::constraintValue (const optimData *oD, double cVals[], const optimMode &oMode)
{

  for (auto obj : objectList)
    {
      obj->constraintValue (oD, cVals, oMode);
    }

}

void gridAreaOpt::constraintJacobianElements (const optimData *oD, matrixData<double> *ad, const optimMode &oMode)
{
  for (auto obj : objectList)
    {
      obj->constraintJacobianElements (oD, ad, oMode);
    }
}
void gridAreaOpt::getObjName (stringVec &objNames, const optimMode &oMode, const std::string &prefix)
{
  for (auto obj : objectList)
    {
      obj->getObjName (objNames, oMode, prefix);
    }
}


void gridAreaOpt::disable ()
{
  enabled = false;
  for (auto &link : linkList)
    {
      link->disable ();
    }
}

void gridAreaOpt::setOffsets (const optimOffsets &newOffsets, const optimMode &oMode)
{

  offsets.setOffsets (newOffsets, oMode);
  optimOffsets no (offsets.getOffsets (oMode));
  no.localLoad ();

  for (auto sa : areaList)
    {
      sa->setOffsets (no, oMode);
      no.increment (sa->offsets.getOffsets (oMode));
    }
  for (auto bus : busList)
    {
      bus->setOffsets (no, oMode);
      no.increment (bus->offsets.getOffsets (oMode));
    }
  for (auto link : linkList)
    {
      link->setOffsets (no, oMode);
      no.increment (link->offsets.getOffsets (oMode));
    }
  for (auto relay : relayList)
    {
      relay->setOffsets (no, oMode);
      no.increment (relay->offsets.getOffsets (oMode));
    }


}


void gridAreaOpt::setOffset (index_t offset, index_t constraintOffset, const optimMode &oMode)
{

  for (auto sa : areaList)
    {
      sa->setOffset (offset, constraintOffset, oMode);
      constraintOffset += sa->constraintSize (oMode);
      offset += sa->objSize (oMode);

    }
  for (auto bus : busList)
    {
      bus->setOffset (offset, constraintOffset, oMode);
      constraintOffset += bus->constraintSize (oMode);
      offset += bus->objSize (oMode);
    }
  for (auto link : linkList)
    {
      link->setOffset (offset, constraintOffset, oMode);
      constraintOffset += link->constraintSize (oMode);
      offset += link->objSize (oMode);
    }
  for (auto relay : relayList)
    {
      relay->setOffset (offset, constraintOffset, oMode);
      constraintOffset += relay->constraintSize (oMode);
      offset += relay->objSize (oMode);
    }
  offsets.setConstraintOffset (constraintOffset, oMode);
  offsets.setOffset (offset, oMode);

}


void gridAreaOpt::add (gridCoreObject *obj)
{
  if (dynamic_cast<gridArea *> (obj))
    {
      area = static_cast<gridArea *> (obj);
      if (name.empty ())
        {
          name = area->getName ();
        }
      id = area->getUserID ();
      return;
    }
  gridAreaOpt *sa = dynamic_cast<gridAreaOpt *> (obj);
  if (sa)
    {
      return add (sa);
    }

  gridBusOpt *bus = dynamic_cast<gridBusOpt *> (obj);
  if (bus)
    {
      return add (bus);
    }

  gridLinkOpt *lnk = dynamic_cast<gridLinkOpt *> (obj);
  if (lnk)
    {
      return add (lnk);
    }
  gridRelayOpt *relay = dynamic_cast<gridRelayOpt *> (obj);
  if (relay)
    {
      return add (relay);
    }
  throw(invalidObjectException(this));
}

void gridAreaOpt::remove (gridCoreObject *obj)
{
  gridAreaOpt *sa = dynamic_cast<gridAreaOpt *> (obj);
  if (sa)
    {
      return remove (sa);
    }

  gridBusOpt *bus = dynamic_cast<gridBusOpt *> (obj);
  if (bus)
    {
      return remove (bus);
    }

  gridLinkOpt *lnk = dynamic_cast<gridLinkOpt *> (obj);
  if (lnk)
    {
      return remove (lnk);
    }
  gridRelayOpt *relay = dynamic_cast<gridRelayOpt *> (obj);
  if (relay)
    {
      return remove (relay);
    }
  throw(invalidObjectException(this));
}

//TODO:: make this work like gridArea
void gridAreaOpt::add (gridBusOpt *bus)
{
  if (!isMember (bus))
    {
      busList.push_back (bus);
      bus->setParent (this);
      bus->locIndex = static_cast<index_t> (busList.size ()) - 1;
      optObList.insert (bus);
      objectList.push_back (bus);
    }

}

void gridAreaOpt::add (gridAreaOpt *newArea)
{
  if (!isMember (newArea))
    {
      areaList.push_back (newArea);
      newArea->parent = this;
      newArea->locIndex = static_cast<index_t> (areaList.size ()) - 1;
      optObList.insert (newArea);
      objectList.push_back (newArea);
    }
}

// add link
void gridAreaOpt::add (gridLinkOpt *lnk)
{
  if (!isMember (lnk))
    {
      linkList.push_back (lnk);
      lnk->setParent (this);
      lnk->locIndex = static_cast<index_t> (linkList.size ()) - 1;
      optObList.insert (lnk);
      objectList.push_back (lnk);
    }
}


// add link
void gridAreaOpt::add (gridRelayOpt *relay)
{
  if (!isMember (relay))
    {
      relayList.push_back (relay);
      relay->setParent (this);
      relay->locIndex = static_cast<index_t> (relayList.size ()) - 1;
      optObList.insert (relay);
      objectList.push_back (relay);

    }
}

// --------------- remove components ---------------
// remove bus
void gridAreaOpt::remove (gridBusOpt *bus)
{
  if (busList[bus->locIndex]->getID () == bus->getID ())
    {
      busList[bus->locIndex]->setParent (nullptr);
      busList.erase (busList.begin () + bus->locIndex);
      for (auto kk = bus->locIndex; kk < busList.size (); ++kk)
        {
          busList[kk]->locIndex = kk;
        }
      auto oLb = objectList.begin ();
      auto oLe = objectList.end ();
      auto bid = bus->getID ();
      while (oLb != oLe)
        {
          if ((*oLb)->getID () == bid)
            {
              objectList.erase (oLb);
            }
          break;
        }
    }

}

// remove link
void gridAreaOpt::remove (gridLinkOpt *lnk)
{
  if (linkList[lnk->locIndex]->getID () == lnk->getID ())
    {
      linkList[lnk->locIndex]->setParent (nullptr);
      linkList.erase (linkList.begin () + lnk->locIndex);

      for (auto kk = lnk->locIndex; kk < linkList.size (); ++kk)
        {
          linkList[kk]->locIndex = kk;
        }
      auto oLb = objectList.begin ();
      auto oLe = objectList.end ();
      auto lid = lnk->getID ();
      while (oLb != oLe)
        {
          if ((*oLb)->getID () == lid)
            {
              objectList.erase (oLb);
            }
          break;
        }
    }
}

// remove area
void gridAreaOpt::remove (gridAreaOpt *rarea)
{
  if (areaList[rarea->locIndex]->getID () == rarea->getID ())
    {
      areaList[rarea->locIndex]->parent = nullptr;
      areaList.erase (areaList.begin () + rarea->locIndex);
      for (auto kk = rarea->locIndex; kk < areaList.size (); ++kk)
        {
          areaList[kk]->locIndex = kk;
        }
      auto oLb = objectList.begin ();
      auto oLe = objectList.end ();
      auto aid = rarea->getID ();
      while (oLb != oLe)
        {
          if ((*oLb)->getID () == aid)
            {
              objectList.erase (oLb);
            }
          break;
        }
    }

}

// remove area
void gridAreaOpt::remove (gridRelayOpt *relay)
{
  if (relayList[relay->locIndex]->getID () == relay->getID ())
    {
      relayList[relay->locIndex]->setParent (nullptr);
      relayList.erase (relayList.begin () + relay->locIndex);
      for (auto kk = relay->locIndex; kk < relayList.size (); ++kk)
        {
          relayList[kk]->locIndex = kk;
        }
      auto oLb = objectList.begin ();
      auto oLe = objectList.end ();
      auto rid = relay->getID ();
      while (oLb != oLe)
        {
          if ((*oLb)->getID () == rid)
            {
              objectList.erase (oLb);
            }
          break;
        }
    }
}




void gridAreaOpt::setAll (const std::string &type, std::string param, double val, gridUnits::units_t unitType)
{

  if (type == "area")
    {
      set (param, val, unitType);
      for (auto &subarea : areaList)
        {
          subarea->setAll (type, param, val, unitType);
        }
    }
  else if (type == "bus")
    {
      for (auto &bus : busList)
        {
          bus->set (param, val, unitType);
        }
    }
  else if (type == "link")
    {
      for (auto &lnk : linkList)
        {
          lnk->set (param, val, unitType);
        }
    }
  else if (type == "relay")
    {
      for (auto &rel : relayList)
        {
          rel->set (param, val, unitType);
        }
    }
  else if ((type == "gen") || (type == "load") || (type == "generator"))
    {
      for (auto &bus : busList)
        {
          bus->setAll (type, param, val, unitType);
        }
      for (auto &subarea : areaList)
        {
          subarea->setAll (type, param, val, unitType);
        }
    }


}

// set properties
void gridAreaOpt::set (const std::string &param,  const std::string &val)
{
  if (param == "status")
    {
      auto v2 = convertToLowerCase (val);
      if (v2 == "out")
        {
          if (enabled)
            {
              disable ();
            }

        }
      if (v2 == "in")
        {
          if (!enabled)
            {
              enable ();
            }
        }
    }
  else
    {
      gridOptObject::set (param, val);
    }
}

void gridAreaOpt::set (const std::string &param, double val, units_t unitType)
{
 
  if ((param == "voltagetolerance") || (param == "vtol"))
    {

    }
  else if ((param == "anareaeetolerance") || (param == "atol"))
    {

    }
  else
    {
      gridOptObject::set (param, val, unitType);
    }

}




gridCoreObject *gridAreaOpt::find (const std::string &objname) const
{
  gridCoreObject *obj = nullptr;
  if (objname == this->name)
    {
      return const_cast<gridAreaOpt *> (this);
    }
  if (objname == "root")
    {
      if (parent)
        {
          return (parent->find (objname));
        }
      else
        {
          return const_cast<gridAreaOpt *> (this);
        }
    }
  for (auto ob : busList)
    {
      if (objname == ob->getName ())
        {
          obj = ob;
          break;
        }
    }
  if (obj == nullptr)
    {
      for (auto ob : areaList)
        {
          if (objname == ob->getName ())
            {
              obj = ob;
              break;
            }
        }
    }
  return obj;
}

gridCoreObject *gridAreaOpt::getSubObject (const std::string &typeName, index_t num) const
{
  if (typeName == "link")
    {
      return getLink (num - 1);
    }
  else if (typeName == "bus")
    {
      return getBus (num - 1);
    }
  else if (typeName == "area")
    {
      return getArea (num - 1);
    }
  else if (typeName == "relay")
    {
      return getRelay (num - 1);
    }
  else
    {
      return nullptr;
    }
}

gridCoreObject *gridAreaOpt::findByUserID (const std::string &typeName, index_t searchID) const
{
  gridCoreObject *A1;
  if (typeName == "area")
    {
      for (auto &subarea : areaList)
        {
          if (subarea->getUserID () == searchID)
            {
              return subarea;
            }
        }
    }
  if (typeName == "bus")
    {
      for (auto &bus : busList)
        {
          if (bus->getUserID () == searchID)
            {
              return bus;
            }
        }
    }
  else if (typeName == "link")
    {
      for (auto &lnk : linkList)
        {
          if (lnk->getUserID () == searchID)
            {
              return lnk;
            }
        }
    }
  else if (typeName == "relay")
    {
      for (auto &rel : relayList)
        {
          if (rel->getUserID () == searchID)
            {
              return rel;
            }
        }
    }

  else if ((typeName == "gen") || (typeName == "load") || (typeName == "generator"))
    {
      for (auto &bus : busList)
        {
          A1 = bus->findByUserID (typeName, searchID);
          if (A1)
            {
              return A1;
            }
        }
    }
  //if we haven't found it yet search the subareas
  for (auto &subarea : areaList)
    {
      A1 = subarea->findByUserID (typeName, searchID);
      if (A1)
        {
          return A1;
        }
    }
  return nullptr;
}


// check bus members
bool gridAreaOpt::isMember (gridCoreObject *object) const
{
  return optObList.isMember (object);
}

gridOptObject *gridAreaOpt::getBus (index_t x) const
{
  return (x < busList.size ()) ? busList[x] : nullptr;
}

gridOptObject *gridAreaOpt::getLink (index_t x) const
{
  return (x < linkList.size ()) ? linkList[x] : nullptr;
}

gridOptObject *gridAreaOpt::getArea (index_t x) const
{
  return (x < areaList.size ()) ? areaList[x] : nullptr;
}

gridOptObject *gridAreaOpt::getRelay (index_t x) const
{
  return (x < relayList.size ()) ? relayList[x] : nullptr;
}

double gridAreaOpt::get (const std::string &param, gridUnits::units_t unitType) const
{
  double fval = kNullVal;
  size_t ival = kNullLocation;
  if (param == "buscount")
    {
      ival = busList.size ();
    }
  else if (param == "linkcount")
    {
      ival = linkList.size ();
    }
  else if (param == "areacount")
    {
      ival = areaList.size ();
    }
  else if (param == "relaycount")
    {
      ival = relayList.size ();
    }
  else
    {
      fval = gridCoreObject::get (param, unitType);
    }
  return (ival != kNullLocation) ? static_cast<double> (ival) : fval;
}


gridAreaOpt * getMatchingArea (gridAreaOpt *area, gridOptObject *src, gridOptObject *sec)
{
  if (area->getParent () == nullptr)
    {
      return nullptr;
    }
  gridAreaOpt *A2;
  if (area->getParent ()->getID () == src->getID ())    //if this is true then things are easy
    {
      A2 = static_cast<gridAreaOpt *> (sec->getArea (area->locIndex));
    }
  else
    {
      std::vector<int> lkind;
	  gridOptObject *par = dynamic_cast<gridOptObject *> (area->getParent ());
      if (par == nullptr)
        {
          return nullptr;
        }
      lkind.push_back (area->locIndex);
      while (par->getID () != src->getID ())
        {
          lkind.push_back (par->locIndex);
          par = dynamic_cast<gridOptObject *> (par->getParent ());
          if (par == nullptr)
            {
              return nullptr;
            }
        }
      //now work our way backwards through the secondary
      par = sec;
      for (auto kk = lkind.size () - 1; kk > 0; --kk)
        {
          par = static_cast<gridOptObject *> (par->getArea (lkind[kk]));
        }
      A2 = static_cast<gridAreaOpt *> (par->getArea (lkind[0]));

    }
  return A2;
}


