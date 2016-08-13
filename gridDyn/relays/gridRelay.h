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

#ifndef GRIDDYN_RELAY_H_
#define GRIDDYN_RELAY_H_

#include "gridObjects.h"

class stateGrabber;
class gridGrabber;
class gridCondition;
class gridCommunicator;
class eventAdapter;
class gridEvent;
class commMessage;

enum class change_code;

/**
*@brief relay class:
 relay's are sensors and actuators.  They can read data from gridCoreObjects and then take actions on other
* objects on a regular schedule or on a functional basis.
**/
class gridRelay : public gridPrimary
{
public:
  static count_t relayCount;  //!< counter for the number of relays
  /** @brief enumeration fo the relay condition states*/
  enum class condition_states
  {
    active, //!< the relay condition is active
    triggered,  //!< the relay condition is triggered and waiting a timeout
    disabled,  //!< the relay condition is disabled and not scanning
  };

protected:
  count_t numAlgRoots;        //!< counter for the number of root finding operations related to the condition checking
  enum relay_flags
  {
    continuous_flag = object_flag1,             //!< flag indicating the relay has some continuous checks
    resettable_flag = object_flag2,             //!< flag indicating that the conditions can be reset
    use_commLink = object_flag3,             //!< flag indicating that the relay uses communications
    power_flow_checks_flag = object_flag4,             //!< flag indicating that the relay should be in operation during power flow
    extra_relay_flag = object_flag5,             //!< just defining an extra name for additional relay flags

  };
  double triggerTime = kBigNum;            //!<the next time execute
  gridCoreObject *m_sourceObject = nullptr;       //!<the default object where the data comes from
  gridCoreObject *m_sinkObject = nullptr;            //!<the default object where the actions occur
  count_t triggerCount = 0;        //!< count of the number of triggers
  count_t actionsTakenCount = 0;        //!< count of the number of actions taken

  // comm fields
  std::string commName;        //!< The name to use on the commlink
  std::uint64_t commId = 0;        //!< the id to use on the commlink
  std::string commType;       //!< the type of comms to construct
  std::string commDestName;       //!< the default communication destination as a string
  std::uint64_t commDestId = 0;       //!< the default communication destination id
  std::shared_ptr<gridCommunicator> commLink;             //!<communicator link

  double m_nextSampleTime = 0.0;        //!< the next time to sample the conditions

public:
  gridRelay (const std::string &objName = "relay_$");

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual ~gridRelay ();

  virtual int add (gridCoreObject *obj) override;

  /**
  *@brief add a gridEvent to the system
  **/
  virtual int add (std::shared_ptr<gridEvent> ge);
  /**
  *@brief add an EventAdapter to the system
  **/
  virtual int add (std::shared_ptr<eventAdapter> geA);
  /**
  * @brief add an condition to the system
  **/
  virtual int add (std::shared_ptr<gridCondition> gc);

  /**
  *@brief update a specific action
  **/
  virtual int updateAction (std::shared_ptr<gridEvent> ge, index_t actionNumber);
  /**
  *@brief update a specfic action
  **/
  virtual int updateAction (std::shared_ptr<eventAdapter> geA, index_t actionNumber);
  /**
  *@brief update a specific condition
  **/
  virtual int updateCondition (std::shared_ptr<gridCondition> gc,  index_t conditionNumber);

  void setTime (double time) override;
  /**
  *@brief reset the relay
  **/
  void resetRelay ();
  /**
  * @briefset the relay source object
  */
  void setSource (gridCoreObject *obj);
  /**
  * @brief set the relay sink object
  */
  void setSink (gridCoreObject *obj);

  void setConditionState (index_t conditionNumber,condition_states newState = condition_states::active);
  void removeAction (index_t conditionNumber);

  std::shared_ptr<gridCondition> getCondition (index_t conditionNumber);
  std::shared_ptr<eventAdapter> getAction (index_t actionNumber);

