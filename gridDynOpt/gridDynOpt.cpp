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

#include "gridDynOpt.h"
#include "models/gridAreaOpt.h"
#include "objectFactoryTemplates.h"
#include "optObjectFactory.h"
#include "gridOptObjects.h"
#include "stringOps.h"
#include "gridCoreTemplates.h"
//system headers


#include <cstdio>
#include <fstream>
#include <iostream>
#include <cmath>
#include <map>


static typeFactory< gridDynOptimization> gfo ("simulation", stringVec { "optimization", "optim" });

gridDynOptimization::gridDynOptimization (const std::string &simName) : gridDynSimulation (simName)
{
  // defaults
  areaOpt = new gridAreaOpt (this);
}

gridDynOptimization::~gridDynOptimization ()
{
  delete areaOpt;
}

gridCoreObject *gridDynOptimization::clone (gridCoreObject *obj) const
{
  gridDynOptimization *sim = cloneBase<gridDynOptimization, gridDynSimulation> (this, obj);
  if (sim == nullptr)
    {
      return obj;
    }

  return sim;
}

void gridDynOptimization::setupOptOffsets (const optimMode &oMode, int setupMode)
{

  if (setupMode == 0)      //no distinction between Voltage, angle, and others
    {
      areaOpt->setOffset (1, 0, oMode);
      return;
    }
  optimOffsets baseOffset;
  if (setupMode == 1) //use all the distinct categories
    {
      baseOffset.setOffset (1);
      baseOffset.constraintOffset = 0;
    }
  else if (setupMode == 2) //discriminate continuous and discrete objective variables
    {
      baseOffset.constraintOffset = 0;
      baseOffset.contOffset = 1;
      baseOffset.intOffset = 0;
    }

  //call the area setOffset function to distribute the offsets
  areaOpt->setOffsets (baseOffset, oMode);


}



// --------------- set properties ---------------
int gridDynOptimization::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;
  std::string temp;
  if (param == "flags")
    {
      auto v = splitline (val);
      int ot;
      for (auto &flagstr : v)
        {
          ot = setFlag (flagstr, true);
          if (ot != PARAMETER_FOUND)
            {
              std::cout << "Flag " << flagstr << " not found\n";
            }
        }
    }
  else if ((param == "defaultoptmode") || (param == "defaultopt"))
    {

      auto ocf = coreOptObjectFactory::instance ();
      if (ocf->isValidType (val))
        {
          defaultOptMode = val;
          ocf->setDefaultType (val);
        }
    }

  else if (param == "optimization_mode")
    {
      /*default_solution,
  dcflow_only, powerflow_only, iterated_powerflow, contingency_powerflow,
  steppedP, steppedPQ, dynamic, dyanmic_contingency,*/
      temp = val;
      makeLowerCase (temp);
      if ((temp == "dcopf") || (temp == "opf"))
        {
          optimization_mode = DCOPF;
        }
      else if ((temp == "acopf") || (temp == "ac"))
        {
          optimization_mode = ACOPF;
        }
      else if (temp == "bidstack")
        {
          optimization_mode = bidstack;
        }
      else
        {
          LOG_WARNING ("unknown optimization mode " + temp);
        }
    }
  else
    {
      out = gridDynSimulation::set (param, val);
    }


  return out;
}

int gridDynOptimization::setFlag (const std::string &flag, bool val)
{
  int out = PARAMETER_FOUND;
  //int nval = static_cast<int> (val);
  /*
  constraints_disabled = 1,
  sparse_solver = 2,
  threads_enabled = 3,
  ignore_voltage_limits = 4,
  power_adjust_enabled = 5,
  dcFlow_initialization = 6,*/
  if (flag[0] == '#')
    {

    }
  else
    {
      out = gridDynSimulation::setFlag (flag, val);
    }
  return out;
}
int gridDynOptimization::setFlags (size_t param, int val)
{
  if (param > 32)
    {
      return PARAMETER_NOT_FOUND;
    }

  controlFlags.set (param, (val > 0));
  return PARAMETER_FOUND;

}

int gridDynOptimization::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;

  if (param == "optimtol")
    {
      tols.rtol = val;
    }

  else
    {
      //out = setFlags (param, val);
      out = gridDynSimulation::set (param, val, unitType);
      if (out == PARAMETER_NOT_FOUND)
        {
          out = setFlag (param, (val > 0.1));
        }

    }
  return out;
}


double gridDynOptimization::get (const std::string &param, gridUnits::units_t unitType) const
{
  double val;
  if (param == "voltagetolerance")
    {
      val = tols.voltageTolerance;
    }
  if (param == "angletolerance")
    {
      val = tols.angleTolerance;
    }
  else
    {
      val = gridDynSimulation::get (param,unitType);
    }
  return val;
}


gridCoreObject * gridDynOptimization::find (const std::string &objname) const
{
  if (objname == "optroot")
    {
      return areaOpt;
    }
  else if (objname.substr (0, 3) == "opt")
    {
      return areaOpt->find (objname.substr (3));
    }
  else
    {
      return gridDynSimulation::find (objname);
    }

}

gridCoreObject * gridDynOptimization::getSubObject (const std::string &typeName, index_t num) const
{
  if (typeName.substr (0, 3) == "opt")
    {
      return areaOpt->getSubObject (typeName.substr (3), num);
    }
  else
    {
      return gridDynSimulation::getSubObject (typeName, num);
    }
}
gridCoreObject * gridDynOptimization::findByUserID (const std::string &typeName, index_t searchID) const
{
  if (typeName.substr (0, 3) == "opt")
    {
      return areaOpt->findByUserID (typeName.substr (3), searchID);
    }
  else
    {
      return gridDynSimulation::findByUserID (typeName, searchID);
    }
}


gridOptObject * gridDynOptimization::getOptData (gridCoreObject *obj)
{
  if (obj)
    {
      gridCoreObject *nobj = areaOpt->find (obj->getName ());
      if (nobj)
        {
          return static_cast<gridOptObject *> (nobj);
        }
      else
        {
          return nullptr;
        }
    }
  else
    {
      return areaOpt;
    }
}

gridOptObject *gridDynOptimization::makeOptObjectPath (gridCoreObject *obj)
{
  gridOptObject *oo = getOptData (obj);
  if (oo)
    {
      return oo;
    }
  else
    {
      if (obj->getParent ())
        {
          auto oop = makeOptObjectPath (obj->getParent ());
          oo = coreOptObjectFactory::instance ()->createObject (obj);
          oop->add (oo);
          return oo;
        }
      else
        {
          return nullptr;
        }
    }
}

optimizerInterface *gridDynOptimization::updateOptimizer (const optimMode &oMode)
{

  oData[oMode.offsetIndex] = makeOptimizer (this, oMode);
  optimizerInterface *od = oData[oMode.offsetIndex].get ();

  return od;
}

