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

#ifndef GRIDDYN_CONTROL_MESSAGE_H_
#define GRIDDYN_CONTROL_MESSAGE_H_
#pragma once

#include "comms/commMessage.h"
#include <string>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#define BASE_CONTROL_MESSAGE_NUMBER 500

namespace griddyn
{
namespace comms
{
/** a message type used for requesting control actions to relays and sending values in response
 */
class controlMessage : public commMessage
{
  public:
    /** the enumeration of the message types available in control messages*/
    enum control_message_t : std::uint32_t
    {
        SET = BASE_CONTROL_MESSAGE_NUMBER + 3,  //!< set a single value
        GET = BASE_CONTROL_MESSAGE_NUMBER + 4,  //!< request a single value
        GET_MULTIPLE = BASE_CONTROL_MESSAGE_NUMBER + 5,  //!< request multiple values
        GET_PERIODIC = BASE_CONTROL_MESSAGE_NUMBER + 6,  //!< get values on a periodic basis
        SET_MULTIPLE = BASE_CONTROL_MESSAGE_NUMBER + 8,  //!< set multiple values
        SET_SUCCESS = BASE_CONTROL_MESSAGE_NUMBER + 10,  //!< acknowledge that the set operation was successful
        SET_FAIL = BASE_CONTROL_MESSAGE_NUMBER + 12,  //!< response when the set operation failed
        GET_RESULT = BASE_CONTROL_MESSAGE_NUMBER + 14,  //!< the result of get request
        GET_RESULT_MULTIPLE = BASE_CONTROL_MESSAGE_NUMBER + 16,  //!< the results of a get multiple request
        SET_SCHEDULED = BASE_CONTROL_MESSAGE_NUMBER + 18,  //!< set operation to be scheduled in the future
        GET_SCHEDULED = BASE_CONTROL_MESSAGE_NUMBER + 20,  //!< get operation to be scheduled in the future
        CANCEL = BASE_CONTROL_MESSAGE_NUMBER + 22,  //!< cancel a scheduled operation
        CANCEL_SUCCESS =
          BASE_CONTROL_MESSAGE_NUMBER + 24,  //!< acknowledge that the cancel operation was successful
        CANCEL_FAIL = BASE_CONTROL_MESSAGE_NUMBER +
                      26  //!< respond to a cancel with an indication that the cancel operation was not successful
    };
    std::string m_field;  //!< the field in question
    double m_value = 0.0;  //!< the value of the field
    double m_time = -1.0;  //!< the time of the action
    std::int64_t m_actionID = 0;  //!< an identifier for reference of cancel operations
    std::vector<std::string> multiFields;  //!< a vector of strings in the *_MULTIPLE types
    std::vector<double> multiValues;  //!< a vector of values in the *_MULTIPLE operations
    std::string m_units;  //!< the units requested
    std::vector<std::string> multiUnits;  //!< the multiple units associated with different measurements
    /** default constructor*/
    controlMessage () = default;

    explicit controlMessage (std::uint32_t type) : commMessage (type) {}
    /** constructor
    @param[in] type the type of the message
    @param[in] fld the name of the field in question
    @param[in] val the value associated with the field
    @param[in] time the time for a schedule
    */
    controlMessage (std::uint32_t type, const std::string &fld, double val, coreTime time)
        : commMessage (type), m_field (fld), m_value (val), m_time (time)
    {
    }

    virtual std::string to_string (int modifiers = comm_modifiers::none) const override;
    virtual void loadString (const std::string &fromString) override;

  private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize (Archive &ar, const unsigned int /*version*/)
    {
        ar &boost::serialization::base_object<commMessage> (*this);
        ar &m_actionID;
        switch (getMessageType ())
        {
        case GET_RESULT:
        case SET:
            ar &m_field;
            ar &m_units;
            ar &m_value;
            ar &m_time;
            break;
        case GET:
            ar &m_field;
            ar &m_units;
            break;
        case GET_MULTIPLE:
            ar &m_time;
            ar &multiFields;
            ar &multiUnits;
            break;
        case SET_MULTIPLE:
            ar &m_time;
            ar &multiFields;
            ar &multiValues;
            ar &multiUnits;
            break;
        case GET_PERIODIC:
            ar &m_field;
            ar &m_units;
            ar &m_time;
            break;
        case GET_RESULT_MULTIPLE:
            ar &m_time;
            ar &multiFields;
            ar &multiValues;

            break;
        default:
            break;
        }
    }
};
}  // namespace comms
}  // namespace griddyn
// BOOST_CLASS_EXPORT_GUID(controlMessage, "controlMessage");
#endif
