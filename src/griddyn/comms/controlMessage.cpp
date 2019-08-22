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
#include "controlMessage.h"
#include "../gridDynDefinitions.hpp"
#include "gmlc/utilities/stringConversion.h"

namespace griddyn
{
namespace comms
{
static dPayloadFactory<controlMessagePayload, BASE_CONTROL_MESSAGE_NUMBER, BASE_CONTROL_MESSAGE_NUMBER + 16>
  dmf ("control");

REGISTER_MESSAGE_TYPE (m1, "SET", controlMessagePayload::SET);
REGISTER_MESSAGE_TYPE (m2, "GET", controlMessagePayload::GET);
REGISTER_MESSAGE_TYPE (m3, "GET MULTIPLE", controlMessagePayload::GET_MULTIPLE);
REGISTER_MESSAGE_TYPE (m4, "GET PERIODIC", controlMessagePayload::GET_PERIODIC);
REGISTER_MESSAGE_TYPE (m5, "SET MULTIPLE", controlMessagePayload::SET_MULTIPLE);
REGISTER_MESSAGE_TYPE (m6, "SET SUCCESS", controlMessagePayload::SET_SUCCESS);
REGISTER_MESSAGE_TYPE (m7, "SET FAIL", controlMessagePayload::SET_FAIL);
REGISTER_MESSAGE_TYPE (m8, "GET RESULT", controlMessagePayload::GET_RESULT);
REGISTER_MESSAGE_TYPE (m9, "GET RESULT MULTIPLE", controlMessagePayload::GET_RESULT_MULTIPLE);
REGISTER_MESSAGE_TYPE (m10, "SET SCHEDULED", controlMessagePayload::SET_SCHEDULED);
REGISTER_MESSAGE_TYPE (m11, "GET SCHEDULED", controlMessagePayload::GET_SCHEDULED);
REGISTER_MESSAGE_TYPE (m12, "CANCEL", controlMessagePayload::CANCEL);
REGISTER_MESSAGE_TYPE (m13, "CANCEL SUCCESS", controlMessagePayload::CANCEL_SUCCESS);
REGISTER_MESSAGE_TYPE (m14, "CANCEL FAIL", controlMessagePayload::CANCEL_FAIL);
using namespace gmlc::utilities::stringOps;
using namespace gmlc::utilities;
std::string controlMessagePayload::to_string (uint32_t type, uint32_t /*code*/) const
{
    std::string temp;
    switch (type)
    {
    case SET:
        if (m_actionID > 0)
        {
            temp = "(" + std::to_string (m_actionID) + ")" + m_field;
        }
        else
        {
            temp = m_field;
        }
        if (!m_units.empty ())
        {
            temp += '(' + m_units + ')';
        }
        temp += " = " + std::to_string (m_value) + '@' + std::to_string (m_time);
        break;
    case GET:
        temp = m_field;
        if (!m_units.empty ())
        {
            temp += '(' + m_units + ')';
        }
        break;
    case GET_MULTIPLE:
        for (auto &fld : multiFields)
        {
            temp += ' ' + fld + ',';
        }
        temp.pop_back ();  // get rid of the last comma
        break;
    case GET_PERIODIC:
        temp = m_field;
        if (!m_units.empty ())
        {
            temp += '(' + m_units + ')';
        }
        temp += " @" + std::to_string (m_time);
        break;
    case GET_RESULT_MULTIPLE:
        for (size_t ii = 0; ii < multiFields.size (); ++ii)
        {
            temp += ' ' + multiFields[ii] + '=' + std::to_string (multiValues[ii]) + ',';
        }
        temp.pop_back ();  // get rid of the last comma
        break;
    case SET_SUCCESS:
        if (m_actionID > 0)
        {
            temp =std::to_string (m_actionID);
        }
        break;
    case SET_FAIL:
        if (m_actionID > 0)
        {
            temp = std::to_string (m_actionID);
        }
        break;
    case GET_RESULT:
        temp = m_field;
        if (!m_units.empty ())
        {
            temp += '(' + m_units + ')';
        }
        temp += " = " + std::to_string (m_value) + '@' + std::to_string (m_time);
        break;
    case SET_SCHEDULED:
        temp = std::to_string (m_actionID);
        break;
    case GET_SCHEDULED:
        temp = std::to_string (m_actionID);
        break;
    case CANCEL:
        temp = std::to_string (m_actionID);
        break;
    case CANCEL_SUCCESS:
        temp = std::to_string (m_actionID);
        break;
    case CANCEL_FAIL:
        temp = std::to_string (m_actionID);
        break;
    default:
        break;
    }
    return temp;
}

void controlMessagePayload::from_string (uint32_t type, uint32_t /*code*/, const std::string &fromString, size_t offset)
{
    std::string idstring;
    bool vstrid = false;

        auto vstring = fromString.substr (offset, std::string::npos);
        if (vstring[0] == '(')
        {
            auto cp = vstring.find_first_of (')');
            idstring = vstring.substr (0, cp - 1);
            if (idstring.empty())
            {
                m_actionID = 0;
            }
            else
            {
                m_actionID = std::stoull(idstring);
            }
        }

        switch (type)
        {
        case SET:
        {
            auto svec = splitline(vstring, "=@");
            auto psep = splitline(svec[0], "()");
            if (psep.size() > 1)
            {
                m_units = psep[1];
            }
            m_field = psep[0];
            m_value = numeric_conversion(svec[1], kNullVal);
            if (svec.size() > 2)
            {
                m_time = numeric_conversion(svec[2], kNullVal);
            }
        }
        break;
        case GET:
        {
            auto svec = splitline(vstring, '@');
            auto psep = splitline(svec[0], "()");
            if (psep.size() > 1)
            {
                m_units = psep[1];
            }
            m_field = psep[0];

            if (svec.size() > 1)
            {
                m_time = numeric_conversion(svec[1], kNullVal);
            }
        }
        break;
        case GET_PERIODIC:
        {
            auto svec = splitline(vstring, '@');
            auto psep = splitline(vstring, ',');
            if (psep.size() > 1)
            {
                m_units = psep[1];
            }
            m_field = psep[0];

            if (svec.size() > 1)
            {
                m_time = numeric_conversion(svec[1], kNullVal);
            }
        }
        break;
        case GET_MULTIPLE:
        {
            multiFields = splitline(vstring, ',');
        }
        break;
        case GET_RESULT_MULTIPLE:
        {
            auto mf = splitline(vstring, ',');
            multiFields.resize(0);
            multiValues.resize(0);
            for (auto &mfl : mf)
            {
                auto fsep = splitline(mfl, '=');
                if (fsep.size() == 2)
                {
                    multiFields.push_back(fsep[0]);
                    multiValues.push_back(numeric_conversion(fsep[1], kNullVal));
                }
            }
        }
        break;
        case GET_RESULT:
        {
            auto svec = splitline(vstring, "=@");
            auto psep = splitline(svec[0], "()");
            if (psep.size() > 1)
            {
                m_units = psep[1];
            }
            m_field = psep[0];
            m_value = numeric_conversion(svec[1], kNullVal);
            if (svec.size() > 2)
            {
                m_time = numeric_conversion(svec[2], kNullVal);
            }
        }
        break;
        default:
            vstrid = true;
            break;
    }

    if (vstrid)
    {
        if (vstring.empty ())
        {
            m_actionID = 0;
        }
        else
        {
            m_actionID = std::stoull (vstring);
        }
    }
}

}  // namespace comms
}  // namespace griddyn