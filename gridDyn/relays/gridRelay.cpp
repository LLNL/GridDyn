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

#include "gridRelay.h"
#include "zonalRelay.h"
#include "busRelay.h"
#include "loadRelay.h"
#include "fuse.h"
#include "breaker.h"
#include "sensor.h"
#include "differentialRelay.h"
#include "controlRelay.h"
#include "objectFactoryTemplates.h"
#include "gridCondition.h"
#include "stateGrabber.h"
#include "eventAdapters.h"
#include "gridEvent.h"
#include "comms/gridCommunicator.h"
#include "comms/relayMessage.h"
#include "gridCoreTemplates.h"
#include "stringOps.h"
#include "core/propertyBuffer.h"
#include "core/gridDynExceptions.h"

#include <boost/format.hpp>

using namespace gridUnits;

static typeFactory<gridRelay> gbf ("relay", stringVec { "basic" }, "basic");
static typeFactory<zonalRelay> zr ("relay", stringVec { "zonal", "z", "impedance", "distance" });
static typeFactory<differentialRelay> dr ("relay", stringVec { "differential", "diff" });

static typeFactory<busRelay> br ("relay", "bus");
static typeFactory<loadRelay> lr ("relay", "load");
static typeFactory<fuse> fr ("relay", "fuse");
static typeFactory<breaker> brkr ("relay","breaker");
static typeFactory<sensor> snsr ("relay", "sensor");
static typeFactory<controlRelay> cntrl ("relay", "control");

std::atomic<count_t> gridRelay::relayCount(0);

gridRelay::gridRelay (const std::string &objName) : gridPrimary (objName)
{
  // default values
  id = ++relayCount;
  updateName();
  opFlags.set (no_pflow_states);
  opFlags.set (no_dyn_states);
}

gridCoreObject * gridRelay::clone (gridCoreObject *obj) const
{
  gridRelay *nobj = cloneBase<gridRelay, gridPrimary> (this, obj);
  if (!(nobj))
    {
      return obj;
    }

 
  //clone the conditions
  for (index_t kk = 0; kk <conditions.size(); ++kk)
  {
	  if (nobj->conditions.size() <= kk)
	  {
		  //the other things which depend on this are being duplicated later so just use push_back
		  nobj->conditions.push_back(conditions[kk]->clone());
	  }
	  else
	  {
		  conditions[kk]->clone(nobj->conditions[kk]);
	  }
  }
  //clone the actions
  for (index_t kk = 0; kk < actions.size(); ++kk)
  {
	  if (nobj->actions.size() <= kk)
	  {
		  //the other things which depend on this are being duplicated later so just use push_back
		  nobj->actions.push_back(actions[kk]->clone());
	  }
	  else
	  {
		  actions[kk]->clone(nobj->actions[kk]);
	  }
  }
  //clone everything else
  nobj->triggerTime = triggerTime;
  nobj->actionTriggers = actionTriggers;
  nobj->actionDelays = actionDelays;
  nobj->cStates = cStates;
  nobj->conditionTriggerTimes = conditionTriggerTimes;
  nobj->condChecks = condChecks;
  nobj->multiConditionTriggers = multiConditionTriggers;

  nobj->cManager = cManager;  //TODO:: this isn't correct yet
 
  return nobj;
}

gridRelay::~gridRelay ()
{
	
}

void gridRelay::add (gridCoreObject *obj)
{
  m_sourceObject = obj;
  m_sinkObject = obj;
}

void gridRelay::add (std::shared_ptr<gridCondition> gc)
{
  conditions.push_back (gc);
  actionTriggers.resize (conditions.size ());
  actionDelays.resize (conditions.size ()); //!<the periods of time in which the condition must be true for an action to occur
  cStates.resize (conditions.size (),condition_states::active); //!< a vector of states for the conditions
  conditionTriggerTimes.resize (conditions.size ()); //!< the times at which the condition triggered
  multiConditionTriggers.resize (conditions.size ());
}

