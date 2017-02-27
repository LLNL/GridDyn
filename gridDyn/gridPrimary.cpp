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
#include <cstdio>
#include <iostream>

#include <map>
#include <algorithm>

gridPrimary::gridPrimary (const std::string &objName) : gridObject (objName)
{
}

coreObject * gridPrimary::clone(coreObject *obj) const
{
	gridPrimary *nobj = cloneBase<gridPrimary, gridObject>(this, obj);
	if (!(nobj))
	{
		return obj;
	}
	nobj->zone = zone;
	return nobj;
}

void gridPrimary::pFlowInitializeA (coreTime time0, unsigned long flags)
{
	gridObject::pFlowInitializeA(time0, flags);

}

void gridPrimary::pFlowInitializeB ()
{
	gridObject::pFlowInitializeB();
}

void gridPrimary::dynInitializeA (coreTime time0, unsigned long flags)
{
	gridObject::dynInitializeA(time0, flags);
 
}

void gridPrimary::dynInitializeB (const IOdata & inputs, const IOdata & desiredOutput, IOdata &fieldSet)
{
  if (isEnabled())
    {
	  gridObject::dynInitializeB(inputs, desiredOutput, fieldSet);
      updateLocalCache ();
    }
}


static const std::map<int, int> alertFlags {
  std::make_pair (FLAG_CHANGE, 1),
  std::make_pair (STATE_COUNT_INCREASE, 3),
  std::make_pair (STATE_COUNT_DECREASE, 3),
  std::make_pair (STATE_COUNT_CHANGE, 3),
  std::make_pair (ROOT_COUNT_INCREASE, 2),
  std::make_pair (ROOT_COUNT_DECREASE, 2),
  std::make_pair (ROOT_COUNT_CHANGE, 2),
  std::make_pair (JAC_COUNT_INCREASE, 2),
  std::make_pair (JAC_COUNT_DECREASE, 2),
  std::make_pair (JAC_COUNT_CHANGE, 2),
  std::make_pair (OBJECT_COUNT_INCREASE, 3),
  std::make_pair (OBJECT_COUNT_DECREASE, 3),
  std::make_pair (OBJECT_COUNT_CHANGE, 3),
  std::make_pair (CONSTRAINT_COUNT_DECREASE, 1),
  std::make_pair (CONSTRAINT_COUNT_INCREASE, 1),
  std::make_pair (CONSTRAINT_COUNT_CHANGE, 1),
};

void gridPrimary::alert (coreObject *object, int code)
{
  if ((code >= MIN_CHANGE_ALERT)  && (code < MAX_CHANGE_ALERT))
    {
      auto res = alertFlags.find (code);
      if (res != alertFlags.end ())
        {
          if (!opFlags[disable_flag_updates])
            {
              updateFlags ();
            }
          else
            {
              opFlags.set (flag_update_required);
            }
		  switch (res->second)
		  {
		  case 3:
			  offsets.stateUnload();
			  break;
		  case 2:
			  offsets.rjUnload(true);
			  break;
		  default:
			  break;
		  }
        }
    }
  coreObject::alert(object, code);
}


void gridPrimary::set(const std::string &param, const std::string &val)
{
	gridObject::set(param, val);
}

void gridPrimary::set(const std::string &param, double val, gridUnits::units_t unitType)
{
	if ((param == "zone") || (param == "zone number"))
	{
		zone = static_cast<int>(val);
	}
	else
	{
		gridObject::set(param, val, unitType);
	}
}


double gridPrimary::get(const std::string &param, gridUnits::units_t unitType) const
{
	if (param == "zone")
	{
		return static_cast<double>(zone);
	}
	else
	{
		return gridObject::get(param, unitType);
	}
 }

void gridPrimary::converge (coreTime /*ttime*/, double /*state*/[], double /*dstate_dt*/[], const solverMode &, converge_mode, double /*tol*/)
{

}


void  gridPrimary::delayedResidual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  residual (inputs, sD, resid, sMode);
}


void  gridPrimary::delayedDerivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  derivative (inputs, sD, deriv, sMode);
}


void  gridPrimary::delayedAlgebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha)
{
  algebraicUpdate (inputs, sD, update, sMode, alpha);
}


void  gridPrimary::delayedJacobian (const IOdata &inputs, const stateData &sD, matrixData<double> &ad,const IOlocs &inputLocs, const solverMode &sMode)
{
  jacobianElements (inputs, sD, ad, inputLocs,sMode);
}

void gridPrimary::pFlowCheck (std::vector<violation> & /*Violation_vector*/)
{
}


void gridPrimary::updateLocalCache ()
{
}

gridBus * gridPrimary::getBus(index_t /*num*/) const
{
	return nullptr;
}

gridLink * gridPrimary::getLink(index_t /*num*/) const
{
	return nullptr;
}

gridArea * gridPrimary::getArea(index_t /*num*/) const
{
	return nullptr;
}

gridRelay * gridPrimary::getRelay(index_t /*num*/) const
{
	return nullptr;
}