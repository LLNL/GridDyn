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

#include "isocController.h"
#include "generators/gridDynGenerator.h"

#include "objectFactoryTemplates.h"
#include "core/gridDynExceptions.h"
#include "vectorOps.hpp"

#include <algorithm>

isocController::isocController(const std::string &objName) : gridSubModel(objName)
{
	
}


gridCoreObject *isocController::clone(gridCoreObject *obj) const
{
	isocController *nobj;
	if (obj == nullptr)
	{
		nobj = new isocController();
	}
	else
	{
		nobj = dynamic_cast<isocController *> (obj);
		if (nobj == nullptr)
		{
			//if we can't cast the pointer clone at the next lower m_output
			gridCoreObject::clone(obj);
			return obj;
		}
	}
	gridCoreObject::clone(nobj);
	nobj->db = db;
	nobj->upStep = upStep;
	nobj->downStep = downStep;
	nobj->upPeriod = upPeriod;
	nobj->downPeriod = downPeriod;
	nobj->maxLevel = maxLevel;
	nobj->minLevel = minLevel;
	nobj->integralTrigger = integralTrigger;
	return nobj;
}

void isocController::objectInitializeA(gridDyn_time time0, unsigned long /*flags*/)
{
	gen = dynamic_cast<gridDynGenerator *>(parent);
	updatePeriod = upPeriod;
	nextUpdateTime = time0 + upPeriod;
	opFlags.set(has_updates);
	integratorLevel = 0;
}

void isocController::objectInitializeB(const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{
	if (args.size() > 0)
	{
		lastFreq = args[0];
		if (lastFreq < -db)
		{
			updatePeriod = downPeriod;
		}
	}
	if (outputSet.size() > 0)
	{
		m_output = outputSet[0];
		fieldSet[0] = 0;
	}
	else
	{
		fieldSet[0] = m_output;
	}
	
	
	

}

void isocController::setLimits(double maxV, double minV)
{
	minLevel = std::min(maxV,minV);
	maxLevel = std::max(maxV, minV);
	m_output = valLimit(m_output, minLevel, maxLevel);

}



void isocController::updateA(double time)
{
	if (time+kSmallTime < nextUpdateTime)
	{
		assert(false);
		return;
	}
	integratorLevel += lastFreq*updatePeriod;
	if (lastFreq > db)	
	{
		m_output += upStep;
		updatePeriod = upPeriod;
	}
	else if (lastFreq < -db)
	{
		m_output += downStep;
		updatePeriod = downPeriod;
		
	}
	else
	{
		updatePeriod = upPeriod;
		if (integratorLevel > integralTrigger)
		{
			m_output += upStep;
			
		}
		else if (integratorLevel < -integralTrigger)
		{
			m_output += downStep;
		}
	}
	m_output=valLimit(m_output, minLevel, maxLevel);
	lastUpdateTime = time;
	//printf("t=%f,output=%f\n", time, m_output);
}

void isocController::timestep(double ttime, const IOdata &args, const solverMode &)
{
	prevTime = ttime;
	lastFreq = args[0];
	while (nextUpdateTime <= ttime)
	{
		updateA(ttime);
		updateB();
	}

}

void isocController::set(const std::string &param, const std::string &val)
{
	gridSubModel::set(param, val);
}

void isocController::set(const std::string &param, double val, gridUnits::units_t unitType)
{

	if ((param == "deadband")||(param=="db"))
	{
		db = val;
	}
	else if (param == "upstep")
	{
		upStep = val;
	}
	else if (param == "downstep")
	{
		downStep = val;
	}
	else if (param == "upperiod")
	{
		upPeriod = val;
	}
	else if (param == "downperiod")
	{
		downPeriod = val;
	}
	else if ((param == "max")||(param=="maxlevel"))
	{
		maxLevel = val;
	}
	else if ((param == "min") || (param == "minLevel"))
	{
		minLevel = val;
	}
	else if (param == "m_output")
	{
		m_output = val;
	}
	
	else
	{
		gridSubModel::set(param, val, unitType);
	}

}


void isocController::setLevel(double newLevel)
{
	m_output = valLimit(newLevel, minLevel, maxLevel);
}

void isocController::setFreq(double freq)
{
	lastFreq = freq;
}

void isocController::deactivate()
{
	m_output = 0;
	nextUpdateTime = kBigNum;
}

void isocController::activate(double time)
{
	nextUpdateTime = time + upPeriod;
}