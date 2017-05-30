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

#include "breaker.h"
#include "measurement/gridCondition.h"

#include "events/gridEvent.h"
#include "core/coreObjectTemplates.h"
#include "linkModels/gridLink.h"
#include "measurement/gridGrabbers.h"
#include "measurement/stateGrabber.h"
#include "measurement/grabberSet.h"
#include "gridBus.h"
#include "utilities/matrixDataSparse.h"


#include <boost/format.hpp>

#include <cmath>

using namespace gridUnits;
breaker::breaker (const std::string&objName) : gridRelay (objName)
{
  opFlags.set (continuous_flag);
  opFlags.reset (no_dyn_states);
}

coreObject *breaker::clone (coreObject *obj) const
{
  breaker *nobj = cloneBase<breaker, gridRelay> (this, obj);
  if (!(nobj))
    {
      return obj;
    }


  nobj->limit = limit;
  nobj->minClearingTime = minClearingTime;
  nobj->recloseTime1 = recloseTime1;
  nobj->recloseTime2 = recloseTime2;
  nobj->recloserTap = recloserTap;
  nobj->recloserResetTime = recloserResetTime;
  nobj->lastRecloseTime = -lastRecloseTime;
  nobj->maxRecloseAttempts = maxRecloseAttempts;
  nobj->limit = limit;
  nobj->m_terminal = m_terminal;
  nobj->recloseAttempts = recloseAttempts;
  nobj->cTI = cTI;

  nobj->Vbase = Vbase;
  return nobj;
}

void breaker::setFlag (const std::string &flag, bool val)
{
 
  if (flag == "nondirectional")
    {
      opFlags.set (nondirectional_flag,val);
    }
  else
    {
      gridRelay::setFlag (flag, val);
    }

}
/*
std::string commDestName;
std::uint64_t commDestId=0;
std::string commType;
*/
void breaker::set (const std::string &param,  const std::string &val)
{

  if (param == "#")
    {

    }
  else
    {
      gridRelay::set (param, val);
    }
}

void breaker::set (const std::string &param, double val, gridUnits::units_t unitType)
{


  if (param == "reclosetime")
    {
      recloseTime1 = val;
      recloseTime2 = val;
    }
  else if (param == "reclosetime1")
    {
      recloseTime1 = val;
    }
  else if (param == "reclosetime2")
    {
      recloseTime2 = val;
    }
  else if ((param == "maxrecloseattempts")||(param == "reclosers"))
    {
      maxRecloseAttempts = static_cast<count_t> (val);
    }
  else if ((param == "minclearingtime")||(param == "cleartime"))
    {
      minClearingTime = val;
    }
  else if (param == "limit")
    {
      limit = unitConversion (val, unitType, puA, systemBasePower, Vbase);
    }
  else if ((param == "reclosertap")||(param == "tap"))
    {
      recloserTap = val;
    }
  else if (param == "terminal")
    {
      m_terminal = static_cast<index_t> (val);
    }
  else if ((param == "recloserresettime")||(param == "resettime"))
    {
      recloserResetTime = val;
    }
  else
    {
      gridRelay::set (param, val, unitType);
    }
}

