/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2014, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

// headers
#include "gridArea.h"

#include "generators/gridDynGenerator.h"
#include "loadModels/gridLoad.h"
#include "linkModels/gridLink.h"
#include "gridCoreTemplates.h"
#include "infiniteBus.h"
#include "gridBus.h"
#include "acBus.h"
#include "dcBus.h"
#include "objectFactoryTemplates.h"
#include "vectorOps.hpp"
#include "core/gridDynExceptions.h"
#include "stringOps.h"


#include <iostream>
#include <cmath>
#include <cassert>


std::atomic<count_t> gridBus::busCount(0);
static typeFactory<gridBus> gbf ("bus", stringVec { "basic" });
static childTypeFactory<acBus,gridBus> gbfac ("bus", stringVec {"ac","pq","pv","slk","slack","afix","ref"}, "ac");
static childTypeFactory<dcBus,gridBus> gbfdc ("bus", stringVec { "dc", "hvdc" });
static childTypeFactory<infiniteBus,gridBus> igbc ("bus", stringVec { "inf", "infinite" });

using namespace gridUnits;

gridBus::gridBus (const std::string &objName) : gridPrimary (objName),outputs (3),outLocs (3)
{
  // default values
  id = ++busCount;
  updateName ();


}

gridBus::gridBus (double vStart, double angleStart,const std::string &objName) : gridPrimary (objName),angle (angleStart), voltage (vStart)
{
  // default values
  id = ++busCount;
  updateName ();

}

coreObject *gridBus::clone (coreObject *obj) const
{
  gridBus *nobj = cloneBaseFactory<gridBus, gridPrimary> (this, obj, &gbf);
  if (!(nobj))
    {
      return obj;
    }
  nobj->type = type;
  nobj->dynType = dynType;
  nobj->angle = angle;
  nobj->voltage = voltage;
  nobj->baseVoltage = baseVoltage;
  nobj->freq = freq;
  nobj->Vtol = Vtol;
  nobj->Atol = Atol;
  nobj->Network = Network;
  nobj->zone = zone;
  nobj->lowVtime = lowVtime;
  //now clone all the loads and generators
  //cloning the links from this component would be bad
  //clone the generators and loads

  for (size_t kk = 0; kk < attachedGens.size (); ++kk)
    {
      if (kk >= nobj->attachedGens.size ())
        {
          nobj->add (static_cast<gridDynGenerator *> (attachedGens[kk]->clone (nullptr)));
        }
      else
        {
          attachedGens[kk]->clone (nobj->attachedGens[kk]);
        }
    }
  for (size_t kk = 0; kk < attachedLoads.size (); ++kk)
    {
      if (kk >= nobj->attachedLoads.size ())
        {
          nobj->add (static_cast<gridLoad *> (attachedLoads[kk]->clone (nullptr)));
        }
      else
        {
          attachedLoads[kk]->clone (nobj->attachedLoads[kk]);
        }
    }
  return nobj;
}

bool gridBus::checkCapable ()
{
  double tP = 0;
  if (!opFlags[pFlow_initialized])
    {
      return true;
    }
  for (auto &load : attachedLoads)
    {
      if (load->enabled)
        {
          tP -= load->getRealPower ();
        }
    }
  for (auto &gen : attachedGens)
    {
      if (gen->enabled)
        {
          tP += gen->getPmax ();
        }
    }
  for (auto &link : attachedLinks)
    {
      if (link->enabled)
        {
          tP += link->getMaxTransfer ();
        }
    }
  if (tP <= 0)
    {
      LOG_WARNING ("BUS failed");
    }
  return ((tP < 0) ? false : true);
}

void gridBus::disable ()
{
  enabled = false;
  alert (this, STATE_COUNT_CHANGE);
  for (auto &link : attachedLinks)
    {
      link->disable ();
    }
}

// destructor
gridBus::~gridBus ()
{
  for (auto &ld : attachedLoads)
    {
      condDelete (ld, this);
    }

  for (auto &gen : attachedGens)
    {
      condDelete (gen, this);
    }
}

void gridBus::add (coreObject *obj)
{
  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld)
    {
      return add (ld);
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen)
    {
      return add (gen);
    }

  gridLink *lnk = dynamic_cast<gridLink *> (obj);
  if (lnk)
    {
      return add (lnk);
    }
  throw(invalidObjectException(this));
}

template<class X>
void addObject (gridBus *bus, X* obj, std::vector<X *> &objVector)
{
  coreObject *foundObj = bus->find (obj->getName ());
  if (foundObj == nullptr)
    {
      obj->locIndex = static_cast<index_t> (objVector.size ());
      objVector.push_back (obj);
      obj->setParent (bus);
      obj->set ("basepower", bus->systemBasePower);
      obj->set ("basefreq", bus->m_baseFreq);
      obj->set ("basevoltage", bus->baseVoltage);
      if (bus->checkFlag (pFlow_initialized))
        {
          bus->alert (bus, OBJECT_COUNT_INCREASE);
        }
    }
  else if (obj->getID () != foundObj->getID ())
    {
	  throw(objectAddFailure(bus));
    }
}

// add load
void gridBus::add (gridLoad *ld)
{
  addObject (this, ld, attachedLoads);
}

// add generator
void gridBus::add (gridDynGenerator *gen)
{
  addObject (this, gen, attachedGens);
}

// add link
void gridBus::add (gridLink *lnk)
{
  for (auto &links : attachedLinks)
    {
      if (links->getID () == lnk->getID ())
        {
          return;
        }
    }
  attachedLinks.push_back (lnk);
}


void gridBus::remove (coreObject *obj)
{
  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld)
    {
      return (remove (ld));
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen)
    {
      return(remove (gen));
    }

  gridLink *lnk = dynamic_cast<gridLink *> (obj);
  if (lnk)
    {
      return(remove (lnk));
    }

  throw(invalidObjectException(this));
}

template<class X>
void removeObject (X* obj, std::vector<X *> &objVector)
{
  if (obj->locIndex > objVector.size ())
    {
	  return;
    }
  if (obj->getID () == objVector[obj->locIndex]->getID ())
    {
      //alert that the states might have changed
      if (obj->checkFlag (has_dyn_states))
        {
          obj->getParent ()->alert (obj->getParent (), STATE_COUNT_DECREASE);
        }
      else if (obj->checkFlag (has_pflow_states))
        {
          obj->getParent ()->alert (obj->getParent (), STATE_COUNT_DECREASE);
        }

      objVector[obj->locIndex]->setParent (nullptr);
      objVector.erase (objVector.begin () + obj->locIndex);
    }
}

