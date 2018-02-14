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



#include "outputEstimator.h"
#include <algorithm>
#include <cmath>

namespace griddyn
{
namespace fmi
{

outputEstimator::outputEstimator(std::vector<int> sDep, std::vector<int> iDep)
{
	stateDep = sDep;
	inputDep = iDep;
	stateDiff.resize(stateDep.size(), 0);
	inputDiff.resize(inputDep.size(), 0);
	prevStates.resize(stateDep.size());
	prevInputs.resize(inputDep.size());
}

double outputEstimator::estimate(coreTime t, const IOdata &inputs, const double state[])
{
	double val = prevValue;
	for (size_t kk = 0; kk < stateDep.size(); ++kk)
	{
		val += (state[stateDep[kk]] - prevStates[kk])*stateDiff[kk];
	}
	for (size_t kk = 0; kk < inputDep.size(); ++kk)
	{
		val += (inputs[inputDep[kk]] - prevInputs[kk])*inputDiff[kk];
	}
	val += timeDiff*(t - time);
	return val;
}

bool outputEstimator::update(coreTime t, double val, const IOdata &inputs, const double state[])
{
	time = t;
	
	double pred = estimate(t, inputs, state);
	prevValue = val;
	for (size_t kk = 0; kk < stateDep.size(); ++kk)
	{
		prevStates[kk] = state[stateDep[kk]];
	}
	for (size_t kk = 0; kk < inputDep.size(); ++kk)
	{
		prevInputs[kk] = inputs[inputDep[kk]];
	}
	double diff = (std::abs)(pred - val);
	if ((diff>1e-4) && (diff / (std::max)(std::abs( pred), std::abs(val))>0.02))
	{
		return true;
	}
	return false;
}

}//namespace fmi
}//namespace griddyn