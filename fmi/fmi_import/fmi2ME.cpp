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

#include "fmiObjects.h"


fmi2ME::fmi2ME(fmi2Component cmp, std::shared_ptr<const fmiInfo> keyInfo, std::shared_ptr<const fmiCommonFunctions> comFunc,std::shared_ptr<const fmiMEFunctions> meFunc) :fmi2Object(cmp,keyInfo,comFunc),MEFunctions(meFunc)
{
	numIndicators = info->getCounts("events");
	numStates = info->getCounts("states");
}

void fmi2ME::setMode(fmuMode mode)
{
	fmi2Status ret=fmi2Error;
	switch (currentMode)
	{
	case fmuMode::initializationMode:
		
		if (mode == fmuMode::continuousTimeMode)
		{
			fmi2Object::setMode(fmuMode::eventMode);
			ret = MEFunctions->fmi2EnterContinuousTimeMode(comp);
		}
		else
		{
			fmi2Object::setMode(mode);
		}
		break;
	case fmuMode::continuousTimeMode:
		if (mode == fmuMode::eventMode)
		{
			ret = MEFunctions->fmi2EnterEventMode(comp);
		}
		break;
	case fmuMode::eventMode:
		if (mode == fmuMode::eventMode)
		{
			ret = MEFunctions->fmi2EnterEventMode(comp);
		}
		else if (mode==fmuMode::continuousTimeMode)
		{
			ret = MEFunctions->fmi2EnterContinuousTimeMode(comp);
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

void fmi2ME::newDiscreteStates(fmi2EventInfo* fmi2eventInfo)
{
	auto ret=MEFunctions->fmi2NewDiscreteStates(comp, fmi2eventInfo);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ME::completedIntegratorStep(fmi2Boolean noSetFMUStatePriorToCurrentPoint, fmi2Boolean *enterEventMode, fmi2Boolean *terminatesSimulation)
{
	auto ret = MEFunctions->fmi2CompletedIntegratorStep(comp, noSetFMUStatePriorToCurrentPoint, enterEventMode, terminatesSimulation);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ME::setTime(fmi2Real time)
{
	auto ret = MEFunctions->fmi2SetTime(comp, time);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ME::setStates(const fmi2Real x[])
{
	auto ret = MEFunctions->fmi2SetContinuousStates(comp, x, numStates);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ME::getDerivatives(fmi2Real derivatives[]) const
{
	auto ret = MEFunctions->fmi2GetDerivatives(comp, derivatives, numStates);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ME::getEventIndicators(fmi2Real eventIndicators[]) const
{
	auto ret = MEFunctions->fmi2GetEventIndicators(comp, eventIndicators, numIndicators);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ME::getStates(fmi2Real x[]) const
{
	auto ret = MEFunctions->fmi2GetContinuousStates(comp, x, numStates);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2ME::getNominalsOfContinuousStates(fmi2Real x_nominal[]) const
{
	auto ret = MEFunctions->fmi2GetNominalsOfContinuousStates(comp, x_nominal,numStates);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}

std::vector<std::string> fmi2ME::getStateNames() const
{
	return info->getVariableNames("state");

}