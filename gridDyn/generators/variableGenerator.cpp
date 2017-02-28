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

#include "variableGenerator.h"
#include "gridBus.h"
#include "core/coreObjectTemplates.h"
#include "sourceModels/gridSource.h"
#include "submodels/gridControlBlocks.h"

using namespace gridUnits;


variableGenerator::variableGenerator (const std::string &objName) : gridDynGenerator (objName)
{
  opFlags[variable_generation] = true;
  opFlags.reset(adjustable_P);
  opFlags.reset(local_power_control);
}

variableGenerator::variableGenerator (dynModel_t dynModel, const std::string &objName) : gridDynGenerator (dynModel,objName)
{
  opFlags[variable_generation] = true;
  opFlags.reset(adjustable_P);
  opFlags.reset(local_power_control);
}

coreObject *variableGenerator::clone (coreObject *obj) const
{
  variableGenerator *gen = cloneBase<variableGenerator, gridDynGenerator> (this, obj);
  if (!(gen))
    {
      return obj;
    }

  gen->mp_Vcutout = mp_Vcutout;
  gen->mp_Vmax = mp_Vmax;
  return gen;
}

variableGenerator::~variableGenerator ()
{

}

// initial conditions of dynamic states


// initial conditions of dynamic states
void variableGenerator::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
  gridDynGenerator::dynObjectInitializeB (inputs,desiredOutput,fieldSet);
  IOdata args2 {
    P
  };
  IOdata inputSet (4);
  if (m_source)
    {
      m_source->dynInitializeB (inputs, { 0.0 }, inputSet);
    }
  if (m_cBlock)
    {
      m_cBlock->dynInitializeB (inputs, { 0.0 }, inputSet);
    }



}

void variableGenerator::add (coreObject *obj)
{
  if (dynamic_cast<gridSubModel *> (obj))
    {
      add (static_cast<gridSubModel *> (obj));
    }
  else
    {
	  gridDynGenerator::add(obj);
    }
}


void variableGenerator::add (gridSubModel *obj)
{
  if (dynamic_cast<gridSource *> (obj))
    {
      if (m_source)
        {
          if (obj->getID () == m_source->getID ())
            {
			  return;
            }
          else
            {
			  gridObject::remove(m_source);
 
            }
        }
      m_source = static_cast<gridSource *> (obj);
      m_source->locIndex = source_loc;
      
      obj->set ("basefreq",m_baseFreq);
      addSubObject(obj);
    }
  else if (dynamic_cast<basicBlock *> (obj))
    {
      if (m_cBlock)
        {
          if (obj->getID () == m_cBlock->getID ())
            {
              return;
            }
          else
            {
			  gridObject::remove(m_cBlock);
            }
        }
      m_cBlock = static_cast<basicBlock *> (obj);
      m_cBlock->locIndex = control_block_loc;
      obj->set("basefreq",m_baseFreq);
	  addSubObject(obj);
    }
  else
    {
      gridDynGenerator::add (obj);
    }

}

// set properties
void variableGenerator::set (const std::string &param,  const std::string &val)
{
  gridDynGenerator::set (param, val);

  
}


void variableGenerator::set (const std::string &param, double val, units_t unitType)
{
  if (param == "vcutout")
    {
      mp_Vcutout = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
    }
  else if (param == "vmax")
    {
      mp_Vmax = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
    }
  else

    {
      gridDynGenerator::set (param, val, unitType);
    }

}


// compute the residual for the dynamic states
void variableGenerator::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  gridDynGenerator::residual (inputs, sD, resid, sMode);
  if ((m_source) && (m_source->isEnabled()))
    {
      m_source->residual ( inputs, sD, resid, sMode);
    }
  if ((m_cBlock) && (m_cBlock->isEnabled()))
    {
	  //TODO:: this needs to be tied to the source
      m_cBlock->residElements(Pset,dPdt,sD, resid,  sMode);
    }

}
void variableGenerator::jacobianElements (const IOdata &inputs, const stateData &sD,
                                          matrixData<double> &ad,
                                          const IOlocs &inputLocs,const solverMode &sMode)
{
  gridDynGenerator::jacobianElements  (inputs,sD, ad, inputLocs,sMode);
  if ((m_source) && (m_source->isEnabled()))
    {
      m_source->jacobianElements (inputs,sD,ad,inputLocs,sMode);
    }
  if ((m_cBlock) && (m_cBlock->isEnabled()))
    {
      m_cBlock->jacobianElements (inputs, sD, ad,  inputLocs,sMode);
    }
}


coreObject *variableGenerator::find (const std::string &object) const
{
  if (object == "source")
    {
      return m_source;
    }
  else if (object == "cblock")
    {
      return m_cBlock;
    }
  else
    {

      return gridDynGenerator::find (object);
    }
}

coreObject *variableGenerator::getSubObject (const std::string &typeName, index_t num) const
{
  auto out = gridDynGenerator::getSubObject (typeName,num);
  if (!out)
    {
      out = find (typeName);
    }
  return out;

}


double variableGenerator::pSetControlUpdate(const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
	if ((m_cBlock) && (m_cBlock->isEnabled()))
	{
		return m_cBlock->getOutput();
	}
	else
	{
		return gridDynGenerator::pSetControlUpdate(inputs, sD, sMode);
	}
}

index_t variableGenerator::pSetLocation(const solverMode &sMode)
{
	
	if ((m_cBlock) && (m_cBlock->isEnabled()))
	{
		return m_cBlock->getOutputLoc(sMode);
	}
	else
	{
		return gridDynGenerator::pSetLocation(sMode);
	}
}