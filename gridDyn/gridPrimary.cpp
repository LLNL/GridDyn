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

#include "gridObjects.h"
#include <cstdio>
#include <iostream>

#include <map>
#include <algorithm>

gridPrimary::gridPrimary (const std::string &objName) : gridObject (objName)
{
}


void gridPrimary::pFlowInitializeA (double time0, unsigned long flags)
{
  if (enabled)
    {
      pFlowObjectInitializeA (time0, flags);
      prevTime = time0;
      updateFlags (false);
      setupPFlowFlags ();
    }

}

void gridPrimary::pFlowInitializeB ()
{
  if (enabled)
    {
      pFlowObjectInitializeB ();
      opFlags.set (pFlow_initialized);
    }
}

void gridPrimary::dynInitializeA (double time0, unsigned long flags)
{
  if (enabled)
    {
      dynObjectInitializeA (time0, flags);
      prevTime = time0;
      updateFlags (true);
      setupDynFlags ();
    }
}

void gridPrimary::dynInitializeB (IOdata &outputSet)
{
  if (enabled)
    {
      dynObjectInitializeB (outputSet);
      if (updatePeriod < kHalfBigNum)
        {
          opFlags.set (has_updates);
          nextUpdateTime = prevTime + updatePeriod;
          alert (this, UPDATE_REQUIRED);
        }
      opFlags.set (dyn_initialized);
      updateLocalCache ();
    }
}


void gridPrimary::pFlowObjectInitializeA (double time0, unsigned long flags)
{
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->pFlowInitializeA (time0, flags);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->pFlowInitializeA (time0, flags);
            }
        }
    }

}

void gridPrimary::pFlowObjectInitializeB ()
{
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->pFlowInitializeB ();
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->pFlowInitializeB ();
            }
        }
    }
}

void gridPrimary::dynObjectInitializeA (double time0, unsigned long flags)
{
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->initializeA (time0, flags);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->dynInitializeA (time0, flags);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->dynInitializeA (time0, flags);
            }
        }
    }

}

void gridPrimary::dynObjectInitializeB (IOdata &inputSet)
{
  if (!subObjectList.empty ())
    {
      IOdata out;
      auto args = getOutputs (nullptr, cLocalSolverMode);
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->initializeB (args, out, inputSet);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->dynInitializeB (inputSet);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->dynInitializeB (args, inputSet);
            }
        }
    }
}


static const std::map<int, int> alertFlags {
  std::make_pair (FLAG_CHANGE, 1),
  std::make_pair (STATE_COUNT_INCREASE, 3),
  std::make_pair (STATE_COUNT_DECREASE, 3),
  std::make_pair (STATE_COUNT_CHANGE, 3),
  std::make_pair (ROOT_COUNT_INCREASE, 2),
  std::make_pair (ROOT_COUNT_DECREASE, 2),
  std::make_pair (ROOT_COUNT_CHANGE, 2),
  std::make_pair (JAC_COUNT_INCREASE, 2),
  std::make_pair (JAC_COUNT_DECREASE, 2),
  std::make_pair (JAC_COUNT_CHANGE, 2),
  std::make_pair (OBJECT_COUNT_INCREASE, 3),
  std::make_pair (OBJECT_COUNT_DECREASE, 3),
  std::make_pair (OBJECT_COUNT_CHANGE, 3),
  std::make_pair (CONSTRAINT_COUNT_DECREASE, 1),
  std::make_pair (CONSTRAINT_COUNT_INCREASE, 1),
  std::make_pair (CONSTRAINT_COUNT_CHANGE, 1),
};

void gridPrimary::alert (gridCoreObject *object, int code)
{
  if ((code >= MIN_CHANGE_ALERT)  && (code < MAX_CHANGE_ALERT))
    {
      int flagNum;
      auto res = alertFlags.find (code);
      if (res != alertFlags.end ())
        {
          flagNum = res->second;
          if (!opFlags[disable_flag_updates])
            {
              updateFlags ();
            }
          else
            {
              opFlags.set (flag_update_required);
            }
          if (flagNum == 3)
            {

              offsets.stateUnload ();
            }
          else if (flagNum == 2)
            {
              offsets.rjUnload (true);
            }

        }
    }
  if (parent)
    {
      parent->alert (object, code);
    }
}

//TODO:: PT make the rest of these functions to use the subObjectList and do the appropriate thing
void gridPrimary::preEx (const stateData *, const solverMode &)
{

}

void gridPrimary::converge (double /*ttime*/, double /*state*/[], double /*dstate_dt*/[], const solverMode &, converge_mode, double /*tol*/)
{

}

//Jacobian computation
void gridPrimary::jacobianElements (const stateData *sD, arrayData<double> *ad, const solverMode & sMode)
{
  if (!subObjectList.empty ())
    {
      auto args = getOutputs (sD, sMode);
      auto argLocs = getOutputLocs (sMode);

      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->jacobianElements (args, sD, ad,argLocs, sMode);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->jacobianElements ( sD, ad, sMode);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->jacobianElements (args, sD, ad, argLocs, sMode);
            }
        }
    }
}

void gridPrimary::outputPartialDerivatives (const stateData *, arrayData<double> *, const solverMode &)
{
  //there are no standard outputs for a primary object so this is just a stub to do nothing
}

