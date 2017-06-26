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
#ifndef CONTROL_RELAY_H_
#define CONTROL_RELAY_H_

#include <unordered_map>
#include "Relay.h"
#include "Link.h"  //some special features for links

namespace griddyn
{

class commMessage;
namespace comms
{
	class controlMessage;
}


class gridSimulation;
class functionEventAdapter;
class gridGrabber;

enum class change_code;
namespace relays
{
/**helper class for delayed execution of set functions*/
struct delayedControlAction
{
  std::uint64_t sourceID;
  std::uint64_t actionID;
  std::string field;
  coreTime triggerTime;
  coreTime executionTime;
  double val;
  gridUnits::units_t unitType = gridUnits::defUnit;
  bool executed;
  bool measureAction;
};

/** @brief relay with control functionality  ie the ability to control an object through a comm channel
*/
class controlRelay : public Relay
{
public:
  enum controlrelay_flags
  {
    link_type_source = object_flag9,
    link_type_sink = object_flag10,
    no_message_reply = object_flag11,
  };
protected:
  coreTime actionDelay = timeZero;		//!< the delay between comm signal and action
  coreTime measureDelay = timeZero;	//!< the delay between comm measure request and action measurement extraction
  count_t instructionCounter = 0;	//!< counter for the number of instructions
  //NOTE: probably have a 4 byte padding gap
  std::vector<delayedControlAction> actions;	//!< queue for delayed control actions
  gridSimulation *rootSim = nullptr;		//!< pointer to the root object
  index_t m_terminal=1;		//!< the terminal of a link device to act upon(if source or sink is a link
  int autoName = -1;			//!< variable for autonaming
  std::vector<std::unique_ptr<gridGrabber>> measurement_points_;  //!< vector of grabbers defining measurement points
  std::unordered_map<std::string, index_t> pointNames_;  //!< vector of names for the pointlist;
private:
  std::string m_terminal_key;
public:
  explicit controlRelay (const std::string &objName = "controlRelay_$");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void updateObject(coreObject *obj, object_update_mode mode = object_update_mode::direct) override;

  void addMeasurement(const std::string &measure);
  double getMeasurement(index_t num);
  double getMeasurement(const std::string &pointName);
  index_t findMeasurement(const std::string &pointName) const;
protected:
  virtual void actionTaken (index_t ActionNum, index_t conditionNum, change_code actionReturn, coreTime actionTime) override;

  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;
  std::string generateAutoName (int code);
  std::string generateCommName() override;

  change_code executeAction (index_t actionNum);

  index_t findAction (std::uint64_t actionID);
  index_t getFreeAction ();
  

  std::unique_ptr<functionEventAdapter> generateGetEvent (coreTime eventTime, std::uint64_t sourceID, std::shared_ptr<comms::controlMessage> &message);
  std::unique_ptr<functionEventAdapter> generateSetEvent (coreTime eventTime, std::uint64_t sourceID, std::shared_ptr<comms::controlMessage> &message);
};
}//namespace relays
}//namespace griddyn

#endif
