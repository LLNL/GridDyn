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
#include "gridRandom.h"
#include "stringOps.h"
#include "core/gridDynExceptions.h"
#include "gridCoreTemplates.h"
#include <iostream>
#include <cassert>

randomSource::randomSource (const std::string &objName, double startVal) : rampSource (objName,startVal),mean_L(startVal)
{
  timeGenerator = std::make_unique<gridRandom> (gridRandom::dist_type_t::constant);
  valGenerator = std::make_unique<gridRandom> (gridRandom::dist_type_t::constant);
}


coreObject *randomSource::clone (coreObject *obj) const
{
	randomSource *src = cloneBase<randomSource, rampSource>(this, obj);
  if (src == nullptr)
    {
	  return obj;
    }
  src->min_t = min_t;
  src->max_t = max_t;
  src->min_L = min_L;
  src->max_L = max_L;
  src->mean_t = mean_t;
  src->mean_L = mean_L;
  src->scale_t = scale_t;
  src->stdev_L = stdev_L;
  src->opFlags.reset (triggered_flag);
  src->zbias = zbias;
  src->opFlags.reset (object_armed_flag);
  src->keyTime = keyTime;
  src->timeGenerator->setDistribution (timeGenerator->getDistribution ());
  src->valGenerator->setDistribution (valGenerator->getDistribution ());
  return src;
}

// set properties
void randomSource::set (const std::string &param,  const std::string &val)
{

  if ((param == "trigger_dist")|| (param == "time_dist"))
    {
      auto v2 = convertToLowerCase (val);
      timeGenerator->setDistribution (getDist (v2));
    }
  else if ((param == "size_dist")||(param == "change_dist"))
    {
      auto v2 = convertToLowerCase (val);
      valGenerator->setDistribution (getDist (v2));
    }
  else
    {
      gridSource::set (param, val);
    }

}

void randomSource::setFlag (const std::string &flag, bool val)
{

  /*
  independent_flag=object_flag3,
  interpolate_flag=object_flag4,
  repeated_flag=object_flag5,
  proportional_flag=object_flag6,
  triggered_flag=object_flag7,
  armed_flag=object_flag8,*/
  if (flag == "interpolate")
    {
      opFlags.set (interpolate_flag, val);
    }
  else if (flag == "step")
    {
      opFlags.set (interpolate_flag, !val);
    }
  else if (flag == "repeated")
    {

      opFlags.set (repeated_flag, val);
    }
  else if (flag == "proportional")
    {
      opFlags.set (proportional_flag, val);
    }
  else
    {
      gridSource::setFlag (flag,val);
    }
 
}

void randomSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "min_t")
    {
      if (val <= 0)
        {
		  throw(invalidParameterValue());
        }
      else
        {
          min_t = val;
        }
    }
  else if (param == "max_t")
    {
      max_t = val;
    }
  else if (param == "min_l")
    {
      min_L = val;
    }
  else if (param == "max_l")
    {
      max_L = val;
    }
  else if (param == "mean_t")
    {

      if (val <= 0)
        {
          LOG_WARNING ("mean_t parameter must be > 0");
		  throw(invalidParameterValue());
        }
      else
        {
          mean_t = val;
        }
    }
  else if (param == "mean_l")
    {
      mean_L = val;
    }
  else if (param == "scale_t")
    {
      if (val <= 0)
        {
          LOG_WARNING ("scale_t parameter must be > 0");
		  throw(invalidParameterValue());
        }
      else
        {
          scale_t = val;
        }
    }
  else if (param == "stdev_l")
    {
      stdev_L = val;
    }

  else if (param == "interpolate")
    {
      opFlags.set (interpolate_flag, (val > 0));
    }
  else if (param == "repeated")
    {
      opFlags.set (repeated_flag, (val > 0));
    }
  else if (param == "proportional")
    {
      opFlags.set (proportional_flag, (val > 0));
    }
  else if (param == "seed")
    {
      timeGenerator->setSeed (static_cast<int> (val));
    }
  else
    {
      //I am purposely skipping over the rampLoad the functionality is needed but the access is not
      gridSource::set (param, val, unitType);
    }

}

