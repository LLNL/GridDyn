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

// headers
#include "linkModels/acLine.h"
#include "gridBus.h"
#include "gridArea.h"
#include "objectFactoryTemplates.h"
#include "vectorOps.hpp"
#include "linkModels/dcLink.h"
#include "objectInterpreter.h"
#include "acdcConverter.h"
#include "gridCoreTemplates.h"
#include "simulation/contingency.h"
#include "stringOps.h"
#include "matrixDataCompact.h"
#include "core/gridDynExceptions.h"

#include <complex>
#include <cmath>

using namespace gridUnits;

//make the object factory types

static typeFactory<gridLink> blf ("link", stringVec { "trivial","basic","transport" });

static childTypeFactory<acLine,gridLink> glf ("link", stringVec { "ac", "line", "phaseshifter", "phase_shifter","transformer" }, "ac");

static childTypeFactory<adjustableTransformer,gridLink> gfad ("link", stringVec { "adjust", "adjustable", "adjustabletransformer"});

static childTypeFactory<dcLink,gridLink> dclnk ("link", stringVec {"dc", "dclink", "dcline"});

static typeFactoryArg<acdcConverter,acdcConverter::mode_t> dcrect ("link", stringVec { "rectifier", "rect" },acdcConverter::mode_t::rectifier);
static typeFactoryArg<acdcConverter, acdcConverter::mode_t> dcinv ("link", stringVec { "inverter", "inv" },acdcConverter::mode_t::inverter);
static childTypeFactory<acdcConverter, gridLink> acdc ("link", stringVec { "acdc", "acdcconverter", "dcconverter" });

std::atomic<count_t> gridLink::linkCount(0);
//helper defines to have things make more sense
#define DEFAULTPOWERCOMP (this->*(flowCalc[0]))
#define MODEPOWERCOMP (this->*(flowCalc[getLinkApprox (sMode)]))
#define DERIVCOMP (this->*(derivCalc[getLinkApprox (sMode)]))
#define DEFAULTDERIVCOMP (this->*(derivCalc[0]))

gridLink::gridLink (const std::string &objName) : gridPrimary (objName)
{
  // default values
  id = ++linkCount;
  updateName ();



}

gridCoreObject *gridLink::clone (gridCoreObject *obj) const
{
  gridLink *lnk = cloneBaseFactory<gridLink, gridPrimary> (this, obj, &glf);
  if (!(lnk))
    {
      return obj;
    }

  lnk->Pset = Pset;
  lnk->Erating = Erating;
  lnk->ratingB = ratingB;
  lnk->ratingA = ratingA;
  lnk->lossFraction = lossFraction;
  lnk->curcuitNum = curcuitNum;
  lnk->zone = zone;
  return lnk;
}


gridLink::~gridLink ()
{

}

// timestepP link's buses
void gridLink::updateBus (gridBus *bus, index_t busnum)
{
  if (busnum == 1)
    {
      if (B1)
        {
          B1->remove (this);
        }
      B1 = bus;
      B1->add (this);

    }
  else if (busnum == 2)
    {
      if (B2)
        {
          B2->remove (this);
        }
      B2 = bus;
      B2->add (this);
    }
  else
    {
	  throw(objectAddFailure(this));
    }
}

void gridLink::followNetwork (int network, std::queue<gridBus *> & stk)
{
	if (isConnected()&&opFlags[network_connected])
	{
		if (B1->Network != network)
		{
			stk.push(B1);
		}
		if (B2->Network != network)
		{
			stk.push(B2);
		}
	}
}

void gridLink::pFlowCheck (std::vector<violation> &Violation_vector)
{
  double mva = std::max (getCurrent (0), getCurrent (1));
  if (mva > ratingA)
    {
      violation V (name, MVA_EXCEED_RATING_A);
      V.level = mva;
      V.limit = ratingA;
      V.percentViolation = (mva - ratingA) / ratingA * 100;
      Violation_vector.push_back (V);
    }
  if (mva > ratingB)
    {
      violation V (name, MVA_EXCEED_RATING_B);

      V.level = mva;
      V.limit = ratingB;
      V.percentViolation = (mva - ratingB) / ratingB * 100;
      Violation_vector.push_back (V);
    }
  if (mva > Erating)
    {
      violation V (name, MVA_EXCEED_ERATING);
      V.level = mva;
      V.limit = Erating;
      V.percentViolation = (mva - Erating) / Erating * 100;
      Violation_vector.push_back (V);
    }
}