// remove load
void gridBus::remove (gridLoad *ld)
{
  removeObject (ld, attachedLoads);
}

// remove generator
void gridBus::remove (gridDynGenerator *gen)
{
  removeObject (gen, attachedGens);
}

// remove link
void gridBus::remove (gridLink *lnk)
{
  for (size_t kk = 0; kk < attachedLinks.size (); ++kk)
    {
      if (lnk->getID () == attachedLinks[kk]->getID ())
        {
          attachedLinks.erase (attachedLinks.begin () + kk);
		  return;
        }
    }
}




void gridBus::alert (coreObject *obj, int code)
{
  switch (code)
    {
    case OBJECT_NAME_CHANGE:
    case OBJECT_ID_CHANGE:
      break;
    case POTENTIAL_FAULT_CHANGE:
      if (opFlags[disconnected])
        {
          reconnect ();
        }
    //fall through to the primary alert;
    default:
      gridPrimary::alert (obj, code);
    }
}


void gridBus::followNetwork (int networkNum, std::queue<gridBus *> &bstk)
{
  Network = networkNum;
  for (auto &link : attachedLinks)
    {
      link->followNetwork (networkNum, bstk);
    }
}

// initializeB states
void gridBus::pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  //run the subObjects
  if (Vtol < 0)
    {
      Vtol = find ("root")->get ("voltagetolerance");
    }
  if (Atol < 0)
    {
      Atol = find ("root")->get ("angletolerance");
    }
  for (auto &gen : attachedGens)
    {
      gen->pFlowInitializeA (time0, flags);
    }
  for (auto &load : attachedLoads)
    {
      load->pFlowInitializeA (time0, flags);
    }
  if (CHECK_CONTROLFLAG (flags, low_voltage_checking))
    {
      opFlags.set (low_voltage_check_flag);
    }
}

void gridBus::pFlowObjectInitializeB ()
{

  for (auto &gen : attachedGens)
    {
      gen->pFlowInitializeB ();
    }
  for (auto &load : attachedLoads)
    {
      load->pFlowInitializeB ();
    }
  m_dstate_dt.resize (3, 0);
  m_dstate_dt[angleInLocation] = m_baseFreq * (freq - 1.0);
  m_state = { voltage,angle,freq };

}


void gridBus::preEx (const stateData &sD, const solverMode &sMode)
{
  auto args = getOutputs (sD, sMode);
  for (auto &gen : attachedGens)
    {
      if (gen->checkFlag (preEx_requested))
        {
          gen->preEx (args, sD, sMode);
        }
    }
  for (auto &load : attachedLoads)
    {
      if (load->checkFlag (preEx_requested))
        {
          load->preEx (args, sD, sMode);
        }
    }

}
//function to reset the bus type and voltage

void gridBus::reset (reset_levels level)
{
  if (opFlags[disconnected])
    {
      for (auto &link : attachedLinks)
        {
          if (link->isConnected ())
            {
              reconnect ();
              break;
            }
        }
    }

  for (auto &gen : attachedGens)
    {
      if (gen->checkFlag (has_powerflow_adjustments))
        {
          gen->reset (level);
        }
    }
  for (auto &ld : attachedLoads)
    {
      if (ld->checkFlag (has_powerflow_adjustments))
        {
          ld->reset (level);
        }
    }
}

change_code gridBus::powerFlowAdjust (unsigned long flags, check_level_t level)
{


  auto out = change_code::no_change;
  IOdata args = { voltage,angle,freq };
  for (auto &gen : attachedGens)
    {
      if (gen->checkFlag (has_powerflow_adjustments))
        {
          auto pout = gen->powerFlowAdjust (args, flags, level);
          out = (std::max)(pout, out);
        }
    }
  for (auto &ld : attachedLoads)
    {
      if (ld->checkFlag (has_powerflow_adjustments))
        {
          auto pout = ld->powerFlowAdjust (args, flags, level);
          out = (std::max)(pout, out);
        }
    }
  return out;

}


// initializeB states for dynamic solution
void gridBus::dynObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  opFlags[preEx_requested] = false;
  opFlags[has_constraints] = false;
  offsets.unload (true);
  for (auto &gen : attachedGens)
    {
      gen->dynInitializeA (time0, flags);
    }
  for (auto &load : attachedLoads)
    {
      load->dynInitializeA (time0, flags);
    }
  //check for any roots
  //localRoots = 0;

}

// initializeB states for dynamic solution part 2  //final clean up
void gridBus::dynObjectInitializeB (IOdata &outputSet)
{
  if (outputSet.size () > 0)
    {
      if (outputSet[voltageInLocation] > 0)
        {
          voltage = outputSet[voltageInLocation];
        }
      if (outputSet[angleInLocation] > -kHalfBigNum)
        {
          angle = outputSet[angleInLocation];
        }
      if (std::abs (outputSet[frequencyInLocation] - 1.0) < 0.5)
        {
          freq = outputSet[frequencyInLocation];
        }
    }
  updateLocalCache ();

  m_state[voltageInLocation] = voltage;
  m_state[angleInLocation] = angle;
  m_state[frequencyInLocation] = freq;

  //first get the state size for the internal state ordering
  IOdata args {
    voltage,angle,freq
  };

  IOdata pc;
  for (auto &gen : attachedGens)
    {
      gen->dynInitializeB (args, pc);
    }
  for (auto &load : attachedLoads)
    {
      load->dynInitializeB (args, pc);

    }

}

void gridBus::powerAdjust (double /*adjustment*/)
{

  //adjust the real power flow

}

void gridBus::timestep (gridDyn_time ttime, const solverMode &sMode)
{

  auto args = getOutputs (emptyStateData,sMode);
  for (auto &load : attachedLoads)
    {
      load->timestep (ttime, args, sMode);
    }
  for (auto &gen : attachedGens)
    {
      gen->timestep (ttime, args, sMode);
    }
  //localConverge (sMode, 0);
  //updateLocalCache ();
  prevTime = ttime;
}


void gridBus::setAll (const std::string &objtype, std::string param, double val, gridUnits::units_t unitType)
{

  if ((objtype == "gen") || (objtype == "generator"))
    {
      for (auto &gen : attachedGens)
        {
          gen->set (param, val, unitType);
        }
    }
  else if (objtype == "load")
    {
      for (auto &ld : attachedLoads)
        {
          ld->set (param, val, unitType);
        }
    }

}

static const stringVec locNumStrings {
  "voltage","angle","basevoltage","p","q","g","b","zone"
};
static const stringVec locStrStrings {
  "status"
};

