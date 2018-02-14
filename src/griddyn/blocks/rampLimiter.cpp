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

#include "rampLimiter.h"
#include <algorithm>
namespace griddyn
{
namespace blocks
{
rampLimiter::rampLimiter (double nmin, double nmax) : minRamp (nmin), maxRamp (nmax) {}
void rampLimiter::setLimits (double nmin, double nmax)
{
    minRamp = nmin;
    maxRamp = nmax;
}
void rampLimiter::setResetLevel (double newReset) { resetLevel = newReset; }
double rampLimiter::limitCheck (double currentVal, double input, double dIdt) const
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
        val = std::min (maxRamp - dIdt, dIdt - minRamp);
    }
    return val;
}

void rampLimiter::changeLimitActivation (double dIdt)
{
    if (limiterEngaged)
    {
        if (limiterHigh)
        {
            if (dIdt <= maxRamp)
            {
                limiterHigh = false;
            }
            if (dIdt >= minRamp)
            {
                limiterEngaged = false;
            }
        }
        else
        {
            if (dIdt >= minRamp)
            {
                if (dIdt <= maxRamp)
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
        if (dIdt >= maxRamp)
        {
            limiterHigh = true;
            limiterEngaged = true;
        }
        else if (dIdt <= minRamp)
        {
            limiterEngaged = true;
        }
    }
}

double rampLimiter::output (double dIdt) const
{
    if (limiterEngaged)
    {
        return (limiterHigh) ? maxRamp : minRamp;
    }
    return dIdt;
}

double rampLimiter::deriv (double dIdt) const
{
    if (limiterEngaged)
    {
        return (limiterHigh) ? maxRamp : minRamp;
    }
    return dIdt;
}

double rampLimiter::DoutDin () const { return (limiterEngaged) ? 0.0 : 1.0; }
double rampLimiter::clampOutputRamp (double dIdt) const
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
}  // namespace blocks
}  // namespace griddyn