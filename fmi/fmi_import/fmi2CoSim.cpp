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

fmi2CoSim::fmi2CoSim(fmi2Component cmp, std::shared_ptr<const fmiInfo> keyInfo, std::shared_ptr<const fmiCommonFunctions> comFunc, std::shared_ptr<const fmiCoSimFunctions> csFunc) :fmi2Object(cmp,keyInfo, comFunc), CoSimFunctions(csFunc)
{

}

void fmi2CoSim::SetRealInputDerivatives(const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], const fmi2Real value[])
{
	auto ret = CoSimFunctions->fmi2SetRealInputDerivatives(comp, vr, nvr, order, value);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2CoSim::getRealOutputDerivatives(const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], fmi2Real value[]) const
{
	auto ret = CoSimFunctions->fmi2GetRealOutputDerivatives(comp, vr, nvr, order, value);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2CoSim::doStep(fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPoint)
{
	auto ret = CoSimFunctions->fmi2DoStep(comp, currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint);
	if (ret != fmi2Status::fmi2OK)
	{
		if (ret == fmi2Status::fmi2Pending)
		{
			stepPending = true;
		}
		else
		{
			handleNonOKReturnValues(ret);
		}
		
	}
	else
	{
		stepPending = false;
	}
}
void fmi2CoSim::cancelStep()
{
	auto ret = CoSimFunctions->fmi2CancelStep(comp);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}

fmi2Real fmi2CoSim::getLastStepTime() const
{
	fmi2Real lastTime;
	auto ret = CoSimFunctions->fmi2GetRealStatus(comp, fmi2StatusKind::fmi2LastSuccessfulTime, &lastTime);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
	return lastTime;
}

bool fmi2CoSim::isPending()
{
	if (stepPending)
	{
		fmi2Status status;
		auto ret = CoSimFunctions->fmi2GetStatus(comp, fmi2StatusKind::fmi2DoStepStatus, &status);
		if (ret != fmi2Status::fmi2OK)
		{
			handleNonOKReturnValues(ret);
		}
		if (status == fmi2Status::fmi2Pending)
		{
			return true;
		}
		else
		{
			stepPending = false;
			return false;
		}
	}
	else
	{
		return false;
	}
}

std::string fmi2CoSim::getStatus() const
{
	if (stepPending)
	{
		fmi2String s;
		auto ret = CoSimFunctions->fmi2GetStringStatus(comp, fmi2StatusKind::fmi2PendingStatus, &s);
		if (ret != fmi2Status::fmi2OK)
		{
			handleNonOKReturnValues(ret);
		}
		return std::string(s);
	}
	else
	{
		return "";
	}
}
