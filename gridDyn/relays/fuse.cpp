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

#include "fuse.h"
#include "measurement/gridCondition.h"
#include "events/eventQueue.h"
#include "events/gridEvent.h"
#include "linkModels/gridLink.h"
#include "gridBus.h"
#include "core/coreObjectTemplates.h"
#include "utilities/matrixDataSparse.h"
#include "measurement/gridGrabbers.h"
#include "measurement/stateGrabber.h"
#include "measurement/grabberSet.h"
#include "core/coreExceptions.h"


#include <boost/format.hpp>
#include <cmath>
using namespace gridUnits;
fuse::fuse (const std::string&objName) : gridRelay (objName)
{
  opFlags.set (continuous_flag);
  opFlags.reset (no_dyn_states);
}

coreObject *fuse::clone (coreObject *obj) const
{
  fuse *nobj = cloneBase<fuse, gridRelay> (this, obj);
  if (!(nobj))
    {
      return obj;
    }


  nobj->limit = limit;
  nobj->mp_I2T = mp_I2T;
  nobj->minBlowTime = minBlowTime;
  nobj->Vbase = Vbase;
  nobj->m_terminal = m_terminal;
  return nobj;
}

void fuse::setFlag (const std::string &flag, bool val)
{

  if (flag[0] == '#')
    {

    }
  else
    {
      gridRelay::setFlag (flag, val);
    }

}

void fuse::set (const std::string &param,  const std::string &val)
{

  if (param == "#")
    {

    }
  else
    {
      gridRelay::set (param, val);
    }
}

void fuse::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "limit")
    {
      limit = unitConversion (val, unitType, puA, systemBasePower, Vbase);

    }
  else if (param == "i2t")
    {
      mp_I2T = unitConversion (val, unitType, puA, systemBasePower, Vbase);
    }
  else if (param == "terminal")
    {
      m_terminal = static_cast<int> (val);
    }
  else if (param == "minblowtime")
    {
      if (val > 0.001)
        {
          minBlowTime = val;
        }
      else
        {
          LOG_WARNING ("minimum blow time must be greater or equal to 0.001");
		  throw(invalidParameterValue());
        }
    }
  else
    {
      gridRelay::set (param, val, unitType);
    }

}

void fuse::dynObjectInitializeA (coreTime time0, unsigned long flags)
{
  auto ge = std::make_shared<gridEvent> ();

  if (dynamic_cast<gridLink *> (m_sourceObject))
    {
      add (std::shared_ptr<gridCondition>(make_condition ("current" + std::to_string (m_terminal), ">=", limit, m_sourceObject)));
      ge->setTarget (m_sinkObject, "switch" + std::to_string(m_terminal));
	  ge->setValue(1.0);
      bus = static_cast<gridLink *> (m_sourceObject)->getBus (m_terminal);

    }
  else
    {
      add (std::shared_ptr<gridCondition>(make_condition ("sqrt(p^2+q^2)/@bus:v", ">=", limit, m_sourceObject)));
      opFlags.set (nonlink_source_flag);
      ge->setTarget (m_sinkObject,"status");
	  ge->setValue(0.0);
      bus = static_cast<gridBus *> (m_sourceObject->find ("bus"));
    }

  add (std::move(ge));
  //now make the gridCondition for the I2T condition
  auto gc = std::make_unique<gridCondition> ();
  auto gc2 = std::make_unique<gridCondition> ();

  auto cg = std::make_unique<customGrabber> ();
  cg->setGrabberFunction ("I2T", [ this ](coreObject *){
    return cI2T;
  });

  auto cgst = std::make_unique<customStateGrabber> (this);
  cgst->setGrabberFunction ([ ](coreObject *obj, const stateData &sD,const solverMode &sMode) -> double {
    return sD.state[static_cast<fuse *>(obj)->offsets.getDiffOffset (sMode)];
  });

  //this one needs to be shared since I use it twice
  auto gset = std::make_shared<grabberSet>(std::move(cg), std::move(cgst));
  gc->setConditionLHS(gset);

  gc2->setConditionLHS(std::move(gset));
  gc->setConditionRHS(mp_I2T);
  gc2->setConditionRHS(-mp_I2T / 2.0);

  gc->setComparison (comparison_type::gt);
  gc2->setComparison (comparison_type::lt);

  add (std::shared_ptr<gridCondition>(std::move(gc)));
  add (std::shared_ptr<gridCondition>(std::move(gc2)));
  setConditionState (1,condition_states::disabled);
  setConditionState (2, condition_states::disabled);

  cI2T = 0;

  //add the event for setting up the fuse evaluation
  auto ge2 = std::make_unique<functionEventAdapter> ([this]() {
    return setupFuseEvaluation ();
  });
  add (std::shared_ptr<functionEventAdapter>(std::move(ge2)));
  if (mp_I2T <= 0.0)
    {
      setActionTrigger (0, 1, 0.0);
    }
  else
    {
      setActionTrigger (0, 1, minBlowTime);
    }

  //add the event for blowing the fuse after i2T is exceeded
  auto ge3 = std::make_unique<functionEventAdapter> ([this]() {
    return blowFuse ();
  });
  add (std::shared_ptr<functionEventAdapter>(std::move(ge3)));
  setActionTrigger (1, 2, 0.0);

  gridRelay::dynObjectInitializeA (time0, flags);
}


