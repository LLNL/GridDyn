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
#ifndef CONTROL_RELAY_H_
#define CONTROL_RELAY_H_

#include "gridRelay.h"
#include "linkModels/gridLink.h"  //some special features for links

class commMessage;
class controlMessage;

class gridEvent;
class gridSimulation;
class functionEventAdapter;

enum class change_code;

//helper class for delayed execution of set functions
struct delayedControlAction
{
  std::uint64_t sourceID;
  std::shared_ptr<gridEvent> sEvent;
  std::uint64_t actionID;
  double executionTime;
  bool executed;
  bool measureAction;
};


class controlRelay : public gridRelay
{
public:
  enum controlrelay_flags
  {
    link_type_source = object_flag9,
    link_type_sink = object_flag10,
    no_message_reply = object_flag11,
  };
protected:
  int autoName = -1;
  double actionDelay = 0.0;
  double measureDelay = 0.0;
  count_t instructionCounter = 0;

  std::vector<delayedControlAction> actions;
  gridSimulation *rootSim = nullptr;
  index_t m_terminal;
private:
  std::string m_terminal_key;
public:
  controlRelay (const std::string &objName = "controlRelay_$");
  gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual int setFlag (const std::string &flag, bool val = true) override;
  virtual int set (const std::string &param,  const std::string &val) override;

  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;
protected:
  void actionTaken (index_t ActionNum, index_t conditionNum, change_code actionReturn, double actionTime) override;

  void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;
  std::string generateAutoName (int code);

  change_code executeAction (index_t index);

  index_t findAction (std::uint64_t actionID);
  index_t getFreeAction ();

  std::shared_ptr<functionEventAdapter> generateGetEvent (double eventTime, std::uint64_t sourceID, std::shared_ptr<controlMessage> message);
  std::shared_ptr<functionEventAdapter> generateSetEvent (double eventTime, std::uint64_t sourceID, std::shared_ptr<controlMessage> message);
};



#endif
