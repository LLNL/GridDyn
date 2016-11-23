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
/*
enum pulse_type_t{ square = 0, triangle = 1, guassian = 2, biexponential = 3, exponential = 4 };
pulse_type_t ptype;
protected:
double period;
double duty_cylce;
double A;
double nextCycleTime;*/
using namespace gridUnits;
gridPulseLoad::gridPulseLoad (const std::string &objName) : gridLoad (objName)
{

}

gridPulseLoad::gridPulseLoad (double rP, double rQ, const std::string &objName) : gridLoad (rP, rQ,objName),baseLoadP (rP),baseLoadQ (rQ)
{

}


gridCoreObject *gridPulseLoad::clone (gridCoreObject *obj) const
{
  gridPulseLoad *nobj = cloneBase<gridPulseLoad, gridLoad> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }

  nobj->ptype = ptype;
  nobj->dutyCycle = dutyCycle;
  nobj->A = A;
  nobj->cycleTime = cycleTime;
  nobj->baseLoadP = baseLoadP;
  nobj->baseLoadQ = baseLoadQ;
  nobj->transTime = transTime;
  nobj->shift = shift;
  nobj->Pfrac = Pfrac;
  nobj->Qfrac = Qfrac;
  return nobj;
}

void gridPulseLoad::pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  cycleTime = time0 - shift * period - period;  //subtract a period so it cycles properly the first time
  if ((baseLoadP == 0.0)&&(baseLoadQ == 0))
    {
      Pfrac = 1.0;
      Qfrac = 0.0;
    }
  loadUpdateForward (time0);
  return gridLoad::pFlowObjectInitializeA (time0,flags);
}

void gridPulseLoad::dynObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  cycleTime = time0 - shift * period - period;  //subtract a period so it cycles properly the first time
  loadUpdateForward (time0);

  return gridLoad::dynObjectInitializeA (time0, flags);

}


void gridPulseLoad::loadUpdate (gridDyn_time ttime)
{
  double tdiff = ttime - cycleTime;
  if (ttime - lastTime == 0.0)
    {
      return;
    }
  tdiff = std::fmod (tdiff,period);
  double pcalc = pulseCalc (tdiff);

  P = baseLoadP + Pfrac * pcalc;
  Q = baseLoadQ + Qfrac * pcalc;

  lastTime = ttime;
}

void gridPulseLoad::loadUpdateForward (gridDyn_time ttime)
{
  double tdiff = ttime - cycleTime;
  if (ttime - lastTime == 0.0)
    {
      return;
    }
  if (tdiff > period)
    {
      tdiff = std::fmod (tdiff, period);
      cycleTime += period * (std::floor (tdiff / period));
    }
  double pcalc = pulseCalc (tdiff);

  P = baseLoadP + Pfrac * pcalc;
  Q = baseLoadQ + Qfrac * pcalc;

  lastTime = ttime;
  prevTime = ttime;
}

void gridPulseLoad::set (const std::string &param,  const std::string &val)
{
  if ((param == "type") || (param == "pulsetype"))
    {
      if (val == "square")
        {
          ptype = pulse_type_t::square;
          if (transTime == 0.05) //if it hasn't been changed change the default
            {
              transTime = 0.001;
            }
        }
      else if (val == "triangle")
        {
          ptype = pulse_type_t::triangle;
        }
      else if (val == "gaussian")
        {
          ptype = pulse_type_t::gaussian;
        }
      else if (val == "exponential")
        {
          ptype = pulse_type_t::exponential;
        }
      else if (val == "biexponential")
        {
          ptype = pulse_type_t::biexponential;
        }
      else if (val == "monocycle")
        {
          ptype = pulse_type_t::monocycle;
        }
      else if ((val == "sine")|| (param == "cosine"))
        {
          ptype = pulse_type_t::cosine;
        }

      cycleTime = cycleTime - period;
    }
  else
    {
      gridLoad::set (param, val);
    }

}