static const stringVec flagStrings {
  "connected"
};

void gridBus::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
  getParamString<gridBus, gridObject> (this, pstr, locNumStrings, locStrStrings, flagStrings, pstype);
}

void gridBus::setFlag (const std::string &flag, bool val)
{
  if (flag == "connected")
    {
      if (val)
        {
          if (isConnected ())
            {
              disconnect ();
            }
        }
      else
        {
          if (!isConnected ())
            {
              reconnect ();
            }
        }
    }
  else
    {
      gridPrimary::setFlag (flag, val);
    }

}

// set properties
void gridBus::set (const std::string &param, const std::string &vali)
{
  auto val = convertToLowerCase (vali);

  if (param == "status")
    {
      makeLowerCase (val);
      if ((val == "out") || (val == "off") || (val == "disconnected"))
        {
          if (enabled)
            {
              disable ();
            }

        }
      else if ((val == "in") || (val == "on"))
        {
          if (!enabled)
            {
              enable ();
            }
        }
      else if (val == "disconnected")
        {
          disconnect ();
        }
      else
        {
		  throw(invalidParameterValue());
        }
    }
  else
    {
      gridPrimary::set (param, vali);
    }
}

void gridBus::set (const std::string &param, double val, units_t unitType)
{

  if ((param == "voltage") || (param == "vol"))
    {
		if (voltage<0.25)
		{
			if (opFlags[dyn_initialized])
			{
				parent->alert(this, POTENTIAL_FAULT_CHANGE);
			}
		}
      voltage = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
    }
  else if ((param == "angle") || (param == "ang"))
    {
      angle = unitConversion (val, unitType, rad);
    }
  else if ((param == "basevoltage") || (param == "base vol"))
    {
      baseVoltage = unitConversionPower (val, unitType, kV);
      for (auto &gen : attachedGens)
        {
          gen->set ("basevoltage", val);
        }
      for (auto &ld : attachedLoads)
        {
          ld->set ("basevoltage", val);
        }
    }
  else if (param == "basepower")
    {
      systemBasePower = unitConversionPower (val, unitType, MW);           //

      for (auto &gen : attachedGens)
        {
          gen->set ("basepower", val);
        }
      for (auto &ld : attachedLoads)
        {
          ld->set ("basepower", val);
        }
    }
  else if ((param == "basefrequency") || (param == "basefreq"))
    {
      m_baseFreq = unitConversionFreq (val, unitType, rps);

      for (auto &gen : attachedGens)
        {
          gen->set ("basefreq", m_baseFreq);
        }
      for (auto &ld : attachedLoads)
        {
          ld->set ("basefreq", m_baseFreq);
        }

    }
  else if (param == "disconnect")
    {
      if (val > 0.1)
        {
          disconnect ();
        }
      else
        {
          reconnect ();
        }
    }

  else if ((param == "p") || (param == "gen p"))
    {
      S.genP = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      if (attachedGens.size () == 1)
        {
          attachedGens[0]->set ("p", S.genP);
        }
      else if (attachedGens.empty ())
        {
          if (val != 0.0)
            {
              //not sure this is the wisest thing to do here should be smarter about it
              add (new gridDynGenerator ());
              attachedGens[0]->set ("p", S.genP);
            }
          else
            {
              return;
            }
        }
    }
  else if ((param == "q") || (param == "gen q"))
    {
      S.genQ = unitConversion (val, unitType, puMW);
      if (attachedGens.size () == 1)
        {
          attachedGens[0]->set ("q", S.genQ);
        }
      else if (attachedGens.empty ())
        {
          if (val != 0.0)
            {
              add (new gridDynGenerator ());
              attachedGens[0]->set ("q", S.genQ);
            }
          else
            {
              return;
            }
        }
    }
  else if ((param == "load p") || (param == "load q") || (param == "shunt g") || (param == "g"))
    {
      if (attachedLoads.empty ())
        {
          if (val != 0.0)
            {
              add (new gridLoad ());  //TODO :: use the object factory instead
            }
          else
            {
              return;
            }
        }
      std::string b{param.back()};
      attachedLoads[0]->set (b, val, unitType);
    }
  else if ((param == "shunt b") || (param == "b"))
    {
      if (attachedLoads.empty ())
        {
          if (val != 0.0)
            {
              add (new gridLoad ());
            }
          else
            {
              return;
            }
        }
      attachedLoads[0]->set ("b", -val, unitType);
    }
  else if ((param == "zone")||(param=="zone number"))
    {
      zone = static_cast<int> (val);
    }
  else if ((param=="area")||(param == "area number"))
  {
	  //Here to catch a specific issue while the area controls are being developed
  }
  else
    {
      gridPrimary::set (param, val, unitType);
    }


}

void gridBus::setVoltageAngle (double Vnew, double Anew)
{
  voltage = Vnew;
  angle = Anew;
}

static const IOdata kNullVec;

IOdata gridBus::getOutputs (const stateData &sD, const solverMode &sMode) const
{

  if ((sMode.local) || (sD.empty()))
    {
      return {
               voltage,angle,freq
      };
    }
  else
    {
      return {
               getVoltage (sD,sMode),getAngle (sD,sMode),getFreq (sD,sMode)
      };
    }
}

static const IOlocs noLocs {
  kNullLocation,kNullLocation, kNullLocation
};

IOlocs gridBus::getOutputLocs  (const solverMode &) const
{
  return noLocs;
}

const IOdata &gridBus::getOutputsRef () const
{
  return outputs;
}

const IOlocs &gridBus::getOutputLocsRef () const
{
  return noLocs;
}

double gridBus::getOutput (const stateData &sD, const solverMode &sMode, index_t outNum) const
{
  switch (outNum)
    {
    case voltageInLocation:
      return getVoltage (sD, sMode);
    case angleInLocation:
      return getAngle (sD, sMode);
    case frequencyInLocation:
      return getFreq (sD, sMode);
    default:
      return kNullVal;
    }
}

double gridBus::getVoltage (const double /*state*/[], const solverMode &) const
{
  return voltage;
}

double gridBus::getAngle (const double /*state*/[], const solverMode &) const
{
  return angle;
}

double gridBus::getVoltage (const stateData &, const solverMode &) const
{
  return voltage;
}

double gridBus::getAngle (const stateData &, const solverMode &) const
{
  return angle;
}

bool gridBus::hasInertialAngle () const
{
  return ((attachedGens.size () > 0) && (isConnected ()));
}

double gridBus::getFreq (const stateData &, const solverMode &) const
{
  return freq;
}

