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
#include "reserveDispatcher.h"
#include "timeSeries.h"
#include "comms/schedulerMessage.h"
#include "gridCoreTemplates.h"
#include "stringOps.h"

schedulerRamp::schedulerRamp (const std::string &objName) : scheduler (objName)
{

}

schedulerRamp::schedulerRamp (double initialValue, const std::string &objName) : scheduler (initialValue,objName)
{

}

gridCoreObject *schedulerRamp::clone (gridCoreObject *obj) const
{
  schedulerRamp *nobj = cloneBase<schedulerRamp, scheduler> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }

  nobj->rampUp = rampUp;
  nobj->rampDown = rampDown;
  nobj->PRampCurr = PRampCurr;
  nobj->rampTime = rampTime;
  nobj->dPdt = dPdt;
  nobj->lastTargetTime = lastTargetTime;
  nobj->mode = mode;
  nobj->reserveAvail = reserveAvail;
  nobj->reserveUse = reserveUse;
  nobj->reserveAct = reserveAct;
  nobj->reservePriority = reservePriority;
  nobj->reserveRampTime = reserveRampTime;
  nobj->ramp10Up = ramp10Up;
  nobj->ramp30Up = ramp30Up;
  nobj->ramp10Down = ramp10Down;
  nobj->ramp30Down = ramp30Down;

  return nobj;
}

void schedulerRamp::setTarget (double target)
{
  insertTarget (tsched(prevTime, target));
}


void schedulerRamp::setTarget (double time, double target)
{

  insertTarget (tsched(time, target));
  if (time == nextUpdateTime)
    {
      updatePTarget ();
    }
}



void schedulerRamp::updateA (double time)
{
  double dt = (time - prevTime);

  if (dt == 0)
    {
      return;
    }

  if (time >= nextUpdateTime)
    {
      double otime = nextUpdateTime;
      dt = nextUpdateTime - prevTime;
      PCurr = PCurr + PRampCurr * dt;
      dPdt = getRamp ();
      m_output = m_output + dPdt * dt;
      prevTime = nextUpdateTime;

      updatePTarget ();

      dt = time - otime;
    }

  PCurr = PCurr + PRampCurr * dt;
  dPdt = getRamp ();
  m_output = m_output + dPdt * dt;
  reserveAct = m_output - PCurr;
  prevTime = time;

}

double schedulerRamp::predict (double time)
{
  double dt = (time - prevTime);
  if (dt == 0)
    {
      return m_output;
    }
  double ramp = getRamp ();
  return (m_output + ramp * dt);

}


void schedulerRamp::objectInitializeA (double time0, unsigned long flags)
{
  scheduler::objectInitializeA (time0, flags);
  prevTime = time0 - 0.001;
  lastTargetTime = time0 - 0.001;
}

void schedulerRamp::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
  scheduler::objectInitializeB (args, outputSet, inputSet);
  if (reserveAvail > 0)
    {
      //if (resDispatch==NULL)
      //	{
      //		reserveDispatcherLink();
      //	}
    }
  while (!pTarget.empty ())
    {
      if ((pTarget.front ()).time < prevTime)
        {
          pTarget.pop_front ();
        }
      else
        {
          break;
        }
    }
  updatePTarget ();
  dPdt = PRampCurr;
}


double schedulerRamp::getRamp () const
{
  double ramp;
  double diff = reserveUse - reserveAct;
  if (diff > 0.001)
    {
      ramp = rampUp;
    }
  else if (diff < -0.001)
    {
      ramp = -rampDown;
    }
  else
    {
      ramp = PRampCurr;
    }

  return ramp;
}
double schedulerRamp::getRamp (double *tRem) const
{

  double ramp = 0;
  double diff = reserveUse - reserveAct;
  if (diff > 0.001)
    {
      ramp = rampUp;
      *tRem = (diff) / (ramp - PRampCurr);
    }
  else if (diff < -0.001)
    {
      ramp = -rampDown;
      *tRem = (diff) / (ramp - PRampCurr);
    }
  else
    {
      if (pTarget.empty ())
        {
          *tRem = kDayLength;
        }
      else
        {
          *tRem = (pTarget.front ()).time - prevTime;
        }

    }
  return ramp;
}

double schedulerRamp::getMax (const double /*time*/) const
{
  return Pmax;
}

double schedulerRamp::getMin (double /*time*/) const
{
  return Pmin;
}

void schedulerRamp::setReserveTarget (double target)
{
  if (target <= reserveAvail)
    {
      reserveUse = target;
    }
  else
    {
      reserveUse = reserveAvail;
    }
}

