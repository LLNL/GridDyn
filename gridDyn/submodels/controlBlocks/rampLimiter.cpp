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

#include "submodels/controlBlocks/rampLimiter.h"
#include <algorithm>

	rampLimiter::rampLimiter()
	{

	}
	rampLimiter::rampLimiter(double nmin, double nmax):minRamp(nmin),maxRamp(nmax)
	{

	}
	void rampLimiter::setLimits(double nmin, double nmax)
	{
		minRamp = nmin;
		maxRamp = nmax;
	}
	void rampLimiter::setResetLevel(double newReset)
	{
		resetLevel = newReset;
	}
	double rampLimiter::limitCheck(double currentVal, double input, double dIdt) const
	{
		double val;
		if (limiterEngaged)
		{
			if (limiterHigh)
			{
				val = input - currentVal + resetLevel;
			}
			else
			{
				val = currentVal - input + resetLevel;
			}
		}
		else
		{
			val = std::min(maxRamp - dIdt, dIdt - minRamp);
		}
		return val;
	}

	void rampLimiter::changeLimitActivation(double input)
	{
		if (limiterEngaged)
		{
			if (limiterHigh)
			{
				if (input <= maxRamp)
				{
					limiterHigh = false;
				}
				if (input >= minRamp)
				{
					limiterEngaged = false;
				}
			}
			else
			{
				if (input >= minRamp)
				{
					if (input <= maxRamp)
					{
						limiterEngaged = false;
					}
					else
					{
						limiterHigh = true;
					}
					
				}
			}
		}
		else
		{
			if (input >= maxRamp)
			{
				limiterHigh = true;
				limiterEngaged = true;
			}
			else if (input <= minRamp)
			{
				limiterEngaged = true;
			}
		}
	}

	double rampLimiter::output(double input) const
	{
		if (limiterEngaged)
		{
			return (limiterHigh) ? maxRamp : minRamp;
		}
		else
		{
			return input;
		}
	}

	double rampLimiter::deriv(double dIdt) const
	{
		if (limiterEngaged)
		{
			return (limiterHigh) ? maxRamp : minRamp;
		}
		else
		{
			return dIdt;
		}
	}

	double rampLimiter::DoutDin() const
	{
		return (limiterEngaged) ? 0.0 : 1.0;
		
	}

	double rampLimiter::clampOutputRamp(double dIdt) const
	{
		if (dIdt > maxRamp)
		{
			return maxRamp;
		}
		if (dIdt < minRamp)
		{
			return minRamp;
		}
		return dIdt;
	}
