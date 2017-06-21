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

#include "fncsSource.h"
#include "core/coreObjectTemplates.h"
#include "gridBus.h"
#include "stringOps.h"
#include "fncsLibrary.h"
#include "fncsSupport.h"
#include "vectorOps.hpp"


fncsSource::fncsSource(const std::string &objName) : rampSource(objName)
{

}

coreObject *fncsSource::clone(coreObject *obj) const
{
	fncsSource *nobj = cloneBase<fncsSource, rampSource>(this, obj);
	if (!(nobj))
	{
		return obj;
	}
	nobj->inputUnits = inputUnits;
	nobj->outputUnits = outputUnits;
	nobj->scaleFactor = scaleFactor;
	nobj->valKey = valKey;

	return nobj;
}


void fncsSource::dynObjectInitializeA(coreTime time0, unsigned long flags)
{
	rampSource::dynObjectInitializeA(time0, flags);

	if (updatePeriod == maxTime)
	{
		LOG_WARNING("no period specified defaulting to 10s");
		updatePeriod = 10.0;
	}
	nextUpdateTime = time0;
	updateA(time0);
	nextUpdateTime = time0 + updatePeriod;
}


void fncsSource::updateA(coreTime time)
{
	if (time < nextUpdateTime)
	{
		return;
	}
	auto res = fncsGetVal(valKey)*scaleFactor;
	double newVal = unitConversion(res, inputUnits, outputUnits, systemBasePower);
	if (newVal == kNullVal)
	{
		mp_dOdt = 0.0;
		prevVal = m_output;
		lastUpdateTime = time;
		prevTime = time;
		return;
	}
	

	if (opFlags[use_ramp])
	{
		if (opFlags[predictive_ramp]) //ramp uses the previous change to guess into the future
		{
			m_output = newVal;
			if ((time - lastUpdateTime) > 0.001)
			{
				mp_dOdt = (newVal - prevVal) / (time - lastUpdateTime);
			}
			else
			{
				mp_dOdt = 0;
			}
			prevVal = newVal;
			lastUpdateTime = time;
		}
		else // output will ramp up to the specified value in the update period
		{
			mp_dOdt = (newVal - m_output) / updatePeriod;
			prevVal = m_output;
			lastUpdateTime = prevTime;
		}
	}
	else
	{
		m_output = newVal;
		m_tempOut = newVal;
		prevVal = newVal;
		mp_dOdt = 0;
		lastUpdateTime = time;
	}
	lastTime = time;
	prevTime = time;

}

void fncsSource::timestep(coreTime ttime, const IOdata &inputs, const solverMode &sMode)
{
	while (ttime >= nextUpdateTime)
	{
		updateA(nextUpdateTime);
		updateB();
	}

	rampSource::timestep(ttime, inputs, sMode);
}


void fncsSource::setFlag(const std::string &param, bool val)
{

	if (param == "initial_queury")
	{
		opFlags.set(initial_query, val);
	}
	else if (param == "predictive")
	{
			opFlags.set(use_ramp, val);
			opFlags.set(predictive_ramp, val);
		}
	else if (param == "interpolate")
	{
		opFlags.set(use_ramp, val);
		opFlags.set(predictive_ramp, !val);
	}
	else if (param == "step")
	{
		opFlags.set(use_ramp, !val);
	}
	else if (param == "use_ramp")
	{
		opFlags.set(use_ramp, val);
	}
	else
	{
		rampSource::setFlag(param, val);
	}

}

void fncsSource::set(const std::string &param, const std::string &val)
{

	if ((param == "valkey")||(param=="key"))
	{
		valKey = val;
		updateSubscription();
	}
	else if ((param == "input_units")||(param=="inunits")||(param=="inputunits"))
	{
		inputUnits = gridUnits::getUnits(val);
		updateSubscription();
	}
	else if ((param == "output_units") || (param == "outunits")||(param=="outputunits"))
	{
		outputUnits = gridUnits::getUnits(val);
		updateSubscription();
		
		}
	else
	{
		//no reason to set the ramps in fncs load so go to zipLoad instead
		gridSource::set(param, val);
	}

}


void fncsSource::set(const std::string &param, double val, gridUnits::units_t unitType)
{

	if ((param == "scalefactor") || (param == "scaling"))
	{
		scaleFactor = val;
		updateSubscription();
	}
	else
	{
		gridSource::set(param, val, unitType);
	}
}

void fncsSource::updateSubscription()
{
	if (!valKey.empty())
	{
		std::string def = std::to_string(gridUnits::unitConversion(m_output / scaleFactor, outputUnits, inputUnits, systemBasePower));
		fncsRegister::instance()->registerSubscription(valKey, fncsRegister::dataType::fncsDouble, def);
	}
}