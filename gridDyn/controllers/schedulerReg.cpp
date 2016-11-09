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

#include "scheduler.h"
#include "AGControl.h"
#include "gridCoreTemplates.h"
#include "comms/schedulerMessage.h"

#include <cstdio>

schedulerReg::schedulerReg (const std::string &objName) : schedulerRamp (objName),
                                                          regMax (Pmax), regMin (Pmin), regRampUp (rampUp), regRampDown (rampDown)
{
  rampTime = 600;
}

schedulerReg::schedulerReg (double initialValue, const std::string &objName) : schedulerRamp (initialValue, objName),
                                                                               regMax (Pmax), regMin (Pmin), regRampUp (rampUp), regRampDown (rampDown)
{
  rampTime = 600;
}

schedulerReg::schedulerReg (double initialValue,double initialReg,const std::string &objName) : schedulerRamp (initialValue,objName),
                                                                                                regMax (Pmax), regMin (Pmin), regRampUp (rampUp), regRampDown (rampDown),regCurr (initialReg), regTarget (initialReg)
{

  rampTime = 600;
}

gridCoreObject *schedulerReg::clone (gridCoreObject *obj) const
{
  schedulerReg *nobj = cloneBase<schedulerReg, schedulerRamp> (this, obj);
  if (nobj == nullptr)
    {
      return nobj;
    }

  nobj->regCurr = regCurr;
  nobj->regTarget = regTarget;
  nobj->regUpFrac = regUpFrac;
  nobj->regDownFrac = regDownFrac;
  nobj->regMax = regMax;
  nobj->regMin = regMin;
  nobj->regRampUp = regRampUp;
  nobj->regRampDown = regRampDown;
  nobj->regEnabled = regEnabled;
  nobj->rampTime = rampTime;

  nobj->pr = pr;
  //copy the scheduler object last as it runs an initialize routine
  schedulerRamp::clone (nobj);

  return nobj;
}

schedulerReg::~schedulerReg ()
{
  clearSchedule ();
  if (agc != nullptr)
    {
      agc->remove (this);
    }

}


void schedulerReg::setReg (double regLevel)
{
  pr = (m_Base >= kHalfBigNum) ? regMax : m_Base;

  if (regLevel > regUpFrac * pr)
    {
      regTarget = regUpFrac * pr;
    }
  else if (regLevel < -regDownFrac * pr)
    {
      regTarget = -regDownFrac * pr;
    }
  else
    {
      regTarget = regLevel;
    }
}


void schedulerReg::updateA (double time)
{
  double dt = (time - prevTime);

  if (dt == 0)
    {
      return;
    }
  double prevOutput = m_output;
  schedulerRamp::updateA (time);

  double ramp = (regTarget - regCurr) / dt + dPdt;
  if (ramp > regRampUp)
    {
      ramp = regRampUp;
    }
  else if (ramp < -regRampDown)
    {
      ramp = -regRampDown;
    }

  m_output = prevOutput + ramp * dt;

  dPdt = ramp;
  regCurr = m_output - PCurr - reserveAct;

}

double schedulerReg::predict (double time)
{
  double dt = (time - prevTime);
  if (dt == 0)
    {
      return m_output;
    }
  double toutput = schedulerRamp::predict (time);

  double ramp = (regTarget - regCurr) / dt + (toutput - m_output) / dt;
  if (ramp > regRampUp)
    {
      ramp = regRampUp;
    }
  else if (ramp < -regRampDown)
    {
      ramp = -regRampDown;
    }

  double retout = m_output + ramp * dt;
  return retout;
}

void schedulerReg::objectInitializeA (double time0, unsigned long flags)
{
  schedulerRamp::objectInitializeA (time0, flags);
  pr = (m_Base >= kHalfBigNum) ? regMax : m_Base;

  if ((regUpFrac > 0) | (regDownFrac > 0))
    {
      if (agc == nullptr)
        {
          dispatcherLink ();
        }
    }
}

void schedulerReg::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
  schedulerRamp::objectInitializeB (args, outputSet, inputSet);
  double AGClevel = (outputSet.size () > 2) ? outputSet[2] : 0;
  if (AGClevel > regUpFrac * pr)
    {
      regCurr = regUpFrac * pr;
    }
  else if (AGClevel < -regDownFrac * pr)
    {
      regCurr = -regDownFrac * pr;
    }
  else
    {
      regCurr = AGClevel;
    }

  m_output = regCurr + PCurr + reserveAct;

}

double schedulerReg::getRamp (double *tRem) const
{
  double ramp = 0;
  double diff = regTarget - regCurr;
  if (diff > 0.001)
    {
      ramp = regRampUp;
      *tRem = (regTarget - regCurr) / (ramp - PRampCurr);
    }
  else if (diff < 0.001)
    {
      ramp = regRampDown;
      *tRem = (regTarget - regCurr) / (ramp - PRampCurr);
    }
  else
    {
      ramp = schedulerRamp::getRamp (tRem);
    }
  return ramp;

}

