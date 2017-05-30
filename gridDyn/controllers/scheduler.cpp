/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include "scheduler.h"
#include "utilities/timeSeries.h"
#include "dispatcher.h"
#include "comms/gridCommunicator.h"
#include "comms/schedulerMessage.h"
#include "core/coreObjectTemplates.h"

using namespace gridUnits;

//operator overloads for Tsched object
bool operator< (const tsched &td1, const tsched &td2)
{
  return (td1.time < td2.time);
}
bool operator<= (const tsched &td1, const tsched &td2)
{
  return (td1.time <= td2.time);
}
bool operator> (const tsched &td1, const tsched &td2)
{
  return (td1.time > td2.time);
}
bool operator>= (const tsched &td1, const tsched &td2)
{
  return (td1.time >= td2.time);
}
bool operator== (const tsched &td1, const tsched &td2)
{
  return (td1.time == td2.time);
}
bool operator!= (const tsched &td1, const tsched &td2)
{
  return (td1.time != td2.time);
}
bool operator< (const tsched &td1, coreTime timeC)
{
  return (td1.time < timeC);
}
bool operator<= (const tsched &td1, coreTime timeC)
{
  return (td1.time <= timeC);
}
bool operator> (const tsched &td1, coreTime timeC)
{
  return (td1.time > timeC);
}
bool operator>= (const tsched &td1, coreTime timeC)
{
  return (td1.time >= timeC);
}
bool operator== (const tsched &td1, coreTime timeC)
{
  return (td1.time == timeC);
}
bool operator!= (const tsched &td1, coreTime timeC)
{
  return (td1.time != timeC);
}

scheduler::scheduler(const std::string &objName, double initialValue) : gridSource(objName, initialValue), PCurr(initialValue)
{
	prevTime = negTime;     //override default setting

}

scheduler::scheduler (double initialValue, const std::string &objName) : scheduler(objName,initialValue)
{
  
}



coreObject *scheduler::clone (coreObject *obj) const
{
  scheduler *nobj = cloneBase<scheduler, gridSource> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }
  nobj->Pmax = Pmax;
  nobj->Pmin = Pmin;
  nobj->pTarget = pTarget;
  nobj->m_Base = m_Base;

  return nobj;
}

scheduler::~scheduler ()
{
  clearSchedule ();
}

void scheduler::setTarget (double target)
{
  insertTarget (tsched (prevTime, target));
}


void scheduler::setTarget (coreTime time, double target)
{

  insertTarget (tsched (time, target));

}

void scheduler::setTarget (std::vector<double> &time, std::vector<double> &target)
{

  auto tm = time.begin ();
  auto tg = target.begin ();
  auto tme = time.end ();
  auto tge = target.end ();
  std::list<tsched> flist;
  while ((tm != tme)&&(tg != tge))
    {
      //pTarget.push_back (tsched (*tm, *tg));
	  pTarget.emplace_back(*tm, *tg);
      ++tm;
      ++tg;
    }
  pTarget.sort ();
  if (pTarget.front ().time != nextUpdateTime)
    {
      nextUpdateTime = (pTarget.front ()).time;
      alert (this, UPDATE_TIME_CHANGE);
    }
}

void scheduler::setTarget (const std::string &filename)
{
  timeSeries<double,coreTime> targets;
  targets.loadFile (filename);
  
  std::list<tsched> flist;
  for (index_t kk = 0; kk < targets.size(); ++kk)
    {
      //flist.push_back (tsched (targets.time(kk), targets.data(kk)));
	  flist.emplace_back(targets.time(kk), targets.data(kk));
      //setTarget(targets.time[kk],targets.data[kk]);
    }
  flist.sort ();
  pTarget.merge (flist);
  if (pTarget.front ().time != nextUpdateTime)
    {
      nextUpdateTime = (pTarget.front ()).time;
      alert (this, UPDATE_TIME_CHANGE);
    }

}



void scheduler::updateA (coreTime time)
{
  auto dt = (time - prevTime);
  if (dt == timeZero)
    {
      return;
    }
  if (time >= nextUpdateTime)
    {
      while (time >= pTarget.front ().time)
        {
          PCurr = (pTarget.front ()).target;
          if (PCurr > Pmax)
            {
              PCurr = Pmax;
            }
          else if (PCurr < Pmin)
            {
              PCurr = Pmin;
            }

          pTarget.pop_front ();
          if (pTarget.empty ())
            {
              nextUpdateTime = maxTime;

              break;
            }
          nextUpdateTime = (pTarget.front ()).time;
        }
    }
  m_output = PCurr;
  prevTime = time;
  lastUpdateTime = time;
}