//residual computation
void gridPrimary::residual (const stateData *sD, double resid[], const solverMode & sMode)
{
  if (!subObjectList.empty ())
    {
      auto args = getOutputs (sD, sMode);
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->residual (args,sD, resid, sMode);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->residual ( sD, resid, sMode);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->residual (args, sD, resid, sMode);
            }
        }
    }
}
void gridPrimary::derivative (const stateData *sD, double deriv[], const solverMode & sMode)
{
  if (!subObjectList.empty ())
    {
      auto args = getOutputs (sD, sMode);
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->derivative (args, sD, deriv, sMode);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->derivative (sD, deriv, sMode);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->derivative (args, sD, deriv, sMode);
            }
        }
    }
}

void gridPrimary::algebraicUpdate ( const stateData *sD, double update[], const solverMode & sMode, double alpha)
{
  if (!subObjectList.empty ())
    {
      auto args = getOutputs (sD, sMode);
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->algebraicUpdate (args, sD, update, sMode,alpha);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->algebraicUpdate (sD, update, sMode,alpha);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->algebraicUpdate (args, sD, update, sMode,alpha);
            }
        }
    }
}


void  gridPrimary::delayedResidual (const stateData *sD, double resid[], const solverMode &sMode)
{
  residual (sD, resid, sMode);
}


void  gridPrimary::delayedDerivative (const stateData *sD, double deriv[], const solverMode &sMode)
{
  derivative (sD, deriv, sMode);
}


void  gridPrimary::delayedAlgebraicUpdate (const stateData *sD, double update[], const solverMode &sMode, double alpha)
{
  algebraicUpdate (sD, update, sMode, alpha);
}


void  gridPrimary::delayedJacobian (const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{
  jacobianElements (sD, ad, sMode);
}


//for the stepwise dynamic system
double gridPrimary::timestep (double ttime, const solverMode & sMode)
{
  prevTime = ttime;
  if (!subObjectList.empty ())
    {
      auto args = getOutputs (nullptr, sMode);
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->timestep (ttime, args, sMode);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->timestep (ttime, sMode);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->timestep (ttime,args, sMode);
            }
        }
    }
  return getOutput (0);
}

void gridPrimary::setAll (const std::string & type, std::string param, double val, gridUnits::units_t unitType)
{
  if (type == "all")
    {
      for (auto &so : subObjectList)
        {
          so->set (param, val, unitType);
        }
    }
}

void gridPrimary::rootTest (const stateData *sD, double roots[], const solverMode & sMode)
{
  if (!subObjectList.empty ())
    {
      auto args = getOutputs (nullptr, sMode);
      for (auto &subobj : subObjectList)
        {
          if ((!subobj) || (!subobj->checkFlag (has_roots)))
            {
              continue;
            }

          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->rootTest (args, sD, roots, sMode);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->rootTest (sD, roots, sMode);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->rootTest (args, sD, roots, sMode);
            }
        }
    }
}

void gridPrimary::rootTrigger (double /*ttime*/, const std::vector<int> & /*rootMask*/, const solverMode & /*sMode*/)
{
}

change_code gridPrimary::rootCheck (const stateData *sD, const solverMode & sMode, check_level_t level)
{
  auto ret = change_code::no_change;
  if (!subObjectList.empty ())
    {
      auto args = getOutputs (sD, sMode);
      for (auto &subobj : subObjectList)
        {

          if ((!subobj) || (!subobj->checkFlag (has_roots)))
            {
              continue;
            }
          auto retval = change_code::no_change;
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              retval = static_cast<gridSubModel *> (subobj)->rootCheck (args, sD, sMode,level);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              retval = static_cast<gridPrimary *> (subobj)->rootCheck (sD, sMode,level);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              retval = static_cast<gridSecondary *> (subobj)->rootCheck (args, sD, sMode, level);
            }
          ret = (std::max) (ret, retval);
        }
    }

  return ret;
}

change_code gridPrimary::powerFlowAdjust (unsigned long /*flags*/, check_level_t /*level*/)
{
  return change_code::no_change;
}

void gridPrimary::reset (reset_levels /*level*/)
{
}

void gridPrimary::pFlowCheck (std::vector<violation> & /*Violation_vector*/)
{
}

void gridPrimary::updateLocalCache (const stateData *, const solverMode &)
{

}

void gridPrimary::updateLocalCache ()
{
}

static const IOdata nullVec (0);

//TODO:: PT really should do something more intelligent here
IOdata gridPrimary::getOutputs (const stateData *, const solverMode &)
{
  return nullVec;
}


IOlocs gridPrimary::getOutputLocs (const solverMode &) const
{
	return IOlocs(0);
}

index_t gridPrimary::getOutputLoc(const solverMode &, index_t /*num*/) const
{
	return kNullLocation;
}

double gridPrimary::getDoutdt(const stateData *, const solverMode &, index_t /*num*/) const
{
	return 0;
}
double gridPrimary::getOutput(const stateData *, const solverMode &, index_t /*num*/) const
{
	return 0;
}

double gridPrimary::getOutput(index_t /*num*/) const
{
	return 0;
}


gridBus * gridPrimary::getBus(index_t /*num*/) const
{
	return nullptr;
}

gridLink * gridPrimary::getLink(index_t /*num*/) const
{
	return nullptr;
}

gridArea * gridPrimary::getArea(index_t /*num*/) const
{
	return nullptr;
}

gridRelay * gridPrimary::getRelay(index_t /*num*/) const
{
	return nullptr;
}