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

#include "AGControl.h"
#include "generators/gridDynGenerator.h"
#include "gridArea.h"
#include "reserveDispatcher.h"
#include "scheduler.h"
/*

class reserveDispatcher
{
public:
        std::string name;
        gridArea *Parent;
        bool enabled;

protected:
        double threshold;
        double dispatch
        double reserveAvailable;

        count_t schedCount;

        std::vector<scheduler *> schedList;
        std::vector<double> resAvailable;
        std::vector<double> resUsed;

public:
        reserveDispatcher();

        virtual ~reserveDispatcher();


        virtual double initialize(double time0,double dispatchSet);


        void setTime(double time);
        virtual double updateP(double time);
        virtual double testP(double time);
        double currentValue(){return dispatch;};

        virtual void addGen(scheduler *sched);
        virtual void removeSched(scheduler *sched);
        virtual int set (const std::string &param, double val,units_t unitType=defUnit);
        virtual int set (const std::string &param, double val,units_t unitType=defUnit){return set(param,&val, unitType);};

        double getAvailable(){return sum(&resAvailable)-sum(&resUsed);};

        virtual void schedChange();
protected:
        virtual void checkGen();
};


*/
reserveDispatcher::reserveDispatcher (const std::string &objName) : gridCoreObject (objName)
{

}

gridCoreObject *reserveDispatcher::clone (gridCoreObject *obj) const
{
  reserveDispatcher *nobj;
  if (obj == nullptr)
    {
      nobj = new reserveDispatcher ();
    }
  else
    {
      nobj = dynamic_cast<reserveDispatcher *> (obj);
      if (nobj == nullptr)
        {
          //if we can't cast the pointer clone at the next lower level
          gridCoreObject::clone (obj);
          return obj;
        }
    }
  gridCoreObject::clone (nobj);
  nobj->thresholdStart = thresholdStart;
  nobj->thresholdStop = thresholdStop;
  nobj->dispatchInterval = dispatchInterval;        //5 minutes
  return nobj;
}

reserveDispatcher::~reserveDispatcher ()
{
  index_t kk;
  for (kk = 0; kk < schedCount; kk++)
    {
      //schedList[kk]->reserveDispatcherUnlink();
    }
}


void reserveDispatcher::moveSchedulers (reserveDispatcher *rD)
{
  index_t kk;
  schedList.resize (this->schedCount + rD->schedCount);
  resUsed.resize (this->schedCount + rD->schedCount);
  resAvailable.resize (this->schedCount + rD->schedCount);

  for (kk = 0; kk < rD->schedCount; kk++)
    {
      //	rD->schedList[kk]->reserveDispatcherUnlink();
      this->schedList[this->schedCount + kk] = rD->schedList[kk];
      //	rD->schedList[kk]->reserveDispatcherLink(this);
    }
  checkGen ();
}


double reserveDispatcher::initializeA (double time0,double dispatchSet)
{

  currDispatch = dispatchSet;
  if (dispatchSet > 0)
    {
      dispatch (dispatchSet);
      dispatchTime = time0;
    }
  prevTime = time0;
  return currDispatch;
}


void reserveDispatcher::setTime (double time)
{
  prevTime = time;
}

double reserveDispatcher::updateP (double time,double pShort)
{
  if (currDispatch > 0)
    {
      if (time > (dispatchTime + dispatchInterval))
        {
          if (currDispatch + pShort < thresholdStop)
            {
              dispatch (0);
              dispatchTime = time;
            }
          else
            {
              dispatch (currDispatch + pShort);
              dispatchTime = time;
            }
        }
    }
  else
    {
      if (pShort > thresholdStart)
        {
          if ((time - dispatchTime) > dispatchInterval)
            {
              dispatch (pShort);
              dispatchTime = time;
            }

        }
    }
  return currDispatch;
}

double reserveDispatcher::testP (double time,double pShort)
{
  double output = 0;
  if (currDispatch > 0)
    {
      if (time > (dispatchTime + dispatchInterval))
        {
          if (currDispatch + pShort > thresholdStop)
            {
              output = currDispatch + pShort;
            }
        }
    }
  else
    {
      if (pShort > thresholdStart)
        {
          if ((time - dispatchTime) > dispatchInterval)
            {
              dispatch (pShort);
              dispatchTime = time;
            }
        }
    }
  return output;
}