void schedulerRamp::set (const std::string &param,  const std::string &val)
{
 
  if (param == "rampmode")
    {
      auto v2 = convertToLowerCase (val);
      if (v2 == "midpoint")
        {
          mode = midPoint;
        }
      else if (v2 == "justInTime")
        {
          mode = justInTime;
        }
      else if (v2 == "ontargetramp")
        {
          mode = onTargetRamp;
        }
      else if (v2 == "delayed")
        {
          mode = delayed;
        }
      else if (v2 == "interp")
        {
          mode = interp;
        }
    }
  else
    {
      scheduler::set (param, val);
    }

}

void schedulerRamp::dispatcherLink ()
{

}

void schedulerRamp::set (const std::string &param, double val,gridUnits::units_t unitType)
{
  double temp;
  if (param == "ramp")
    {
      rampUp = gridUnits::unitConversion (val,unitType,gridUnits::puMWps,m_Base);
      rampDown = rampUp;
    }
  else if (param == "rampup")
    {
      rampUp = val;
    }
  else if (param == "rampdown")
    {
      rampDown = val;
      if (rampDown < 0)
        {
          rampDown = -rampDown;
        }
    }
  if (param == "ramp10")
    {
      temp = val;
      ramp10Up = temp;
      ramp10Down = temp;
    }
  else if (param == "ramp10up")
    {
      ramp10Up = val;
    }
  else if (param == "ramp10down")
    {
      ramp10Down = val;
      if (ramp10Down < 0)
        {
          ramp10Down = -ramp10Down;
        }
    }

  if (param == "ramp30")
    {
      temp = val;
      ramp30Up = temp;
      ramp30Down = temp;
    }
  else if (param == "ramp30up")
    {
      ramp30Up = val;
    }
  else if (param == "ramp30down")
    {
      ramp30Down = val;
      if (ramp30Down < 0)
        {
          ramp30Down = -ramp30Down;
        }
    }
  else if (param == "ramptime")
    {
      rampTime = val;
    }
  else if (param == "target")
    {
      setTarget (val);
    }
  else if (param == "reserve")
    {
      temp = val;
      //check to updateP the reservedispatcher
      if (temp != reserveAvail)
        {
          /*	if (reserveAvail==0)
                  {
                          reserveAvail=temp;
                          if (resDispatch==NULL)
                          {
                                  reserveDispatcherLink(NULL);
                          }
                  }
                  else
                  {
                          reserveAvail=temp;
                          if (resDispatch!=NULL)
                          {
                                  resDispatch->schedChange();
                          }
                  }
*/
        }

    }
  else if (param == "reserveramptime")
    {
      reserveRampTime = val;
    }
  else
    {
      scheduler::set (param,val);
    }
  updatePTarget ();

}

void schedulerRamp::setTarget (const std::string &filename)
{
  scheduler::setTarget (filename);
  updatePTarget ();

}


