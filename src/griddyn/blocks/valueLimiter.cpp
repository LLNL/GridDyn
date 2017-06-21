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

#include "valueLimiter.h"
#include <algorithm>

namespace griddyn
{
namespace blocks
{

valueLimiter::valueLimiter (double nmin, double nmax) : minVal (nmin), maxVal (nmax) {}
void valueLimiter::setLimits (double nmin, double nmax)
{
    minVal = nmin;
    maxVal = nmax;
}
void valueLimiter::setResetLevel (double newReset) { resetLevel = newReset; }
double valueLimiter::limitCheck (double input) const
{
    double val;
    if (limiterEngaged)
    {
        if (limiterHigh)
        {
            val = input - maxVal + resetLevel;
        }
        else
        {
            val = minVal - input + resetLevel;
        }
    }
    else
    {
        val = std::min (maxVal - input, input - minVal);
    }
    return val;
}

void valueLimiter::changeLimitActivation (double input)
{
    if (limiterEngaged)
    {
        if (limiterHigh)
        {
            if (input <= maxVal)
            {
                limiterHigh = false;
            }
            if (input >= minVal)
            {
                limiterEngaged = false;
            }
        }
        else
        {
            if (input >= minVal)
            {
                if (input <= maxVal)
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
        if (input >= maxVal)
        {
            limiterHigh = true;
            limiterEngaged = true;
        }
        else if (input <= minVal)
        {
            limiterEngaged = true;
        }
    }
}

double valueLimiter::output (double input) const
{
    return (limiterEngaged) ? ((limiterHigh) ? maxVal : minVal) : input;
}

double valueLimiter::deriv (double dIdt) const { return (limiterEngaged) ? 0.0 : dIdt; }
double valueLimiter::DoutDin () const { return (limiterEngaged) ? 0.0 : 1.0; }
double valueLimiter::clampOutput (double input) const
{
    return (input > maxVal) ? maxVal : (input < minVal) ? minVal : input;
}

}  // namespace blocks
}  // namespace griddyn