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

#include "gridOptObjects.h"
#include "core/coreObjectTemplates.h"
#include "stringOps.h"
#include <cstdio>
#include <iostream>

gridOptObject::gridOptObject (const std::string &objName) : coreObject (objName)
{

}

coreObject *gridOptObject::clone (coreObject *obj) const
{
  gridOptObject *nobj = cloneBase<gridOptObject, coreObject> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }
  nobj->numParams = numParams;
  nobj->optFlags = optFlags;
  return nobj;
}


// size getter functions
count_t gridOptObject::objSize (const optimMode &oMode)
{
  count_t size = 0;
  optimOffsets *oo = offsets.getOffsets (oMode);
  if (oo->loaded)
    {
      size = oo->total.genSize + oo->total.vSize + oo->total.aSize + oo->total.qSize + oo->total.contSize + oo->total.intSize;
    }
  else
    {
      loadSizes (oMode);
      size = oo->total.genSize + oo->total.qSize + oo->total.vSize + oo->total.aSize + oo->total.contSize + oo->total.intSize;
    }
  return size;
}

count_t gridOptObject::contObjSize (const optimMode &oMode)
{
  count_t size = 0;
  optimOffsets *oo = offsets.getOffsets (oMode);
  if (oo->loaded)
    {
      size = oo->total.genSize + oo->total.qSize + oo->total.vSize + oo->total.aSize + oo->total.contSize;
    }
  else
    {
      loadSizes (oMode);
      size = oo->total.genSize + oo->total.qSize + oo->total.vSize + oo->total.aSize + oo->total.contSize;
    }
  return size;
}

count_t gridOptObject::intObjSize (const optimMode &oMode)
{
  count_t size = 0;
  optimOffsets *oo = offsets.getOffsets (oMode);
  if (oo->loaded)
    {
      size = oo->total.intSize;
    }
  else
    {
      loadSizes (oMode);
      size = oo->total.intSize;
    }
  return size;
}

count_t gridOptObject::genSize (const optimMode &oMode)
{
  optimOffsets *oo = offsets.getOffsets (oMode);
  if (!oo->loaded)
    {
	  loadSizes(oMode);
      
    }
    return oo->total.genSize;
}

count_t gridOptObject::qSize (const optimMode &oMode)
{
  count_t size = 0;
  optimOffsets *oo = offsets.getOffsets (oMode);
  if (oo->loaded)
    {
      size = oo->total.qSize;
    }
  else
    {
      loadSizes (oMode);
      size = oo->total.qSize;
    }
  return size;
}

count_t gridOptObject::vSize (const optimMode &oMode)
{
  count_t size = 0;
  optimOffsets *oo = offsets.getOffsets (oMode);
  if (oo->loaded)
    {
      size = oo->total.vSize;
    }
  else
    {
      loadSizes (oMode);
      size = oo->total.vSize;
    }
  return size;
}

count_t gridOptObject::aSize (const optimMode &oMode)
{
  count_t size = 0;
  optimOffsets *oo = offsets.getOffsets (oMode);
  if (oo->loaded)
    {
      size = oo->total.aSize;
    }
  else
    {
      loadSizes (oMode);
      size = oo->total.aSize;
    }
  return size;
}

count_t gridOptObject::constraintSize (const optimMode &oMode)
{
  count_t size = 0;
  optimOffsets *oo = offsets.getOffsets (oMode);
  if (oo->loaded)
    {
      size = oo->total.constraintsSize;
    }
  else
    {
      loadSizes (oMode);
      size = oo->total.constraintsSize;
    }
  return size;
}

// size getter functions
count_t gridOptObject::objSize (const optimMode &oMode) const
{
  const optimOffsets *oo = offsets.getOffsets (oMode);
  return (oo->total.genSize + oo->total.vSize + oo->total.aSize + oo->total.qSize + oo->total.contSize + oo->total.intSize);
}

count_t gridOptObject::contObjSize (const optimMode &oMode) const
{
  const optimOffsets *oo = offsets.getOffsets (oMode);
  return (oo->total.genSize + oo->total.qSize + oo->total.vSize + oo->total.aSize + oo->total.contSize);
}

count_t gridOptObject::intObjSize (const optimMode &oMode) const
{
  const optimOffsets *oo = offsets.getOffsets (oMode);
  return oo->total.intSize;
}