double schedulerReg::getMax (const double /*time*/) const
{
  return Pmax;
}

double schedulerReg::getMin (double /*time*/) const
{
  return Pmin;
}


void schedulerReg::regSettings (bool active, double upFrac,double downFrac)
{
  if (upFrac < 0)
    {
      if (regEnabled)
        {
          if (active == false)
            {
              if (agc != NULL)
                {
                  agc->remove (this);
                }
              regEnabled = false;
            }
        }
      else
        {
          if (active)
            {
              regEnabled = true;
              if (agc != NULL)
                {
                  agc->add (this);
                }
              else
                {
                  dispatcherLink ();
                }

            }
        }
    }
  else
    {
      regEnabled = active;
      if (downFrac < 0)
        {
          regUpFrac = upFrac;
          regDownFrac = downFrac;
        }
      else
        {
          regUpFrac = upFrac;
          regDownFrac = downFrac;
        }
    }
  if (regEnabled)
    {
      pr = (m_Base >= kHalfBigNum) ? regMax : m_Base;
      rampUp = regRampUp - regUpFrac * pr / 600;
      rampDown = regRampDown - regDownFrac * pr / 600;
      Pmax = regMax - regUpFrac * pr;
      Pmin = regMin + regDownFrac * pr;
    }
  else
    {
      rampUp = regRampUp;
      rampDown = regRampDown;
      Pmax = regMax;
      Pmin = regMin;
    }
  updatePTarget ();
  if (agc != NULL)
    {
      agc->regChange ();
    }
}


void schedulerReg::set (const std::string &param,  const std::string &val)
{

  schedulerRamp::set (param, val);

}

void schedulerReg::set (const std::string &param, double val,gridUnits::units_t unitType)
{
 
  double temp;
  if (param == "max")
    {
      regMax = val;
    }
  else if (param == "min")
    {
      regMin = val;

    }
  else if (param == "rampup")
    {
      regRampUp = val;
    }
  else if (param == "rampdown")
    {
      regRampDown = val;
      if (regRampDown < 0)
        {
          regRampDown = -regRampDown;
        }

    }
  else if (param == "ramp")
    {
      regRampUp = val;
      regRampDown = val;

    }
  else if ((param == "rating")||(param == "base"))
    {
      m_Base = val;
      if (agc)
        {
          agc->regChange ();
        }
    }
  else if (param == "regfrac")
    {
      temp = val;
      regUpFrac = temp;
      regDownFrac = temp;
      if (agc)
        {
          agc->regChange ();
        }
    }
  else if (param == "regupfrac")
    {
      regUpFrac = val;
      if (agc)
        {
          agc->regChange ();
        }
    }
  else if (param == "regdownfrac")
    {
      regDownFrac = val;

      if (agc)
        {
          agc->regChange ();
        }
    }
  else if (param == "regenabled")
    {
      bool active = (val > 0) ? true : false;
      if (regEnabled)
        {
          if (active == false)
            {
              if (agc)
                {
                  agc->remove (this);
                }
              regEnabled = false;
            }
        }
      else
        {
          if (active)
            {
              regEnabled = true;
              if (agc)
                {
                  agc->add (this);
                }
            }
        }
    }
  else
    {
      schedulerRamp::set (param,val,unitType);
    }
  if (regEnabled)
    {
      pr = (m_Base >= kHalfBigNum) ? regMax : m_Base;
      rampUp = regRampUp - regUpFrac * pr / 600;
      rampDown = regRampDown - regDownFrac * pr / 600;
      Pmax = regMax - regUpFrac * pr;
      Pmin = regMin + regDownFrac * pr;
    }
  else
    {
      rampUp = regRampUp;
      rampDown = regRampDown;
      Pmax = regMax;
      Pmin = regMin;
    }
  updatePTarget ();

}



void schedulerReg::dispatcherLink ()
{
  agc = static_cast<AGControl *> (find ("agc"));
  if (agc)
    {
      agc->add (this);
    }
  schedulerRamp::dispatcherLink ();
}

double schedulerReg::get (const std::string &param,gridUnits::units_t unitType) const
{
  double val;
  if (param == "min")
    {
      val = Pmin;
    }
  else if (param == "max")
    {
      val = Pmax;
    }
  else
    {
      return schedulerRamp::get (param,unitType);
    }
  return val;
}


void schedulerReg::receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message)
{
  auto sm = std::dynamic_pointer_cast<schedulerMessage> (message);
  switch (sm->getMessageType ())
    {
    case schedulerMessage::CLEAR_TARGETS:
      clearSchedule ();
      break;
    case schedulerMessage::SHUTDOWN:
      break;
    case schedulerMessage::STARTUP:
      break;
    case schedulerMessage::UPDATE_TARGETS:
      break;
    default:
      schedulerRamp::receiveMessage (sourceID, message);
      break;
    }

}