double gridLink::quickupdateP ()
{
  return Pset;
}


void gridLink::timestep (const gridDyn_time ttime, const solverMode &)
{

  if (!enabled)
    {
      return;

    }

  updateLocalCache ();
  prevTime = ttime;
  /*if (scheduled)
  {
  Psched=sched->timestepP(time);
  }*/

}



static const stringVec locNumStrings {
  "loss","switch1","switch2","p"
};
static const stringVec locStrStrings {
  "from", "to"
};
static const stringVec flagStrings {};
void gridLink::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
  getParamString<gridLink, gridPrimary> (this, pstr, locNumStrings, locStrStrings, flagStrings, pstype);
}

// set properties
void gridLink::set (const std::string &param,  const std::string &val)
{

  if ((param == "bus1")||(param == "from"))
    {

      gridBus *bus = dynamic_cast<gridBus *> (locateObject (val, parent));
      
      if (bus)
        {
          updateBus (bus, 1);
        }
	  else
	  {
		  throw(invalidParameterValue());
	  }
    }
  else if ((param == "bus2")||(param == "to"))
    {
      gridBus *bus = dynamic_cast<gridBus *> (locateObject (val, parent));
      if (bus)
        {
          updateBus (bus, 2);
        }
	  else
	  {
		  throw(invalidParameterValue());
	  }
    }
  else if (param == "status")
    {
      auto v2 = convertToLowerCase (val);
      if ((v2 == "closed")||(v2 == "connected"))
        {
          reconnect ();
        }
      else if ((v2 == "open")||(v2 == "disconnected"))
        {
          disconnect ();
        }
    }
  else
    {
      gridPrimary::set (param, val);
    }
}


//true is open
//false is closed
void gridLink::switchMode (index_t num, bool mode)
{
  if (num == 2)
    {
      if (mode == opFlags[switch2_open_flag])
        {
          return;
        }

      opFlags.flip (switch2_open_flag);

      if (opFlags[pFlow_initialized])
        {
          LOG_DEBUG ("Switch2 changed||state =" + ((opFlags[switch2_open_flag]) ? std::string ("OPEN") : std::string ("CLOSED")) + ", link status =" + ((isConnected ()) ? std::string ("CONNECTED") : std::string ("DISCONNECTED")));
          if (isConnected ())
            {

              reconnect ();
            }
          else
            {
              switchChange (2);

              if (!B1->checkCapable ())
                {
                  B1->disconnect ();
                }
              if (!B2->checkCapable ())
                {
                  B2->disconnect ();
                }
              updateLocalCache ();
              alert (this, CONNECTIVITY_CHANGE);
            }



        }
    }
  else
    {
      if (mode == opFlags[switch1_open_flag])
        {
          return;
        }
      opFlags.flip (switch1_open_flag);

      if (opFlags[pFlow_initialized])
        {
          LOG_DEBUG ("Switch2 changed||state =" + ((opFlags[switch2_open_flag]) ? std::string ("OPEN") : std::string ("CLOSED")) + ", link status =" + ((isConnected ()) ? std::string ("CONNECTED") : std::string ("DISCONNECTED")));
          if (isConnected ())
            {
              reconnect ();
            }
          else
            {
              switchChange (1);
              if (!B1->checkCapable ())
                {
                  B1->disconnect ();
                }
              if (!B2->checkCapable ())
                {
                  B2->disconnect ();
                }
              updateLocalCache ();
              alert (this, CONNECTIVITY_CHANGE);
            }


        }


    }

}


void gridLink::switchChange (int /*switchNum*/)
{
  computePowers ();
}

