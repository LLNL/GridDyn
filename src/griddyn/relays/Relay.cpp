/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "../comms/Communicator.h"
#include "../comms/relayMessage.h"
#include "../events/Event.h"
#include "../events/eventAdapters.h"
#include "../measurement/Condition.h"
#include "../measurement/gridGrabbers.h"
#include "../measurement/objectGrabbers.h"
#include "../measurement/stateGrabber.h"
#include "breaker.h"
#include "busRelay.h"
#include "controlRelay.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "core/objectFactoryTemplates.hpp"
#include "core/propertyBuffer.h"
#include "differentialRelay.h"
#include "fuse.h"
#include "loadRelay.h"
#include "pmu.h"
#include "utilities/stringConversion.h"
#include "zonalRelay.h"
#include <stdexcept>
#include <boost/format.hpp>

namespace griddyn
{
using namespace gridUnits;

static typeFactory<Relay> gbf ("relay", stringVec{"basic"}, "basic");
static typeFactory<sensor> snsr ("relay", "sensor");
namespace relays
{
static typeFactory<zonalRelay> zr ("relay", stringVec{"zonal", "z", "impedance", "distance"});
static typeFactory<differentialRelay> dr ("relay", stringVec{"differential", "diff"});

static typeFactory<busRelay> br ("relay", "bus");
static typeFactory<loadRelay> lr ("relay", "load");
static typeFactory<fuse> fr ("relay", "fuse");
static typeFactory<breaker> brkr ("relay", "breaker");
static childTypeFactory<pmu, sensor> pmur ("relay", stringVec{"pmu", "phasor", "PMU", "synchrophasor"});
static typeFactory<controlRelay> cntrl ("relay", "control");
}  // namespace relays

std::atomic<count_t> Relay::relayCount (0);

Relay::Relay (const std::string &objName) : gridPrimary (objName)
{
    // default values
    setUserID (++relayCount);
    updateName ();
}

coreObject *Relay::clone (coreObject *obj) const
{
    auto nobj = cloneBase<Relay, gridPrimary> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }

    // clone the conditions
    for (index_t kk = 0; kk < static_cast<index_t> (conditions.size ()); ++kk)
    {
        if (static_cast<index_t> (nobj->conditions.size ()) <= kk)
        {
            // the other things which depend on this are being duplicated later so just use push_back
            nobj->conditions.emplace_back (conditions[kk]->clone ());
        }
        else
        {
            conditions[kk]->cloneTo (nobj->conditions[kk].get ());
        }
    }
    // clone the actions
    for (index_t kk = 0; kk < static_cast<index_t> (actions.size ()); ++kk)
    {
        if (static_cast<index_t> (nobj->actions.size ()) <= kk)
        {
            // the other things which depend on this are being duplicated later so just use push_back
            nobj->actions.emplace_back (actions[kk]->clone ());
        }
        else
        {
            actions[kk]->cloneTo (nobj->actions[kk].get ());
        }
    }
    // clone everything else
    nobj->triggerTime = triggerTime;
    nobj->actionTriggers = actionTriggers;
    nobj->actionDelays = actionDelays;
    nobj->cStates = cStates;
    nobj->conditionTriggerTimes = conditionTriggerTimes;
    nobj->condChecks = condChecks;
    nobj->multiConditionTriggers = multiConditionTriggers;

    nobj->cManager = cManager;
    if (nobj->m_sourceObject == nullptr)
    {
        nobj->m_sourceObject = m_sourceObject;
    }
    if (nobj->m_sinkObject == nullptr)
    {
        nobj->m_sinkObject = m_sinkObject;
    }
    return nobj;
}

void Relay::updateObjectLinkages (coreObject *newRoot)
{
    updateObject (newRoot, object_update_mode::match);
    gridComponent::updateObjectLinkages (newRoot);
}

void Relay::add (coreObject *obj)
{
    m_sourceObject = obj;
    m_sinkObject = obj;
}

void Relay::add (std::shared_ptr<Condition> gc)
{
    conditions.push_back (std::move (gc));
    actionTriggers.resize (conditions.size ());
    actionDelays.resize (
      conditions.size ());  //!< the periods of time in which the condition must be true for an action to occur
    cStates.resize (conditions.size (), condition_status_t::active);  //!< a vector of states for the conditions
    conditionTriggerTimes.resize (conditions.size ());  //!< the times at which the condition triggered
    multiConditionTriggers.resize (conditions.size ());
}

