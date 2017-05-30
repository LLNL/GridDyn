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

#ifndef GRIDDYN_RELAY_MESSAGE_H_
#define GRIDDYN_RELAY_MESSAGE_H_

#include "comms/commMessage.h"
#include <string>

#include <boost/serialization/base_object.hpp>


#define BASE_RELAY_MESSAGE_NUMBER 400

class relayMessage : public commMessage
{
public:
  enum relay_message_type_t
  {
    NO_EVENT = BASE_RELAY_MESSAGE_NUMBER,
    LOCAL_FAULT_EVENT = BASE_RELAY_MESSAGE_NUMBER + 3,
    REMOTE_FAULT_EVENT = BASE_RELAY_MESSAGE_NUMBER + 4,
    BREAKER_TRIP_EVENT = BASE_RELAY_MESSAGE_NUMBER + 5,
    BREAKER_CLOSE_EVENT = BASE_RELAY_MESSAGE_NUMBER + 6,
    LOCAL_FAULT_CLEARED = BASE_RELAY_MESSAGE_NUMBER + 7,
    REMOTE_FAULT_CLEARED = BASE_RELAY_MESSAGE_NUMBER + 8,
    BREAKER_TRIP_COMMAND = BASE_RELAY_MESSAGE_NUMBER + 9,
    BREAKER_CLOSE_COMMAND = BASE_RELAY_MESSAGE_NUMBER + 10,
    BREAKER_OOS_COMMAND = BASE_RELAY_MESSAGE_NUMBER + 11,
    ALARM_TRIGGER_EVENT = BASE_RELAY_MESSAGE_NUMBER + 12,
    ALARM_CLEARED_EVENT = BASE_RELAY_MESSAGE_NUMBER + 13,
  };
  std::uint32_t m_code = 0;
  relayMessage ()
  {
  }
  /** construct from type */
  explicit relayMessage (std::uint32_t type) : commMessage (type)
  {
  }
  relayMessage (std::uint32_t type, std::uint32_t code) : commMessage (type), m_code (code)
  {
  }
  virtual std::string toString (int modifiers=comm_modifiers::none) const override;
  virtual void loadString (const std::string &fromString) override;
private:
  //boost serialization stuff
  friend class boost::serialization::access;
  template <class Archive>
  void serialize (Archive & ar, const unsigned int /*version*/)
  {
    ar & boost::serialization::base_object<commMessage> (*this);
    ar & m_code;
  }
};

//defining a number of alarm codes
enum alarmCode
{
  OVERCURRENT_ALARM = 101,
  UNDERCURRENT_ALARM = 102,
  OVERVOLTAGE_ALARM = 111,
  UNDERVOLTAGE_ALARM = 112,
  TEMPERATURE_ALARM1 = 201,
  TEMPERATURE_ALARM2 = 202,
  UNDERFREQUENCY_ALARM = 301,
  OVERFREQUENCY_ALARM = 303,

};

std::uint32_t getAlarmCode (const std::string &alarmStr);
//BOOST_CLASS_EXPORT_GUID(relayMessage, "relayMessage");
#endif