void gridRelay::add (std::shared_ptr<gridEvent> ge)
{
  actions.push_back (std::make_shared<eventTypeAdapter<std::shared_ptr<gridEvent>>> (ge));
}
/**
*add an EventAdapter to the system
**/
void gridRelay::add (std::shared_ptr<eventAdapter> geA)
{
  actions.push_back (geA);
}


void gridRelay::setSource (gridCoreObject *obj)
{
  m_sourceObject = obj;
}
/**
* set the relay sink object
*/
void gridRelay::setSink (gridCoreObject *obj)
{
  m_sinkObject = obj;
}

void gridRelay::setActionTrigger (index_t conditionNumber, index_t actionNumber, double delayTime)
{
  if (conditionNumber >= conditions.size ())
    {
      return;
    }
  if (actionNumber >= actions.size ())
    {
      return;
    }
  //search for an existing entry
  for (size_t pp = 0; pp < actionTriggers[conditionNumber].size (); ++pp)
    {
      if (actionTriggers[conditionNumber][pp] == actionNumber)
        {
          actionDelays[conditionNumber][pp] = delayTime;
          return;
        }
    }
  //if no existing entry add a new one
  actionTriggers[conditionNumber].push_back (actionNumber);
  actionDelays[conditionNumber].push_back (delayTime);

}

void gridRelay::setActionMultiTrigger (std::vector<index_t> multi_conditions, index_t actionNumber, double delayTime)
{
  if (actionNumber >= actions.size ())
    {
      return;
    }
  for (auto &cnum : multi_conditions)
    {
      multiConditionTriggers[cnum].emplace_back (actionNumber,multi_conditions,delayTime);
    }
}

void gridRelay::setResetMargin (index_t conditionNumber, double margin)
{
  if (conditionNumber >= conditions.size ())
    {
      return;
    }
  conditions[conditionNumber]->setMargin (margin);
}

void gridRelay::setConditionState (index_t conditionNumber, condition_states newState)
{
  if (conditionNumber >= conditions.size ())
    {
      return;
    }
  cStates[conditionNumber] = newState;
  if (newState == condition_states::disabled)
    {
      clearCondChecks (conditionNumber);
    }
  else if (newState == condition_states::active)
    {
      conditions[conditionNumber]->useMargin (false);
    }
  else if (newState == condition_states::triggered)
    {
      conditions[conditionNumber]->useMargin (true);
    }
  updateRootCount (true);
}

double gridRelay::getConditionValue (index_t conditionNumber) const
{
  if (conditionNumber >= conditions.size ())
    {
      return kNullVal;
    }
  return conditions[conditionNumber]->getVal (1);
}

double gridRelay::getConditionValue (index_t conditionNumber, const stateData *sD, const solverMode &sMode) const
{
  if (conditionNumber >= conditions.size ())
    {
      return kNullVal;
    }
  return conditions[conditionNumber]->getVal (1,sD,sMode);
}

bool gridRelay::checkCondition (index_t conditionNumber) const
{
  if (conditionNumber >= conditions.size ())
    {
      return false;
    }
  return conditions[conditionNumber]->checkCondition ();
}

void gridRelay::setConditionLevel (index_t conditionNumber, double levelVal)
{
  if (conditionNumber < conditions.size ())
    {
      conditions[conditionNumber]->setConditionRHS (levelVal);
    }
}

gridRelay::condition_states gridRelay::getConditionStatus (index_t conditionNumber)
{
  if (conditionNumber < conditions.size ())
    {
      return cStates[conditionNumber];
    }
  else
    {
      return condition_states::disabled;
    }
}

void gridRelay::removeAction (index_t actionNumber)
{
  for (size_t kk = 0; kk < actionTriggers.size (); ++kk)
    {
      for (size_t pp = 0; pp < actionTriggers[kk].size (); ++pp)
        {
          if (actionTriggers[kk][pp] == actionNumber)
            {
              //set all the existing delay time for this action to a very large number
              actionDelays[kk][pp] = kBigNum;
            }
        }
    }
  //now check the multiCondition triggers
  for (auto &mcond:multiConditionTriggers)
    {
      for (auto &mcd:mcond)
        {
          if (mcd.actionNum == actionNumber)
            {
              //set all the existing delay time for this action to a very large number
              mcd.delayTime = kBigNum;
            }
        }
    }
}


