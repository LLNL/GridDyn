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

#ifndef GRIDDYN_CONTROL_MESSAGE_H_
#define GRIDDYN_CONTROL_MESSAGE_H_

#include "comms/commMessage.h"
#include <string>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#define BASE_CONTROL_MESSAGE_NUMBER 500

class controlMessage : public commMessage
{
public:
  enum control_message_t
  {
    SET = BASE_CONTROL_MESSAGE_NUMBER + 3,
    GET = BASE_CONTROL_MESSAGE_NUMBER + 4,
    GET_MULTIPLE = BASE_CONTROL_MESSAGE_NUMBER + 5,
    GET_PERIODIC = BASE_CONTROL_MESSAGE_NUMBER + 6,
    SET_SUCCESS = BASE_CONTROL_MESSAGE_NUMBER + 7,
    SET_FAIL = BASE_CONTROL_MESSAGE_NUMBER + 8,
    GET_RESULT = BASE_CONTROL_MESSAGE_NUMBER + 9,
    GET_RESULT_MULTIPLE = BASE_CONTROL_MESSAGE_NUMBER + 10,
    SET_SCHEDULED = BASE_CONTROL_MESSAGE_NUMBER + 11,
    GET_SCHEDULED = BASE_CONTROL_MESSAGE_NUMBER + 12,
    CANCEL = BASE_CONTROL_MESSAGE_NUMBER + 13,
    CANCEL_SUCCESS = BASE_CONTROL_MESSAGE_NUMBER + 14,
    CANCEL_FAIL = BASE_CONTROL_MESSAGE_NUMBER + 15
  };
  std::string m_field;
  double m_value;
  double m_time = -1;
  size_t m_actionID;
  std::vector<std::string> multiFields;
  std::vector<double> multiValues;
  std::string m_units;
  controlMessage ()
  {
  }
  controlMessage (std::uint32_t type) : commMessage (type)
  {
  }
  controlMessage (std::uint32_t type, std::string fld, double val, double time) : commMessage (type), m_field (fld), m_value (val), m_time (time)
  {
  }
  virtual ~controlMessage ()
  {
  }

  virtual std::string toString () override;
  virtual void loadString (const std::string &fromString) override;
private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize (Archive & ar, const unsigned int /*version*/)
  {
    ar & boost::serialization::base_object<commMessage> (*this);
    ar & m_actionID;
    switch (getMessageType ())
      {
      case GET_RESULT:
      case SET:
        ar & m_field;
        ar & m_units;
        ar & m_value;
        ar & m_time;
        break;
      case GET:
        ar & m_field;
        ar & m_units;
        break;
      case GET_MULTIPLE:
        ar & multiFields;
        break;
      case GET_PERIODIC:
        ar & m_field;
        ar & m_units;
        ar & m_time;
        break;
      case GET_RESULT_MULTIPLE:
        ar & multiFields;
        ar & multiValues;
        ar & m_time;
        break;
      default:
        break;
      }
  }
};

//BOOST_CLASS_EXPORT_GUID(controlMessage, "controlMessage");
#endif