void breaker::dynObjectInitializeA (coreTime time0, unsigned long flags)
{

  auto ge = std::make_shared<gridEvent> ();
  auto ge2 = std::make_shared<gridEvent> ();
  if (dynamic_cast<gridLink *> (m_sourceObject))
    {
      add (std::shared_ptr<gridCondition>(make_condition ("current" + std::to_string (m_terminal), ">=", limit, m_sourceObject)));
      ge->setTarget (m_sinkObject, "switch" + std::to_string(m_terminal));
	  ge->setValue(1.0);
      //action 2 to re-close switch
      ge2->setTarget (m_sinkObject, "switch" + std::to_string(m_terminal));
	  ge2->setValue(0.0);
      bus = static_cast<gridLink *> (m_sourceObject)->getBus (m_terminal);

    }
  else
    {
      add (std::shared_ptr<gridCondition>(make_condition ("sqrt(p^2+q^2)/@bus:v", ">=", limit, m_sourceObject)));
      opFlags.set (nonlink_source_flag);
      ge->setTarget (m_sinkObject,"status");
	  ge->setValue(0.0);
      //action 2 to re-enable object
      ge2->setTarget (m_sinkObject,"status");
	  ge2->setValue(0.0);
      bus = static_cast<gridBus *> (m_sourceObject->find ("bus"));
    }

  add (std::move(ge));
  add (std::move(ge2));
  //now make the gridCondition for the I2T condition
  auto gc = std::make_shared<gridCondition> ();
  auto gc2 = std::make_shared<gridCondition> ();

  auto cg = std::make_unique<customGrabber> ();
  cg->setGrabberFunction ("I2T", [this](coreObject *){
    return cTI;
  });

  auto cgst = std::make_unique<customStateGrabber> (this);
  cgst->setGrabberFunction ([ ](coreObject *obj, const stateData &sD, const solverMode &sMode) -> double {
    return sD.state[static_cast<breaker *>(obj)->offsets.getDiffOffset (sMode)];
  });

  auto gset = std::make_shared<grabberSet>(std::move(cg), std::move(cgst));
  gc->setConditionLHS(gset);

  gc2->setConditionLHS(std::move(gset));  //done with gset don't need it after this point

  gc->setConditionRHS (1.0);
  gc2->setConditionRHS (-0.5);
  gc->setComparison (comparison_type::gt);
  gc2->setComparison (comparison_type::lt);

  add (std::move(gc));
  add (std::move(gc2));
  setConditionState (1, condition_states::disabled);
  setConditionState (2, condition_states::disabled);


  gridRelay::dynObjectInitializeA (time0, flags);
}


void breaker::conditionTriggered (index_t conditionNum, coreTime triggeredTime)
{
  if (conditionNum == 0)
    {
      opFlags.set (overlimit_flag);
      setConditionState (0, condition_states::disabled);
      if (recloserTap == 0.0)
        {
          if (minClearingTime <= kMin_Res)
            {
              tripBreaker (triggeredTime);
            }
          else
            {
              nextUpdateTime = triggeredTime + minClearingTime;
              alert (this, UPDATE_TIME_CHANGE);
            }

        }
      else
        {
          cTI = 0;
          setConditionState (1, condition_states::active);
          setConditionState (2, condition_states::active);
          alert (this, JAC_COUNT_INCREASE);
          useCTI = true;
        }
    }
  else if (conditionNum == 1)
    {
      assert (opFlags[overlimit_flag]);
      tripBreaker (triggeredTime);
    }
  else if (conditionNum == 2)
    {
      assert (opFlags[overlimit_flag]);

      setConditionState (1, condition_states::disabled);
      setConditionState (2, condition_states::disabled);
      setConditionState (0, condition_states::active);
      alert (this, JAC_COUNT_DECREASE);
      opFlags.reset (overlimit_flag);
      useCTI = false;
    }
}

void breaker::updateA (coreTime time)
{
  if (opFlags[breaker_tripped_flag])
    {
      if (time+kSmallTime >= nextUpdateTime)
        {
          resetBreaker (time);
        }
    }
  else if (opFlags[overlimit_flag])
    {
      if (time+kSmallTime >= nextUpdateTime)
        {
          if (checkCondition (0))
            {//still over the limit->trip the breaker
              tripBreaker (time);
            }
          else
            {
              opFlags.reset (overlimit_flag);
              setConditionState (0, condition_states::active);
            }
        }
    }
  else
    {
      gridRelay::updateA (time);
    }
 lastUpdateTime = time;
}


void breaker::loadSizes (const solverMode &sMode, bool dynOnly)
{
  gridRelay::loadSizes (sMode, dynOnly);
  auto so = offsets.getOffsets (sMode);

  if ((!isAlgebraicOnly (sMode)) && (recloserTap > 0))
    {
      so->total.diffSize = 1;
      so->total.jacSize = 12;
    }

}