std::shared_ptr<gridCondition> gridRelay::getCondition (index_t conditionNumber)
{
  if (conditionNumber < conditions.size ())
    {
      return conditions[conditionNumber];
    }
    return nullptr;
}

std::shared_ptr<eventAdapter> gridRelay::getAction (index_t actionNumber)
{
  if (actionNumber < actions.size ())
    {
      return actions[actionNumber];
    }
     return nullptr;
}



int gridRelay::updateAction (std::shared_ptr<gridEvent> ge, index_t actionNumber)
{
  if (actionNumber >= actions.size ())
    {
      return NOT_LOADED;
    }
  actions[actionNumber] = std::make_shared<eventTypeAdapter<std::shared_ptr<gridEvent>>> (ge);
  return LOADED;
}

int gridRelay::updateAction (std::shared_ptr<eventAdapter> geA, index_t actionNumber)
{
  if (actionNumber >= actions.size ())
    {
      return NOT_LOADED;
    }
  actions[actionNumber] = geA;
  return LOADED;
}

int gridRelay::updateCondition (std::shared_ptr<gridCondition> gc, index_t conditionNumber)
{
  if (conditionNumber >= conditions.size ())
    {
      return NOT_LOADED;
    }
  conditions[conditionNumber] = gc;
  cStates[conditionNumber] = condition_states::active;
  conditionTriggerTimes[conditionNumber] = -kBigNum;
  updateRootCount (true);
  return LOADED;
}

void gridRelay::setTime (double time)
{
  prevTime = time;
}
void gridRelay::resetRelay ()
{
}

void gridRelay::set (const std::string &param,  const std::string &val)
{
  if (param == "condition")
    {
        add (make_condition (val, m_sourceObject?m_sourceObject:parent));
    }
  else if (param == "action")
    {
      bool isAlarm = false;
      if ((val[0] == 'a') || (val[0] == 'A'))
        {
          auto e = make_alarm (val);
          if (e)
            {
              isAlarm = true;
              add (e);
            }

        }
      if (!isAlarm)
        {
            add (make_event (val, m_sinkObject?m_sinkObject:parent));
        }

    }
  else if (param == "flags")
    {
      auto flagid = splitlineTrim (val);
      for (auto &flg : flagid)
        {
          setFlag (flg, true);
        }
    }
  else
    {
	  if (cManager.set(param, val))
	  {
		  opFlags.set(use_commLink);
	  }
	  else
	  {
		  gridPrimary::set(param, val);
	  }
    }
}

void gridRelay::set (const std::string &param, double val, units_t unitType)
{

  if ((param == "samplingrate")||(param == "ts"))
    {
      gridCoreObject::set ("period", val, unitType);
    }
  else
    {
	  if (cManager.set(param, val))
	  {
		  opFlags.set(use_commLink);
	  }
	  else
	  {
		  gridPrimary::set(param, val, unitType);
	  }
    }
}

void gridRelay::setFlag (const std::string &flag, bool val)
{
  if (flag == "continuous")
    {
      opFlags.set (continuous_flag, val);
    }
  else if (flag == "sampled")
    {
      opFlags.set (continuous_flag, !val);
    }
  else if ((flag == "comm_enabled") || (flag == "comms") || (flag == "usecomms"))
    {
      opFlags.set (use_commLink,val);
    }
  else if (flag == "resettable")
    {
      opFlags.set (resettable_flag, val);
    }
  else if (flag == "powerflow_check")
    {
      opFlags.set (power_flow_checks_flag, val);
    }
  else
    {
	  if (cManager.setFlag(flag, val))
	  {
		  opFlags.set(use_commLink);
	  }
	  else
	  {
		  gridPrimary::setFlag(flag, val);
	  }
      
    }
}