bool gridBus::directPath (gridObject *target, gridObject *source)
{

  auto tid = target->getID ();
  if (tid == getID ())
    {
      return true;
    }
  for (auto &gen : attachedGens)
    {
      if (tid == gen->getID ())
        {
          return true;
        }
    }
  for (auto &ld : attachedLoads)
    {
      if (tid == ld->getID ())
        {
          return true;
        }
    }
  auto sid = (source) ? source->getID () : 0;
  int lnkcnt = 0;
  gridLink *nLink = nullptr;
  for (auto &lnk : attachedLinks)
    {
      if (lnk->enabled)
        {
          if (lnk->getID () != sid)
            {
              ++lnkcnt;
              if (lnkcnt > 1)
                {
                  return false;
                }
              nLink = lnk;
            }
        }
    }
  if (nLink)
    {
      if (nLink->getBus (1)->getID () == tid)
        {
          return true;
        }
      else if (nLink->getBus (2)->getID () == tid)
        {
          return true;
        }
      else if (nLink->getBus (1)->getID () == getID ())
        {
          return nLink->getBus (2)->directPath (target, nLink);
        }
      else
        {
          return nLink->getBus (1)->directPath (target, nLink);
        }
    }
  return false;
}


std::vector<gridObject *> gridBus::getDirectPath (gridObject *target, gridObject *source)
{
  std::vector<gridObject *> opath {
    source
  };

  auto tid = target->getID ();
  if (tid == getID ())
    {
      opath.push_back (target);
      return opath;
    }
  for (auto &gen : attachedGens)
    {
      if (tid == gen->getID ())
        {
          opath.push_back (target);
          return opath;
        }
    }
  for (auto &ld : attachedLoads)
    {
      if (tid == ld->getID ())
        {
          opath.push_back (target);
          return opath;
        }
    }
  auto sid = (source) ? source->getID () : 0;
  int lnkcnt = 0;
  gridLink *nLink = nullptr;
  for (auto &lnk : attachedLinks)
    {
      if (lnk->enabled)
        {
          if (lnk->getID () != sid)
            {
              ++lnkcnt;
              if (lnkcnt > 1)
                {
                  return {};
                }
              nLink = lnk;
            }
        }
    }
  if (nLink)
    {
      if (nLink->getBus (1)->getID () == tid)
        {
          opath.push_back (nLink);
          opath.push_back (target);
          return opath;
        }
      else if (nLink->getBus (2)->getID () == tid)
        {
          opath.push_back (nLink);
          opath.push_back (target);
          return opath;
        }
      else if (nLink->getBus (1)->getID () == getID ())
        {
          auto npath = nLink->getBus (2)->getDirectPath (target, nLink);
          if (npath.empty ())
            {
              return npath;
            }
          else
            {
              for (auto &pp : npath)
                {
                  opath.push_back (pp);
                }
              return opath;
            }
        }
      else
        {
          auto npath = nLink->getBus (1)->getDirectPath (target, nLink);
          if (npath.empty ())
            {
              return npath;
            }
          else
            {
              for (auto &pp : npath)
                {
                  opath.push_back (pp);
                }
              return opath;
            }
        }
    }
  return {};
}

int gridBus::propogatePower (bool /*makeSlack*/)
{
  int ret = 0;

  int unfixed_lines = 0;
  gridLink *unfixed_line = nullptr;
  double Pexp = 0;
  double Qexp = 0;
  for (auto &lnk : attachedLinks)
    {
      if (lnk->checkFlag (gridLink::fixed_target_power))
        {
          Pexp += lnk->getRealPower (getID ());
          Qexp += lnk->getReactivePower (getID ());
          continue;
        }
      ++unfixed_lines;
      unfixed_line = lnk;

    }
  if (unfixed_lines > 1)
    {
      return ret;
    }

  int adjPSecondary = 0;
  int adjQSecondary = 0;
  for (auto &ld : attachedLoads)
    {
      if (ld->checkFlag (adjustable_P))
        {
          ++adjPSecondary;
        }
      else
        {
          Pexp += ld->getRealPower ();
        }
      if (ld->checkFlag (adjustable_Q))
        {
          ++adjQSecondary;
        }
      else
        {
          Qexp += ld->getReactivePower ();
        }
    }
  for (auto &gen : attachedGens)
    {
      if (gen->checkFlag (adjustable_P))
        {
          ++adjPSecondary;
        }
      else
        {
          Pexp -= gen->getRealPower ();
        }
      if (gen->checkFlag (adjustable_Q))
        {
          ++adjQSecondary;
        }
      else
        {
          Qexp -= gen->getReactivePower ();
        }
    }
  if (unfixed_lines == 1)
    {
      if ((adjPSecondary == 0) && (adjQSecondary == 0))
        {
          unfixed_line->fixPower (-Pexp, -Qexp, getID (), getID ());
        }
    }
  else         //no lines so adjust the generators and load
    {
      if ((adjPSecondary == 1) && (adjQSecondary == 1))
        {
          int found = 0;
          for (auto &gen : attachedGens)
            {
              if (gen->checkFlag (adjustable_P))
                {
                  gen->set ("p", Pexp);
                  ++found;
                }
              if (gen->checkFlag (adjustable_Q))
                {
                  gen->set ("q", Qexp);
                  ++found;
                }
              if (found == 2)
                {
                  return 1;
                }
            }
          for (auto &ld : attachedLoads)
            {
              if (ld->checkFlag (adjustable_P))
                {
                  ld->set ("p", -Pexp);
                  ++found;
                }
              if (ld->checkFlag (adjustable_Q))
                {
                  ld->set ("q", -Qexp);
                  ++found;
                }
              if (found == 2)
                {
                  return 1;
                }
            }
        }
      else           //TODO::PT:deal with multiple adjustable controls
        {
          return 0;
        }

    }
  return 0;
}
// -------------------- Power Flow --------------------


//guess the solution
void gridBus::guess (gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  for (auto &gen : attachedGens)
    {
      if ((gen->enabled) && (gen->stateSize (sMode) > 0))
        {
          gen->guess (ttime, state, dstate_dt, sMode);
        }
    }
  for (auto &load : attachedLoads)
    {
      if ((load->stateSize (sMode) > 0) && (load->enabled))
        {
          load->guess (ttime, state, dstate_dt, sMode);
        }
    }

}