void fuse::conditionTriggered (index_t conditionNum, coreTime /*triggerTime*/)
{
  if (conditionNum == 2)
    {
      assert (opFlags[overlimit_flag]);

      setConditionState (1, condition_states::disabled);
      setConditionState (2, condition_states::disabled);
      setConditionState (0,condition_states::active);
      alert (this, JAC_COUNT_DECREASE);
      opFlags.reset (overlimit_flag);
      useI2T = false;
    }
}


change_code fuse::blowFuse ()
{
  opFlags.set (overlimit_flag);
  setConditionState (0, condition_states::disabled);
  setConditionState (1, condition_states::disabled);
  setConditionState (2, condition_states::disabled);
  alert (this, FUSE_BLOWN_CURRENT);
  LOG_NORMAL ("Fuse " + std::to_string (m_terminal) + " blown on object " + m_sourceObject->getName ());
  opFlags.set (fuse_blown_flag);
  change_code cchange = change_code::non_state_change;
  if (mp_I2T > 0.0)
    {
      alert (this, JAC_COUNT_DECREASE);
      cchange = change_code::jacobian_change;
    }
  return std::max (triggerAction (0), cchange);

}

change_code fuse::setupFuseEvaluation ()
{
  if (mp_I2T <= 0.0)
    {
      return blowFuse ();
    }
  else
    {
      opFlags.set (overlimit_flag);
      setConditionState (0, condition_states::disabled);
      double I = getConditionValue (0);
      cI2T = I2Tequation (I) * minBlowTime;
      if (cI2T > mp_I2T)
        {
          return blowFuse ();
        }
      else
        {
          setConditionState (1, condition_states::active);
          setConditionState (2, condition_states::active);
          alert (this, JAC_COUNT_INCREASE);
          useI2T = true;
          return change_code::jacobian_change;
        }

    }

}

void fuse::loadSizes (const solverMode &sMode, bool dynOnly)
{
  gridRelay::loadSizes (sMode,dynOnly);
  auto so = offsets.getOffsets (sMode);
  if ((!isAlgebraicOnly (sMode))&&(mp_I2T > 0.0))
    {
      so->total.diffSize = 1;
      so->total.jacSize = 12;
    }

}

void fuse::timestep (coreTime ttime, const IOdata & /*inputs*/, const solverMode &)
{

  if (limit < kBigNum / 2.0)
    {
      double val = getConditionValue (0);
      if (val > limit)
        {
          opFlags.set (fuse_blown_flag);
          disable ();
          alert (this, FUSE1_BLOWN_CURRENT);
        }
    }
  prevTime = ttime;
}

void fuse::converge (coreTime ttime, double state[], double dstate_dt[], const solverMode &sMode, converge_mode, double /*tol*/)
{
  guess (ttime, state, dstate_dt, sMode);
}