void gridRelay::updateA (double time)
{
  auto ncond = condChecks;
  condChecks.clear ();
  nextUpdateTime = kBigNum;
  if (opFlags[continuous_flag])
    {
      for (auto &cond : ncond)
        {
          evaluateCondCheck (cond,time);
        }
      for (auto &cond : condChecks)
        {
          if (cond.testTime < nextUpdateTime)
            {
              nextUpdateTime = cond.testTime;
            }
        }

      for (index_t kk = 0; kk < conditions.size (); ++kk)
        {
          if (cStates[kk] == condition_states::active)
            {
              if (conditions[kk]->checkCondition ())
                {
                  triggerCondition (kk, time, 0.0);

                }
            }
        }
    }
  else
    {
      for (auto &cond : ncond)
        {
          evaluateCondCheck (cond, time);
        }

      for (auto &cond : condChecks)
        {
          if (cond.testTime < nextUpdateTime)
            {
              nextUpdateTime = cond.testTime;
            }
        }

      if (time + kMin_Res >= m_nextSampleTime)
        {
          for (index_t kk = 0; kk < conditions.size (); ++kk)
            {
              if (cStates[kk] == condition_states::active)
                {
                  if (conditions[kk]->checkCondition ())
                    {
                      triggerCondition (kk, time, 0.0);
                    }
                }
            }
          m_nextSampleTime += updatePeriod;
          nextUpdateTime = std::min (nextUpdateTime, m_nextSampleTime);
        }

    }
  lastUpdateTime = time;
}


void gridRelay::pFlowObjectInitializeA (double time0, unsigned long /*flags*/)
{
  if ((opFlags[use_commLink]) && (!(commLink)))
    {
	  commLink = cManager.build();
	  
      if (commLink)
        {
		  commLink->initialize();
          commLink->registerReceiveCallback ([this](std::uint64_t sourceID, std::shared_ptr<commMessage> message) {
        receiveMessage (sourceID, message);
      });
        }
      else
        {
          LOG_WARNING ("unrecognized commLink type ");
          opFlags.reset (use_commLink);
        }
    }
  if (opFlags[power_flow_checks_flag])
    {
      for (auto &cs : cStates)
        {
          if (cs == condition_states::active)
            {
              opFlags.set (has_powerflow_adjustments);
			  break;
            }
        }
    }
  prevTime = time0;
}


void gridRelay::dynObjectInitializeA (double time0, unsigned long /*flags*/)
{
  if (opFlags[continuous_flag])
    {
      updateRootCount (false);
    }
  else
    {
      if (updatePeriod > kHalfBigNum)
        {        //set the period to the period of the simulation
          updatePeriod = parent->find ("root")->get ("steptime");
          if (updatePeriod < 0)
            {
              updatePeriod = 1.0;
            }
        }
      m_nextSampleTime = nextUpdateTime = time0 + updatePeriod;
    }

  //*update the flag for future power flow check  BUG noticed by Colin Ponce 10/21/16
  if (opFlags[power_flow_checks_flag])
  {
	  for (auto &cs : cStates)
	  {
		  if (cs == condition_states::active)
		  {
			  opFlags.set(has_powerflow_adjustments);
			  break;
		  }
	  }
  }
}

change_code gridRelay::triggerAction (index_t actionNumber)
{
  if (actionNumber < actions.size ())
    {
      return executeAction (actionNumber, kNullLocation, prevTime);
    }
  return change_code::not_triggered;
}

void gridRelay::updateRootCount (bool alertChange)
{
  auto prevRoots = offsets.local->local.algRoots;
  offsets.local->local.algRoots = 0;
  conditionsWithRoots.clear ();
  for (index_t kk = 0; kk < cStates.size (); ++kk)
    {
      if (cStates[kk] == condition_states::active)
        {
          ++offsets.local->local.algRoots;
          conditionsWithRoots.push_back (kk);
        }
      else if ((cStates[kk] == condition_states::triggered) && (opFlags[resettable_flag]))
        {
          ++offsets.local->local.algRoots;
          conditionsWithRoots.push_back (kk);
        }
    }
  if (prevRoots != offsets.local->local.algRoots)
    {
      if (offsets.local->local.algRoots > 0)
        {
          opFlags.set (has_alg_roots);
          opFlags.set (has_roots);
        }
      else
        {
          opFlags.reset (has_alg_roots);
          opFlags.reset (has_roots);
        }
      if (alertChange)
        {
          alert (this, ROOT_COUNT_CHANGE);
        }
    }

}

