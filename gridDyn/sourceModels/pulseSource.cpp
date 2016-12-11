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

#include "sourceTypes.h"
#include "vectorOps.hpp"
#include "stringOps.h"
#include "gridCoreTemplates.h"
#include <cmath>

/*
enum pulse_type_t{ square = 0, triangle = 1, guassian = 2, biexponential = 3, exponential = 4 };
pulse_type_t ptype;
protected:
double period;
double duty_cylce;
double A;
double nextCycleTime;*/
using namespace gridUnits;


pulseSource::pulseSource (const std::string &objName, double startVal) : gridSource (objName,startVal), baseValue (startVal)
{

}


coreObject *pulseSource::clone (coreObject *obj) const
{
	pulseSource *nobj = cloneBase<pulseSource, gridSource>(this, obj);
  if (nobj == nullptr)
    {
	  return obj;
    }
  nobj->ptype = ptype;
  nobj->dutyCycle = dutyCycle;
  nobj->A = A;
  nobj->cycleTime = cycleTime;
  nobj->baseValue = baseValue;
  nobj->shift = shift;
  return nobj;
}

void pulseSource::objectInitializeA (gridDyn_time time0, unsigned long flags)
{
  cycleTime = time0 - shift * period - period;  //subtract a period so it cycles properly the first time
  updateOutput (time0);
  return gridSource::objectInitializeA (time0,flags);
}


void pulseSource::updateOutput(gridDyn_time ttime)
{

	if (ttime == prevTime)
	{
		return;
	}
	gridDyn_time tdiff = ttime - cycleTime;
	if (tdiff > period)
	{
		cycleTime += period * (std::floor(tdiff / period));
		tdiff = tdiff%period;
	}

	double pcalc = pulseCalc(static_cast<double>(tdiff));

	m_output = baseValue + pcalc;
	prevTime = ttime;
}

double pulseSource::computeOutput(gridDyn_time ttime) const
{
	if (ttime == prevTime)
	{
		return m_output;
	}
	auto tdiff = (ttime - cycleTime)%period;

	const double pcalc = pulseCalc(static_cast<double>(tdiff));

	return baseValue + pcalc;
}



double pulseSource::getDoutdt (const stateData &sD, const solverMode &, index_t /*num*/) const
{
  double o1, o2;
  if (!sD.empty())
    {
      o1 = computeOutput(sD.time-0.0001);
      o2 = computeOutput(sD.time);
    }
  else
    {
      o1 = computeOutput(prevTime - 0.0001);
      o2 = m_output;
    }
  return ((o2 - o1) / 0.0001);
}

void pulseSource::set (const std::string &param,  const std::string &val)
{

  if ((param == "type") || (param == "pulsetype"))
    {
      auto vtype = convertToLowerCase (val);
      if (vtype == "square")
        {
          ptype = pulse_type_t::square;
        }
      else if (vtype == "triangle")
        {
          ptype = pulse_type_t::triangle;
        }
      else if (vtype == "gaussian")
        {
          ptype = pulse_type_t::gaussian;
        }
      else if (vtype == "exponential")
        {
          ptype = pulse_type_t::exponential;
        }
      else if (vtype == "biexponential")
        {
          ptype = pulse_type_t::biexponential;
        }
      else if (vtype == "monocycle")
        {
          ptype = pulse_type_t::monocycle;
        }
      else if ((vtype == "sine")|| (vtype == "cosine"))
        {
          ptype = pulse_type_t::cosine;
        }

      cycleTime -= period;
    }
  else
    {
      gridSource::set (param, val);
    }

}

void pulseSource::setLevel(double val)
{
	baseValue = val;
	m_output = m_tempOut = val;
	cycleTime = cycleTime - period;
}

void pulseSource::set (const std::string &param, double val, units_t unitType)
{

  if ((param == "a")|| (param == "amplitude"))
    {
      A = val;
	  //done to ensure a new value is computed at the next update
      cycleTime -=  period;
    }
  else if (param == "period")
    {
      period = val;
    }
  else if (param == "dutycycle")
    {
      dutyCycle = val;
	  //done to ensure a new value is computed at the next update
      cycleTime -= period;
    }
  else if (param == "shift")
    {
      cycleTime = cycleTime + (shift - val) * period;
      shift = val;
    }
  else if (param == "base")
    {
      baseValue = val;
      cycleTime -= period;
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
      cycleTime -= period;
    }
  else
    {
      gridSource::set (param, val, unitType);
    }

}

double pulseSource::pulseCalc (double td) const
{
  
  double mult=1.0;
  double cloc = td / period;
  double prop = (cloc - dutyCycle / 2) / dutyCycle;
  if ((prop < 0) || (prop >= 1.0))
    {
      return (opFlags[invert_flag]) ? A : 0.0;
    }

  //calculate the multiplier
  if (prop < 0.05)
    {
      mult = 20.0 * prop;
    }
  else if (prop > 0.95)
    {
      mult = 20.0 * (1 - prop);
    }

  double pamp = 0.0;
  switch (ptype)
    {
    case pulse_type_t::square:
      pamp = A;
      break;
    case pulse_type_t::triangle:
      pamp = 2.0 * A * ((prop < 0.5)?prop:(1.0-prop));
      break;
    case pulse_type_t::gaussian:
      pamp = mult * A * exp ((prop - 0.5) * (prop - 0.5) * 25.0);
      break;
    case pulse_type_t::monocycle:
      pamp = 11.6583 * (prop - 0.5) * exp (-(prop - 0.5) * (prop - 0.5));
      break;
    case pulse_type_t::biexponential:
      if (prop < 0.5)
        {
          pamp = mult * A * exp (-(0.5 - prop) * 12.0);
        }
      else
        {
          pamp = mult * A * exp (-(prop - 0.5) * 12.0);
        }
      break;
    case pulse_type_t::exponential:
      if (prop < 0.5)
        {
          mult = 1.0;
        }
      pamp = mult * A * exp (-prop * 6.0);
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
	default:
		break;
    }
  if (opFlags[invert_flag])
    {
      pamp = A - pamp;
    }
  return pamp;
}
