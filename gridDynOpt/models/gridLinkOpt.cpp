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
#include "gridLinkOpt.h"
#include "gridBusOpt.h"
#include "gridAreaOpt.h"
#include "linkModels/gridLink.h"
#include "optObjectFactory.h"
#include "vectorOps.hpp"
#include "vectData.h"
#include "core/gridDynExceptions.h"

#include <cmath>
#include <utility>


static optObjectFactory<gridLinkOpt, gridLink> opLink ("basic", "link");

using namespace gridUnits;

gridLinkOpt::gridLinkOpt (const std::string &objName) : gridOptObject (objName)
{

}

gridLinkOpt::gridLinkOpt (coreObject *obj, const std::string &objName) : gridOptObject (objName),link (dynamic_cast<gridLink *> (obj))
{

  if (link)
    {
      if (name.empty ())
        {
          name = link->getName ();
        }
      id = link->getUserID ();
    }
}

coreObject *gridLinkOpt::clone (coreObject *obj) const
{
  gridLinkOpt *nobj;
  if (obj == nullptr)
    {
      nobj = new gridLinkOpt ();
    }
  else
    {
      nobj = dynamic_cast<gridLinkOpt *> (obj);
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



void gridLinkOpt::objectInitializeA (unsigned long /*flags*/)
{


}


void gridLinkOpt::loadSizes (const optimMode &oMode)
{
  auto oo = offsets.getOffsets (oMode);
  oo->reset ();
  switch (oMode.flowMode)
    {
    case flowModel_t::none:
      oo->local.contSize = 0;
      break;
    case flowModel_t::transport:
      oo->local.contSize = 1;
      break;
    case flowModel_t::dc:
      oo->local.contSize = 0;
      oo->local.constraintsSize = 1;
      break;
    case flowModel_t::ac:
      oo->local.contSize = 0;
      oo->local.constraintsSize = 1;
      break;
    }
  oo->localLoad (true);
}



void gridLinkOpt::add (coreObject *obj)
{
  if (dynamic_cast<gridLink *> (obj))
    {

      link = static_cast<gridLink *> (obj);
      if (name.empty ())
        {
          name = link->getName ();
        }
      id = link->getUserID ();
    }
  else
  {
	  throw(invalidObjectException(this));
  }

}



void gridLinkOpt::remove (coreObject *)
{

  
}

void gridLinkOpt::setValues (const optimData *, const optimMode &)
{
}
//for saving the state
void gridLinkOpt::guess (double /*ttime*/, double /*val*/[], const optimMode &)
{

}

void gridLinkOpt::getVariableType (double /*sdata*/[], const optimMode &)
{

}


void gridLinkOpt::getTols (double /*tols*/[], const optimMode &)
{

}
void gridLinkOpt::valueBounds (double /*ttime*/, double /*upperLimit*/[], double /*lowerLimit*/[], const optimMode &)
{

}

void gridLinkOpt::linearObj (const optimData *, vectData<double> * /*linObj*/, const optimMode &)
{

}
void gridLinkOpt::quadraticObj (const optimData *, vectData<double> * /*linObj*/, vectData<double> * /*quadObj*/, const optimMode &)
{

}

void gridLinkOpt::constraintValue (const optimData *, double /*cVals*/[], const optimMode &)
{
}
void gridLinkOpt::constraintJacobianElements (const optimData *, matrixData<double> &, const optimMode &)
{
}

double gridLinkOpt::objValue (const optimData *, const optimMode &)
{
  double cost = 0;

  return cost;
}

void gridLinkOpt::gradient (const optimData *, double /*deriv*/[], const optimMode &)
{

}
void gridLinkOpt::jacobianElements (const optimData *, matrixData<double> &, const optimMode &)
{

}
void gridLinkOpt::getConstraints (const optimData *, matrixData<double> & /*cons*/, double /*upperLimit*/[], double /*lowerLimit*/[], const optimMode &)
{

}
void gridLinkOpt::getObjName (stringVec & /*objNames*/, const optimMode &, const std::string & /*prefix*/)
{

}


void gridLinkOpt::disable ()
{
  enabled = false;

}

void gridLinkOpt::setOffsets (const optimOffsets & /*newOffset*/, const optimMode & /*oMode*/)
{




}


// destructor
gridLinkOpt::~gridLinkOpt ()
{

}


// set properties
void gridLinkOpt::set (const std::string &param,  const std::string &val)
{
  if (param == "#")
    {

    }
  else
    {
      gridOptObject::set (param, val);
    }
}

void gridLinkOpt::set (const std::string &param, double val, units_t unitType)
{

  if ((param == "voltagetolerance") || (param == "vtol"))
    {

    }
  else if ((param == "angleetolerance") || (param == "atol"))
    {

    }
  else
    {
      gridOptObject::set (param, val, unitType);
    }


}




coreObject *gridLinkOpt::find (const std::string &objname) const
{
  if ((objname == this->name) || (objname == "link"))
    {
      return const_cast<gridLinkOpt *> (this);
    }
  if ((objname == "b1") || (objname == "bus1")||(objname == "bus"))
    {
      return B1;
    }
  if ((objname == "b2") || (objname == "bus2"))
    {
      return B2;
    }
  if (objname == "root")
    {
      if (parent)
        {
          return (parent->find (objname));
        }
      else
        {
          return const_cast<gridLinkOpt *> (this);
        }
    }

  return (parent->find (objname));
}

coreObject *gridLinkOpt::getSubObject (const std::string &typeName, index_t num) const
{
  if (typeName == "bus")
    {
      if (num == 1)
        {
          return B1;
        }
      else if (num == 2)
        {
          return B2;
        }
    }
  return nullptr;
}

coreObject *gridLinkOpt::findByUserID (const std::string &typeName, index_t searchID) const
{
  if (typeName == "bus")
    {
      if (B1->getUserID () == searchID)
        {
          return B1;
        }
      else if (B2->getUserID () == searchID)
        {
          return B2;
        }
    }

  return nullptr;
}



gridOptObject *gridLinkOpt::getBus (index_t x) const
{
  if (x == 1)
    {
      return B1;
    }
  else if (x == 2)
    {
      return B2;
    }
  else
    {
      return nullptr;
    }

}

gridOptObject *gridLinkOpt::getArea (index_t /*index*/) const
{
  return dynamic_cast<gridOptObject *> (parent);
}


double gridLinkOpt::get (const std::string &param, gridUnits::units_t unitType) const
{
  double val = kNullVal;
  if (param[0] == '#')
    {
    }
  else
    {
      val = gridOptObject::get (param, unitType);
    }
  return val;
}






