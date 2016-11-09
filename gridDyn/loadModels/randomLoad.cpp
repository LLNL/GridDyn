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
#include "gridBus.h"
#include "gridRandom.h"
#include "stringOps.h"
#include "gridCoreTemplates.h"
#include "core/gridDynExceptions.h"
#include <iostream>
#include <cassert>

using namespace gridUnits;

gridRandomLoad::gridRandomLoad (const std::string &objName) : gridRampLoad (objName)
{
  timeGenerator = std::unique_ptr<gridRandom> (new gridRandom (gridRandom::dist_type_t::constant));
  valGenerator = std::unique_ptr<gridRandom> (new gridRandom (gridRandom::dist_type_t::constant));
}

gridRandomLoad::gridRandomLoad (double rP, double rQ, const std::string &objName) : gridRampLoad (rP,rQ,objName), mean_L (rP)
{
  // default values
  timeGenerator = std::unique_ptr<gridRandom> (new gridRandom (gridRandom::dist_type_t::constant));
  valGenerator = std::unique_ptr<gridRandom> (new gridRandom (gridRandom::dist_type_t::constant));
}


gridCoreObject *gridRandomLoad::clone (gridCoreObject *obj) const
{
  gridRandomLoad *ld = cloneBase<gridRandomLoad, gridRampLoad> (this, obj);
  if (ld == nullptr)
    {
      return obj;
    }

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
  ld->opFlags.reset (armed_flag);
  ld->keyTime = keyTime;
  ld->timeGenerator->setDistribution (timeGenerator->getDistribution ());
  ld->valGenerator->setDistribution (valGenerator->getDistribution ());
  return ld;
}

// destructor
gridRandomLoad::~gridRandomLoad ()
{
}


// set properties
void gridRandomLoad::set (const std::string &param,  const std::string &val)
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
      gridLoad::set (param, val);
    }

}

void gridRandomLoad::setFlag (const std::string &flag, bool val)
{


  /*
  independent_flag=object_flag3,
  interpolate_flag=object_flag4,
  repeated_flag=object_flag5,
  proportional_flag=object_flag6,
  triggered_flag=object_flag7,
  armed_flag=object_flag8,*/
  if (flag == "independent")
    {
      opFlags.set (independent_flag, val);
    }
  else if (flag == "interpolate")
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
      gridLoad::setFlag (flag, val);
    }
 
}