change_code gridRelay::powerFlowAdjust (unsigned long /*flags*/, check_level_t level)
{
  change_code ret = change_code::no_change;
  if (level >= check_level_t::full_check)
    {
      for (index_t kk = 0; kk < conditions.size (); ++kk)
        {
          if (cStates[kk] == condition_states::active)
            {
              if (conditions[kk]->checkCondition ())
                {
                  triggerCondition (kk, prevTime, kBigNum);
                }
            }
        }
    }
  return ret;
}


void gridRelay::rootTest (const stateData *sD, double roots[], const solverMode &sMode)
{
  auto ro = offsets.getRootOffset (sMode);
  for (auto condNum : conditionsWithRoots)
    {
      roots[ro] = conditions[condNum]->evalCondition (sD, sMode);
      ++ro;
    }
}

void gridRelay::rootTrigger (double ttime, const std::vector<int> &rootMask, const solverMode &sMode)
{
  auto ro = offsets.getRootOffset (sMode);
  //Because conditionsWithRoots can change on a condition Trigger leading to an actionTaken
  //so we need to cache the conditions first to prevent manipulation
  auto checkConditions = conditionsWithRoots;
  for (auto conditionToCheck:checkConditions)
    {
      if (cStates[conditionToCheck] == condition_states::active)
        {
          if (rootMask[ro])
            {
              triggerCondition (conditionToCheck, ttime,0.0);
            }
          ++ro;
        }
      else if ((cStates[conditionToCheck] == condition_states::triggered) && (opFlags[resettable_flag]))
        {

          if (rootMask[ro])
            {

              cStates[conditionToCheck] = condition_states::active;
              conditions[conditionToCheck]->useMargin (false);
              clearCondChecks (conditionToCheck);
              conditionCleared (conditionToCheck, ttime);
            }
          ++ro;
        }
    }
  updateRootCount (true);

}

change_code gridRelay::rootCheck (const stateData *sD, const solverMode &, check_level_t /*level*/)
{
  count_t prevTrig = triggerCount;
  count_t prevAct = actionsTakenCount;
  double ctime = (sD) ? (sD->time) : prevTime;
  updateA (ctime);
  if ((triggerCount > prevTrig) || (actionsTakenCount > prevAct))
    {
      alert (this, UPDATE_TIME_CHANGE);
      updateRootCount (true);
      return change_code::non_state_change;
    }
  else
    {
      return change_code::no_change;
    }
}

void gridRelay::clearCondChecks (index_t condNum)
{
  auto cc = condChecks;
  condChecks.resize (0);
  double mTime = nextUpdateTime;
  for (auto &cond : cc)
    {
      if (cond.conditionNum != condNum)
        {
          condChecks.push_back (cond);
          if (cond.testTime < mTime)
            {
              mTime = cond.testTime;
            }
        }
    }
  if (mTime != nextUpdateTime)
    {
      nextUpdateTime = mTime;
      alert (this, UPDATE_TIME_CHANGE);
    }

}

std::shared_ptr<eventAdapter> gridRelay::make_alarm (const std::string &val)
{
  auto lc = convertToLowerCase (val);
  if (lc.compare (0, 5, "alarm") == 0)
    {
      auto codeStr = lc.substr (6);
      std::uint32_t code = intRead (codeStr, std::uint32_t (-1));
      if (code == std::uint32_t (-1))
        {
          code = getAlarmCode (codeStr);
        }
      return std::make_shared<functionEventAdapter> ([ = ]() {
      return (sendAlarm (code) == FUNCTION_EXECUTION_SUCCESS) ? change_code::no_change : change_code::execution_failure;
    });
    }
  else
    {
      return nullptr;
    }
}

void gridRelay::receiveMessage (std::uint64_t /*sourceID*/, std::shared_ptr<commMessage> /*message*/)
{

}