count_t gridOptObject::genSize (const optimMode &oMode) const
{
  const optimOffsets *oo = offsets.getOffsets (oMode);
  return oo->total.genSize;
}

count_t gridOptObject::qSize (const optimMode &oMode) const
{
  const optimOffsets *oo = offsets.getOffsets (oMode);
  return oo->total.qSize;
}

count_t gridOptObject::vSize (const optimMode &oMode) const
{

  const optimOffsets *oo = offsets.getOffsets (oMode);
  return oo->total.vSize;
}

count_t gridOptObject::aSize (const optimMode &oMode) const
{
  const optimOffsets *oo = offsets.getOffsets (oMode);
  return oo->total.aSize;
}

count_t gridOptObject::constraintSize (const optimMode &oMode) const
{
  const optimOffsets *oo = offsets.getOffsets (oMode);

  return oo->total.constraintsSize;
}


void gridOptObject::dynInitializeA (unsigned long flags)
{
  dynObjectInitializeA (flags);

}


void gridOptObject::dynInitializeB (unsigned long flags)
{
  dynObjectInitializeB (flags);
  optFlags.set (opt_initialized);

}

void gridOptObject::setOffsets (const optimOffsets &newOffsets, const optimMode &oMode)
{
  optimOffsets *oo = offsets.getOffsets (oMode);
  if (!oo->loaded)
    {
      loadSizes (oMode);
    }
  oo->setOffsets (newOffsets);
}

void gridOptObject::setOffset (index_t offset, index_t constraintOffset, const optimMode &oMode)
{
  offsets.setOffset (offset, oMode);
  offsets.setConstraintOffset (constraintOffset, oMode);
}

void gridOptObject::set (const std::string &param, const std::string &val)
{
  if (param == "status")
    {
      auto v2 = convertToLowerCase (val);
      if (val == "out")
        {
          if (isEnabled())
            {
              disable ();
            }

        }
      else if (val == "in")
        {
          if (!isEnabled())
            {
              enable ();
            }
        }
      else
        {
          coreObject::set (param, val);
        }
    }
  else
    {
      coreObject::set (param, val);
    }
}

void gridOptObject::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "#")
    {
    }
  else
    {
      coreObject::set (param, val, unitType);
    }

}


void gridOptObject::getVariableType (double sdata[], const optimMode &oMode)
{
  if (offsets.isLoaded (oMode))
    {
      int intIndex = offsets.getIntOffset (oMode);
      auto iSize = intObjSize (oMode);
      for (size_t kk = 0; kk < iSize; ++kk)
        {
          sdata[intIndex + kk] = INTEGER_OBJECTIVE_VARIABLE;
        }
    }
}