  condition_states getConditionStatus (index_t conditionNumber);
  double getConditionValue (index_t conditionNumber) const;
  double getConditionValue (index_t conditionNumber, const stateData *sD, const solverMode &sMode) const;
  bool checkCondition (index_t conditionNumber) const;
  void setConditionLevel (index_t conditionNumber, double levelVal);
  virtual void setActionTrigger (index_t conditionNumber, index_t actionNumber, double delayTime = 0.0);
  virtual change_code triggerAction (index_t actionNumber);
  virtual void setActionMultiTrigger (std::vector<index_t> multi_conditions, index_t actionNumber, double delayTime = 0.0);

  void setResetMargin (index_t conditionNumber, double margin);
  virtual int setFlag (const std::string &flag, bool val = true)  override;
  virtual int set (const std::string &param,  const std::string &val) override;

  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit)  override;

  virtual void updateA (double time)  override;
  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (double time0, unsigned long flags)  override;

  virtual change_code powerFlowAdjust (unsigned long flags, check_level_t level) override;
  virtual void rootTest (const stateData *sD, double roots[], const solverMode &sMode)  override;
  virtual void rootTrigger (double ttime, const std::vector<int> &rootMask, const solverMode &sMode)  override;
  virtual change_code rootCheck (const stateData *sD, const solverMode &sMode,  check_level_t level)  override;

  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message);

  int sendAlarm (std::uint32_t code);
  std::shared_ptr<eventAdapter> make_alarm (const std::string &val);
protected:
  virtual void updateRootCount (bool alertChange = true);
  virtual void actionTaken (index_t ActionNum, index_t conditionNum,  change_code actionReturn, double actionTime);
  virtual void conditionTriggered (index_t conditionNum, double timeTriggered);
  virtual void conditionCleared (index_t conditionNum, double timeTriggered);
private:
private:
  /** @brief subclass  data container for helping with condition time checks*/
  class condCheckTime
  {
public:
    index_t conditionNum;                      //!< the condition Number
    index_t actionNum;                     //!< the action number
    double testTime;                      //!< the time the test should be performed
    bool multiCondition = false;                      //!< flag if the condition is part of a multicondition

    condCheckTime ()
    {
    }
    /** @brief constructor with all the data
    @param[in] cNum the conditionNumber
    @param[in] aNum the actionNumber
    @param[in] ttime the time to conduct a test
    @param[in] mcond  the value of the multicondition flag
    */
    condCheckTime (index_t cNum, index_t aNum, double ttime, bool mcond = false) : conditionNum (cNum), actionNum (aNum), testTime (ttime), multiCondition (mcond)
    {
    }
  };
  /** @brief data type declaration for a multiCondition trigger*/
  typedef struct
  {
    index_t actionNum;                      //!< the related ActionNumber
    std::vector<index_t> multiConditions;                      //!< identification of the conditions involved
    double delayTime;                     //!< the delay time all conditions must be true before the action is taken
  } mcondTrig;
  /** enumeration of relay flags*/

  std::vector < std::shared_ptr < gridCondition >> conditions;                //!<state conditionals for the system
  std::vector < std::shared_ptr < eventAdapter >> actions;                //!<actions to take in response to triggers
  std::vector < std::vector < index_t >> actionTriggers;                //!<the conditions that cause actions
  std::vector < std::vector < double >> actionDelays;               //!<the periods of time in which the condition must be true for an action to occur
  std::vector<condition_states> cStates;               //!< a vector of states for the conditions
  std::vector<double> conditionTriggerTimes;               //!< the times at which the condition triggered
  std::vector<condCheckTime> condChecks;               //!<a vector of condition action pairs that are in wait and see mode
  std::vector < std::vector < mcondTrig >> multiConditionTriggers;               //!<a vector for action which have multiple triggers
  std::vector<index_t> conditionsWithRoots;
private:
  void clearCondChecks (index_t condNum);
  change_code executeAction (index_t actionNum, index_t conditionNum, double actionTime);
  change_code triggerCondition (index_t conditionNum,double conditionTriggerTime,double ignoreDelayTime);
  change_code multiConditionCheckExecute (index_t conditionNum, double conditionTriggerTime, double ignoreDelayTime);
  change_code evaluateCondCheck (condCheckTime &cond, double currentTime);
};



#endif