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



#include "outputEstimator.h"
#include "gridObjects.h"
#include <algorithm>
#include <cmath>


outputEstimator::outputEstimator(std::vector<int> sDep, std::vector<int> iDep)
{
	stateDep = sDep;
	inputDep = iDep;
	stateDiff.resize(stateDep.size(), 0);
	inputDiff.resize(inputDep.size(), 0);
	prevStates.resize(stateDep.size());
	prevInputs.resize(inputDep.size());
}

double outputEstimator::estimate(gridDyn_time t, const IOdata &args, const double state[])
{
	double val = prevValue;
	for (size_t kk = 0; kk < stateDep.size(); ++kk)
	{
		val += (state[stateDep[kk]] - prevStates[kk])*stateDiff[kk];
	}
	for (size_t kk = 0; kk < inputDep.size(); ++kk)
	{
		val += (args[inputDep[kk]] - prevInputs[kk])*inputDiff[kk];
	}
	val += timeDiff*(t - time);
	return val;
}

bool outputEstimator::update(gridDyn_time t, double val, const IOdata &args, const double state[])
{
	time = t;
	
	double pred = estimate(t, args, state);
	prevValue = val;
	for (size_t kk = 0; kk < stateDep.size(); ++kk)
	{
		prevStates[kk] = state[stateDep[kk]];
	}
	for (size_t kk = 0; kk < inputDep.size(); ++kk)
	{
		prevInputs[kk] = args[inputDep[kk]];
	}
	double diff = (std::abs)(pred - val);
	if ((diff>1e-4) && (diff / (std::max)(std::abs( pred), std::abs(val))>0.02))
	{
		return true;
	}
	return false;
}