int gridRelay::sendAlarm (std::uint32_t code)
{
  if (commLink)
    {
      auto m = std::make_shared<relayMessage> (relayMessage::ALARM_TRIGGER_EVENT, code);
	  cManager.send(m);
      
      return FUNCTION_EXECUTION_SUCCESS;
    }
  return FUNCTION_EXECUTION_FAILURE;
}

change_code gridRelay::triggerCondition (index_t conditionNum, double conditionTriggerTime,double ignoreDelayTime)
{
  change_code eventReturn = change_code::no_change;
  cStates[conditionNum] = condition_states::triggered;
  conditions[conditionNum]->useMargin (true);

  conditionTriggerTimes[conditionNum] = conditionTriggerTime;
  ++triggerCount;
  conditionTriggered (conditionNum, conditionTriggerTime);
  for (index_t mm = 0; mm < actionTriggers[conditionNum].size (); ++mm)
    {
      if (actionDelays[conditionNum][mm] <= ignoreDelayTime)
        {
          auto iret = executeAction (actionTriggers[conditionNum][mm], conditionNum, conditionTriggerTime);
          if (iret > eventReturn)
            {
              eventReturn = iret;
            }
        }
      else
        {
          condChecks.push_back ({ conditionNum, mm, conditionTriggerTime + actionDelays[conditionNum][mm] });
          if (opFlags[has_updates])
            {
              nextUpdateTime = std::min (nextUpdateTime, conditionTriggerTime + actionDelays[conditionNum][mm]);
              alert (this, UPDATE_TIME_CHANGE);
            }
          else
            {
              nextUpdateTime = conditionTriggerTime + actionDelays[conditionNum][mm];
              opFlags.set (has_updates);
              alert (this, UPDATE_REQUIRED);

            }
        }
    }
  auto iret = multiConditionCheckExecute (conditionNum, conditionTriggerTime, ignoreDelayTime);
  if (iret > eventReturn)
    {
      eventReturn = iret;
    }
  return eventReturn;
}


change_code gridRelay::executeAction (index_t actionNum, index_t conditionNum, double actionTime)
{
  auto eventReturn = actions[actionNum]->execute (actionTime);
  ++actionsTakenCount;
  actionTaken (actionNum, conditionNum, eventReturn, actionTime);
  return eventReturn;
}

change_code gridRelay::multiConditionCheckExecute (index_t conditionNum, double conditionTriggerTime, double ignoreDelayTime)
{
  change_code eventReturn = change_code::no_change;
  //now check the multiCondition triggers
  for (auto &mct : multiConditionTriggers[conditionNum])
    {
      bool all_triggered = false;
      for (auto &cn : mct.multiConditions)
        {
          if (cStates[cn] != condition_states::triggered)
            {
              all_triggered = false;
              break;
            }
        }
      if (all_triggered)
        {
          if (mct.delayTime <= ignoreDelayTime)
            {
              auto iret = executeAction (mct.actionNum, conditionNum, prevTime);
              if (iret > eventReturn)
                {
                  eventReturn = iret;
                }
            }
          else
            {
              condChecks.push_back ({ conditionNum, mct.actionNum, conditionTriggerTime + mct.delayTime, true });
              nextUpdateTime = std::min (nextUpdateTime, conditionTriggerTime + mct.delayTime);
              alert (this, UPDATE_TIME_CHANGE);
            }
        }
    }
  return eventReturn;
}


