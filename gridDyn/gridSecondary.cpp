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

#include "gridObjects.h"
#include "core/coreObjectTemplates.h"
#include "core/objectInterpreter.h"
#include "utilities/stringOps.h"
#include "gridBus.h"
#include <cstdio>
#include <iostream>

static gridBus defBus(1.0, 0);

gridSecondary::gridSecondary (const std::string &objName) : gridObject (objName), bus(&defBus)
{
	m_outputSize = 2;
	m_inputSize = 3;
}

coreObject * gridSecondary::clone(coreObject *obj) const
{
	gridSecondary *nobj = cloneBase<gridSecondary, gridObject>(this, obj);
	if (!(nobj))
	{
		return obj;
	}
	nobj->baseVoltage = baseVoltage;
	nobj->bus = bus;
	return nobj;
}


void gridSecondary::updateObjectLinkages(coreObject *newRoot)
{
	if (opFlags[pFlow_initialized])
	{
		auto nobj = findMatchingObject(bus, newRoot);
		if (dynamic_cast<gridBus *>(nobj))
		{
			bus = static_cast<gridBus *>(nobj);
		}
	}
	gridObject::updateObjectLinkages(newRoot);
}

void gridSecondary::pFlowInitializeA (coreTime time0, unsigned long flags)
{

		bus = static_cast<gridBus *> (getParent()->find("bus"));
		if (!bus)
		{
			bus = &defBus;
		}
		gridObject::pFlowInitializeA(time0, flags);
}

void gridSecondary::pFlowInitializeB ()
{
	gridObject::pFlowInitializeB();
}

void gridSecondary::dynInitializeA (coreTime time0, unsigned long flags)
{
	gridObject::dynInitializeA(time0, flags);
}

void gridSecondary::dynInitializeB (const IOdata & inputs, const IOdata & desiredOutput, IOdata &fieldSet)
{
  if (isEnabled())
    {

      auto ns = stateSize (cLocalSolverMode);
      m_state.resize (ns, 0);
      m_dstate_dt.clear ();
      m_dstate_dt.resize (ns, 0);
      dynObjectInitializeB (inputs, desiredOutput,fieldSet);
      if (updatePeriod < maxTime)
        {
          opFlags.set (has_updates);
          nextUpdateTime = prevTime + updatePeriod;
          alert (this, UPDATE_REQUIRED);
        }
      opFlags.set (dyn_initialized);
    }
}


void gridSecondary::pFlowObjectInitializeA (coreTime time0, unsigned long flags)
{
  if (!getSubObjects().empty ())
    {
      for (auto &subobj : getSubObjects())
        {
		  if (dynamic_cast<gridSubModel *>(subobj))
		  {
			  if (subobj->checkFlag(pflow_init_required))
			  {
				  static_cast<gridSubModel *> (subobj)->dynInitializeA(time0, flags);
			  }
		  }
		  else 
            {
              subobj->pFlowInitializeA (time0, flags);
            }
        }
    }

}

void gridSecondary::set(const std::string &param, const std::string &val)
{
	gridObject::set(param, val);
}

void gridSecondary::set(const std::string &param, double val, gridUnits::units_t unitType)
{
	if ((param == "basevoltage") || (param == "basev")||(param=="bv")||(param=="base voltage"))
	{
		baseVoltage = gridUnits::unitConversion(val, unitType, gridUnits::kV);
	}
	else
	{
		gridObject::set(param, val, unitType);
	}
}

double gridSecondary::getRealPower (const IOdata & /*inputs*/, const stateData &, const solverMode & /*sMode*/) const
{
  return 0.0;
}

double gridSecondary::getReactivePower (const IOdata & /*inputs*/, const stateData &, const solverMode & /*sMode*/) const
{
  return 0.0;
}


double gridSecondary::getRealPower () const
{
  return 0.0;
}

double gridSecondary::getReactivePower () const
{
  return 0.0;
}


double gridSecondary::getAdjustableCapacityUp (coreTime /*time*/) const
{
  return 0.0;
}

double gridSecondary::getAdjustableCapacityDown (coreTime /*time*/) const
{
  return 0.0;
}

double gridSecondary::getDoutdt (const IOdata & /*inputs*/, const stateData &, const solverMode &, index_t /*num*/) const
{
  return 0.0;
}


double gridSecondary::getOutput (const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num) const
{
  if (num == PoutLocation)
    {
      return getRealPower (inputs, sD, sMode);
    }
  else if (num == QoutLocation)
    {
      return getReactivePower (inputs, sD, sMode);
    }
  else
    {
      return kNullVal;
    }
}


double gridSecondary::getOutput (index_t num) const
{
  if (num == PoutLocation)
    {
      return getRealPower ();
    }
  else if (num == QoutLocation)
    {
      return getReactivePower ();
    }
  else
    {
      return kNullVal;
    }
}

IOdata gridSecondary::getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
  IOdata out (2);
  out[PoutLocation] = getRealPower (inputs, sD, sMode);
  out[QoutLocation] = getReactivePower (inputs, sD, sMode);
  return out;
}

IOdata gridSecondary::predictOutputs (coreTime /*predictionTime*/, const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
  IOdata out (2);
  out[PoutLocation] = getRealPower (inputs, sD, sMode);
  out[QoutLocation] = getReactivePower (inputs, sD, sMode);
  return out;
}
