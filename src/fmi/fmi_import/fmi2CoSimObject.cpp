/*
* LLNS Copyright Start
* Copyright (c) 2014-2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "fmiObjects.h"

#define MAX_DERIV_ORDER 10
#define MAX_IO 1000

#include <array>
/**the co-simulation functions need an array of the derivative order, which for the primary function calls
will be all identical, so there was no need to have to reconstruct this every time so this function just builds
a common case to handle a large majority of cases without any additional copying or allocation
*/
auto makeDerivOrderBlocks()
{
	std::array<std::array< fmi2Integer, MAX_IO>, MAX_DERIV_ORDER+1> dblock;
	for (int ii = 0; ii <= MAX_DERIV_ORDER; ++ii)
	{
		dblock[ii].fill(ii);
	}
	return dblock;
}
static const auto derivOrder = makeDerivOrderBlocks();


fmi2CoSimObject::fmi2CoSimObject(fmi2Component cmp, std::shared_ptr<const fmiInfo> keyInfo, std::shared_ptr<const fmiCommonFunctions> comFunc, std::shared_ptr<const fmiCoSimFunctions> csFunc) :fmi2Object(std::move(cmp),std::move(keyInfo), std::move(comFunc)), CoSimFunctions(std::move (csFunc))
{

}

void fmi2CoSimObject::setInputDerivatives(int order, const fmi2Real value[])
{
	auto ret = CoSimFunctions->fmi2SetRealInputDerivatives(comp, activeInputs.getValueRef(), activeInputs.getVRcount(), derivOrder[order].data(), value);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2CoSimObject::getOutputDerivatives(int order, fmi2Real value[]) const
{
	auto ret = CoSimFunctions->fmi2GetRealOutputDerivatives(comp, activeOutputs.getValueRef(), activeOutputs.getVRcount(), derivOrder[order].data(), value);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}
void fmi2CoSimObject::doStep(fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPoint)
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
void fmi2CoSimObject::cancelStep()
{
	auto ret = CoSimFunctions->fmi2CancelStep(comp);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
}

fmi2Real fmi2CoSimObject::getLastStepTime() const
{
	fmi2Real lastTime;
	auto ret = CoSimFunctions->fmi2GetRealStatus(comp, fmi2StatusKind::fmi2LastSuccessfulTime, &lastTime);
	if (ret != fmi2Status::fmi2OK)
	{
		handleNonOKReturnValues(ret);
	}
	return lastTime;
}

bool fmi2CoSimObject::isPending()
{
    if (!stepPending)
    {
        return false;
    }

    fmi2Status status;
    auto ret = CoSimFunctions->fmi2GetStatus(comp, fmi2StatusKind::fmi2DoStepStatus, &status);
    if (ret != fmi2Status::fmi2OK)
    {
        handleNonOKReturnValues(ret);
    }
    if (status != fmi2Status::fmi2Pending)
    {
        stepPending = false;
        return false;
    }
    return true;
}

std::string fmi2CoSimObject::getStatus() const
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
    return "";
}