// set algebraic and dynamic variables assume preset to differential
void gridBus::getVariableType (double sdata[], const solverMode &sMode)
{

  for (auto &gen : attachedGens)
    {
      if (gen->enabled)
        {
          gen->getVariableType (sdata, sMode);
        }
    }

  for (auto &load : attachedLoads)
    {
      if (load->checkFlag (has_dyn_states))
        {
          load->getVariableType (sdata, sMode);
        }
    }

}

void gridBus::getTols (double tols[], const solverMode &sMode)
{

  for (auto &gen : attachedGens)
    {
      if (gen->enabled)
        {
          gen->getTols (tols, sMode);
        }
    }

  for (auto &load : attachedLoads)
    {
      if ((load->stateSize (sMode) > 0) && (load->enabled))
        {
          load->getTols (tols, sMode);
        }
    }
}

// pass the solution
void gridBus::setState (gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  for (auto &gen : attachedGens)
    {
      if (gen->enabled)
        {
          gen->setState (ttime, state, dstate_dt, sMode);
        }
    }
  for (auto &ld : attachedLoads)
    {
      if (ld->enabled)
        {
          ld->setState (ttime, state, dstate_dt, sMode);
        }
    }

  //	assert(voltage > 0.0);
  prevTime = ttime;
}

// residual
void gridBus::residual (const stateData &sD, double resid[], const solverMode &sMode)
{

  updateLocalCache (sD, sMode);
  if ((opFlags[low_voltage_check_flag])&&(outputs[voltageInLocation] < Vtol / 2.0)&& (isConnected ()))
    {
      alert (this,INVALID_STATE_ALERT);
      alert (this, VERY_LOW_VOLTAGE_ALERT);
	  lowVtime = (!sD.empty()) ? sD.time : prevTime;
      return;
    }
  for (auto &gen : attachedGens)
    {
      if ((gen->stateSize (sMode) > 0) && (gen->enabled))
        {
          gen->residual (outputs, sD, resid, sMode);
        }
      else
        {
          gen->updateLocalCache (outputs, sD, sMode);
        }
    }
  for (auto &load : attachedLoads)
    {
      if ((load->stateSize (sMode) > 0) && (load->enabled))
        {
          load->residual (outputs, sD, resid, sMode);
        }
      else
        {
          load->updateLocalCache (outputs, sD, sMode);
        }
    }

}

void gridBus::derivative (const stateData &sD, double deriv[], const solverMode &sMode)
{
  auto args = getOutputs (sD, sMode);
  for (auto &gen : attachedGens)
    {
      if (gen->diffSize (sMode) > 0)
        {
          gen->derivative (args, sD, deriv, sMode);
        }
    }

  for (auto &load : attachedLoads)
    {
      if (load->diffSize (sMode) > 0)
        {
          load->derivative (args, sD, deriv, sMode);
        }
    }
}

static const IOlocs kNullLocations {
  kNullLocation,kNullLocation,kNullLocation
};

// Jacobian
void gridBus::jacobianElements (const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
{
  updateLocalCache (sD, sMode);
  // import bus values (current theta and voltage)

  //printf("t=%f,id=%d, dpdt=%f, dpdv=%f, dqdt=%f, dqdv=%f\n", ttime, id, Ptii, Pvii, Qvii, Qtii);

  const IOlocs &coutLocs = (hasAlgebraic (sMode)) ? outLocs : kNullLocations;
  for (auto &gen : attachedGens)
    {
      if (gen->jacSize (sMode) > 0)
        {
          gen->jacobianElements (outputs, sD, ad, coutLocs, sMode);
        }
    }
  for (auto &load : attachedLoads)
    {
      if (load->jacSize (sMode) > 0)
        {
          load->jacobianElements (outputs, sD, ad, coutLocs, sMode);
        }

    }

}

double gridBus::lastError () const
{
  return std::abs (S.sumP ()) + std::abs (S.sumQ ());
}

inline double dVcheck (double dV, double currV, double drFrac = 0.75, double mxRise = 0.2, double cRcheck = 0)
{

  if (currV - dV > cRcheck)
    {
      if (dV < -mxRise)
        {
          dV = -mxRise;
        }
    }
  if (dV > drFrac * currV)
    {
      dV = drFrac * currV;
    }
  return dV;
}

inline double dAcheck (double dT, double /*currA*/, double mxch = kPI / 8.0)
{
  if (std::abs (dT) > mxch)
    {
      dT = std::copysign (mxch, dT);
    }
  return dT;
}

void gridBus::voltageUpdate (const stateData &, double /*update*/[], const solverMode &, double /*alpha*/)
{

}

void gridBus::algebraicUpdate (const stateData &sD, double update[], const solverMode &sMode, double alpha)
{
  if (algSize (sMode) == offsets.getOffsets (sMode)->local.algSize)
    {
      //no algebraic states in the secondary objects
      return;
    }
  updateLocalCache (sD, sMode);

  for (auto &gen : attachedGens)
    {
      if (gen->algSize (sMode) > 0)
        {
          gen->algebraicUpdate (outputs, sD, update, sMode,alpha);
        }
    }
  for (auto &load : attachedLoads)
    {
      if (load->algSize (sMode) > 0)
        {
          load->algebraicUpdate (outputs, sD, update, sMode, alpha);
        }
    }
}



void gridBus::converge (gridDyn_time /*ttime*/, double /*state*/[], double /*dstate_dt*/[], const solverMode &,  converge_mode /*mode*/,double /*tol*/ )
{

}


double gridBus::computeError (const stateData &sD, const solverMode &sMode)
{
  updateLocalCache (sD, sMode);

  double err = std::abs (S.sumP ()) + std::abs (S.sumQ ());

  return err;
}

void gridBus::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  std::string prefix2 = prefix + name + "::";

  if (stateSize (sMode) == 0)
    {
      return;
    }
  for (auto &gen : attachedGens)
    {
      if (gen->enabled)
        {
          gen->getStateName (stNames, sMode, prefix2);
        }
    }

  for (auto &load : attachedLoads)
    {
      if (load->enabled)
        {
          load->getStateName (stNames, sMode, prefix2);
        }
    }
}


void gridBus::setOffsets (const solverOffsets &newOffsets, const solverMode &sMode)
{
  offsets.setOffsets (newOffsets, sMode);
  solverOffsets no (newOffsets);
  no.localIncrement (offsets.getOffsets (sMode));
  for (auto ld : attachedLoads)
    {
      ld->setOffsets (no, sMode);
      no.increment (ld->getOffsets (sMode));
    }
  for (auto gen : attachedGens)
    {
      gen->setOffsets (no, sMode);
      no.increment (gen->getOffsets (sMode));
    }
}