void Relay::add (std::shared_ptr<Event> ge)
{
    actions.emplace_back (std::make_shared<eventTypeAdapter<std::shared_ptr<Event>>> (std::move (ge)));
}
/**
 *add an EventAdapter to the system
 **/
void Relay::add (std::shared_ptr<eventAdapter> geA) { actions.emplace_back (std::move (geA)); }

void Relay::setSource (coreObject *obj) { m_sourceObject = obj; }
/**
 * set the relay sink object
 */
void Relay::setSink (coreObject *obj) { m_sinkObject = obj; }

void Relay::setActionTrigger (index_t actionNumber, index_t conditionNumber, coreTime delayTime)
{
    if (conditionNumber >= static_cast<index_t> (conditions.size ()))
    {
        LOG_WARNING ("attempted set of invalid conditonNumber");
        return;
    }
    if (actionNumber >= static_cast<index_t> (actions.size ()))
    {
        LOG_WARNING ("attempted set of invaid actionNumber");
        return;
    }
    // search for an existing entry
    for (index_t pp = 0; pp < static_cast<index_t> (actionTriggers[conditionNumber].size ()); ++pp)
    {
        if (actionTriggers[conditionNumber][pp] == actionNumber)
        {
            actionDelays[conditionNumber][pp] = delayTime;
            return;
        }
    }
    // if no existing entry add a new one
    actionTriggers[conditionNumber].push_back (actionNumber);
    actionDelays[conditionNumber].push_back (delayTime);
}

void Relay::setActionMultiTrigger (index_t actionNumber, const IOlocs &multi_conditions, coreTime delayTime)
{
    if (actionNumber >= static_cast<index_t> (actions.size ()))
    {
        return;
    }
    for (auto &cnum : multi_conditions)
    {
        multiConditionTriggers[cnum].emplace_back (actionNumber, multi_conditions, delayTime);
    }
}

void Relay::setResetMargin (index_t conditionNumber, double margin)
{
    if (conditionNumber >= static_cast<index_t> (conditions.size ()))
    {
        return;
    }
    conditions[conditionNumber]->setMargin (margin);
}

void Relay::setConditionStatus (index_t conditionNumber, condition_status_t newStatus)
{
    if (!isValidIndex (conditionNumber, conditions))
    {
        return;
    }
    cStates[conditionNumber] = newStatus;
    switch (newStatus)
    {
    case condition_status_t::disabled:
        clearCondChecks (conditionNumber);
        break;
    case condition_status_t::active:
        conditions[conditionNumber]->useMargin (false);
        break;
    case condition_status_t::triggered:
        conditions[conditionNumber]->useMargin (true);
        break;
    default:
        break;
    }
    updateRootCount (true);
}

double Relay::getConditionValue (index_t conditionNumber) const
{
    if (isValidIndex (conditionNumber, conditions))
    {
        return conditions[conditionNumber]->getVal (1);
    }

    return kNullVal;
}

double Relay::getConditionValue (index_t conditionNumber, const stateData &sD, const solverMode &sMode) const
{
    if (isValidIndex (conditionNumber, conditions))
    {
        return conditions[conditionNumber]->getVal (1, sD, sMode);
    }
    return kNullVal;
}

bool Relay::checkCondition (index_t conditionNumber) const
{
    if (isValidIndex (conditionNumber, conditions))
    {
        return conditions[conditionNumber]->checkCondition ();
    }
    return false;
}

void Relay::setConditionLevel (index_t conditionNumber, double levelVal)
{
    if (isValidIndex (conditionNumber, conditions))
    {
        conditions[conditionNumber]->setConditionRHS (levelVal);
    }
}

Relay::condition_status_t Relay::getConditionStatus (index_t conditionNumber)
{
    if (isValidIndex (conditionNumber, conditions))
    {
        return cStates[conditionNumber];
    }
    return condition_status_t::disabled;
}

void Relay::removeAction (index_t actionNumber)
{
    for (index_t kk = 0; kk < static_cast<index_t> (actionTriggers.size ()); ++kk)
    {
        for (index_t pp = 0; pp < static_cast<index_t> (actionTriggers[kk].size ()); ++pp)
        {
            if (actionTriggers[kk][pp] == actionNumber)
            {
                // set all the existing delay time for this action to a very large number
                actionDelays[kk][pp] = kBigNum;
            }
        }
    }
    // now check the multiCondition triggers
    for (auto &mcond : multiConditionTriggers)
    {
        for (auto &mcd : mcond)
        {
            if (mcd.actionNum == actionNumber)
            {
                // set all the existing delay time for this action to a very large number
                mcd.delayTime = maxTime;
            }
        }
    }
}