change_code gridRelay::evaluateCondCheck (condCheckTime &cond, double checkTime)
{
  change_code eventReturn = change_code::no_change;
  if (checkTime + kSmallTime >= cond.testTime)
    {
      if (conditions[cond.conditionNum]->checkCondition ())
        {
          if (!cond.multiCondition)
            {
              auto iret = executeAction (cond.actionNum, cond.conditionNum, checkTime);
              if (iret > eventReturn)
                {
                  eventReturn = iret;
                }
            }
          else               //it was a multiCondition trigger
            {
              bool all_triggered = true;
              double trigDelay = multiConditionTriggers[cond.conditionNum][cond.actionNum].delayTime;
              for (auto &cnum : multiConditionTriggers[cond.conditionNum][cond.actionNum].multiConditions)
                {
                  if (cStates[cnum] != condition_states::triggered)
                    {
                      all_triggered = false;
                      break;
                    }
                  else if (checkTime + kSmallTime - conditionTriggerTimes[cnum] < trigDelay)
                    {
                      cond.testTime = conditionTriggerTimes[cnum] + trigDelay;
                      condChecks.push_back (cond);
                      all_triggered = false;
                      break;
                    }
                }
              if (all_triggered)
                {
                  auto iret = executeAction (multiConditionTriggers[cond.conditionNum][cond.actionNum].actionNum, cond.conditionNum, checkTime);
                  if (iret > eventReturn)
                    {
                      eventReturn = iret;
                    }

                }

            }
        }
      else
        {
          cStates[cond.conditionNum] = condition_states::active;
          conditions[cond.conditionNum]->useMargin (false);

          conditionCleared (cond.conditionNum, checkTime);
          updateRootCount (true);
        }
    }
  else
    {
      if (cStates[cond.conditionNum] == condition_states::triggered)
        {
          condChecks.push_back (cond);
        }
    }
  return eventReturn;
}

#ifdef DEBUG_LOG_ENABLE
void gridRelay::actionTaken ( index_t ActionNum, index_t conditionNum, change_code actionReturn, double /*actionTime*/)
{
  LOG_DEBUG ((boost::format ("action %d taken based on condition %d  with return code %d") % ActionNum % conditionNum  % static_cast<int> (actionReturn)).str ());

}
void gridRelay::conditionTriggered (index_t conditionNum, double timeTriggered)
{
  if (conditionTriggerTimes[conditionNum] > 0)
    {
      LOG_DEBUG ((boost::format ("condition %d triggered again at %f") % conditionNum % timeTriggered).str ())
    }
  else
    {
      LOG_DEBUG ((boost::format ("condition %d triggered at %f") % conditionNum % timeTriggered).str ());
    }

}
void gridRelay::conditionCleared (index_t conditionNum, double timeTriggered)
{
  if (conditionTriggerTimes[conditionNum] > 0)
    {
      LOG_DEBUG ((boost::format ("condition %d cleared again at %f") % conditionNum % timeTriggered).str ());
    }
  else
    {
      LOG_DEBUG ((boost::format ("condition %d cleared at %f") % conditionNum % timeTriggered).str ());
    }

}

#else

void gridRelay::actionTaken (index_t /*ActionNum*/, index_t /*conditionNum*/, change_code /*actionReturn*/, double /*actionTime*/)
{

}
void gridRelay::conditionTriggered (index_t /*conditionNum*/, double /*timeTriggered*/)
{

}
void gridRelay::conditionCleared (index_t /*conditionNum*/, double /*timeTriggered*/)
{

}
#endif


void gridRelay::updateObject(gridCoreObject *obj, object_update_mode mode)
{
	if (mode == object_update_mode::direct)
	{
		setSource(obj);
	}
	else if (mode==object_update_mode::match)
	{
		if (m_sourceObject)
		{
			setSource(findMatchingObject(m_sourceObject, obj));
		}
		if (m_sinkObject)
		{
			setSink(findMatchingObject(m_sourceObject, obj));
		}
		for (auto &cond : conditions)
		{
			cond->updateObject(obj, mode);
		}
		for (auto &act : actions)
		{
			act->updateObject(obj, mode);
		}
	}
}

gridCoreObject * gridRelay::getObject() const
{
	if (m_sourceObject)
	{
		return m_sourceObject;
	}
	else if (m_sinkObject)
	{
		return m_sinkObject;
	}
	return nullptr;
}

void gridRelay::getObjects(std::vector<gridCoreObject *> &objects) const
{
	if (m_sourceObject)
	{
		objects.push_back(m_sourceObject);
	}
	if ((m_sinkObject)&&(m_sourceObject!=m_sinkObject))
	{
		objects.push_back(m_sinkObject);
	}
	for (auto &cond : conditions)
	{
		cond->getObjects(objects);
	}
	//for (auto &act : actions)
	//{
		//act->getObjects(objects);
	//}
}