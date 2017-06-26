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

#ifndef VALUE_LIMITER_H_
#define VALUE_LIMITER_H_
#pragma once

#include <limits>

namespace griddyn
{
namespace blocks
{
/** class that clamps a value between an upper and lower limit and maintains state of whether it is clamping or not
*/
class valueLimiter
{
private:
	double minVal = std::numeric_limits<double>::min();  //!< Minimum value
	double maxVal = std::numeric_limits<double>::max();	//!< maximum value
	double resetLevel = 0;	//!< the amount the value has to go above or below a min or max to be considered reset
	bool limiterEngaged = false;	//!< flag indicating the limiter is active
	bool limiterHigh = false;		//!< flag indicating the limit is engaged on the high side
public:
	valueLimiter() = default;
	valueLimiter(double nmin, double nmax);
	void setLimits(double nmin, double nmax);
	void setResetLevel(double newReset);
	double limitCheck(double input) const;
	void changeLimitActivation(double input);
	double output(double input) const;
	double deriv(double dIdt) const;
	double DoutDin() const;
	double clampOutput(double input) const;
	bool isActive() const
	{
		return limiterEngaged;
	}

};
}//namespace blocks
}//namespace griddyn
#endif