std::shared_ptr<Condition> Relay::getCondition (index_t conditionNumber)
{
    if (isValidIndex (conditionNumber, conditions))
    {
        return conditions[conditionNumber];
    }
    return nullptr;
}

std::shared_ptr<eventAdapter> Relay::getAction (index_t actionNumber)
{
    if (isValidIndex (actionNumber, actions))
    {
        return actions[actionNumber];
    }
    return nullptr;
}

void Relay::updateAction (std::shared_ptr<Event> ge, index_t actionNumber)
{
    if (isValidIndex (actionNumber, actions))
    {
        actions[actionNumber] = std::make_shared<eventTypeAdapter<std::shared_ptr<Event>>> (std::move (ge));
    }
    else
    {
        throw (invalidParameterValue ("actionNumber"));
    }
}

void Relay::updateAction (std::shared_ptr<eventAdapter> geA, index_t actionNumber)
{
    if (isValidIndex (actionNumber, actions))
    {
        actions[actionNumber] = std::move (geA);
    }
    else
    {
        throw (invalidParameterValue ("actionNumber"));
    }
}

void Relay::updateCondition (std::shared_ptr<Condition> gc, index_t conditionNumber)
{
    if (!isValidIndex (conditionNumber, conditions))
    {
        throw (invalidParameterValue ("conditionNumber"));
    }
    conditions[conditionNumber] = std::move (gc);
    cStates[conditionNumber] = condition_status_t::active;
    conditionTriggerTimes[conditionNumber] = negTime;
    updateRootCount (true);
}

void Relay::resetRelay () {}

void Relay::set (const std::string &param, const std::string &val)
{
    if (param == "condition")
    {
        add (std::shared_ptr<Condition> (
          make_condition (val, (m_sourceObject != nullptr) ? m_sourceObject : getParent ())));
    }
    else if (param == "action")
    {
        bool isAlarm = false;
        if ((val.front () == 'a') || (val.front () == 'A'))
        {
            auto e = make_alarm (val);
            if (e)
            {
                isAlarm = true;
                add (std::shared_ptr<eventAdapter> (std::move (e)));
            }
        }
        if (!isAlarm)
        {
            add (
              std::shared_ptr<Event> (make_event (val, (m_sinkObject != nullptr) ? m_sinkObject : getParent ())));
        }
    }
    else
    {
        if (cManager.set (param, val))
        {
            opFlags.set (use_commLink);
        }
        else
        {
            gridPrimary::set (param, val);
        }
    }
}

void Relay::set (const std::string &param, double val, units_t unitType)
{
    if ((param == "samplingperiod") || (param == "ts") || (param == "sampleperiod"))
    {
        coreObject::set ("period", val, unitType);
        m_nextSampleTime = timeZero;
    }
    else if ((param == "rate") || (param == "fs") || (param == "samplerate"))
    {
        coreObject::set ("period", 1.0 / unitConversion (val, unitType, Hz));
        m_nextSampleTime = timeZero;
    }
    else
    {
        if (cManager.set (param, val))
        {
            opFlags.set (use_commLink);
        }
        else
        {
            gridPrimary::set (param, val, unitType);
        }
    }
}

double Relay::get (const std::string &param, gridUnits::units_t unitType) const
{
    auto fptr = getObjectFunction (this, param);
    if (fptr.first)
    {
        coreObject *tobj = const_cast<Relay *> (this);
        return unitConversion (fptr.first (tobj), fptr.second, unitType, systemBasePower);
    }
    return gridPrimary::get (param, unitType);
}

void Relay::setFlag (const std::string &flag, bool val)
{
    if (flag == "continuous")
    {
        opFlags.set (continuous_flag, val);
        if (!val)
        {
            m_nextSampleTime = (prevTime < timeZero) ? timeZero : prevTime;
        }
    }
    else if (flag == "sampled")
    {
        opFlags.set (continuous_flag, !val);
        if (val)
        {
            m_nextSampleTime = (prevTime < timeZero) ? timeZero : prevTime;
        }
    }
    else if ((flag == "comm_enabled") || (flag == "comms") || (flag == "usecomms"))
    {
        opFlags.set (use_commLink, val);
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
        if (cManager.setFlag (flag, val))
        {
            opFlags.set (use_commLink);
        }
        else
        {
            gridPrimary::setFlag (flag, val);
        }
    }
}