void gridRandomLoad::set (const std::string &param, double val, units_t unitType)
{

  if (param == "min_t")
    {
      if (val <= 0)
        {
          LOG_WARNING ("min_t parameter must be > 0");
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
      min_L = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "max_l")
    {
      max_L = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "mean_t")
    {

      if (val <= 0)
        {
          LOG_WARNING ( "mean_t parameter must be > 0" );
		  throw(invalidParameterValue());
        }
      else
        {
          mean_t = val;
        }
    }
  else if (param == "mean_l")
    {
      mean_L = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "scale_t")
    {
      if (val <= 0)
        {
          LOG_WARNING ("scale_t parameter must be > 0" );
		  throw(invalidParameterValue());
        }
      else
        {
          scale_t = val;
        }
    }
  else if (param == "stdev_l")
    {
      stdev_L = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }

  else if (param == "interpolate")
    {
      opFlags.set (interpolate_flag, (val > 0));
    }
  else if (param == "repeated")
    {
      opFlags.set (repeated_flag, (val > 0));
    }
  else if (param == "independent")
    {
      opFlags.set (independent_flag, (val > 0));
    }
  else if (param == "proportional")
    {
      opFlags.set (proportional_flag, (val > 0));
    }
  else if (param == "seed")
    {
      timeGenerator->setSeed (static_cast<int> (val));
      valGenerator->setSeed (static_cast<int> (val + 1)); //just so it is different than the timeGenerator
    }
  else
    {
      //I am purposely skipping over the rampLoad the functionality is needed but the access is not
      gridLoad::set (param, val, unitType);
    }
  
}

void gridRandomLoad::pFlowObjectInitializeA (double time0, unsigned long flags)
{
  double triggerTime;

  if ((opFlags[triggered_flag]) || (isArmed ()))    //first time setup
    {
      dynInitializeA (time0, flags);
    }
  else
    {
      keyTime = time0;
      prevTime = time0;
      triggerTime = prevTime + ntime ();

      if (opFlags.test (interpolate_flag))
        {
          nextStep (triggerTime);
        }
      nextUpdateTime = triggerTime;
      opFlags.set (armed_flag);
      alert (this, UPDATE_REQUIRED);
      opFlags.set (has_updates, true);
    }
  gridLoad::pFlowObjectInitializeA (time0,flags);
}

void gridRandomLoad::dynObjectInitializeA (double time0, unsigned long flags)
{
  if (isArmed ())
    {
      if (time0 != keyTime)
        {
          if (time0 + kSmallTime >= nextUpdateTime)
            {
              updateA (time0);
            }
          gridRampLoad::setState (time0, nullptr, nullptr, cLocalSolverMode);
        }
      gridRampLoad::dynObjectInitializeA (time0, flags);
    }
}


void gridRandomLoad::updateA (double time)
{
  if (time + kSmallTime < nextUpdateTime)
    {
      return;
    }


  m_lastUpdateTime = nextUpdateTime;
  opFlags.set (triggered_flag);
  double triggerTime = m_lastUpdateTime + ntime ();
  if (opFlags.test (interpolate_flag))
    {
      gridRampLoad::setState (nextUpdateTime, nullptr,nullptr,cLocalSolverMode);
      if (opFlags.test (repeated_flag))
        {
          nextStep (triggerTime);
          nextUpdateTime = triggerTime;
          gridRampLoad::setState (time, nullptr, nullptr, cLocalSolverMode);
        }
      else
        {
          gridRampLoad::clearRamp ();
          nextUpdateTime = kBigNum;
          opFlags.set (armed_flag,false);
          prevTime = time;
          keyTime = time;
        }

    }
  else
    {
      double rval;
      if (opFlags.test (independent_flag))
        {
          if (P != 0)
            {
              rval = nval ();
              P = (opFlags.test (proportional_flag)) ? P + rval * P : P + rval;
            }
          if (Q != 0)
            {
              rval = nval ();
              Q = (opFlags.test (proportional_flag)) ? Q + rval * Q : Q + rval;
            }
          if (Yp != 0)
            {
              rval = nval ();
              Yp = (opFlags.test (proportional_flag)) ? Yp + rval * Yp : Yp + rval;
            }
          if (Yq != 0)
            {
              rval = nval ();
              Yq = (opFlags.test (proportional_flag)) ? Yq + rval * Yq : Yq + rval;
            }
          if (Ip != 0)
            {
              rval = nval ();
              Ip = (opFlags.test (proportional_flag)) ? Ip + rval * Ip : Ip + rval;
            }
          if (Iq != 0)
            {
              rval = nval ();
              Iq = (opFlags.test (proportional_flag)) ? Iq + rval * Iq : Iq + rval;
            }
        }
      else
        {
          rval = nval ();
          if (P != 0)
            {
              P = (opFlags.test (proportional_flag)) ? P + rval * P : P + rval;
            }
          if (Q != 0)
            {
              Q = (opFlags.test (proportional_flag)) ? Q + rval * Q : Q + rval;
            }
          if (Yp != 0)
            {
              Yp = (opFlags.test (proportional_flag)) ? Yp + rval * Yp : Yp + rval;
            }
          if (Yq != 0)
            {
              Yq = (opFlags.test (proportional_flag)) ? Yq + rval * Yq : Yq + rval;
            }
          if (Ip != 0)
            {
              Ip = (opFlags.test (proportional_flag)) ? Ip + rval * Ip : Ip + rval;
            }
          if (Iq != 0)
            {
              Iq = (opFlags.test (proportional_flag)) ? Iq + rval * Iq : Iq + rval;
            }
        }
      if (opFlags.test (repeated_flag))
        {
          nextUpdateTime = triggerTime;
        }
      else
        {
          nextUpdateTime = kBigNum;
          opFlags.reset (armed_flag);
        }
      prevTime = time;
      keyTime = time;
    }
}


double gridRandomLoad::ntime ()
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
      triggerTime = timeGenerator->getNewValue (mean_t,0.0);
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

double gridRandomLoad::nval ()
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
      nextVal = valGenerator->getNewValue (mean_L,stdev_L) - stdev_L * zbias * (offset);
      break;
    default:
      assert (false);

    }
  if (!(opFlags.test (independent_flag)))
    {
      offset = offset + nextVal;
    }
  return nextVal;
}

