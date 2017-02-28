#pragma once
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

#ifndef RAMP_LIMITER_H_
#define RAMP_LIMITER_H_
#include <limits>

/** class that limits the rate of change of a value between an upper and lower limit and maintains state of whether it is clamping or not
*/
class rampLimiter
{
private:
	double minRamp = std::numeric_limits<double>::min();
	double maxRamp = std::numeric_limits<double>::max();
	double resetLevel = 0.0;
	bool limiterEngaged = false;
	bool limiterHigh = false;
public:
	rampLimiter();
	rampLimiter(double nmin, double nmax);
	void setLimits(double nmin, double nmax);
	void setResetLevel(double newReset);
	double limitCheck(double currentVal, double input, double dIdt) const;
	void changeLimitActivation(double input);
	double output(double input) const;
	double deriv(double dIdt) const;
	double DoutDin() const;
	double clampOutputRamp(double input) const;

};
#endif
