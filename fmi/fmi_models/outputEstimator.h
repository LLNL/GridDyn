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

#ifndef OUTPUT_ESTIMATOR_H_
#define OUTPUT_ESTIMATOR_H_

#include "gridDynDefinitions.h"

/** class to help with estimating outputs based on changes to the states and inputs in intermediate steps.
The output may not be a state variable in these contexts the FMI itself doesn't always stay updated in intermediate time values
Thus the output may need to be estimated for use in the rest of the system.  The estimate is done by using the Jacobian values for the output
from both the input and states and summing them with the actual difference between the last known value
*/
class outputEstimator
{
public:
	coreTime time;
	double prevValue;
	std::vector<int> stateDep;
	std::vector<int> inputDep;
	std::vector<double> stateDiff;
	std::vector<double> inputDiff;
	std::vector<double> prevInputs;
	std::vector<double> prevStates;
	double timeDiff = 0.0;

	outputEstimator(std::vector<int> sDep, std::vector<int> iDep);
	double estimate(coreTime time, const IOdata &inputs, const double state[]);
	bool update(coreTime time, double val, const IOdata &inputs, const double state[]);
};


#endif