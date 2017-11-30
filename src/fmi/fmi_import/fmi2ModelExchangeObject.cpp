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

#include "fmiObjects.h"


fmi2ModelExchangeObject::fmi2ModelExchangeObject(fmi2Component cmp, std::shared_ptr<const fmiInfo> keyInfo, std::shared_ptr<const fmiCommonFunctions> comFunc,std::shared_ptr<const fmiModelExchangeFunctions> meFunc) :fmi2Object(cmp,keyInfo,comFunc),ModelExchangeFunctions(meFunc)
{
	numIndicators = info->getCounts("events");
	numStates = info->getCounts("states");
	if (numStates == 0)
	{
		hasTime = false;
	}
}

void fmi2ModelExchangeObject::setMode(fmuMode mode)
{
	fmi2Status ret=fmi2Error;
	switch (currentMode)
	{
	case fmuMode::instantiatedMode:
	case fmuMode::initializationMode:
		
		if (mode == fmuMode::continuousTimeMode)
		{
			fmi2Object::setMode(fmuMode::eventMode);
			if (numStates > 0)
			{
				ret = ModelExchangeFunctions->fmi2EnterContinuousTimeMode(comp);
			}
			else
			{
				ret = fmi2OK;
			}
			
		}
		else
		{
			fmi2Object::setMode(mode);
		}
		break;
	case fmuMode::continuousTimeMode:
		if (mode == fmuMode::eventMode)
		{
			ret = ModelExchangeFunctions->fmi2EnterEventMode(comp);
		}
		break;
	case fmuMode::eventMode:
		if (mode == fmuMode::eventMode)
		{
			ret = ModelExchangeFunctions->fmi2EnterEventMode(comp);
		}
		else if (mode==fmuMode::continuousTimeMode)
		{
			if (numStates > 0)
			{
				ret = ModelExchangeFunctions->fmi2EnterContinuousTimeMode(comp);
			}
			else
			{
				ret = fmi2OK;
			}
		}
		break;
	default:
		fmi2Object::setMode(mode);
		return;
	}

	if (ret == fmi2OK)
	{
		currentMode = mode;
	}
	else if (currentMode!=mode)
	{
		handleNonOKReturnValues(ret);
	}
	
}

void fmi2ModelExchangeObject::newDiscreteStates(fmi2EventInfo* fmi2eventInfo)
{
	auto ret=ModelExchangeFunctions->fmi2NewDiscreteStates(comp, fmi2eventInfo);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ModelExchangeObject::completedIntegratorStep(fmi2Boolean noSetFMUStatePriorToCurrentPoint, fmi2Boolean *enterEventMode, fmi2Boolean *terminatesSimulation)
{
	auto ret = ModelExchangeFunctions->fmi2CompletedIntegratorStep(comp, noSetFMUStatePriorToCurrentPoint, enterEventMode, terminatesSimulation);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ModelExchangeObject::setTime(fmi2Real time)
{
	if (hasTime)
	{
		auto ret = ModelExchangeFunctions->fmi2SetTime(comp, time);
		if (ret != fmi2Status::fmi2OK)
		{
			handleNonOKReturnValues(ret);
		}
	}
	
}
void fmi2ModelExchangeObject::setStates(const fmi2Real x[])
{
	auto ret = ModelExchangeFunctions->fmi2SetContinuousStates(comp, x, numStates);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ModelExchangeObject::getDerivatives(fmi2Real derivatives[]) const
{
	auto ret = ModelExchangeFunctions->fmi2GetDerivatives(comp, derivatives, numStates);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ModelExchangeObject::getEventIndicators(fmi2Real eventIndicators[]) const
{
	auto ret = ModelExchangeFunctions->fmi2GetEventIndicators(comp, eventIndicators, numIndicators);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ModelExchangeObject::getStates(fmi2Real x[]) const
{
	auto ret = ModelExchangeFunctions->fmi2GetContinuousStates(comp, x, numStates);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ModelExchangeObject::getNominalsOfContinuousStates(fmi2Real x_nominal[]) const
{
	auto ret = ModelExchangeFunctions->fmi2GetNominalsOfContinuousStates(comp, x_nominal,numStates);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}

std::vector<std::string> fmi2ModelExchangeObject::getStateNames() const
{
	return info->getVariableNames("state");

}