void gridBus::setOffset (index_t offset, const solverMode &sMode)
{
  for (auto ld : attachedLoads)
    {
      ld->setOffset (offset, sMode);
      offset += ld->stateSize (sMode);

    }
  for (auto gen : attachedGens)
    {
      gen->setOffset (offset, sMode);
      offset += gen->stateSize (sMode);
    }


}

void gridBus::setRootOffset (index_t Roffset, const solverMode &sMode)
{

  offsets.setRootOffset (Roffset, sMode);
  auto so = offsets.getOffsets (sMode);
  auto nR = so->local.algRoots + so->local.diffRoots;
  for (auto &gen : attachedGens)
    {
      gen->setRootOffset (Roffset + nR, sMode);
      nR += gen->rootSize (sMode);
    }
  for (auto &ld : attachedLoads)
    {
      ld->setRootOffset (Roffset + nR, sMode);
      nR += ld->rootSize (sMode);
    }
}

void gridBus::disconnect ()
{
	if (!opFlags[disconnected])
	{
		opFlags.set(disconnected);
		outLocs[voltageInLocation] = kNullLocation;
		outLocs[angleInLocation] = kNullLocation;
		outLocs[frequencyInLocation] = kNullLocation;
		alert(this, JAC_COUNT_DECREASE);
		LOG_DEBUG("disconnecting bus");
		voltage = 0;
		angle = 0;
	}
}

void gridBus::reconnect (gridBus *mapBus)
{
  if (opFlags[disconnected])
    {
      LOG_DEBUG ("reconnecting to network");
      opFlags.reset (disconnected);
      alert (this, JAC_COUNT_INCREASE);
      if (mapBus)
        {
          angle = mapBus->angle;
          voltage = mapBus->voltage;
          freq = mapBus->freq;
        }
      else
        {
          reset (reset_levels::low_voltage_dyn1);
        }
      for (auto &lnk : attachedLinks)
        {
          lnk->reconnect ();
        }
    }

}

void gridBus::reconnect ()
{
  if (opFlags[disconnected])
    {
      LOG_DEBUG ("reconnecting to network");
      opFlags.reset (disconnected);
      alert (this, JAC_COUNT_INCREASE);

      reset (reset_levels::low_voltage_dyn1);
      for (auto &lnk : attachedLinks)
        {
          lnk->reconnect ();
        }
    }
}


void gridBus::loadSizes (const solverMode &sMode, bool dynOnly)
{
  if (isLoaded (sMode, dynOnly))
    {
      return;
    }
  auto so = offsets.getOffsets (sMode);
  if (!enabled)
    {
      so->reset ();
      so->stateLoaded = true;
      so->rjLoaded = true;
      return;
    }
  if (dynOnly)
    {
      so->rootAndJacobianCountReset ();
    }
  else
    {
      so->reset ();
    }





  for (auto ld : attachedLoads)
    {
      if (!(ld->isLoaded (sMode, dynOnly)))
        {
          ld->loadSizes (sMode, dynOnly);
        }
      if (dynOnly)
        {
          so->addRootAndJacobianSizes (ld->getOffsets (sMode));
        }
      else
        {
          so->addSizes (ld->getOffsets (sMode));
        }
    }
  for (auto gen : attachedGens)
    {
      if (!(gen->isLoaded (sMode, dynOnly)))
        {
          gen->loadSizes (sMode, dynOnly);
        }
      if (dynOnly)
        {
          so->addRootAndJacobianSizes (gen->getOffsets (sMode));
        }
      else
        {
          so->addSizes (gen->getOffsets (sMode));
        }
    }
  if (!dynOnly)
    {
      so->stateLoaded = true;

    }
  so->rjLoaded = true;
  /*if (!isDynamic(sMode))
  {
    if (stateSize(sMode)>2)
    {
          printf("%s %d has statesize=%d\n",name.c_str(),id, stateSize(sMode));
    }
  }*/
}



void gridBus::updateFlags (bool /*dynOnly*/)
{


  opFlags.reset (preEx_requested);
  opFlags.reset (has_powerflow_adjustments);

  for (auto &gen : attachedGens)
    {
      if (gen->enabled)
        {
          opFlags |= gen->cascadingFlags ();
        }

    }
  for (auto &load : attachedLoads)
    {
      if (load->enabled)
        {
          opFlags |= load->cascadingFlags ();
        }
    }

}

static const IOlocs inLoc {
  0,1,2
};


//#define DEBUG_KEY_BUS 1445
// computed power at bus
void gridBus::updateLocalCache  (const stateData &sD, const solverMode &sMode)
{
  if (!S.needsUpdate (sD))
    {
      return;
    }
  S.reset ();
  if (!isConnected ())
    {
      return;
    }
  outputs[voltageInLocation] = getVoltage (sD, sMode);
  outputs[angleInLocation] = getAngle (sD, sMode);
  outputs[frequencyInLocation] = getFreq (sD, sMode);
#if DEBUG_KEY_BUS > 0
  if (id == DEBUG_KEY_BUS)
  {
	  printf("%d V=%f, A=%f voltage=%f, angle=%f \n", DEBUG_KEY_BUS, outputs[voltageInLocation], outputs[angleInLocation]*180.0/kPI,voltage,angle*180/kPI );
  }
#endif
  auto cid = getID ();
  for (auto &link : attachedLinks)
    {
      if (link->enabled)
        {
          link->updateLocalCache (sD, sMode);
          S.linkP += link->getRealPower (cid);
          S.linkQ += link->getReactivePower (cid);
#if DEBUG_KEY_BUS > 0
          if (id == DEBUG_KEY_BUS)
            {
              printf ("%d linkP=%f, linkQ=%f line %s\n", DEBUG_KEY_BUS, link->getRealPower (cid), link->getReactivePower (cid), link->name.c_str ());
            }
#endif
        }
    }
  if (isExtended (sMode))
    {
      auto offset = offsets.getAlgOffset (sMode);
      S.loadP = sD.state[offset];
      S.loadQ = sD.state[offset + 1];
      return;
    }


  for (auto &gen : attachedGens)
    {
      if (gen->isConnected())
        {
		  gen->updateLocalCache(outputs, sD, sMode);
          S.genP += gen->getRealPower (outputs, sD, sMode);
          S.genQ += gen->getReactivePower (outputs, sD, sMode);
        }
    }

  for (auto &ld : attachedLoads)
    {
      if (ld->isConnected())
        {
		  ld->updateLocalCache(outputs, sD, sMode);
          S.loadP += ld->getRealPower (outputs, sD, sMode);
          S.loadQ += ld->getReactivePower (outputs, sD, sMode);
        }
    }
  S.seqID = sD.seqID;

}