void Relay::updateA (coreTime time)
{
    auto ncond = condChecks;  // the condition triggers may change the number of conditions so the array needs to
    // be copied first
    condChecks.clear ();
    nextUpdateTime = maxTime;
    if (opFlags[continuous_flag])
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
        auto cz = static_cast<index_t> (conditions.size ());
        for (index_t kk = 0; kk < cz; ++kk)
        {
            if (cStates[kk] == condition_status_t::active)
            {
                if (conditions[kk]->checkCondition ())
                {
                    triggerCondition (kk, time, timeZero);
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

        if (time >= m_nextSampleTime)
        {
            auto cz = static_cast<index_t> (conditions.size ());
            for (index_t kk = 0; kk < cz; ++kk)
            {
                if (cStates[kk] == condition_status_t::active)
                {
                    if (conditions[kk]->checkCondition ())
                    {
                        triggerCondition (kk, time, timeZero);
                    }
                }
            }
            m_nextSampleTime += updatePeriod;
            nextUpdateTime = std::min (nextUpdateTime, m_nextSampleTime);
        }
    }
    assert (nextUpdateTime > negTime / 2);
    lastUpdateTime = time;
}

std::string Relay::generateCommName () { return getName (); }

void Relay::pFlowObjectInitializeA (coreTime time0, std::uint32_t /*flags*/)
{
    if ((opFlags[use_commLink]) && (!(commLink)))
    {
        if (cManager.getName ().empty ())
        {
            cManager.setName (generateCommName ());
        }
        commLink = cManager.build ();

        if (commLink)
        {
            try
            {
                commLink->initialize ();
                commLink->registerReceiveCallback (
                  [this](std::uint64_t sourceID, std::shared_ptr<commMessage> message) {
                      receiveMessage (sourceID, message);
                  });
            }
            catch (const std::invalid_argument &)
            {
                LOG_WARNING ("initial commlink name failed trying full object Name");
                cManager.setName (fullObjectName (this));
                try
                {
                    commLink->initialize ();
                    commLink->registerReceiveCallback (
                      [this](std::uint64_t sourceID, std::shared_ptr<commMessage> message) {
                          receiveMessage (sourceID, message);
                      });
                }
                catch (const std::invalid_argument &)
                {
                    LOG_WARNING ("unable to initialize comm link");
                    commLink = nullptr;
                    opFlags.reset (use_commLink);
                }
            }
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
            if (cs == condition_status_t::active)
            {
                opFlags.set (has_powerflow_adjustments);
                break;
            }
        }
    }
    prevTime = time0;
}

void Relay::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    if (opFlags[continuous_flag])
    {
        updateRootCount (false);
    }
    else
    {
        if (updatePeriod == maxTime)
        {  // set the period to the period of the simulation
            updatePeriod = getRoot ()->get ("steptime");
            if (updatePeriod < timeZero)
            {
                updatePeriod = timeOneSecond;
            }
        }
        m_nextSampleTime = nextUpdateTime = time0 + updatePeriod;
    }

    //*update the flag for future power flow check  BUG noticed by Colin Ponce 10/21/16
    if (opFlags[power_flow_checks_flag])
    {
        for (auto &cs : cStates)
        {
            if (cs == condition_status_t::active)
            {
                opFlags.set (has_powerflow_adjustments);
                break;
            }
        }
    }
    gridComponent::dynObjectInitializeA (time0, flags);
}

coreObject *Relay::find (const std::string &objName) const
{
    if (objName == "target")
    {
        return (m_sourceObject != nullptr) ? m_sourceObject : m_sinkObject;
    }
    if (objName == "source")
    {
        return m_sourceObject;
    }
    if (objName == "sink")
    {
        return m_sinkObject;
    }
    if (objName == "relay")
    {
        return const_cast<Relay *> (this);
    }
    return gridPrimary::find (objName);
}

change_code Relay::triggerAction (index_t actionNumber)
{
    if (isValidIndex (actionNumber, actions))
    {
        return executeAction (actionNumber, kNullLocation, prevTime);
    }
    return change_code::not_triggered;
}

void Relay::updateRootCount (bool alertChange)
{
    // get a reference to the local roots count for simplification
    auto &localRoots = offsets.local ().local.algRoots;
    // store a copy of
    auto prevRoots = localRoots;

    localRoots = 0;  // reset the local roots
    conditionsWithRoots.clear ();
    for (index_t kk = 0; kk < static_cast<index_t> (cStates.size ()); ++kk)
    {
        if (cStates[kk] == condition_status_t::active)
        {
            ++localRoots;
            conditionsWithRoots.push_back (kk);
        }
        else if ((cStates[kk] == condition_status_t::triggered) && (opFlags[resettable_flag]))
        {
            ++localRoots;
            conditionsWithRoots.push_back (kk);
        }
    }
    if (prevRoots != localRoots)
    {
        if (localRoots > 0)
        {
            opFlags.set (has_alg_roots);
            opFlags.set (has_roots);
        }
        else
        {
            opFlags.reset (has_alg_roots);
            opFlags.reset (has_roots);
        }
        offsets.rootUnload (true);
        if (alertChange)
        {
            alert (this, ROOT_COUNT_CHANGE);
        }
    }
}

change_code Relay::powerFlowAdjust (const IOdata & /*inputs*/, std::uint32_t /*flags*/, check_level_t level)
{
    change_code ret = change_code::no_change;
    if (level >= check_level_t::full_check)
    {
        auto cz = static_cast<index_t> (conditions.size ());
        for (index_t kk = 0; kk < cz; ++kk)
        {
            if (cStates[kk] == condition_status_t::active)
            {
                if (conditions[kk]->checkCondition ())
                {
                    ret = std::max (triggerCondition (kk, prevTime, maxTime), ret);
                }
            }
        }
    }
    return ret;
}

void Relay::rootTest (const IOdata & /*inputs*/, const stateData &sD, double roots[], const solverMode &sMode)
{
    auto ro = offsets.getRootOffset (sMode);
    for (auto condNum : conditionsWithRoots)
    {
        roots[ro] = conditions[condNum]->evalCondition (sD, sMode);
        ++ro;
    }
}

void Relay::rootTrigger (coreTime time,
                         const IOdata & /*inputs*/,
                         const std::vector<int> &rootMask,
                         const solverMode &sMode)
{
    auto ro = offsets.getRootOffset (sMode);
    // Because conditionsWithRoots can change on a condition Trigger leading to an actionTaken
    // so we need to cache the conditions first to prevent manipulation
    auto checkConditions = conditionsWithRoots;
    for (auto conditionToCheck : checkConditions)
    {
        if (cStates[conditionToCheck] == condition_status_t::active)
        {
            if (rootMask[ro] != 0)
            {
                triggerCondition (conditionToCheck, time, timeZero);
            }
            ++ro;
        }
        else if ((cStates[conditionToCheck] == condition_status_t::triggered) && (opFlags[resettable_flag]))
        {
            if (rootMask[ro] != 0)
            {
                cStates[conditionToCheck] = condition_status_t::active;
                conditions[conditionToCheck]->useMargin (false);
                clearCondChecks (conditionToCheck);
                conditionCleared (conditionToCheck, time);
            }
            ++ro;
        }
    }
    updateRootCount (true);
}

change_code Relay::rootCheck (const IOdata & /*inputs*/,
                              const stateData &sD,
                              const solverMode & /*sMode*/,
                              check_level_t /*level*/)
{
    auto prevTrig = triggerCount;
    auto prevAct = actionsTakenCount;
    coreTime ctime = (!sD.empty ()) ? (sD.time) : prevTime;
    updateA (ctime);
    if ((triggerCount != prevTrig) || (actionsTakenCount != prevAct))
    {
        alert (this, UPDATE_TIME_CHANGE);
        updateRootCount (true);
        return change_code::non_state_change;
    }
    return change_code::no_change;
}

void Relay::clearCondChecks (index_t condNum)
{
    auto cc = condChecks;
    condChecks.resize (0);
    coreTime mTime = nextUpdateTime;
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

std::unique_ptr<eventAdapter> Relay::make_alarm (const std::string &val)
{
    auto lc = convertToLowerCase (val);
    if (lc.compare (0, 5, "alarm") == 0)
    {
        auto codeStr = lc.substr (6);
        auto code = numeric_conversion<std::uint32_t> (codeStr, std::uint32_t (-1));
        if (code == std::uint32_t (-1))
        {
            code = getAlarmCode (codeStr);
        }
        return std::make_unique<functionEventAdapter> ([this, code]() {
            try
            {
                sendAlarm (code);
                return change_code::no_change;
            }
            catch (const executionFailure &)
            {
                return change_code::execution_failure;
            }
        });
    }
    return nullptr;
}

void Relay::receiveMessage (std::uint64_t /*sourceID*/, std::shared_ptr<commMessage> /*message*/) {}

void Relay::sendAlarm (std::uint32_t code)
{
    if (commLink)
    {
        auto m = std::make_shared<comms::relayMessage> (comms::relayMessage::ALARM_TRIGGER_EVENT, code);
        cManager.send (std::move (m));
        return;
    }
    throw (executionFailure (this, "no communication link"));
}

change_code
Relay::triggerCondition (index_t conditionNum, coreTime conditionTriggerTime, coreTime minimumDelayTime)
{
    change_code eventReturn = change_code::no_change;
    cStates[conditionNum] = condition_status_t::triggered;
    conditions[conditionNum]->useMargin (true);

    conditionTriggerTimes[conditionNum] = conditionTriggerTime;
    ++triggerCount;
    conditionTriggered (conditionNum, conditionTriggerTime);
    for (index_t mm = 0; mm < static_cast<index_t> (actionTriggers[conditionNum].size ()); ++mm)
    {
        if (actionDelays[conditionNum][mm] <= minimumDelayTime)
        {
            auto iret = executeAction (actionTriggers[conditionNum][mm], conditionNum, conditionTriggerTime);
            if (iret > eventReturn)
            {
                eventReturn = iret;
            }
        }
        else
        {
            condChecks.emplace_back (conditionNum, mm, conditionTriggerTime + actionDelays[conditionNum][mm]);
            if (hasUpdates ())
            {
                nextUpdateTime = std::min (nextUpdateTime, conditionTriggerTime + actionDelays[conditionNum][mm]);
                alert (this, UPDATE_TIME_CHANGE);
            }
            else
            {
                nextUpdateTime = conditionTriggerTime + actionDelays[conditionNum][mm];
                enable_updates ();
                alert (this, UPDATE_REQUIRED);
            }
        }
    }
    auto iret = multiConditionCheckExecute (conditionNum, conditionTriggerTime, minimumDelayTime);
    if (iret > eventReturn)
    {
        eventReturn = iret;
    }
    return eventReturn;
}

change_code Relay::executeAction (index_t actionNum, index_t conditionNum, coreTime actionTime)
{
    auto eventReturn = actions[actionNum]->execute (actionTime);
    ++actionsTakenCount;
    actionTaken (actionNum, conditionNum, eventReturn, actionTime);
    return eventReturn;
}

change_code
Relay::multiConditionCheckExecute (index_t conditionNum, coreTime conditionTriggerTime, coreTime minimumDelayTime)
{
    change_code eventReturn = change_code::no_change;
    // now check the multiCondition triggers
    for (auto &mct : multiConditionTriggers[conditionNum])
    {
        bool all_triggered = false;
        for (auto &cn : mct.multiConditions)
        {
            if (cStates[cn] != condition_status_t::triggered)
            {
                all_triggered = false;
                break;
            }
        }
        if (all_triggered)
        {
            if (mct.delayTime <= minimumDelayTime)
            {
                auto iret = executeAction (mct.actionNum, conditionNum, prevTime);
                if (iret > eventReturn)
                {
                    eventReturn = iret;
                }
            }
            else
            {
                condChecks.emplace_back (conditionNum, mct.actionNum, conditionTriggerTime + mct.delayTime, true);
                nextUpdateTime = std::min (nextUpdateTime, conditionTriggerTime + mct.delayTime);
                alert (this, UPDATE_TIME_CHANGE);
            }
        }
    }
    return eventReturn;
}

change_code Relay::evaluateCondCheck (condCheckTime &cond, coreTime checkTime)
{
    change_code eventReturn = change_code::no_change;
    if (checkTime >= cond.testTime)
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
            else  // it was a multiCondition trigger
            {
                bool all_triggered = true;
                coreTime trigDelay = multiConditionTriggers[cond.conditionNum][cond.actionNum].delayTime;
                for (auto &cnum : multiConditionTriggers[cond.conditionNum][cond.actionNum].multiConditions)
                {
                    if (cStates[cnum] != condition_status_t::triggered)
                    {
                        all_triggered = false;
                        break;
                    }
                    if (checkTime - conditionTriggerTimes[cnum] < trigDelay)
                    {
                        cond.testTime = conditionTriggerTimes[cnum] + trigDelay;
                        condChecks.push_back (cond);
                        all_triggered = false;
                        break;
                    }
                }
                if (all_triggered)
                {
                    auto iret = executeAction (multiConditionTriggers[cond.conditionNum][cond.actionNum].actionNum,
                                               cond.conditionNum, checkTime);
                    if (iret > eventReturn)
                    {
                        eventReturn = iret;
                    }
                }
            }
        }
        else
        {
            cStates[cond.conditionNum] = condition_status_t::active;
            conditions[cond.conditionNum]->useMargin (false);

            conditionCleared (cond.conditionNum, checkTime);
            updateRootCount (true);
        }
    }
    else
    {
        if (cStates[cond.conditionNum] == condition_status_t::triggered)
        {
            condChecks.push_back (cond);
        }
    }
    return eventReturn;
}

