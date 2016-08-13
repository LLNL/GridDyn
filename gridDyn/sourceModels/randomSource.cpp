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
#include "gridRandom.h"
#include "stringOps.h"
#include <iostream>
#include <cassert>

randomSource::randomSource (const std::string &objName, double startVal) : rampSource (objName,startVal)
{
  mean_L = startVal;
  timeGenerator = std::unique_ptr<gridRandom> (new gridRandom (gridRandom::dist_type_t::constant));
  valGenerator = std::unique_ptr<gridRandom> (new gridRandom (gridRandom::dist_type_t::constant));
}


gridCoreObject *randomSource::clone (gridCoreObject *obj) const
{
  randomSource *ld;
  if (obj == nullptr)
    {
      ld = new randomSource ();
    }
  else
    {
      ld = dynamic_cast<randomSource *> (obj);
      if (ld == nullptr)
        {
          rampSource::clone (obj);
          return obj;
        }
    }
  rampSource::clone (ld);


  ld->min_t = min_t;
  ld->max_t = max_t;
  ld->min_L = min_L;
  ld->max_L = max_L;
  ld->mean_t = mean_t;
  ld->mean_L = mean_L;
  ld->scale_t = scale_t;
  ld->stdev_L = stdev_L;
  ld->opFlags.reset (triggered_flag);
  ld->zbias = zbias;
  ld->opFlags.reset (object_armed_flag);
  ld->keyTime = keyTime;
  ld->timeGenerator->setDistribution (timeGenerator->getDistribution ());
  ld->valGenerator->setDistribution (valGenerator->getDistribution ());
  return ld;
}

// set properties
int randomSource::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;
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
      out = gridSource::set (param, val);
    }
  return out;
}

int randomSource::setFlag (const std::string &flag, bool val)
{
  int out = PARAMETER_FOUND;

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
      out = gridSource::setFlag (flag,val);
    }
  return out;
}

int randomSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;
  if (param == "min_t")
    {
      if (val <= 0)
        {
          std::cerr << "min_t parameter must be > 0\n";
          out = PARAMETER_NOT_FOUND;
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
          out = INVALID_PARAMETER_VALUE;
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
          out = INVALID_PARAMETER_VALUE;
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
      out = gridSource::set (param, val, unitType);
    }
  return out;
}

void randomSource::reset ()
{
  opFlags.reset (triggered_flag), opFlags.reset (object_armed_flag), offset = 0;
}

void randomSource::objectInitializeA (double time0, unsigned long flags)
{
  double triggerTime;

  if ((opFlags[triggered_flag]) || (isArmed ()))
    {
      if (isArmed ())
        {
          if (time0 != keyTime)
            {
              if (time0 >= nextUpdateTime)
                {
                  updateA (time0);
                }
              rampSource::setState (time0, nullptr, nullptr, cLocalSolverMode);
            }
          rampSource::objectInitializeA (time0, flags);
        }
    }
  else//first time setup
    {
      keyTime = time0;
      prevTime = time0;
      triggerTime = prevTime + ntime ();

      if (opFlags.test (interpolate_flag))
        {
          nextStep (triggerTime);
        }
      nextUpdateTime = triggerTime;
      opFlags.set (object_armed_flag);
    }
}


void randomSource::updateA (double time)
{
  if (time < nextUpdateTime)
    {
      return;
    }


  m_lastUpdateTime = nextUpdateTime;
  opFlags.set (triggered_flag);
  double triggerTime = m_lastUpdateTime + ntime ();
  if (opFlags.test (interpolate_flag))
    {
      rampSource::setState (nextUpdateTime, nullptr,nullptr,cLocalSolverMode);
      if (opFlags.test (repeated_flag))
        {
          nextStep (triggerTime);
          nextUpdateTime = triggerTime;
          rampSource::setState (time, nullptr, nullptr, cLocalSolverMode);
        }
      else
        {
          rampSource::clearRamp ();
          nextUpdateTime = kBigNum;
          opFlags.set (object_armed_flag,false);
          prevTime = time;
          keyTime = time;
        }

    }
  else
    {
      double rval;
      rval = nval ();

      m_output = (opFlags.test (proportional_flag)) ? m_output + rval * m_output : m_output + rval;

      if (opFlags.test (repeated_flag))
        {
          nextUpdateTime = triggerTime;
        }
      else
        {
          nextUpdateTime = kBigNum;
          opFlags.reset (object_armed_flag);
        }
      prevTime = time;
      keyTime = time;
    }
}


double randomSource::ntime ()
{
  double triggerTime = kBigNum;
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

void randomSource::nextStep (double triggerTime)
{
  double rval;
  double nextVal;

  rval = nval ();
  nextVal = (opFlags.test (proportional_flag)) ? m_output + rval * m_output : m_output + rval;
  mp_dOdt = (nextVal - m_output) / (triggerTime - keyTime);

}

void randomSource::setTime (double time)
{
  double in = prevTime - m_lastUpdateTime;

  nextUpdateTime = time + (nextUpdateTime - prevTime);
  m_lastUpdateTime = time - in;
  prevTime = time;
}


double randomSource::timestep (double ttime, const IOdata &args, const solverMode &sMode)
{
  if (ttime > nextUpdateTime)
    {
      m_lastUpdateTime = nextUpdateTime;
      opFlags.set (triggered_flag);
      double triggerTime = m_lastUpdateTime + ntime ();
      if (opFlags.test (interpolate_flag))
        {
          rampSource::timestep (nextUpdateTime,args,sMode);
          if (opFlags.test (repeated_flag))
            {
              nextStep (triggerTime);
              nextUpdateTime = triggerTime;
            }
          else
            {
              clearRamp ();
              nextUpdateTime = kBigNum;
              opFlags.set (object_armed_flag,false);
              prevTime = ttime;
            }
          rampSource::timestep (ttime, args,sMode);
        }
      else
        {
          double rval;

          rval = nval ();
          m_output = (opFlags.test (proportional_flag)) ? m_output + rval * m_output : m_output + rval;

          if (opFlags.test (repeated_flag))
            {
              nextUpdateTime = triggerTime;
            }
          else
            {
              nextUpdateTime = kBigNum;
              opFlags.set (object_armed_flag, false);
            }
          rampSource::timestep (ttime, args,sMode);
        }
      return m_output;
    }
  else
    {
      return rampSource::timestep (ttime,  args,sMode);
    }
}
