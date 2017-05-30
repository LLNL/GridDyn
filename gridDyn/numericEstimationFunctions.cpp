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

#include "numericEstimationFunctions.h"
#include "gridObjects.h"
#include  "utilities/matrixData.h"

//work in progress
void numericJacobianCalculation(gridObject *obj, const IOdata & inputs, const stateData &sD, matrixData<double> &ad, const IOlocs & inputLocs, const solverMode & sMode)
{

	if (sD.hasScratch())
	{
		
	}
	else
	{

	}
	IOdata testInputs = inputs;

}

void copyObjectState(gridObject *obj, const double state[], double newState[], const solverMode &sMode)
{
	auto sts = getObjectStates(obj, sMode);
	for (auto st : sts)
	{
		newState[st] = state[st];
	}

}

std::vector<index_t> getObjectStates(const gridObject *obj, const solverMode &sMode)
{
	std::vector<index_t> states;
	auto offsets = obj->getOffsets(sMode);
	if (hasAlgebraic(sMode))
	{
		if (offsets->local.vSize > 0)
		{
			for (index_t ii = 0; ii < offsets->local.vSize; ++ii)
			{
				states.push_back(offsets->vOffset + ii);
			}
		}
		if (offsets->local.aSize > 0)
		{
			for (index_t ii = 0; ii < offsets->local.aSize; ++ii)
			{
				states.push_back(offsets->aOffset + ii);
			}
		}
		if (offsets->local.algSize > 0)
		{
			for (index_t ii = 0; ii < offsets->local.algSize; ++ii)
			{
				states.push_back(offsets->algOffset + ii);
			}
		}
	}
	if (hasDifferential(sMode))
	{
		if (offsets->local.diffSize > 0)
		{
			for (index_t ii = 0; ii < offsets->local.diffSize; ++ii)
			{
				states.push_back(offsets->diffOffset + ii);
			}
		}
	}
	return states;
}