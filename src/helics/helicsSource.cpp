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

#include "helicsSource.h"
#include "core/coreObjectTemplates.hpp"
#include "helics/helicsCoordinator.h"
#include "gridBus.h"
#include "stringOps.h"
#include "helicsLibrary.h"
#include "helicsSupport.h"
#include "vectorOps.hpp"

namespace griddyn
{
namespace helicsLib
{
helicsSource::helicsSource(const std::string &objName) : rampSource(objName),valueType(helicsValueType::helicsDouble)
{
	opFlags.set(pflow_init_required);
}

coreObject *helicsSource::clone(coreObject *obj) const
{
	auto nobj = cloneBase<helicsSource, rampSource>(this, obj);
	if (nobj==nullptr)
	{
		return obj;
	}
	nobj->inputUnits = inputUnits;
	nobj->outputUnits = outputUnits;
	nobj->scaleFactor = scaleFactor;
	nobj->valKey = valKey;

	return nobj;
}

void helicsSource::pFlowObjectInitializeA(coreTime time0, std::uint32_t flags)
{
	auto obj = getRoot();

	coord_ = dynamic_cast<helicsCoordinator *>(obj->find("helics"));
	rampSource::pFlowObjectInitializeA(time0, flags);

	if (updatePeriod == maxTime)
	{
		LOG_WARNING("no period specified defaulting to 10s");
		updatePeriod = 10.0;
	}
	nextUpdateTime = time0;
	if (valKey.empty())
	{
		valKey = fullObjectName(this) + "value";
	}
	updateSubscription();
}

void helicsSource::pFlowObjectInitializeB()
{
	updateA(prevTime);
	updateB();
}

void helicsSource::dynObjectInitializeA(coreTime time0, std::uint32_t flags)
{
	rampSource::dynObjectInitializeA(time0, flags);

	if (updatePeriod == maxTime)
	{
		LOG_WARNING("no period specified defaulting to 10s");
		updatePeriod = 10.0;
	}
	nextUpdateTime = time0;
	updateA(time0);
	updateB();
}


void helicsSource::updateA(coreTime time)
{
	if (time < nextUpdateTime)
	{
		return;
	}
	lastUpdateTime = time;
	if (!coord_->isUpdated(valueIndex))
	{
		prevTime = time;
		return;
	}
	auto res = coord_->getValueAs<double>(valueIndex);
	if (res == kNullVal)
	{
		mp_dOdt = 0.0;
		prevVal = m_output;
		prevTime = time;
		lastTime = time;
		return;
	}
	
	double newVal = unitConversion(res*scaleFactor, inputUnits, outputUnits, systemBasePower);
	if (opFlags[use_ramp])
	{
		if (opFlags[predictive_ramp]) //ramp uses the previous change to guess into the future
		{
			m_output = newVal;
			if ((time - lastTime) > 0.001)
			{
				mp_dOdt = (newVal - prevVal) / (time - lastTime);
			}
			else
			{
				mp_dOdt = 0;
			}
			prevVal = newVal;
		}
		else // output will ramp up to the specified value in the update period
		{
			mp_dOdt = (newVal - m_output) / updatePeriod;
			prevVal = m_output;
		}
	}
	else
	{
		m_output = newVal;
		m_tempOut = newVal;
		prevVal = newVal;
		mp_dOdt = 0;
	}
	lastTime = time;
	prevTime = time;

}

void helicsSource::timestep(coreTime ttime, const IOdata &inputs, const solverMode &sMode)
{
	while (ttime >= nextUpdateTime)
	{
		updateA(nextUpdateTime);
		updateB();
	}

	rampSource::timestep(ttime, inputs, sMode);
}


void helicsSource::setFlag(const std::string &param, bool val)
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

void helicsSource::set(const std::string &param, const std::string &val)
{

	if ((param == "valkey")||(param=="key"))
	{
		valKey = val;
		updateSubscription();
	}
	else if (param == "valuetype")
	{
		auto vType = helicsValueTypeFromString(val);
		if (vType == helicsValueType::unknown)
		{
			throw(invalidParameterValue("unrecognized value type " + val));
		}
		valueType = vType;
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
	else if (param == "units")
	{
		auto uval= gridUnits::getUnits(val);
		if (uval == gridUnits::defUnit)
		{
			if (val != "default")
			{
				throw(invalidParameterValue("unknown unit type " + val));
			}
		}
		inputUnits = uval;
		outputUnits = uval;
		updateSubscription();
	}
	else
	{
		//no reason to set the ramps in helics source so go to Source instead
		Source::set(param, val);
	}

}


void helicsSource::set(const std::string &param, double val, gridUnits::units_t unitType)
{

	if ((param == "scalefactor") || (param == "scaling"))
	{
		scaleFactor = val;
		updateSubscription();
	}
	else
	{
		Source::set(param, val, unitType);
	}
}

void helicsSource::updateSubscription()
{
	if (coord_)
	{
		if (!valKey.empty())
		{
			//coord_->registerSubscription(valKey, helicsRegister::dataType::helicsDouble, def);

			if (valueIndex < 0)
			{
				valueIndex = coord_->addSubscription(valKey, valueType, inputUnits);
			}
			else
			{
				coord_->updateSubscription(valueIndex, valKey, valueType, inputUnits);
			}
			coord_->setDefault(valueIndex, gridUnits::unitConversion(m_output / scaleFactor, outputUnits, inputUnits, systemBasePower));

		}
	}
	
}

}// namespace helicsLib
} // namespace griddyn