void gridOptObject::getObjName (stringVec &stNames, const optimMode &oMode, const std::string &prefix)
{
  auto os = offsets.getOffsets (oMode);
  //angle variables
  if (stNames.size () < os->total.aSize + os->aOffset + 1)
    {
      stNames.resize (os->total.aSize + os->aOffset + 1);
    }
  for (size_t bb = 0; bb < os->total.aSize; ++bb)
    {
      if (prefix.empty ())
        {
          stNames[os->aOffset + bb] = getName() + ":angle_" + std::to_string (bb);
        }
      else
        {
          stNames[os->aOffset + bb] = prefix + "::" + getName() + ":angle_" + std::to_string (bb);
        }
    }
  //voltage variables
  if (stNames.size () < os->total.vSize + os->vOffset + 1)
    {
      stNames.resize (os->total.vSize + os->vOffset + 1);
    }
  for (size_t bb = 0; bb < os->total.vSize; ++bb)
    {
      if (prefix.empty ())
        {
          stNames[os->vOffset + bb] = getName() + ":voltage_" + std::to_string (bb);
        }
      else
        {
          stNames[os->vOffset + bb] = prefix + "::" + getName() + ":voltage_" + std::to_string (bb);
        }
    }
  //real power variables
  if (stNames.size () < os->total.genSize + os->gOffset + 1)
    {
      stNames.resize (os->total.genSize + os->gOffset + 1);
    }
  for (size_t bb = 0; bb < os->total.genSize; ++bb)
    {
      if (prefix.empty ())
        {
          stNames[os->gOffset + bb] = getName() + ":power_" + std::to_string (bb);
        }
      else
        {
          stNames[os->gOffset + bb] = prefix + "::" + getName() + ":power_" + std::to_string (bb);
        }
    }
  //angle variables
  if (stNames.size () < os->total.qSize + os->qOffset + 1)
    {
      stNames.resize (os->total.qSize + os->qOffset + 1);
    }
  for (size_t bb = 0; bb < os->total.qSize; ++bb)
    {
      if (prefix.empty ())
        {
          stNames[os->qOffset + bb] = getName() + ":reactive_power_" + std::to_string (bb);
        }
      else
        {
          stNames[os->qOffset + bb] = prefix + "::" + getName() + ":reactive_power_" + std::to_string (bb);
        }
    }
  //other continuous variables
  if (stNames.size () < os->total.contSize + os->contOffset + 1)
    {
      stNames.resize (os->total.contSize + os->contOffset + 1);
    }
  for (size_t bb = 0; bb < os->total.contSize; ++bb)
    {
      if (prefix.empty ())
        {
          stNames[os->contOffset + bb] = getName() + ":continuous_" + std::to_string (bb);
        }
      else
        {
          stNames[os->contOffset + bb] = prefix + "::" + getName() + ":continuous_" + std::to_string (bb);
        }
    }
  //integer variables
  if (stNames.size () < os->total.intSize + os->intOffset + 1)
    {
      stNames.resize (os->total.intSize + os->intOffset + 1);
    }
  for (size_t bb = 0; bb < os->total.intSize; ++bb)
    {
      if (prefix.empty ())
        {
          stNames[os->intOffset + bb] = getName() + ":continuous_" + std::to_string (bb);
        }
      else
        {
          stNames[os->intOffset + bb] = prefix + "::" + getName() + ":continuous_" + std::to_string (bb);
        }
    }
}


void gridOptObject::dynObjectInitializeA (unsigned long /*flags*/)
{
}

void gridOptObject::dynObjectInitializeB (unsigned long /*flags*/)
{
}


void gridOptObject::loadSizes (const optimMode &)
{
}

void gridOptObject::setValues (const optimData &, const optimMode &)
{
}

void gridOptObject::guess (double /*ttime*/, double /*val*/[], const optimMode &)
{
}

void gridOptObject::getTols (double /*tols*/[], const optimMode &)
{
}


void gridOptObject::valueBounds (double /*ttime*/, double /*upLimit*/[], double /*lowerLimit*/[], const optimMode &)
{
}

void gridOptObject::linearObj (const optimData &, vectData<double> & /*linObj*/, const optimMode &)
{
}

void gridOptObject::quadraticObj (const optimData &, vectData<double> & /*linObj*/, vectData<double> & /*quadObj*/, const optimMode &)
{
}

double gridOptObject::objValue (const optimData &, const optimMode &)
{
  return 0;
}

void gridOptObject::gradient (const optimData &, double /*grad*/[], const optimMode &)
{
}

void gridOptObject::jacobianElements (const optimData &, matrixData<double> &, const optimMode &)
{
}

void gridOptObject::getConstraints (const optimData &, matrixData<double> & /*cons*/, double /*upperLimit*/[], double /*lowerLimit*/[], const optimMode &)
{
}

void gridOptObject::constraintValue (const optimData &, double /*cVals*/[], const optimMode &)
{
}

void gridOptObject::constraintJacobianElements (const optimData &, matrixData<double> &, const optimMode &)
{
}

void gridOptObject::hessianElements(const optimData &, matrixData<double> &, const optimMode &)
{

}

gridOptObject * gridOptObject::getBus (index_t /*index*/) const
{
  return nullptr;
}

gridOptObject * gridOptObject::getArea (index_t /*index*/) const
{
  return nullptr;
}

gridOptObject * gridOptObject::getLink (index_t /*index*/) const
{
  return nullptr;
}

gridOptObject * gridOptObject::getRelay(index_t /*index*/) const
{
	return nullptr;
}

void printObjStateNames (gridOptObject *obj,const optimMode &oMode)
{
  std::vector < std::string> sNames;
  obj->getObjName (sNames, oMode);
  int kk = 0;
  for (auto &sn : sNames)
    {
      std::cout << kk++ << " " << sn << '\n';
    }
}