void randomSource::reset ()
{
	opFlags.reset(triggered_flag);
	opFlags.reset(object_armed_flag);
	offset = 0.0;
}

void randomSource::objectInitializeA (gridDyn_time time0, unsigned long /*flags*/)
{
	reset();
      keyTime = time0;
	  gridDyn_time triggerTime = time0 + ntime ();

      if (opFlags[interpolate_flag])
        {
          nextStep (triggerTime);
        }
      nextUpdateTime = triggerTime;
      opFlags.set (object_armed_flag);
}


void randomSource::updateOutput(gridDyn_time time)
{
	updateA(time);
	rampSource::updateOutput(time);
}

void randomSource::updateA (gridDyn_time time)
{
  if (time < nextUpdateTime)
    {
      return;
    }


  lastUpdateTime = nextUpdateTime;
  opFlags.set (triggered_flag);
  auto triggerTime = lastUpdateTime + ntime ();
  if (opFlags[interpolate_flag])
    {
      rampSource::setState (nextUpdateTime, nullptr,nullptr,cLocalSolverMode);
      if (opFlags[repeated_flag])
        {
          nextStep (triggerTime);
          nextUpdateTime = triggerTime;
          rampSource::setState (time, nullptr, nullptr, cLocalSolverMode);
        }
      else
        {
          rampSource::clearRamp ();
          nextUpdateTime = maxTime;
          opFlags.set (object_armed_flag,false);
          prevTime = time;
          keyTime = time;
        }

    }
  else
    {
      double rval = nval ();

      m_output = (opFlags[proportional_flag]) ? m_output + rval * m_output : m_output + rval;

      if (opFlags[repeated_flag])
        {
          nextUpdateTime = triggerTime;
        }
      else
        {
          nextUpdateTime = maxTime;
          opFlags.reset (object_armed_flag);
        }
      prevTime = time;
      keyTime = time;
    }
  if (nextUpdateTime <= time)
  {
	  updateA(time);
  }
  m_tempOut = m_output;
}


gridDyn_time randomSource::ntime ()
{
  gridDyn_time triggerTime = maxTime;
  switch (timeGenerator->getDistribution ())
    {
    case gridRandom::dist_type_t::constant:
      triggerTime = mean_t;
      break;
    case  gridRandom::dist_type_t::uniform:
      triggerTime = timeGenerator->getNewValue (min_t, max_t);
      break;
    case  gridRandom::dist_type_t::exponential:
      triggerTime = timeGenerator->getNewValue (mean_t, 0.0);
      break;
    case gridRandom::dist_type_t::normal:
      //mean_t and stdev_t are the location and scale parameter respectively
      triggerTime = exp (mean_t + scale_t * timeGenerator->getNewValue ());
      break;
    default:
      assert (false);
    }
  return triggerTime;
}

double randomSource::nval ()
{
  double nextVal = 0.0;
  switch (valGenerator->getDistribution ())
    {
    case  gridRandom::dist_type_t::constant:
      nextVal = mean_L;
      break;
    case  gridRandom::dist_type_t::uniform:
      nextVal = valGenerator->getNewValue (min_L, max_L) - (max_L - min_L) * zbias * (offset);
      break;
    case  gridRandom::dist_type_t::exponential:      //load varies in a biexponential pattern
      nextVal = valGenerator->getNewValue () + offset / mean_L * zbias - 0.5;

      break;
    case  gridRandom::dist_type_t::normal:
      nextVal = valGenerator->getNewValue (mean_L, stdev_L) - stdev_L * zbias * (offset);
      break;
    default:
      assert (false);
    }

  offset = offset + nextVal;
  return nextVal;
}

void randomSource::nextStep (gridDyn_time triggerTime)
{
  double rval;
  double nextVal;

  rval = nval ();
  nextVal = (opFlags[proportional_flag]) ? m_output + rval * m_output : m_output + rval;
  mp_dOdt = (nextVal - m_output) / (triggerTime - keyTime);

}


void randomSource::timestep (gridDyn_time ttime, const IOdata &args, const solverMode &sMode)
{
	updateA(ttime);
	rampSource::timestep(ttime, args, sMode);
}