void busPowers::reset ()
{
  linkP = 0;
  linkQ = 0;
  genP = 0;
  genQ = 0;
  loadP = 0;
  loadQ = 0;
}

bool busPowers::needsUpdate (const stateData &sD) const
{
  if (sD.empty())
    {
      return true;
    }
  if (sD.seqID != seqID)
    {
      return true;
    }
  if (sD.seqID == 0)
    {
      return true;
    }
  return false;
}



// computed power at bus
void gridBus::updateLocalCache ()
{
  S.reset ();
  auto cid = getID ();
  for (auto &link : attachedLinks)
    {
      if (link->enabled)
        {
		  link->updateLocalCache();
          S.linkP += link->getRealPower (cid);
          S.linkQ += link->getReactivePower (cid);
        }

    }
  for (auto &load : attachedLoads)
    {
      if (load->isConnected ())
        {
          S.loadP += load->getRealPower (voltage);
          S.loadQ += load->getReactivePower (voltage);
        }

    }
  for (auto &gen : attachedGens)
    {
      if (gen->isConnected ())
        {
          S.genP += gen->getRealPower ();
          S.genQ += gen->getReactivePower ();
        }
    }

  if (!opFlags[dyn_initialized])
    {
      if ((type == busType::SLK) || (type == busType::afix))
        {
          S.genP = -(S.loadP + S.linkP);
        }
      if ((type == busType::SLK) || (type == busType::PV))
        {
          // genQ = -(loadQ + linkQ);
        }
    }
  //now adjust the generation values for non PQ buses

  /*else
  {
  if (std::abs(linkP + loadP) > 0.001)
  {
  printf("Bus %s has spurious generation requirement of %f\n",  name.c_str(), linkP + loadP);
  }
  }*/


}



double gridBus::getAdjustableCapacityUp (gridDyn_time /*time*/) const
{
  return 0.0;
}

double gridBus::getAdjustableCapacityDown (gridDyn_time /*time*/) const
{
  return 0.0;
}

double gridBus::getFreqResp () const
{
  return 0;
}

double gridBus::getRegTotal () const
{
  return 0;
}

double gridBus::getSched () const
{
  return 0;
}
gridLink *gridBus::findLink (gridBus *bs) const
{
  gridLink *lnk = nullptr;

  for (auto lnk2 : attachedLinks)
    {
      if ((lnk2->getBus (1)) == bs)
        {
          lnk = lnk2;
          break;
        }
      else if ((lnk2->getBus (2)) == bs)
        {
          lnk = lnk2;
          break;
        }
    }

  return lnk;
}



coreObject *gridBus::find (const std::string &objname) const
{
  coreObject *obj = nullptr;
  if ((objname == this->name) || (objname == "bus"))
    {
      return const_cast<gridBus *> (this);
    }
  if (objname == "root")
    {
      if (parent)
        {
          return (parent->find (objname));
        }
      else
        {
          return const_cast<gridBus *> (this);
        }
    }
  for (auto ob : attachedGens)
    {
      if (objname == ob->getName ())
        {
          obj = ob;
          break;
        }
    }
  if (obj == nullptr)
    {
      for (auto ob : attachedLoads)
        {
          if (objname == ob->getName ())
            {
              obj = ob;
              break;
            }
        }
    }
  return obj;
}

coreObject *gridBus::getSubObject (const std::string &typeName, index_t num) const
{
  if (typeName == "link")
    {
      return getLink (num);
    }
  else if (typeName == "load")
    {
      return getLoad (num);
    }
  else if ((typeName == "gen") || (typeName == "generator"))
    {
      return getGen (num);
    }
  else
    {
      return nullptr;
    }
}

coreObject *gridBus::findByUserID (const std::string &typeName, index_t searchID) const
{
  if (typeName == "load")
    {
      for (auto &LD : attachedLoads)
        {
          if (LD->getUserID () == searchID)
            {
              return LD;
            }
        }
    }
  else if ((typeName == "gen") || (typeName == "generator"))
    {
      for (auto &gen : attachedGens)
        {
          if (gen->getUserID () == searchID)
            {
              return gen;
            }
        }
    }
  else if (typeName == "link")
    {
      for (auto &link : attachedLinks)
        {
          if (link->getUserID () == searchID)
            {
              return link;
            }
        }
    }
  return nullptr;
}

gridLink *gridBus::getLink (index_t x) const
{
  return (x < attachedLinks.size ()) ? attachedLinks[x] : nullptr;
}

gridLoad *gridBus::getLoad (index_t x) const
{
  return (x < attachedLoads.size ()) ? attachedLoads[x] : nullptr;
}

gridDynGenerator *gridBus::getGen (index_t x) const
{
  return (x < attachedGens.size ()) ? attachedGens[x] : nullptr;
}


void gridBus::mergeBus (gridBus *)
{

}

void gridBus::unmergeBus (gridBus *)
{

}

void gridBus::checkMerge ()
{

}



void gridBus::registerVoltageControl (gridObject *)
{

}
/** @brief  remove an object from voltage control on a bus*/
void gridBus::removeVoltageControl (gridObject *)
{

}

void gridBus::registerPowerControl (gridObject *)
{

}

void gridBus::removePowerControl (gridObject *)
{

}

double gridBus::get (const std::string &param, units_t unitType) const
{
  double val;
  if (param == "voltage")
    {
      val = unitConversionPower (voltage, puV, unitType, systemBasePower, baseVoltage);
    }
  else if (param == "angle")
    {
      val = unitConversionAngle (angle, rad, unitType);
    }
  else if (param == "vtol")
    {
      val = Vtol;
    }
  else if (param == "atol")
    {
      val = Atol;
    }
  else if ((param == "basevoltage") || (param == "vbase"))
    {
      val = baseVoltage;
    }
  else if ((param == "genreal") || (param == "generationreal"))
    {
      val = unitConversionPower (getGenerationReal (), puMW, unitType, systemBasePower, baseVoltage);
    }
  else if ((param == "genreactive") || (param == "generationreactive"))
    {
      val = unitConversionPower (getGenerationReactive (), puMW, unitType, systemBasePower, baseVoltage);
    }
  else if (param == "loadreal")
    {
      val = unitConversionPower (getLoadReal (), puMW, unitType, systemBasePower, baseVoltage);
    }
  else if (param == "loadreactive")
    {
      val = unitConversionPower (getLoadReactive (), puMW, unitType, systemBasePower, baseVoltage);
    }
  else if (param == "linkreal")
    {
      val = unitConversionPower (getLinkReal (), puMW, unitType, systemBasePower, baseVoltage);
    }
  else if (param == "linkreactive")
    {
      val = unitConversionPower (getLinkReactive (), puMW, unitType, systemBasePower, baseVoltage);
    }
  else if (param == "gencount")
    {
      val = static_cast<double> (attachedGens.size ());
    }
  else if (param == "linkcount")
    {
      val = static_cast<double> (attachedLinks.size ());
    }
  else if (param == "loadcount")
    {
      val = static_cast<double> (attachedLoads.size ());
    }
  else if (param == "zone")
    {
      val = zone;
    }
  else if ((param == "p")||(param == "q")||(param == "yp")||(param == "yq")||(param == "ip")||(param == "iq"))
    {
      val = 0;
      for (const auto &ld : attachedLoads)
        {
          val += ld->get (param, unitType);
        }
    }
  else
    {
      return gridPrimary::get (param, unitType);
    }
  return val;
}