void fuse::jacobianElements (const IOdata &/*inputs*/, const stateData &sD, matrixData<double> &ad, const IOlocs &/*inputLocs*/, const solverMode &sMode)
{
	//TODO don't use matrixDataSparse here use a translation matrix
  if (useI2T)
    {
      matrixDataSparse<double> d;
      IOdata out;
      auto Voffset = bus->getOutputLoc(sMode,voltageInLocation);
      auto inputs = bus->getOutputs (noInputs,sD,sMode);
      auto inputLocs = bus->getOutputLocs (sMode);
      if (opFlags[nonlink_source_flag])
        {
          gridSecondary *gs = static_cast<gridSecondary *> (m_sourceObject);
          out = gs->getOutputs (inputs,sD,sMode);
          gs->outputPartialDerivatives (inputs,sD,d,sMode);
          gs->ioPartialDerivatives (inputs,sD,d,inputLocs,sMode);
        }
      else
        {
          gridLink *lnk = static_cast<gridLink *> (m_sourceObject);
          int bid = bus->getID ();
          lnk->updateLocalCache (noInputs, sD, sMode);
          out = lnk->getOutputs (bid, sD, sMode);
          lnk->outputPartialDerivatives (bid, sD, d, sMode);
          lnk->ioPartialDerivatives (bid,sD,d,inputLocs,sMode);
        }



      double I = getConditionValue (0,sD,sMode);

      double V = bus->getVoltage (sD,sMode);

      double S = std::hypot (out[PoutLocation], out[QoutLocation]);
      double temp = 1.0 / (S * V);
      double dIdP = out[PoutLocation] * temp;
      double dIdQ = out[QoutLocation] * temp;
      d.scaleRow (PoutLocation,dIdP);
      d.scaleRow (QoutLocation,dIdQ);

      auto offset = offsets.getDiffOffset (sMode);
      d.translateRow (PoutLocation,offset);
      d.translateRow (QoutLocation,offset);
      d.assignCheck (offset, Voffset, -S / (V * V));

      d.scaleRow (offset,2.0 * I);

      ad.merge (d);


      ad.assign (offset, offset, -sD.cj);

    }
  else if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getDiffOffset (sMode);
      ad.assign (offset,offset,-sD.cj);
    }

}

void fuse::setState (coreTime ttime, const double state[], const double /*dstate_dt*/[], const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getDiffOffset (sMode);
      cI2T = state[offset];
    }
  prevTime = ttime;
}

double fuse::I2Tequation (double current)
{
  return (current * current - limit * limit);
}

void fuse::residual (const IOdata & /*inputs*/, const stateData &sD, double resid[], const solverMode &sMode)
{
  if (useI2T)
    {
      auto offset = offsets.getDiffOffset (sMode);
      const double *dst = sD.dstate_dt + offset;

      if (!opFlags[nonlink_source_flag])
        {
          static_cast<gridLink *> (m_sourceObject)->updateLocalCache (noInputs,sD, sMode);
        }
      double I1 = getConditionValue (0,sD,sMode);
      resid[offset] = I2Tequation (I1) - *dst;
      printf ("tt=%f::I1=%f,limit=%f, r[%d]=%f deriv=%f\n", static_cast<double>(sD.time),I1, limit,offset, resid[offset],*dst);

    }
  else if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getDiffOffset (sMode);
      resid[offset] = -sD.dstate_dt[offset];
    }
}

void fuse::guess (const coreTime /*ttime*/, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (useI2T)
    {
      auto offset = offsets.getDiffOffset (sMode);

      double I1 = getConditionValue (0);
      state[offset] = cI2T;
      dstate_dt[offset] = I2Tequation (I1);
    }
  else if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getDiffOffset (sMode);
      state[offset] = 0;
      dstate_dt[offset] = 0;
    }
}

void fuse::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getDiffOffset (sMode);
      if (offset >= (stNames.size ()))
        {
          stNames.resize (offset + 1);
        }
      if (prefix.empty ())
        {
          stNames[offset] = getName() + ":I2T";
        }
      else
        {
          stNames[offset] = prefix + "::" + getName() + ":I2T";
        }
    }
}