void gridLink::disconnect ()
{
  if (isConnected ())
    {
      opFlags.set (switch1_open_flag, true);
      opFlags.set (switch2_open_flag, true);
      switchChange (1);
      switchChange (2);
      if (!B1->checkCapable ())
        {
          B1->disconnect ();
        }
      if (!B2->checkCapable ())
        {
          B2->disconnect ();
        }
      updateLocalCache ();
      alert (this, CONNECTIVITY_CHANGE);
    }
}

void gridLink::reconnect ()
{
  if (!isConnected ())
    {
      if (opFlags[switch1_open_flag])
        {
          opFlags.reset (switch1_open_flag);
          switchChange (1);
        }
      if (opFlags[switch2_open_flag])
        {
          opFlags.reset (switch2_open_flag);
          switchChange (2);
        }
      updateLocalCache ();
    }

  if (B1->checkFlag (disconnected))
    {
      if (!(B2->checkFlag (disconnected)))
        {
          B1->reconnect (B2);
          updateLocalCache ();
        }
    }
  else if (B2->checkFlag (disconnected))
    {
      B2->reconnect (B1);
      updateLocalCache ();
    }
}

void gridLink::set (const std::string &param, double val, units_t unitType)
{

  if ((param == "state") || (param == "switch") || (param == "switch1")||(param == "breaker")||(param == "breaker_open")||(param == "breaker1")||(param == "breaker_open1"))
    {
      switchMode (1, (val > 0));
    }
  else if ((param == "switch2")||(param == "breaker2") || (param == "breaker_open2"))
    {
      switchMode (2, (val > 0));
    }
  else if (param == "pset")
    {
      Pset = unitConversion (val,unitType,puMW,systemBasePower);
      opFlags.set (fixed_target_power);
      computePowers ();
    }
  else if ((param == "loss") || (param == "lossfraction"))
    {
      lossFraction = val;
      computePowers ();
    }
  else if ((param == "ratinga") || (param == "rating"))
    {
      ratingA = unitConversion (val, unitType, puMW, systemBasePower);
    }
  else if (param == "ratingb")
    {
      ratingB = unitConversion (val, unitType, puMW, systemBasePower);
    }
  else if ((param == "ratinge") || (param == "emergency_rating") || (param == "erating")||(param == "ratingc"))
    {
      Erating = unitConversion (val, unitType, puMW, systemBasePower);
    }
  else if (param == "curcuit")
    {
      curcuitNum = static_cast<index_t> (val);
    }
  else if (param == "zone")
    {
      zone = static_cast<int> (zone);
    }
  else
    {
      gridPrimary::set (param, val, unitType);
    }
}


gridCoreObject *gridLink::getSubObject (const std::string &typeName, index_t num) const
{
  if (typeName == "bus")
    {
      return (num == 1) ? B1 : ((num == 2) ? B2 : nullptr);
    }
  else
    {
      return nullptr;
    }
}


double  gridLink::get (const std::string &param, units_t unitType) const
{
  double val = kNullVal;

  if ((param == "breaker1") || (param == "switch1") || (param == "breaker_open1"))
    {
      val = static_cast<double> (opFlags[switch1_open_flag]);
    }
  else if ((param == "breaker2") || (param == "switch2") || (param == "breaker_open2"))
    {
      val = static_cast<double> (opFlags[switch2_open_flag]);
    }
  else if ((param == "connected") || (param == "breaker"))
    {
      val = static_cast<double> (isConnected ());
    }
  else if ((param == "set")||(param == "pset"))
    {
      val = gridUnits::unitConversion (Pset, puMW, unitType, systemBasePower);
    }
  else if (param == "linkcount")
    {
      val = 1.0;
    }
  else if ((param == "buscount")||(param == "gencount")||(param == "loadcount")||(param == "relaycount"))
    {
      val = 0.0;
    }
  else if ((param == "rating")||(param == "ratinga"))
    {
      val = ratingA;
    }
  else if (param == "ratingb")
    {
      val = ratingB;
    }
  else if (param == "erating")
    {
      val = Erating;
    }
  else if (param == "loss")
    {
      val = unitConversion (getLoss (), puMW, unitType, systemBasePower);
    }
  else if (param == "lossfraction")
    {
      val = lossFraction;
    }
  else if (param == "curcuit")
    {
      val = curcuitNum;
    }
  else if (param == "zone")
    {
      val = zone;
    }
  else
    {
      val = gridPrimary::get (param, unitType);
    }
  return val;
}