int reserveDispatcher::remove (schedulerRamp *sched)
{
  size_t kk;
  for (kk = 0; kk < schedCount; kk++)
    {
      if (schedList[kk] == sched)
        {
          schedList.erase (schedList.begin () + kk);
          schedCount--;

          checkGen ();
          return OBJECT_REMOVE_SUCCESS;
        }
    }
  return OBJECT_REMOVE_FAILURE;
}

int reserveDispatcher::add (gridCoreObject *obj)
{
  if (dynamic_cast<schedulerRamp *> (obj))
    {
      return(add (static_cast<schedulerRamp *> (obj)));
    }
  return OBJECT_NOT_RECOGNIZED;
}

int reserveDispatcher::add (schedulerRamp *sched)
{
  schedCount++;
  schedList.push_back (sched);
  resUsed.resize (schedCount);
  resAvailable.resize (schedCount);
  //	sched->reserveDispatcherLink(this);
  checkGen ();
  return OBJECT_ADD_SUCCESS;
}

int reserveDispatcher::remove (gridCoreObject *obj)
{
  if (dynamic_cast<schedulerRamp *> (obj))
    {
      return( remove (static_cast<schedulerRamp *> (obj)));
    }
  return OBJECT_NOT_RECOGNIZED;
}


int reserveDispatcher::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;
  out = gridCoreObject::set (param, val);
  return out;
}

int reserveDispatcher::set (const std::string &param, double val,gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;
  if ((param == "threshold")||(param == "thresholdstart"))
    {
      thresholdStart = val;
      if (thresholdStop > thresholdStart)
        {
          thresholdStop = thresholdStart / 2;
        }
    }
  else if (param == "thresholdstop")
    {
      thresholdStop = val;
    }
  else if ((param == "dispatchinterval")||(param == "interval"))
    {
      dispatchInterval = val;
    }
  else
    {
      out = gridCoreObject::set (param,val,unitType);
    }
  return out;
}

void reserveDispatcher::schedChange ()
{
  checkGen ();
}

void reserveDispatcher::checkGen ()
{
  unsigned int kk;
  reserveAvailable = 0;
  for (kk = 0; kk < schedCount; kk++)
    {
      resAvailable[kk] = schedList[kk]->getReserveTarget ();
      reserveAvailable += resAvailable[kk];

      resUsed[kk] = schedList[kk]->getReserveTarget ();
    }

}

void reserveDispatcher::dispatch (double level)
{
  unsigned int kk;
  double avail = 0;
  int ind = -1;
  double tempAvail;
  //if the dispatch is too low
  while (currDispatch < level)
    {
      for (kk = 0; kk < schedCount; kk++)
        {
          tempAvail = resAvailable[kk] - resUsed[kk];
          if (tempAvail > avail)
            {
              ind = kk;
              avail = tempAvail;
            }
        }
      if (avail == 0)
        {
          break;
        }
      if (avail <= (level - currDispatch))
        {
          schedList[ind]->setReserveTarget (resUsed[ind] + avail);
          resUsed[ind] = resUsed[ind] + avail;
          currDispatch += avail;
        }
      else
        {
          tempAvail = level - currDispatch;
          schedList[ind]->setReserveTarget (resUsed[ind] + tempAvail);
          resUsed[ind] = resUsed[ind] + tempAvail;
          currDispatch += tempAvail;
        }
    }

  //if the dispatch is too high
  while (currDispatch > level)
    {
      for (kk = 0; kk < schedCount; kk++)
        {
          tempAvail = resUsed[kk];
          if (tempAvail > avail)
            {
              ind = kk;
              avail = tempAvail;
            }
        }
      if (avail == 0)
        {
          break;
        }
      if (avail < (currDispatch - level))
        {
          schedList[kk]->setReserveTarget (0);
          resUsed[kk] = 0;
          currDispatch -= avail;
        }
      else
        {
          tempAvail = currDispatch - level;
          schedList[kk]->setReserveTarget (resUsed[kk] - tempAvail);
          resUsed[kk] = resUsed[kk] - tempAvail;
          currDispatch -= tempAvail;
        }
    }
}
