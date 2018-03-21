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

// headers
#include "gridLoadOpt.h"
#include "griddyn/loads/zipLoad.h"
#include "gridBusOpt.h"
#include "../optObjectFactory.h"
#include "utilities/vectorOps.hpp"
#include "utilities/vectData.hpp"
#include "core/coreExceptions.h"

#include <cmath>
#include <utility>


namespace griddyn
{
using namespace gridUnits;

static optObjectFactory<gridLoadOpt, zipLoad> opLoad ("basic", "load");

gridLoadOpt::gridLoadOpt (const std::string &objName) : gridOptObject (objName)
{

}

gridLoadOpt::gridLoadOpt (coreObject *obj, const std::string &objName) : gridOptObject (objName),load (dynamic_cast<zipLoad *> (obj))
{
  if (load)
    {
      if (getName().empty ())
        {
          setName(load->getName ());
        }
      setUserID(load->getUserID ());
    }
}

coreObject *gridLoadOpt::clone (coreObject *obj) const
{
  gridLoadOpt *nobj;
  if (obj == nullptr)
    {
      nobj = new gridLoadOpt ();
    }
  else
    {
      nobj = dynamic_cast<gridLoadOpt *> (obj);
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


void gridLoadOpt::add (coreObject *obj)
{
  if (dynamic_cast<zipLoad *> (obj))
    {
      load = static_cast<zipLoad *> (obj);
      if (getName().empty ())
        {
          setName(load->getName ());
        }
      setUserID(load->getUserID ());
    }
  else
  {
	  throw(unrecognizedObjectException(this));
  }
}

count_t gridLoadOpt::objSize (const optimMode &)
{
  count_t objs = 0;

  return objs;
}
count_t gridLoadOpt::contObjSize (const optimMode &)
{
  count_t objs = 0;

  return objs;
}

count_t gridLoadOpt::intObjSize (const optimMode &)
{
  count_t objs = 0;

  return objs;
}

count_t gridLoadOpt::constraintSize (const optimMode &oMode)
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

void gridLoadOpt::dynObjectInitializeA (std::uint32_t /*flags*/)
{


}


void gridLoadOpt::setValues (const optimData &, const optimMode &)
{
}
//for saving the state
void gridLoadOpt::guessState (double /*time*/, double /*val*/[], const optimMode &)
{

}

void gridLoadOpt::getVariableType (double /*sdata*/[], const optimMode &)
{

}


void gridLoadOpt::getTols (double /*tols*/[], const optimMode &)
{

}
void gridLoadOpt::valueBounds (double /*time*/, double /*upperLimit*/[], double /*lowerLimit*/[], const optimMode &)
{

}

void gridLoadOpt::linearObj (const optimData &, vectData<double> & /*linObj*/, const optimMode &)
{

}
void gridLoadOpt::quadraticObj (const optimData &, vectData<double> & /*linObj*/, vectData<double> & /*quadObj*/, const optimMode &)
{

}

void gridLoadOpt::constraintValue (const optimData &, double /*cVals*/[], const optimMode &)
{
}
void gridLoadOpt::constraintJacobianElements (const optimData &, matrixData<double> & /*md*/, const optimMode &)
{
}

double gridLoadOpt::objValue (const optimData &, const optimMode &)
{
  double cost = 0;

  return cost;
}

void gridLoadOpt::gradient (const optimData &, double /*deriv*/[], const optimMode &)
{

}
void gridLoadOpt::jacobianElements (const optimData &, matrixData<double> & /*md*/, const optimMode &)
{

}
void gridLoadOpt::getConstraints (const optimData &, matrixData<double> & /*cons*/, double /*upperLimit*/[], double /*lowerLimit*/[], const optimMode &)
{

}
void gridLoadOpt::getObjName (stringVec & /*objNames*/, const optimMode &, const std::string & /*prefix*/)
{

}



// destructor
gridLoadOpt::~gridLoadOpt ()
{

}



// set properties
void gridLoadOpt::set (const std::string &param,  const std::string &val)
{

  if (param[0] == '#')
    {

    }
  else
    {
      gridOptObject::set (param, val);
    }
}

void gridLoadOpt::set (const std::string &param, double val, units_t unitType)
{
  if (param[0] == '#')
    {

    }
  else
    {
      gridOptObject::set (param, val, unitType);
    }

}




double gridLoadOpt::get (const std::string &param, gridUnits::units_t unitType) const
{
  double val = kNullVal;
  if (param == "#")
    {

    }
  else
    {
      val = gridOptObject::get (param,unitType);
    }
  return val;
}

gridOptObject *gridLoadOpt::getBus (index_t /*index*/) const
{
  return bus;
}

gridOptObject *gridLoadOpt::getArea (index_t index) const
{
  return bus->getArea (index);
}

}// namespace griddyn