void gridLink::pFlowObjectInitializeA (gridDyn_time /*time0*/, unsigned long /*flags*/)
{
  if (!B1)
    {
      opFlags.set (switch1_open_flag);
    }
  if (!B2)
    {
      opFlags.set (switch2_open_flag);
    }

}

bool gridLink::isConnected () const
{
  return (!(opFlags[switch1_open_flag] || opFlags[switch2_open_flag]));
}

int gridLink::fixRealPower (double power, index_t mterminal,index_t /*fixedTerminal*/,units_t unitType)
{
  if (mterminal == 1)
    {
      Pset = unitConversion (power, unitType, puMW, systemBasePower);
    }
  else
    {
      Pset = unitConversion (power, unitType, puMW, systemBasePower) / (1.0 - lossFraction);
    }
  opFlags.set (fixed_target_power);
  return 1;
}


static IOlocs aLoc {
  0,1
};

int gridLink::fixPower  (double rPower, double /*qPower*/, index_t mterminal, index_t fixedTerminal, gridUnits::units_t unitType)
{
  return fixRealPower (rPower, mterminal, fixedTerminal, unitType);
}

void gridLink::dynObjectInitializeA (gridDyn_time /*time0*/, unsigned long /*flags*/)
{
  if ((B1 == nullptr) || (B2 == nullptr))
    {
      disable ();
    }
  else if ((!B1->enabled) || (!B2->enabled))
    {
      disable ();
    }


}

void gridLink::computePowers ()
{
  if (isConnected ())
    {
      linkFlows.P1 = Pset;
      linkFlows.P2 = Pset - std::abs (Pset) * lossFraction;
    }
  else
    {
      linkFlows.P1 = 0;
      linkFlows.P2 = 0;
    }

}

void gridLink::ioPartialDerivatives (index_t /*busId*/, const stateData *, matrixData<double> &, const IOlocs & /*argLocs*/, const solverMode & /*sMode*/)
{


}

void gridLink::outputPartialDerivatives (const stateData *, matrixData<double> &, const solverMode &)
{

}

void gridLink::outputPartialDerivatives (index_t /*busId*/, const stateData *, matrixData<double> &, const solverMode &)
{


}
IOdata gridLink::getOutputs (const stateData *sD, const solverMode &sMode)
{
  return getOutputs (1,sD,sMode);
}

IOdata gridLink::getOutputs (index_t busId, const stateData *, const solverMode &)
{
  // set from/to buses
  IOdata out {
    0.0,0.0
  };

  if ((busId == 2) || (busId == B2->getID ()))
    {
      out[PoutLocation] = Pset;
    }
  else
    {
      out[PoutLocation] = Pset - std::abs (Pset) * lossFraction;
    }
  return out;
}


void gridLink::disable ()
{
  if (enabled == false)
    {
      return;
    }
  if ((opFlags[has_pflow_states]) || (opFlags[has_dyn_states]))
    {
      alert (this, STATE_COUNT_CHANGE);
    }
  else
    {
      alert (this, JAC_COUNT_CHANGE);
    }
  enabled = false;
  if ((B1) && (B1->enabled))
    {
      if (!(B1->checkCapable ()))
        {
          B1->disable ();
        }
    }
  if ((B2) && (B2->enabled))
    {
      if (!(B2->checkCapable ()))
        {
          B2->disable ();
        }
    }

}