#ifdef DEBUG_LOG_ENABLE
void Relay::actionTaken (index_t ActionNum,
                         index_t conditionNum,
                         change_code actionReturn,
                         coreTime /*actionTime*/)
{
    LOG_DEBUG ((boost::format ("action %d taken based on condition %d  with return code %d") % ActionNum %
                conditionNum % static_cast<int> (actionReturn))
                 .str ());
}
void Relay::conditionTriggered (index_t conditionNum, coreTime timeTriggered)
{
    if (conditionTriggerTimes[conditionNum] > timeZero)
    {
        LOG_DEBUG ((boost::format ("condition %d triggered again at %f") % conditionNum % timeTriggered).str ())
    }
    else
    {
        LOG_DEBUG ((boost::format ("condition %d triggered at %f") % conditionNum % timeTriggered).str ());
    }
}
void Relay::conditionCleared (index_t conditionNum, coreTime timeCleared)
{
    if (conditionTriggerTimes[conditionNum] > timeZero)
    {
        LOG_DEBUG ((boost::format ("condition %d cleared again at %f") % conditionNum % timeCleared).str ());
    }
    else
    {
        LOG_DEBUG ((boost::format ("condition %d cleared at %f") % conditionNum % timeCleared).str ());
    }
}

#else

void Relay::actionTaken (index_t /*ActionNum*/,
                         index_t /*conditionNum*/,
                         change_code /*actionReturn*/,
                         coreTime /*actionTime*/)
{
}
void Relay::conditionTriggered (index_t /*conditionNum*/, coreTime /*timeTriggered*/) {}
void Relay::conditionCleared (index_t /*conditionNum*/, coreTime /*timeCleared*/) {}
#endif