void schedulerRamp::updatePTarget ()
{
  double rempower = 0.0;
  double remtime = 0.0;
  double target;
  double time;
  double td2;
  double rampLimitUp;

  if (reserveAvail < 0.001)
    {
      rampLimitUp = rampUp;
    }
  else if (reserveUse == 0)
    {
      rampLimitUp = rampUp - reserveAvail / reserveRampTime;
    }
  else
    {
      rampLimitUp = rampUp - (reserveAvail - reserveUse) / reserveRampTime;
    }
  if (pTarget.empty ())
    {
      PRampCurr = 0;
      nextUpdateTime = kBigNum;
      return;
    }
  else
    {
      target = (pTarget.front ()).target;
      time = (pTarget.front ()).time;
      if (target > (Pmax - reserveAvail))
        {
          target = (Pmax - reserveAvail);
        }
      else if (target < Pmin)
        {
          target = Pmin;
        }

    }

  tsched tempTarget;
  if (time <= prevTime)
    {
      //get rid of first element
      pTarget.pop_front ();
      rempower = target - PCurr;
      //ignore small variations
      if ((rempower <= 0.0001) & (rempower >= -0.0001))
        {
          rempower = 0;
        }
      lastTargetTime = time;
      if (pTarget.empty ())
        {
          target = (pTarget.front ()).target;
          time = (pTarget.front ()).time;
          if (target > (Pmax - reserveAvail))
            {
              target = (Pmax - reserveAvail);
            }
          else if (target < Pmin)
            {
              target = Pmin;
            }
        }
      else
        {
          if (rempower != 0)
            {
              //assume we were ramp limited so just keep ramping
              remtime = rempower / PRampCurr;
              insertTarget (tsched (target,prevTime + remtime));
              nextUpdateTime = prevTime + remtime;
            }
          else
            {
              PRampCurr = 0;
              nextUpdateTime = kBigNum;
            }
          return;
        }


    }
  double td = (time - prevTime);
  double pdiff = target - PCurr;
  if (rempower == 0)
    {
      if ((pdiff < 0.0001) & (pdiff > -0.0001))
        {
          PRampCurr = 0;
          nextUpdateTime = time;
          return;
        }
    }

  switch (mode)
    {
    case interp:
      nextUpdateTime = time;
      PRampCurr = pdiff / td;
      if (PRampCurr > rampLimitUp)
        {
          PRampCurr = rampLimitUp;
        }
      else if (PRampCurr < -rampDown)
        {
          PRampCurr = -rampDown;
        }
      break;
    case midPoint:
      if (td >= rampTime)
        {
          if (rempower)
            {
              /*keep ramp until we would begin ramping for the next target*/
              remtime = rempower / PRampCurr;
              if (remtime < ((td - rampTime) / 2.0))
                {
                  nextUpdateTime = prevTime + remtime;
                }
              else
                {
                  nextUpdateTime = prevTime + ((td - rampTime) / 2.0);
                }
            }
          else
            {
              td2 = time - lastTargetTime;
              if ((prevTime - lastTargetTime) >= (td2 - rampTime) / 2.0)
                {
                  if (prevTime < (lastTargetTime + (td2 - rampTime) / 2.0 + rampTime))
                    {
                      PRampCurr = pdiff / rampTime;
                      if (PRampCurr > rampLimitUp)
                        {
                          PRampCurr = rampLimitUp;
                        }
                      else if (PRampCurr < -rampDown)
                        {
                          PRampCurr = -rampDown;
                        }
                      nextUpdateTime = lastTargetTime + (td2 - rampTime) / 2.0 + rampTime;
                    }
                  else
                    {
                      remtime = pdiff / PRampCurr;
                      nextUpdateTime = prevTime + remtime;
                      if (time < nextUpdateTime)
                        {
                          nextUpdateTime = time;
                        }

                    }
                }
              else
                {
                  PRampCurr = 0;
                  nextUpdateTime = lastTargetTime + (td2 - rampTime) / 2.0;
                }
            }

        }
      else
        {
          td2 = time - lastTargetTime;
          if (prevTime >= (lastTargetTime + (td2 - rampTime) / 2.0 + rampTime))
            {
              remtime = pdiff / PRampCurr;
              nextUpdateTime = prevTime + remtime;
              if (time < nextUpdateTime)
                {
                  nextUpdateTime = time;
                }
            }
          else
            {
              nextUpdateTime = time;
              if (td == 0)
                {
                  if (pdiff > 0)
                    {
                      PRampCurr = rampLimitUp;
                    }
                  else
                    {
                      PRampCurr = -rampDown;
                    }
                }
              else
                {
                  PRampCurr = pdiff / td;
                  if (PRampCurr > rampLimitUp)
                    {
                      PRampCurr = rampLimitUp;
                    }
                  else if (PRampCurr < -rampDown)
                    {
                      PRampCurr = -rampDown;
                    }

                }
            }

        }
      break;
    case delayed:
      if (rempower)
        {
          if (rempower > 0)
            {
              remtime = rempower / rampLimitUp;
            }
          else
            {
              remtime = rempower / rampDown;
            }
          if (remtime < rampTime)
            {
              remtime = rampTime;
            }
          if (remtime > td)
            {
              remtime = td;
            }
          PRampCurr = rempower / remtime;
          if (PRampCurr > rampLimitUp)
            {
              PRampCurr = rampLimitUp;
            }
          else if (PRampCurr < -rampDown)
            {
              PRampCurr = -rampDown;
            }
          nextUpdateTime = prevTime + remtime;
        }
      else
        {
          PRampCurr = 0;
          nextUpdateTime = time;
        }
      break;
    case justInTime:
    case onTargetRamp:
      break;


    }


}

void schedulerRamp::insertTarget (tsched ts)
{
  scheduler::insertTarget (ts);
  if (nextUpdateTime == ts.time)
    {
      updatePTarget ();
    }
}

double schedulerRamp::get (const std::string &param, gridUnits::units_t unitType) const
{
  double val = kNullVal;
  if (param == "reserve")
    {
      val = reserveAvail;
    }
  else
    {
      val = scheduler::get (param,unitType);
    }
  return val;
}


void schedulerRamp::receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message)
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
    case schedulerMessage::UPDATE_RESERVES:
      break;
    case schedulerMessage::USE_RESERVE:
      break;
    default:
      scheduler::receiveMessage (sourceID, message);
      break;
    }

}

/*
void schedulerRamp::reserveDispatcherLink(reserveDispatcher *rD)
{
        if (rD==NULL)
        {
                resDispatch=(reserveDispatcher *)find("reservedispatcher");
                if (resDispatch!=NULL)
                {
                        resDispatch->addGen(this);
                }
        }
        else
        {
                if (resDispatch==NULL)
                {
                        resDispatch=rD;
                }
                else
                {
                        if (resDispatch!=rD)
                        {
                                resDispatch->removeSched(this);
                                resDispatch=rD;
                        }
                }
        }
}
*/