double gridLink::getMaxTransfer () const
{
  if (!isConnected ())
    {
      return 0;
    }
  if (Erating > 0)
    {
      return Erating;
    }
  else if (ratingB > 0)
    {
      return ratingB;
    }
  else if (ratingA > 0)
    {
      return ratingA;
    }
  else
    {
      return (kBigNum);
    }
}

double gridLink::getAbsAngle (index_t busId) const
{
  if ((busId == 2)||(busId == B2->getID ()))
    {
      return B2->getAngle ();
    }
  else
    {
      return B1->getAngle ();
    }
}

double gridLink::getVoltage (index_t busId) const
{
  if ((busId == 2) || (busId == B2->getID ()))
    {
      return B2->getVoltage ();
    }
  else
    {
      return B1->getVoltage ();
    }
}

void gridLink::setState (gridDyn_time ttime, const double /*state*/[], const double /*dstate_dt*/[], const solverMode &)
{
  prevTime = ttime;

}

void gridLink::updateLocalCache (const stateData *sD, const solverMode &sMode)
{
  if (!enabled)
    {
      return;
    }
  if ((linkInfo.seqID == sD->seqID) && (sD->seqID != 0))
    {
      return;            //already computed
    }
  linkInfo.v1 = B1->getVoltage (sD, sMode);
  double t1 = B1->getAngle (sD, sMode);
  linkInfo.v2 = B2->getVoltage (sD, sMode);
  double t2 = B2->getAngle (sD, sMode);

  linkInfo.theta1 = t1 - t2;
  linkInfo.theta2 = t2 - t1;
  linkInfo.seqID = sD->seqID;

}

void gridLink::updateLocalCache ()
{
  if (!enabled)
    {
      return;
    }
  linkInfo.v1 = B1->getVoltage ();
  double t1 = B1->getAngle ();
  linkInfo.v2 = B2->getVoltage ();
  double t2 = B2->getAngle ();

  linkInfo.theta1 = t1 - t2;
  linkInfo.theta2 = t2 - t1;
}


gridBus * gridLink::getBus (index_t busInd) const
{
  return ((busInd == 1) || (busInd == B1->getID ())) ? B1 : (((busInd == 2) || (busInd == B2->getID ())) ? B2 : nullptr);
}


double gridLink::getRealPower (index_t busId) const
{

  return ((busId == 2) || (busId == B2->getID ())) ? linkFlows.P2 : linkFlows.P1;
}


double gridLink::getReactivePower (index_t busId) const
{
  return ((busId == 2) || (busId == B2->getID ())) ? linkFlows.Q2 : linkFlows.Q1;
}

double gridLink::remainingCapacity () const
{
  return getMaxTransfer () - std::abs (linkFlows.P1);
}

double gridLink::getAngle (const double state[], const solverMode &sMode) const
{
  double t1 = B1->getAngle (state, sMode);
  double t2 = B2->getAngle (state, sMode);
  return t1 - t2;
}

double gridLink::getAngle () const
{
  return (linkInfo.theta1);
}


double gridLink::getLoss () const
{
  return std::abs (linkFlows.P1 + linkFlows.P2);
}

double gridLink::getReactiveLoss () const
{
  return std::abs (linkFlows.Q1 + linkFlows.Q2);
}

double gridLink::getRealImpedance (index_t busId) const
{
  if ((busId == 2) || (busId == B2->getID ()))           // from bus
    {
      std::complex<double> Z = (linkInfo.v2 * linkInfo.v2) / std::complex<double> (linkFlows.P2, linkFlows.Q2);
      return std::isnormal (Z.real ()) ? Z.real () : kBigNum;
    }
  else
    {
      std::complex<double> Z = (linkInfo.v1 * linkInfo.v1) / std::complex<double> (linkFlows.P1, linkFlows.Q1);
      return std::isnormal (Z.real ()) ? Z.real () : kBigNum;
    }
}
double gridLink::getImagImpedance (index_t busId) const
{
  if ((busId == 2) || (busId == B2->getID ()))           // from bus
    {
      std::complex<double> Z = (linkInfo.v2 * linkInfo.v2) / std::complex<double> (linkFlows.P2, linkFlows.Q2);
      return std::isnormal (Z.imag ()) ? Z.imag () : kBigNum;
    }
  else
    {
      std::complex<double> Z = (linkInfo.v1 * linkInfo.v1) / std::complex<double> (linkFlows.P1, linkFlows.Q1);
      return std::isnormal (Z.imag ()) ? Z.imag () : kBigNum;
    }
}