void Relay::updateObject (coreObject *obj, object_update_mode mode)
{
    if (mode == object_update_mode::direct)
    {
        if (m_sourceObject != nullptr)
        {
            setSource (obj);
        }
        if (m_sinkObject != nullptr)
        {
            setSink (obj);
        }
    }
    else if (mode == object_update_mode::match)
    {
        if (m_sourceObject != nullptr)
        {
            setSource (findMatchingObject (m_sourceObject, obj));
        }
        if (m_sinkObject != nullptr)
        {
            setSink (findMatchingObject (m_sinkObject, obj));
        }
        for (auto &cond : conditions)
        {
            cond->updateObject (obj, mode);
        }
        for (auto &act : actions)
        {
            act->updateObject (obj, mode);
        }
    }
}

coreObject *Relay::getObject () const
{
    if (m_sourceObject != nullptr)
    {
        return m_sourceObject;
    }
    if (m_sinkObject != nullptr)
    {
        return m_sinkObject;
    }
    return nullptr;
}

void Relay::getObjects (std::vector<coreObject *> &objects) const
{
    if (m_sourceObject != nullptr)
    {
        objects.push_back (m_sourceObject);
    }
    if ((m_sinkObject != nullptr) && (m_sourceObject != m_sinkObject))
    {
        objects.push_back (m_sinkObject);
    }
    for (auto &cond : conditions)
    {
        cond->getObjects (objects);
    }
    for (auto &act : actions)
    {
        act->getObjects (objects);
    }
}

}  // namespace griddyn
