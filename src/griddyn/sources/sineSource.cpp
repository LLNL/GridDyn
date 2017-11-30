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

#include "sineSource.h"

#include "core/coreObjectTemplates.hpp"
#include <cmath>

namespace griddyn
{
namespace sources
{
/*
enum pulse_type_t{ square = 0, triangle = 1, guassian = 2, biexponential = 3, exponential = 4 };
pulse_type_t ptype;
protected:
double period;
double duty_cylce;
double A;
double nextCycleTime;*/

sineSource::sineSource (const std::string &objName, double startVal) : pulseSource (objName, startVal) {}
coreObject *sineSource::clone (coreObject *obj) const
{
    auto nobj = cloneBase<sineSource, pulseSource> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    nobj->Amp = Amp;
    nobj->frequency = frequency;
    nobj->phase = phase;
    nobj->lastCycle = lastCycle;
    nobj->sinePeriod = sinePeriod;
    nobj->dfdt = dfdt;
    nobj->dAdt = dAdt;
    return nobj;
}

void sineSource::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    lastCycle = time0 - phase / (frequency * 2.0 * kPI);
    pulseSource::pFlowObjectInitializeA (time0, flags);
    updateOutput (time0);
}

double sineSource::computeOutput (coreTime time) const
{
    auto dt = time - prevTime;
    if (dt == timeZero)
    {
        return m_output;
    }
    // account for the frequency shift
    double Nfrequency = frequency + dfdt * dt;
    double NAmp = Amp + dAdt * dt;
    // compute the sine wave component
    auto tdiff = time - lastCycle;
    double addComponent = NAmp * sin (2.0 * kPI * (Nfrequency * tdiff) + phase);
    double mult = 1.0;

    if (opFlags[pulsed_flag])
    {
        auto tdiff2 = time - cycleTime;
        if (tdiff2 > period)
        {
            tdiff2 = tdiff2 % period;
        }
        mult = pulseCalc (tdiff2);
    }

    return baseValue + (mult * addComponent);
}

void sineSource::updateOutput (coreTime time)
{
    auto dt = time - prevTime;
    if (dt == timeZero)
    {
        return;
    }
    auto tdiff = time - lastCycle;
    // account for the frequency shift
    frequency += dfdt * (time - prevTime);
    Amp += dAdt * (time - prevTime);
    // compute the sine wave component
    double addComponent = Amp * sin (2.0 * kPI * (frequency * tdiff) + phase);
    double mult = 1.0;
    while (tdiff > sinePeriod)
    {
        tdiff -= sinePeriod;
        lastCycle += sinePeriod;
    }
    if (opFlags[pulsed_flag])
    {
        auto tdiff2 = time - cycleTime;
        while (tdiff2 > period)
        {
            cycleTime += period;
            tdiff2 -= period;
        }
        mult = pulseCalc (tdiff2);
    }

    m_output = baseValue + (mult * addComponent);
    prevTime = time;
}

void sineSource::set (const std::string &param, const std::string &val) { pulseSource::set (param, val); }
void sineSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "a") || (param == "amplitude") || (param == "amp"))
    {
        Amp = val;
    }
    else if (param == "frequency")
    {
        frequency = val;
        sinePeriod = 1.0 / frequency;
    }
	else if ((param == "period")||(param=="sineperiod"))
	{
		sinePeriod = val;
		frequency = 1.0 / val;
	}
	else if (param == "pulseperiod")
	{
		pulseSource::set("period", val, unitType);
	}
	else if (param == "pulseamplitude")
	{
		pulseSource::set("amplitude", val, unitType);
	}
    else if (param == "phase")
    {
        phase = val;
    }
    else if (param == "dfdt")
    {
        dfdt = val;
    }
    else if (param == "dadt")
    {
        dAdt = val;
    }
    else if (param == "pulsed")
    {
        if (val > 0.0)
        {
            if (!(opFlags[pulsed_flag]))
            {
                cycleTime = prevTime;
            }
            opFlags.set (pulsed_flag);
        }
        else
        {
            opFlags.reset (pulsed_flag);
        }
    }
    else
    {
        pulseSource::set (param, val, unitType);
    }
}
}  // namespace sources
}  // namespace griddyn