void gridRandomLoad::nextStep (double triggerTime)
{
  double rval;
  double nextVal;
  if (opFlags.test (independent_flag))
    {
      if (P != 0)
        {
          rval = nval ();
          nextVal = (opFlags.test (proportional_flag)) ? P + rval * P : P + rval;
          dPdt = (nextVal - P) / (triggerTime - keyTime);
        }
      if (Q != 0)
        {
          rval = nval ();
          nextVal = (opFlags.test (proportional_flag)) ? Q + rval * Q : Q + rval;
          dQdt = (nextVal - Q) / (triggerTime - keyTime);
        }
      if (Yp != 0)
        {
          rval = nval ();
          nextVal = (opFlags.test (proportional_flag)) ? Yp + rval * Yp : Yp + rval;
          dYpdt = (nextVal - Yp) / (triggerTime - keyTime);
        }
      if (Yq != 0)
        {
          rval = nval ();
          nextVal = (opFlags.test (proportional_flag)) ? Yq + rval * Yq : Yq + rval;
          dYqdt = (nextVal - Yq) / (triggerTime - keyTime);
        }
      if (Ip != 0)
        {
          rval = nval ();
          nextVal = (opFlags.test (proportional_flag)) ? Ip + rval * Ip : Ip + rval;
          dIpdt = (nextVal - Ip) / (triggerTime - keyTime);
        }
      if (Iq != 0)
        {
          rval = nval ();
          nextVal = (opFlags.test (proportional_flag)) ? Iq + rval * Iq : Iq + rval;
          dIqdt = (nextVal - P) / (triggerTime - keyTime);
        }
    }
  else
    {
      rval = nval ();
      if (P != 0)
        {
          nextVal = (opFlags.test (proportional_flag)) ? P + rval * P : P + rval;
          dPdt = (nextVal - P) / (triggerTime - keyTime);
        }
      if (Q != 0)
        {
          nextVal = (opFlags.test (proportional_flag)) ? Q + rval * Q : Q + rval;
          dQdt = (nextVal - Q) / (triggerTime - keyTime);
        }
      if (Yp != 0)
        {
          nextVal = (opFlags.test (proportional_flag)) ? Yp + rval * Yp : Yp + rval;
          dYpdt = (nextVal - Yp) / (triggerTime - keyTime);
        }
      if (Yq != 0)
        {
          nextVal = (opFlags.test (proportional_flag)) ? Yq + rval * Yq : Yq + rval;
          dYqdt = (nextVal - Yq) / (triggerTime - keyTime);
        }
      if (Ip != 0)
        {
          nextVal = (opFlags.test (proportional_flag)) ? Ip + rval * Ip : Ip + rval;
          dIpdt = (nextVal - Ip) / (triggerTime - keyTime);
        }
      if (Iq != 0)
        {
          nextVal = (opFlags.test (proportional_flag)) ? Iq + rval * Iq : Iq + rval;
          dIqdt = (nextVal - Iq) / (triggerTime - keyTime);
        }
    }
}




void gridRandomLoad::setTime (double time)
{
  double in = prevTime - m_lastUpdateTime;

  nextUpdateTime = time + (nextUpdateTime - prevTime);
  m_lastUpdateTime = time - in;
  prevTime = time;
}


void gridRandomLoad::timestep (double ttime, const IOdata &args,const solverMode &sMode)
{
  if (ttime > nextUpdateTime)
    {
      m_lastUpdateTime = nextUpdateTime;
      opFlags.set (triggered_flag);
      double triggerTime = m_lastUpdateTime + ntime ();
      if (opFlags.test (interpolate_flag))
        {
          gridRampLoad::timestep (prevTime, args,sMode);
          if (opFlags.test (repeated_flag))
            {
              nextStep (triggerTime);
              nextUpdateTime = triggerTime;
            }
          else
            {
              clearRamp ();
              nextUpdateTime = kBigNum;
              opFlags.set (armed_flag,false);
              prevTime = ttime;
            }
          gridRampLoad::timestep (ttime,args,sMode);
        }
      else
        {
          double rval;

          rval = nval ();
          Psched = (opFlags.test (proportional_flag)) ? Psched + rval * Psched : Psched + rval;

          if (opFlags.test (repeated_flag))
            {
              nextUpdateTime = triggerTime;
            }
          else
            {
              nextUpdateTime = kBigNum;
              opFlags.set (armed_flag, false);
            }
          gridRampLoad::timestep (ttime, args, sMode);
        }
    }
  else
    {
      gridRampLoad::timestep (ttime, args, sMode);
    }
}