void gridPulseLoad::set (const std::string &param, double val, units_t unitType)
{

  if ((param == "a") || (param == "amplitude"))
    {
      A = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      cycleTime = cycleTime - period;
    }
  else if ((param == "period") || (param == "pulseperiod"))
    {
      period = val;
    }
  else if ((param == "frequency") || (param == "freq")||(param == "pulsefreq"))
    {
      double freq = unitConversion (val, unitType, Hz, m_baseFreq);
      period = 1 / freq;
    }
  else if (param == "dutycycle")
    {
      dutyCycle = val;
      cycleTime = cycleTime - period;
    }
  else if (param == "shift")
    {
      cycleTime = cycleTime + (shift - val) * period;
      shift = val;
    }
  else if ((param == "base")|| (param == "baseload") || (param == "baseloadp"))
    {
      baseLoadP = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      if (baseLoadP != 0.0)
        {
          Pfrac = baseLoadP / sqrt (baseLoadP * baseLoadP + baseLoadQ * baseLoadQ);
          Qfrac = baseLoadQ / sqrt (baseLoadP * baseLoadP + baseLoadQ * baseLoadQ);
        }
      else
        {
          Pfrac = 0;
          Qfrac = 1.0;
        }
      cycleTime = cycleTime - period;
    }
  else if (param == "baseloadq")
    {
      baseLoadQ = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      if (baseLoadQ != 0)
        {
          Pfrac = baseLoadP / sqrt (baseLoadP * baseLoadP + baseLoadQ * baseLoadQ);
          Qfrac = baseLoadQ / sqrt (baseLoadP * baseLoadP + baseLoadQ * baseLoadQ);
        }
      else
        {
          Pfrac = 1.0;
          Qfrac = 0;
        }
      cycleTime = cycleTime - period;
    }
  else if (param == "p")
    {
      baseLoadP = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      if (baseLoadP != 0)
        {
          Pfrac = baseLoadP / sqrt (baseLoadP * baseLoadP + baseLoadQ * baseLoadQ);
          Qfrac = baseLoadQ / sqrt (baseLoadP * baseLoadP + baseLoadQ * baseLoadQ);
        }
      else
        {
          Pfrac = 0;
          Qfrac = 1.0;
        }
      P = val;
      cycleTime = cycleTime - period;
    }
  else if ((param == "transition_time")||(param == "transtime")||(param == "transition"))
    {
      transTime = val;
    }
  else if (param == "q")
    {
      baseLoadQ = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      if (baseLoadQ != 0)
        {
          Pfrac = baseLoadP / sqrt (baseLoadP * baseLoadP + baseLoadQ * baseLoadQ);
          Qfrac = baseLoadQ / sqrt (baseLoadP * baseLoadP + baseLoadQ * baseLoadQ);
        }
      else
        {
          Pfrac = 1.0;
          Qfrac = 0;
        }
      Q = val;
      cycleTime = cycleTime - period;
    }
  else if (param == "invert")
    {
      if (val > 0)
        {
          opFlags.set (invert_flag);
        }
      else
        {
          opFlags.reset (invert_flag);
        }
      cycleTime = cycleTime - period;
    }
  else
    {
      gridLoad::set (param, val, unitType);
    }


}

//TODO:PT should convert this to a function map
double gridPulseLoad::pulseCalc (double td)
{
  double pamp = 0;
  double mult = 1.0;
  double cloc = td / period;
  double prop = (cloc - 0.5 + dutyCycle / 2.0) / dutyCycle;
  if ((prop < 0) || (prop >= 1.0))
    {
      return (opFlags.test (invert_flag)) ? A : 0.0;
    }

  //calculate the multiplier
  if (prop < transTime)
    {
      mult =  prop / transTime;
    }
  else if (prop > 1.0 - transTime)
    {
      mult = (1 - prop) / transTime;
    }


  switch (ptype)
    {
    case pulse_type_t::square:
      pamp = A * mult;
      break;
    case pulse_type_t::triangle:
      pamp = 2 * A * ((prop < 0.5) ? prop : (1 - prop));
      break;
    case pulse_type_t::gaussian:
      pamp = mult * A * exp ((prop - 0.5) * (prop - 0.5) * 25);
      break;
    case pulse_type_t::monocycle:
      pamp = 11.6583 * (prop - 0.5) * exp (-(prop - 0.5) * (prop - 0.5));
      break;
    case pulse_type_t::biexponential:
      pamp = mult * A * exp (-((prop < 0.5) ? (0.5 - prop) : (prop - 0.5)) * 12);
      break;
    case pulse_type_t::exponential:
      pamp = mult * A * exp (-prop * 6);
      break;
    case pulse_type_t::cosine:

      pamp = A * sin (prop * kPI);
      break;
    case pulse_type_t::flattop:
      if (prop < 0.25)
        {
          pamp = A / 2.0 * (-cos (kPI * prop * 4) + 1.0);
        }
      else if (prop > 0.75)
        {
          pamp = A / 2.0 * cos (kPI * ((1 - prop) * 4) + 1.0);
        }
      else
        {
          pamp = A;
        }
      break;
    }
  if (opFlags.test (invert_flag))
    {
      pamp = A - pamp;
    }
  return pamp;
}


inline double sign (double x)
{
  return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}