double scheduler::predict (coreTime time)
{
  double out = m_output;
  if (time >= nextUpdateTime)
    {
      out = (pTarget.front ()).target;
      if (out > Pmax)
        {
          out = Pmax;
        }
      else if (out < Pmin)
        {
          out = Pmin;
        }
    }
  return out;
}


void scheduler::dynObjectInitializeA (coreTime time0, unsigned long /*flags*/)
{
	commLink = cManager.build();

  commLink->registerReceiveCallback ([this](std::uint64_t sourceID, std::shared_ptr<commMessage> message) {
    receiveMessage (sourceID, message);
  });
  prevTime = time0;
}

void scheduler::dynObjectInitializeB (const IOdata & /*inputs*/, const IOdata &desiredOutput, IOdata & /*fieldSet*/)
{


  if (desiredOutput[0] > Pmax)
    {
      Pmax = desiredOutput[0];
    }
  else if (desiredOutput[0] < Pmin)
    {
      Pmin = desiredOutput[0];
    }

  //try to register to a dispatcher

  PCurr = desiredOutput[0];
  m_output = PCurr;
}



double scheduler::getTarget () const
{
  return (pTarget.empty ()) ? PCurr : (pTarget.front ()).target;
}


double scheduler::getMax(coreTime /*time*/) const
{
	return Pmax;
}

double scheduler::getMin(coreTime /*time*/) const
{
	return Pmin;
}

void scheduler::set (const std::string &param,  const std::string &val)
{
	if (param[0] == '#')
    {
      
    }
  else
    {
	  if (!cManager.set(param, val))
	  {
		  gridSource::set(param, val);
	  }
    }

}

void scheduler::set (const std::string &param, double val,units_t unitType)
{
  if (param == "min")
    {
      Pmin = unitConversion (val, unitType, puMW,m_Base);
      if (PCurr < Pmin)
        {
          PCurr = Pmin;
        }
    }
  else if (param == "max")
    {
      Pmax = unitConversion (val, unitType, puMW,m_Base);
      if (PCurr > Pmax)
        {
          PCurr = Pmax;
        }
    }
  else if (param == "base")
  {
	  m_Base = unitConversion(val, unitType, MW, systemBasePower);
  }
  else if (param == "target")
    {
      setTarget (unitConversion(val,unitType,puMW,m_Base));
    }
  else
    {
	  if (!cManager.set(param, val))
	  {
		  gridSource::set(param, val, unitType);
	  }
    }

}

void scheduler::setFlag(const std::string &flag, bool val)
{
	if (!cManager.setFlag(flag, val))
	{
		gridSource::setFlag(flag, val);
	}
}

double scheduler::get (const std::string &param, units_t unitType) const
{
  double val = kNullVal;
  if (param == "min")
    {
      val = unitConversion (Pmin, puMW, unitType,m_Base);
    }
  else if (param == "max")
    {
      val = unitConversion (Pmax, puMW, unitType, m_Base);
    }
  else
  {
	  val = gridSource::get(param, unitType);
  }
  return val;
}



void scheduler::clearSchedule ()
{
  if (!pTarget.empty ())
    {
      pTarget.resize (0);
      nextUpdateTime = maxTime;
      alert (this, UPDATE_TIME_CHANGE);
    }
}


void scheduler::insertTarget (tsched ts)
{

  if (ts < nextUpdateTime)
    {
      pTarget.push_front (ts);
      nextUpdateTime = ts.time;
      alert (this, UPDATE_TIME_CHANGE);
    }
  else
    {
      pTarget.push_back (ts);
      pTarget.sort ();
    }
}

void scheduler::receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message)
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
    case schedulerMessage::UPDATE_TARGETS:      //
      clearSchedule ();           //purposeful fall through
    case schedulerMessage::ADD_TARGETS:
      setTarget (sm->m_time, sm->m_target);
      break;
    case schedulerMessage::REGISTER_DISPATCHER:
      dispatcher_id = sourceID;
      break;
    default:
      break;
    }

}

void scheduler::dispatcherLink ()
{

		auto    dispatch = static_cast<dispatcher *> (getParent()->find("dispatcher"));
		if (dispatch)
		{
			dispatch->add(this);
		}
}