double gridLink::getTotalImpedance (index_t busId) const
{
  if ((busId == 2) || (busId == B2->getID ()))      // from bus
    {
      //  printf("id2 impedance=%f\n", signn(linkFlows.P2 + linkFlows.Q2)*(linkInfo.v2*linkInfo.v2) / std::hypot(linkFlows.P2, linkFlows.Q2));
      double val = signn (linkFlows.P2 + linkFlows.Q2) * (linkInfo.v2 * linkInfo.v2) / std::hypot (linkFlows.P2, linkFlows.Q2);
      return (std::isnormal (val) ? val : kBigNum);
    }
  else
    {
      // printf("id1 impedance=%f\n", signn(linkFlows.P1 + linkFlows.Q1)*(linkInfo.v1*linkInfo.v1) / std::hypot(linkFlows.P1, linkFlows.Q1));
      double val = signn (linkFlows.P1 + linkFlows.Q1) * (linkInfo.v1 * linkInfo.v1) / std::hypot (linkFlows.P1, linkFlows.Q1);
      return (std::isnormal (val) ? val : kBigNum);
    }

}

double gridLink::getCurrent (index_t busId) const
{
  double val;
  if ((busId == 2) || (busId == B2->getID ()))           // from bus
    {
      val = std::hypot (linkFlows.P2, linkFlows.Q2) / (linkInfo.v2);

    }
  else
    {
      val = std::hypot (linkFlows.P1, linkFlows.Q1) / (linkInfo.v1);
    }
  return (std::isnormal (val) ? val : 0);
}

double gridLink::getRealCurrent (index_t busId) const
{
  double val;
  if ((busId == 2) || (busId == B2->getID ()))      // from bus
    {
      val = linkFlows.P2 / (linkInfo.v2);
    }
  else
    {
      val = linkFlows.P1 / (linkInfo.v1);
    }
  return (std::isnormal (val) ? val : 0);
}
double gridLink::getImagCurrent (index_t busId) const
{
  double val;
  if ((busId == 2) || (busId == B2->getID ()))      // from bus
    {
      val = linkFlows.Q2 / (linkInfo.v2);
    }
  else
    {
      val = linkFlows.Q1 / (linkInfo.v1);
    }
  return (std::isnormal (val) ? val : 0);
}

gridLink * getMatchingLink (gridLink *lnk, gridPrimary *src, gridPrimary *sec)
{
  gridLink *L2 = nullptr;
  if (lnk->getParent () == nullptr)
    {
      return nullptr;
    }
  if (lnk->getParent ()->getID () == src->getID ())    //if this is true then things are easy
    {
      L2 = sec->getLink (lnk->locIndex);
    }
  else
    {
      gridPrimary *par;
      std::vector<int> lkind;
      par = dynamic_cast<gridPrimary *> (lnk->getParent ());
      if (par == nullptr)
        {
          return nullptr;
        }
      lkind.push_back (lnk->locIndex);
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
      for (size_t kk = lkind.size () - 1; kk > 0; --kk)
        {
          par = dynamic_cast<gridPrimary *> (par->getArea (lkind[kk]));
        }
      L2 = par->getLink (lkind[0]);

    }
  return L2;
}


bool compareLink(gridLink *lnk1, gridLink *lnk2, bool cmpBus, bool printDiff)
{
	if (cmpBus)
	{
		bool ret = compareBus(lnk1->getBus(1), lnk2->getBus(1),printDiff);
		ret = ret&&compareBus(lnk1->getBus(2), lnk2->getBus(2),printDiff);
		return ret;
	}
	else
	{
		return true;
	}
	
}