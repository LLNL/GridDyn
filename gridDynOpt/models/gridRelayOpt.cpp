
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
#include "gridRelayOpt.h"
#include "optObjectFactory.h"
#include "vectorOps.hpp"
#include "vectData.h"
#include "relays/gridRelay.h"
#include <cmath>
#include <utility>

static optObjectFactory<gridRelayOpt, gridRelay> opRelay ("basic", "relay");

using namespace gridUnits;

gridRelayOpt::gridRelayOpt (const std::string &objName) : gridOptObject (objName)
{

}

gridRelayOpt::gridRelayOpt (gridCoreObject *obj, const std::string &objName) : gridOptObject (objName),relay (dynamic_cast<gridRelay *> (obj))
{
  if (relay)
    {
      if (name.empty ())
        {
          name = relay->getName ();
        }
      id = relay->getUserID ();
    }

}

gridCoreObject *gridRelayOpt::clone (gridCoreObject *obj) const
{
  gridRelayOpt *nobj;
  if (obj == nullptr)
    {
      nobj = new gridRelayOpt ();
    }
  else
    {
      nobj = dynamic_cast<gridRelayOpt *> (obj);
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



  return nobj;
}


int gridRelayOpt::add (gridCoreObject *obj)
{
  if (dynamic_cast<gridRelay *> (obj))
    {
      relay = static_cast<gridRelay *> (obj);
      if (name.empty ())
        {
          name = relay->getName ();
        }
      id = relay->getUserID ();
      return OBJECT_ADD_SUCCESS;
    }
  return OBJECT_NOT_RECOGNIZED;
}

count_t gridRelayOpt::objSize (const optimMode &)
{
  count_t objs = 0;

  return objs;
}
count_t gridRelayOpt::contObjSize (const optimMode &)
{
  count_t objs = 0;

  return objs;
}

count_t gridRelayOpt::intObjSize (const optimMode &)
{
  count_t objs = 0;

  return objs;
}

count_t gridRelayOpt::constraintSize (const optimMode &oMode)
{
  count_t objs = 0;
  switch (oMode.linMode)
    {
    case linearityMode_t::linear:
    case linearityMode_t::quadratic:
    case linearityMode_t::nonlinear:
    default:
      objs = 0;
    }

  return objs;
}

void gridRelayOpt::objectInitializeA (unsigned long /*flags*/)
{


}



int gridRelayOpt::remove (gridCoreObject *)
{

  return OBJECT_REMOVE_FAILURE;
}

void gridRelayOpt::setValues (const optimData *, const optimMode &)
{
}
//for saving the state
void gridRelayOpt::guess (double /*ttime*/, double /*val*/[], const optimMode &)
{

}

void gridRelayOpt::getVariableType (double /*sdata*/[], const optimMode &)
{

}


void gridRelayOpt::getTols (double /*tols*/[], const optimMode &)
{

}
void gridRelayOpt::valueBounds (double /*ttime*/, double /*upperLimit*/[], double /*lowerLimit*/[], const optimMode &)
{

}

void gridRelayOpt::linearObj (const optimData *, vectData * /*linObj*/, const optimMode &)
{

}
void gridRelayOpt::quadraticObj (const optimData *, vectData * /*linObj*/, vectData * /*quadObj*/, const optimMode &)
{

}

void gridRelayOpt::constraintValue (const optimData *, double /*cVals*/[], const optimMode &)
{
}
void gridRelayOpt::constraintJacobianElements (const optimData *, arrayData<double> *, const optimMode &)
{
}

double gridRelayOpt::objValue (const optimData *, const optimMode &)
{
  double cost = 0;

  return cost;
}

void gridRelayOpt::derivative (const optimData *, double /*deriv*/[], const optimMode &)
{

}
void gridRelayOpt::jacobianElements (const optimData *, arrayData<double> *, const optimMode &)
{

}
void gridRelayOpt::getConstraints ( const optimData *, arrayData<double> * /*cons*/, double /*upperLimit*/[], double /*lowerLimit*/[], const optimMode &)
{

}
void gridRelayOpt::getObjName (stringVec & /*objNames*/, const optimMode &, const std::string & /*prefix*/)
{

}


void gridRelayOpt::disable ()
{
  enabled = false;

}

void gridRelayOpt::setOffsets (const optimOffsets & /*newOffset*/, const optimMode & /*oMode*/)
{




}

// destructor
gridRelayOpt::~gridRelayOpt ()
{

}



// set properties
int gridRelayOpt::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;

  if (param == "#")
    {

    }
  else
    {
      out = gridOptObject::set (param, val);
    }
  return out;
}

int gridRelayOpt::set (const std::string &param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;

  if ((param == "voltagetolerance") || (param == "vtol"))
    {

    }
  else if ((param == "angleetolerance") || (param == "atol"))
    {

    }
  else
    {
      out = gridOptObject::set (param, val, unitType);
    }


  return out;
}




gridCoreObject *gridRelayOpt::find (const std::string &objname) const
{
  gridCoreObject *obj = nullptr;
  if ((objname == this->name)|| (objname == "relay"))
    {
      return const_cast<gridRelayOpt *> (this);
    }
  if (objname == "root")
    {
      if (parent)
        {
          return (parent->find (objname));
        }
      else
        {
          return const_cast<gridRelayOpt *> (this);
        }
    }


  return obj;
}

gridCoreObject *gridRelayOpt::getSubObject (const std::string &typeName, index_t /*num*/) const
{
  if (typeName == "target")
    {
      return nullptr;
    }

  else
    {
      return nullptr;
    }
}

gridCoreObject *gridRelayOpt::findByUserID (const std::string &typeName, index_t searchID) const
{
  if (typeName == "relay")
    {
      if (searchID == id)
        {
          return const_cast<gridRelayOpt *> (this);
        }

    }
  return nullptr;
}



double gridRelayOpt::get (const std::string &param, gridUnits::units_t unitType) const
{
  double val = kNullVal;
  if (param[0] == '#')
    {

    }
  else
    {
      val = gridOptObject::get (param,unitType);
    }
  return val;
}






