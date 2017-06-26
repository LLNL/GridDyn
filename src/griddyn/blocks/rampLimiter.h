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
#pragma once

#include <limits>
namespace griddyn
{
namespace blocks
{
/** class that limits the rate of change of a value between an upper and lower limit and maintains state of whether it is clamping or not
*/
class rampLimiter
{
private:
	double minRamp = std::numeric_limits<double>::min();
	double maxRamp = std::numeric_limits<double>::max();
	double resetLevel = 0.0;
	bool limiterEngaged = false; //!< flag indicating the limiter is engaged
	bool limiterHigh = false;	//!< flag indicating the high limit is engaged
public:
	rampLimiter() = default;
	/** constructor with the min and max limits
	@param[in] nmin the minimum ramp
	@param[in] nmax the maximum ramp
	*/
	rampLimiter(double nmin, double nmax);
	/** set the limits
	@param[in] nmin the minimum ramp
	@param[in] nmax the maximum ramp
	*/
	void setLimits(double nmin, double nmax);
	void setResetLevel(double newReset);
	double limitCheck(double currentVal, double input, double dIdt) const;
	void changeLimitActivation(double dIdt);
	double output(double dIdt) const;
	double deriv(double dIdt) const;
	double DoutDin() const;
	double clampOutputRamp(double dIdt) const;

};
}//namespace blocks
}//namespace griddyn
#endif