void breaker::timestep (coreTime ttime, const IOdata & /*inputs*/, const solverMode &)
{
  prevTime = ttime;
  if (limit < kBigNum / 2.0)
    {
      double val = getConditionValue (0);
      if (val > limit)
        {
          opFlags.set (breaker_tripped_flag);
          disable ();
          alert (this, BREAKER_TRIP_CURRENT);
        }
    }

}

void breaker::jacobianElements (const IOdata & /*inputs*/, const stateData &sD, matrixData<double> &ad, const IOlocs & /*inputLocs*/, const solverMode &sMode)
{
  if (useCTI)
    {
      matrixDataSparse<double> d;
      IOdata out;
      auto Voffset = bus->getOutputLoc (sMode,voltageInLocation);
      auto inputs = bus->getOutputs (noInputs,sD, sMode);
      auto inputLocs = bus->getOutputLocs (sMode);
      if (opFlags[nonlink_source_flag])
        {
          gridSecondary *gs = static_cast<gridSecondary *> (m_sourceObject);
          out = gs->getOutputs (inputs, sD, sMode);
          gs->outputPartialDerivatives (inputs, sD, d, sMode);
          gs->ioPartialDerivatives (inputs, sD, d, inputLocs, sMode);
        }
      else
        {
          gridLink *lnk = static_cast<gridLink *> (m_sourceObject);
          int bid = bus->getID ();
          lnk->updateLocalCache (noInputs,sD, sMode);
          out = lnk->getOutputs (bid, sD, sMode);
          lnk->outputPartialDerivatives (bid, sD, d, sMode);
          lnk->ioPartialDerivatives (bid, sD, d, inputLocs, sMode);
        }

      auto offset = offsets.getDiffOffset (sMode);

      double I = getConditionValue (0,sD,sMode);

      double V = bus->getVoltage (sD, sMode);

      double S = std::hypot (out[PoutLocation], out[QoutLocation]);
      double temp = 1.0 / (S * V);
      double dIdP = out[PoutLocation] * temp;
      double dIdQ = out[QoutLocation] * temp;

      d.scaleRow (PoutLocation, dIdP);
      d.scaleRow (QoutLocation, dIdQ);
      d.translateRow (PoutLocation, offset);
      d.translateRow (QoutLocation, offset);

      d.assignCheck (offset, Voffset, -S / (V * V));
      double dRdI;
      if (I > limit)
        {
          dRdI = pow ((recloserTap / (pow (I - limit, 1.5)) + minClearingTime), -2.0) * (1.5 * recloserTap / (pow (I - limit, 2.5)));
        }
      else
        {
          dRdI = -pow ((recloserTap / (pow (limit - I + 1e-8, 1.5)) + minClearingTime), -2.0) * (1.5 * recloserTap / (pow (limit - I + 1e-8, 2.5)));
        }

      d.scaleRow (offset, dRdI);

      ad.merge (d);


      ad.assign (offset, offset, -sD.cj);

    }
  else if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getDiffOffset (sMode);
      ad.assign (offset,offset,sD.cj);
    }
}

void breaker::setState (coreTime ttime, const double state[], const double /*dstate_dt*/[], const solverMode &sMode)
{
  if (useCTI)
    {
      auto offset = offsets.getDiffOffset (sMode);
      cTI = state[offset];
    }
  prevTime = ttime;
}