change_code gridBus::rootCheck (const stateData &sD, const solverMode &sMode, check_level_t level)
{
  auto args = getOutputs (sD, sMode);
  // double vcurr = args[voltageInLocation];
  change_code ret = change_code::no_change;
  //make sure we are not in a fault condition
  for (auto &gen : attachedGens)
    {
      if ((gen->checkFlag (has_alg_roots)) && (gen->enabled))
        {
          auto iret = gen->rootCheck (args, sD, sMode, level);
          if (iret > ret)
            {
              ret = iret;
            }
        }
    }
  for (auto &load : attachedLoads)
    {
      if ((load->checkFlag (has_alg_roots)) && (load->enabled))
        {
          auto iret = load->rootCheck (args, sD, sMode, level);
          if (iret > ret)
            {
              ret = iret;
            }
        }
    }
  return ret;
}

void gridBus::rootTest (const stateData &sD, double roots[], const solverMode &sMode)
{
  auto args = getOutputs (sD, sMode);
  for (auto &gen : attachedGens)
    {
      if ((gen->checkFlag (has_roots)) && (gen->enabled))
        {
          gen->rootTest (args, sD, roots, sMode);

        }
    }
  for (auto &load : attachedLoads)
    {
      if ((load->checkFlag (has_roots)) && (load->enabled))
        {
          load->rootTest (args, sD, roots, sMode);
        }
    }
}


void gridBus::rootTrigger (gridDyn_time ttime, const std::vector<int> &rootMask, const solverMode &sMode)
{
  size_t rc = 0;
  int rootOffset = offsets.getRootOffset (sMode);

  auto RF = vecFindne (rootMask, 0, rootOffset + rc, rootOffset + rootSize (sMode));

  if (RF.size () > 0)
    {
      size_t rfi = 0;
      auto args = getOutputs (emptyStateData, cLocalSolverMode);
      auto nR = RF[rfi];
      for (auto &gen : attachedGens)
        {
          if ((gen->checkFlag (has_roots)) && (gen->enabled))
            {
              rc += gen->rootSize (sMode);
              if (nR < rootOffset + rc)
                {
                  gen->rootTrigger (ttime, args, rootMask, sMode);
                  do
                    {
                      ++rfi;
                      if (rfi >= RF.size ())
                        {
                          return;
                        }
                      nR = RF[rfi];
                    }
                  while (nR < rootOffset + rc);
                }

            }
        }
      for (auto &load : attachedLoads)
        {
          if ((load->checkFlag (has_roots)) && (load->enabled))
            {

              rc += load->rootSize (sMode);
              if (nR < rootOffset + rc)
                {
                  load->rootTrigger (ttime, args, rootMask, sMode);
                  do
                    {
                      ++rfi;
                      if (rfi >= RF.size ())
                        {
                          return;
                        }
                      nR = RF[rfi];
                    }
                  while (nR < rootOffset + rc);
                }
            }
        }
    }

}

bool compareBus (gridBus *bus1, gridBus *bus2, bool /*cmpBus*/, bool printDiff)
{
  bool cmp = true;


  if (bus1->dynType != bus2->dynType)
    {
      cmp = false;
    }

  if (std::abs (bus1->baseVoltage - bus2->baseVoltage) > 0.00001)
    {
      cmp = false;
    }
  if (std::abs (bus1->systemBasePower - bus2->systemBasePower) > 0.00001)
    {
      cmp = false;
    }
  if (bus1->attachedLoads.size () != bus2->attachedLoads.size ())
    {
      cmp = false;
    }
  else
    {
      bool fmatch = true;
      for (size_t kk = 0; kk < bus1->attachedLoads.size (); ++kk)
        {
          fmatch = false;
          for (size_t jj = 0; jj < bus2->attachedLoads.size (); ++jj)
            {
              if (compareLoad (bus1->attachedLoads[kk], bus2->attachedLoads[jj], printDiff))
                {
                  fmatch = true;
                  break;
                }
            }
        }
      if (fmatch == false)
        {
          cmp = false;
        }

    }
  if (bus1->attachedGens.size () != bus2->attachedGens.size ())
    {
      cmp = false;
    }
  else
    {

    }
  if (bus1->attachedLinks.size () != bus2->attachedLinks.size ())
    {
      cmp = false;
    }
  else
    {
      for (size_t kk = 0; kk < bus1->attachedLinks.size (); ++kk)
        {
          if (!compareLink (bus1->attachedLinks[kk], bus2->attachedLinks[kk], false, printDiff))
            {
              cmp = false;
            }
        }
    }
  return cmp;
}

gridBus * getMatchingBus (gridBus *bus, const gridPrimary *src, gridPrimary *sec)
{
  
  if (bus->getParent () == nullptr)
    {
      return nullptr;
    }
  if (bus->getParent ()->getID () == src->getID ())                //if this is true then things are easy
    {
      return sec->getBus (bus->locIndex);
    }
  
      auto par = dynamic_cast<gridPrimary *> (bus->getParent ());
      if (par == nullptr)
        {
          return nullptr;
        }
	  std::vector<index_t> lkind={ bus->locIndex };
      while (par->getID () != src->getID ())
        {
          lkind.push_back (par->locIndex);
          par = dynamic_cast<gridPrimary *> (par->getParent ());
          if (par == nullptr)
            {
              return nullptr;
            }
        }
      //now work our way backwards through the secondary
      par = sec;
      for (auto kk = lkind.size () - 1; kk > 0; --kk)
        {
          par = dynamic_cast<gridPrimary *> (par->getArea (lkind[kk]));
        }
      return par->getBus (lkind[0]);

}
