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

#include "gridSource.h"


#include <cmath>

/*
enum pulse_type_t{ square = 0, triangle = 1, guassian = 2, biexponential = 3, exponential = 4 };
pulse_type_t ptype;
protected:
double period;
double duty_cylce;
double A;
double nextCycleTime;*/



gridCoreObject *sineSource::clone (gridCoreObject *obj) const
{
  sineSource *nobj;
  if (obj == nullptr)
    {
      nobj = new sineSource ();
    }
  else
    {
      nobj = dynamic_cast<sineSource *> (obj);
      if (nobj == nullptr)
        {
          //if we can't cast the pointer clone at the next lower level
          pulseSource::clone (obj);
          return obj;
        }
    }
  pulseSource::clone (nobj);
  nobj->Amp = Amp;
  nobj->frequency = frequency;
  nobj->phase = phase;
  nobj->lastCycle = lastCycle;
  nobj->sinePeriod = sinePeriod;
  nobj->dfdt = dfdt;
  nobj->dAdt = dAdt;
  return nobj;
}


void sineSource::objectInitializeA (double time0, unsigned long flags)
{
  lastCycle = time0 - phase / (frequency * 2.0 * kPI);
  pulseSource::objectInitializeA (time0,flags);
  sourceUpdate (time0);
}


void sineSource::sourceUpdate (double ttime)
{

  double tdiff = ttime - lastCycle;
  double dt = ttime - prevTime;
  if (dt == 0.0)
    {
      return;
    }
  //account for the frequency shift
  double Nfrequency = frequency + dfdt * dt;
  double NAmp = Amp + dAdt * dt;
  //compute the sine wave component
  double addComponent = NAmp * sin (2.0 * kPI * (Nfrequency * tdiff) + phase);
  double mult = 1.0;

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

  m_tempOut = baseValue + (mult * addComponent);
  lasttime = ttime;
}

void sineSource::sourceUpdateForward (const double ttime)
{

  double tdiff = ttime - lastCycle;
  double dt = ttime - prevTime;
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
  while (ttime)
    {
      m_output = baseValue + (mult * addComponent);
    }
  lasttime = ttime;
  prevTime = ttime;
}

double sineSource::getDoutdt (const stateData *sD, const solverMode &, index_t /*num*/)
{
  double o1, o2;
  if (sD)
    {
      sourceUpdate (sD->time - 0.0001);
      o1 = m_tempOut;
      sourceUpdate (sD->time);
      o2 = m_tempOut;
    }
  else
    {
      sourceUpdate (prevTime - 0.0001);
      o1 = m_tempOut;
      o2 = m_output;
    }
  return ((o2 - o1) / 0.0001);
}

int sineSource::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;
//	makeLowerCase(param);

  out = pulseSource::set (param, val);
  return out;
}


int sineSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;

  if ((param == "a") || (param == "amplitude")||(param == "amp"))
    {
      Amp = val;
    }
  else if (param == "frequency")
    {
      frequency = val;
      sinePeriod = 1 / frequency;
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
          if (!(opFlags.test (pulsed_flag)))
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
      out = pulseSource::set (param, val, unitType);
    }
  return out;
}

