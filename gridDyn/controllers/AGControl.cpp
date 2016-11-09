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
#include "submodels/otherBlocks.h"
#include "scheduler.h"
#include "objectFactoryTemplates.h"
#include "core/gridDynExceptions.h"
/*
class AGControl
{
public:
        char name[32];
        gridArea *Parent;

protected:
        double KI;
        double KP;
        double beta;
        double deadband;
        double alpha;

        double ACE;
        double fACE;
        double reg;
        double regUpAvailable;
        double regDownAvailable;

        pidBlock *pid;

        int schedCount;

        std::vector<scheduler *> schedList;
        double upRat;
        double downRat;

public:
        AGControl();

        ~AGControl();


        double initialize(double time0,double freq0,double tiedev0);


        void setTime(double time);
        double updateP(double time, double freq, double tiedev);
        double currentValue();

        double addGen(scheduler *sched);
        void set (const std::string &param, double val,units_t unitType=defUnit);
        void set (const std::string &param, double val,units_t unitType=defUnit){return set(param,&val, unitType);};

        void regChange();
protected:
        void checkGen();
};
*/

static typeFactory<basicBlock> bbof ("agc", { "basic", "agc" }, "basic");

AGControl::AGControl (const std::string &objName) : gridSubModel (objName)
{
  pid = std::make_shared<pidBlock> (KP,KI,0,"pid");
  pid->setParent (this);
  filt1 = std::make_shared<delayBlock> (Tf, "delay1");
  filt1->setParent (this);
  filt2 = std::make_shared<delayBlock> (Tr, "delay2");
  filt2->setParent (this);
  db = std::make_shared<deadbandBlock> (deadband,"deadband");
  db->setParent (this);
  db->set ("rampband",4);
  enabled = true;
}

AGControl::~AGControl ()
{

}

gridCoreObject *AGControl::clone (gridCoreObject *obj) const
{
  AGControl *nobj;
  if (obj == nullptr)
    {
      nobj = new AGControl ();
    }
  else
    {
      nobj = dynamic_cast<AGControl *> (obj);
      if (nobj == nullptr)
        {
          //if we can't cast the pointer clone at the next lower level
          gridCoreObject::clone (obj);
          return obj;
        }
    }
  gridCoreObject::clone (nobj);
  nobj->KI = KI;
  nobj->KP = KP;
  nobj->beta = beta;
  nobj->deadband = deadband;

  nobj->Tf = Tf;
  nobj->Tr = Tr;

  pid->clone (nobj->pid.get ());
  filt1->clone (nobj->filt1.get ());
  filt2->clone (nobj->filt2.get ());
  db->clone (nobj->db.get ());
  return nobj;
}


void AGControl::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{

  IOdata iSet (1);
  if (outputSet.empty ())
    {
      ACE = (args[1]) - 10 * beta * args[0];
    }
  else
    {
      ACE = outputSet[0];
    }
  filt1->initializeB ({0},{ACE},iSet);
  fACE = ACE;
  pid->initializeB ({0},{fACE},iSet);
  //freg=filt2->initializeB(time0,reg);
  //freg=db->updateA(time0,freg);
  inputSet[0] = pid->getOutput ();

}


void AGControl::setTime (double time)
{
  prevTime = time;
  pid->setTime (time);
  filt1->setTime (time);
  filt2->setTime (time);
  db->setTime (time);
}


void AGControl::updateA (double /*time*/)
{

}

void AGControl::timestep (double ttime, const IOdata &args, const solverMode &)
{
  prevTime = ttime;

  ACE = (args[1]) - 10 * beta * args[0];
  fACE=filt1->step(ttime, ACE);
  
  reg+=pid->step(ttime,  fACE - reg );
 
  reg=db->step(ttime, reg );

  freg=filt2->step (ttime,reg);

  for (size_t kk = 0; kk < schedCount; kk++)
    {
      if (freg >= 0)
        {
          if (freg > regUpAvailable)
            {
              schedList[kk]->setReg (regUpAvailable * upRat[kk]);
              reg = regUpAvailable;
            }
          else
            {
              schedList[kk]->setReg (freg * upRat[kk]);
            }
        }
      else
        {
          if (freg < -regDownAvailable)
            {
              schedList[kk]->setReg (-regDownAvailable * downRat[kk]);
              reg = -regDownAvailable;
            }
          else
            {
              schedList[kk]->setReg (freg * downRat[kk]);
            }
        }
    }
}

void AGControl::add (gridCoreObject *obj)
{
  if (dynamic_cast<schedulerReg *> (obj))
    {
      add (static_cast<schedulerReg *> (obj));
    }
  else
    {
	  throw(invalidObjectException(this));
    }
}

void AGControl::add (schedulerReg *sched)
{
  schedCount++;
  schedList.push_back (sched);
  //sched->AGClink(this);
  upRat.resize (schedCount);
  downRat.resize (schedCount);
  regChange ();
}


void AGControl::remove (gridCoreObject *sched)
{
  for (size_t kk = 0; kk < schedCount; kk++)
    {
      if (schedList[kk]->getID () == sched->getID ())
        {
          schedList.erase (schedList.begin () + kk);
          schedCount--;
          upRat.resize (schedCount);
          downRat.resize (schedCount);
          regChange ();
		  break;
        }
    }

}

void AGControl::set (const std::string &param,  const std::string &val)
{
  gridCoreObject::set (param, val);
}

void AGControl::set (const std::string &param, double val,gridUnits::units_t unitType)
{

  if (param == "deadband")
    {
      deadband = val;
      db->set ("deadband",deadband);
      db->set ("rampband",0.2 * deadband);
    }
  else if (param == "beta")
    {
      beta = val;
    }
  else if (param == "ki")
    {
      KI = val;
      pid->set ("I",val);
    }
  else if (param == "kp")
    {
      KP = val;
      pid->set ("P",val);
    }
  else if (param == "tf")
    {
      Tf = val;
      filt1->set ("T1",Tf);
    }
  else if (param == "tr")
    {
      Tr = val;
      filt2->set ("T1",Tr);
    }
  else
    {
      gridCoreObject::set (param,val,unitType);
    }

}


void AGControl::regChange ()
{
  size_t kk;

  regUpAvailable = 0;
  regDownAvailable = 0;
  for (kk = 0; kk < schedCount; kk++)
    {

      regUpAvailable += schedList[kk]->getRegUpAvailable ();
      regDownAvailable += schedList[kk]->getRegDownAvailable ();
    }
  for (kk = 0; kk < schedCount; kk++)
    {
      upRat[kk] = schedList[kk]->getRegUpAvailable () / regUpAvailable;
      downRat[kk] = schedList[kk]->getRegDownAvailable () / regDownAvailable;
    }
}


AGControl * newAGC (const std::string &type)
{
  AGControl *agc = nullptr;
  if ((type.empty ()) || (type == "basic"))
    {
      agc = new AGControl ();
    }
  else if (type == "battery")
    {
      //agc = new AGControlBattery();
    }
  else if (type == "battDR")
    {
      //agc= new AGCControlBattDR();
    }
  return agc;
}