void breaker::residual (const IOdata & /*inputs*/, const stateData &sD, double resid[], const solverMode &sMode)
{
  if (useCTI)
    {
      auto offset = offsets.getDiffOffset (sMode);
      const double *dst = sD.dstate_dt + offset;

      if (!opFlags[nonlink_source_flag])
        {
          static_cast<gridLink *> (m_sourceObject)->updateLocalCache (noInputs, sD, sMode);
        }
      double I1 = getConditionValue (0,sD,sMode);
      double temp;
      if (I1 > limit)
        {
          temp = pow (I1 - limit, 1.5);
          resid[offset] = 1.0 / (recloserTap / temp + minClearingTime) - *dst;
          assert (!std::isnan (resid[offset]));
        }
      else
        {
          temp = pow (limit - I1 + 1e-8, 1.5);
          resid[offset] = -1.0 / (recloserTap / temp + minClearingTime) - *dst;
          assert (!std::isnan (resid[offset]));
        }


      // printf("tt=%f::I1=%f, r[%d]=%f, stv=%f\n", sD.time, I1, offset, 1.0 / (recloserTap / temp + minClearingTime),sD.state[offset]);
    }
  else if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getDiffOffset (sMode);
      resid[offset] = sD.dstate_dt[offset];
    }
}

void breaker::guess (const coreTime /*ttime*/, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (useCTI)
    {
      auto offset = offsets.getDiffOffset (sMode);
      double I1 = getConditionValue (0);
      state[offset] = cTI;
      double temp;
      if (I1 > limit)
        {
          temp = pow (I1 - limit, 1.5);
          dstate_dt[offset] = 1.0 / (recloserTap / temp + minClearingTime);
        }
      else
        {
          temp = pow (limit - I1 + 1e-8, 1.5);
          dstate_dt[offset] = -1.0 / (recloserTap / temp + minClearingTime);
        }

    }
  else if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getDiffOffset (sMode);
      state[offset] = 0;
      dstate_dt[offset] = 0;
    }
}

void breaker::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getDiffOffset (sMode);
      if (offset >= stNames.size ())
        {
          stNames.resize (offset + 1);
        }
      if (prefix.empty ())
        {
          stNames[offset] = getName() + ":trigger_proximity";
        }
      else
        {
          stNames[offset] = prefix + "::" + getName() + ":trigger_proximity";
        }
    }
}

void breaker::tripBreaker (coreTime time)
{
  alert (this, BREAKER_TRIP_CURRENT);
  LOG_NORMAL ("breaker " + std::to_string (m_terminal) + " tripped on " + m_sourceObject->getName ());
  triggerAction (0);
  opFlags.set (breaker_tripped_flag);
  useCTI = false;
  if (time > lastRecloseTime + recloserResetTime)
    {
      recloseAttempts = 0;
    }
  if ((recloseAttempts == 0) && (recloseAttempts < maxRecloseAttempts))
    {
      nextUpdateTime = time + recloseTime1;
      alert (this, UPDATE_TIME_CHANGE);
    }
  else if (recloseAttempts < maxRecloseAttempts)
    {
      nextUpdateTime = time + recloseTime2;
      alert (this, UPDATE_TIME_CHANGE);
    }
}


void breaker::resetBreaker (coreTime time)
{
  ++recloseAttempts;
  lastRecloseTime = time;
  alert (this, BREAKER_RECLOSE);
  LOG_NORMAL ("breaker " + std::to_string (m_terminal) + " reset on " + m_sourceObject->getName ());
  opFlags.reset (breaker_tripped_flag);
  //timestep (time, solverMode::pFlow);
  triggerAction (1); //reclose the breaker
  nextUpdateTime = maxTime;
  if (!opFlags[nonlink_source_flag])
    {//do a recompute power
      static_cast<gridLink *> (m_sourceObject)->updateLocalCache ();
    }
  if (checkCondition (0))
    {
      if (recloserTap <= kMin_Res)
        {
          if (minClearingTime <= kMin_Res)
            {
              tripBreaker (time);
            }
          else
            {
              nextUpdateTime = time + minClearingTime;
            }
        }
      else
        {
          cTI = 0;
          setConditionState (1, condition_states::active);
          setConditionState (2, condition_states::active);
          alert (this, JAC_COUNT_INCREASE);
          useCTI = true;
        }
    }
  else
    {
      opFlags.reset (overlimit_flag);
      setConditionState (0, condition_states::active);
      useCTI = false;
    }

  alert (this, UPDATE_TIME_CHANGE);
}
