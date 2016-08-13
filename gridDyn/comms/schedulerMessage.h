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

#ifndef SCHEDULER_MESSAGE_H_
#define SCHEDULER_MESSAGE_H_

#include "comms/commMessage.h"

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>

#define BASE_SCHEDULER_MESSAGE_NUMBER 800

class schedulerMessage : public commMessage
{
public:
  enum scheduler_message_type_t
  {
    CLEAR_TARGETS = BASE_SCHEDULER_MESSAGE_NUMBER + 3,
    SHUTDOWN = BASE_SCHEDULER_MESSAGE_NUMBER + 4,
    STARTUP = BASE_SCHEDULER_MESSAGE_NUMBER + 5,
    ADD_TARGETS = BASE_SCHEDULER_MESSAGE_NUMBER + 6,
    UPDATE_TARGETS = BASE_SCHEDULER_MESSAGE_NUMBER + 7,
    UPDATE_RESERVES = BASE_SCHEDULER_MESSAGE_NUMBER + 8,
    UPDATE_REGULATION_RESERVE = BASE_SCHEDULER_MESSAGE_NUMBER + 9,
    USE_RESERVE = BASE_SCHEDULER_MESSAGE_NUMBER + 10,
    UPDATE_REGULATION_TARGET = BASE_SCHEDULER_MESSAGE_NUMBER + 11,
    REGISTER_DISPATCHER = BASE_SCHEDULER_MESSAGE_NUMBER + 12,
    REGISTER_AGC_DISPATCHER = BASE_SCHEDULER_MESSAGE_NUMBER + 13,
    REGISTER_RESERVE_DISPATCHER = BASE_SCHEDULER_MESSAGE_NUMBER + 14,
    REGISTER_CONTROLLER = BASE_SCHEDULER_MESSAGE_NUMBER + 15,
  };

  schedulerMessage ()
  {
  }
  schedulerMessage (std::uint32_t type) : commMessage (type)
  {
  }
  schedulerMessage (std::uint32_t type, std::vector<double> time, std::vector<double> target);

  virtual ~schedulerMessage ()
  {
  }

  void loadMessage (std::uint32_t messageType, std::vector<double> time, std::vector<double> target);

  std::vector<double> m_time;
  std::vector<double> m_target;

  virtual std::string toString () override;
  virtual void loadString (const std::string &fromString) override;

private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize (Archive & ar, const unsigned int /*version*/)
  {
    ar & boost::serialization::base_object<commMessage> (*this);

    switch (getMessageType ())
      {
      case SHUTDOWN:
      case STARTUP:
        ar & m_time;
        break;
      case ADD_TARGETS:
      case UPDATE_TARGETS:
      case UPDATE_RESERVES:
      case UPDATE_REGULATION_RESERVE:
      case USE_RESERVE:
      case UPDATE_REGULATION_TARGET:
        ar & m_time;
        ar & m_target;
        break;
      default:           //all other ones don't have any information
        break;
      }

  }

  std::string makeTargetString (size_t cnt);
};

//BOOST_CLASS_EXPORT_GUID(schedulerMessage, "schedulerMessage");
#endif
