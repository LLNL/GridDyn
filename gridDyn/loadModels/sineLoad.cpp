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

#include "loadModels/otherLoads.h"

#include "gridCoreTemplates.h"

#include <cmath>

inline double sign (double x);

using namespace gridUnits;

/*
enum pulse_type_t{ square = 0, triangle = 1, guassian = 2, biexponential = 3, exponential = 4 };
pulse_type_t ptype;
protected:
double period;
double duty_cylce;
double A;
double nextCycleTime;*/
gridSineLoad::gridSineLoad (const std::string &objName) : gridPulseLoad (objName)
{

}

gridSineLoad::gridSineLoad (double rP, double rQ, const std::string &objName) : gridPulseLoad (rP,rQ,objName)
{

}


gridCoreObject *gridSineLoad::clone (gridCoreObject *obj) const
{
  gridSineLoad *nobj = cloneBase<gridSineLoad, gridLoad> (this, obj);
  if (!(nobj))
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

void gridSineLoad::pFlowObjectInitializeA (double time0, unsigned long flags)
{

  lastCycle = time0 - phase / (frequency * 2.0 * kPI) - sinePeriod;
  gridPulseLoad::pFlowObjectInitializeA (time0,flags);
  loadUpdateForward (time0);

}

void gridSineLoad::dynObjectInitializeA (double time0, unsigned long flags)
{

  lastCycle = time0 - phase / (frequency * 2.0 * kPI) - sinePeriod;
  gridPulseLoad::dynObjectInitializeA (time0,flags);
  loadUpdateForward (time0);

}


void gridSineLoad::loadUpdate (double ttime)
{

  double tdiff = ttime - lastCycle;
  double dt = ttime - lastTime;
  if (dt == 0.0)
    {
      return;
    }
  //account for the frequency shift
  //TODO:: PT I think there is an error in this calculation if the frequency is changing
  double Nfrequency = frequency + dfdt * dt;
  double NAmp = Amp + dAdt * dt;
  //compute the sine wave component
  double addComponent = NAmp * sin (2.0 * kPI * (Nfrequency * tdiff) + phase);
  double mult = 1.0;
  tdiff = std::fmod (tdiff, sinePeriod);
  if (opFlags[pulsed_flag])
    {
      double tdiff2 = std::fmod (ttime - cycleTime,period);
      mult = pulseCalc (tdiff2);
    }
  P = baseLoadP + Pfrac * (mult * addComponent);

  Q = baseLoadQ + Qfrac * (mult * addComponent);

  lastTime = ttime;
}

void gridSineLoad::loadUpdateForward (double ttime)
{

  double tdiff = ttime - lastCycle;
  double dt = ttime - lastTime;
  if (dt == 0.0)
    {
      return;
    }
  //account for the frequency shift
  frequency = frequency + dfdt * (ttime - prevTime);
  Amp = Amp + dAdt * (ttime - prevTime);
  //compute the sine wave component
  double addComponent = Amp * sin (2.0 * kPI * (frequency * tdiff) + phase);
  double mult = 1.0;
  while (tdiff > sinePeriod)
    {
      tdiff -= sinePeriod;
      lastCycle += sinePeriod;
    }
  if (opFlags[pulsed_flag])
    {
      double tdiff2 = ttime - cycleTime;
      while (tdiff2 > period)
        {
          cycleTime = cycleTime + period;
          tdiff2 = tdiff2 - period;
        }
      mult = pulseCalc (tdiff2);
    }
  P = baseLoadP + Pfrac * (mult * addComponent);
  Q = baseLoadQ + Qfrac * (mult * addComponent);
  lastTime = ttime;
  prevTime = ttime;
}
int gridSineLoad::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;
//	makeLowerCase(param);

  out = gridPulseLoad::set (param, val);
  return out;
}


int gridSineLoad::set (const std::string &param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;

  if ((param == "a") || (param == "amplitude")||(param == "amp"))
    {
      Amp = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if ((param == "frequency")||(param == "freq"))
    {
      frequency = unitConversion (val, unitType, Hz, m_baseFreq);
      sinePeriod = 1.0 / frequency;
    }
  else if ((param == "period") || (param == "sineperiod"))
    {
      sinePeriod = unitConversion (val,unitType,sec);
      frequency = 1.0 / sinePeriod;
    }
  else if (param == "phase")
    {
      phase = unitConversion (val, unitType, rad);
    }
  else if (param == "dfdt")
    {
      dfdt = unitConversion (val, unitType, Hz, m_baseFreq);
    }
  else if (param == "dadt")
    {
      dAdt = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
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
      out = gridPulseLoad::set (param, val, unitType);
    }
  return out;
}


