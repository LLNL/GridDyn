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
#include "utilities/stringConversion.h"

namespace griddyn
{
namespace comms
{
static dMessageFactory<controlMessage, BASE_CONTROL_MESSAGE_NUMBER, BASE_CONTROL_MESSAGE_NUMBER + 16>
  dmf ("control");

using namespace stringOps;

std::string controlMessage::to_string (int modifiers) const
{
    std::string temp = (modifiers == comm_modifiers::with_type) ? encodeTypeInString () : "";
    switch (getMessageType ())
    {
    case SET:
        if (m_actionID > 0)
        {
            temp += "SET(" + std::to_string (m_actionID) + "):" + m_field;
        }
        else
        {
            temp += "SET:" + m_field;
        }
        if (!m_units.empty ())
        {
            temp += '(' + m_units + ')';
        }
        temp += " = " + std::to_string (m_value) + '@' + std::to_string (m_time);
        break;
    case GET:
        temp += "GET:" + m_field;
        if (!m_units.empty ())
        {
            temp += '(' + m_units + ')';
        }
        break;
    case GET_MULTIPLE:
        temp += "GET_MULTIPLE:";
        for (auto &fld : multiFields)
        {
            temp += ' ' + fld + ',';
        }
        temp.pop_back ();  // get rid of the last comma
        break;
    case GET_PERIODIC:
        temp += "GET_PERIODIC:" + m_field;
        if (!m_units.empty ())
        {
            temp += '(' + m_units + ')';
        }
        temp += " @" + std::to_string (m_time);
        break;
    case GET_RESULT_MULTIPLE:
        temp += "GET_RESULT_MULTIPLE:";
        for (size_t ii = 0; ii < multiFields.size (); ++ii)
        {
            temp += ' ' + multiFields[ii] + '=' + std::to_string (multiValues[ii]) + ',';
        }
        temp.pop_back ();  // get rid of the last comma
        break;
    case SET_SUCCESS:
        if (m_actionID > 0)
        {
            temp += "SET SUCCESS:" + std::to_string (m_actionID);
        }
        else
        {
            temp += "SET SUCCESS";
        }
        break;
    case SET_FAIL:
        if (m_actionID > 0)
        {
            temp += "SET FAIL:" + std::to_string (m_actionID);
        }
        else
        {
            temp += "SET FAIL";
        }
        break;
    case GET_RESULT:
        temp += "GET RESULT: " + m_field;
        if (!m_units.empty ())
        {
            temp += '(' + m_units + ')';
        }
        temp += " = " + std::to_string (m_value) + '@' + std::to_string (m_time);
        break;
    case SET_SCHEDULED:
        temp += "SET SCHEDULED:" + std::to_string (m_actionID);
        break;
    case GET_SCHEDULED:
        temp += "GET SCHEDULED:" + std::to_string (m_actionID);
        break;
    case CANCEL:
        temp += "CANCEL:" + std::to_string (m_actionID);
        break;
    case CANCEL_SUCCESS:
        temp += "CANCEL SUCCESS:" + std::to_string (m_actionID);
        break;
    case CANCEL_FAIL:
        temp += "CANCEL FAIL:" + std::to_string (m_actionID);
        break;
    default:
        temp += "<UNKNOWN>";
    }
    return temp;
}

void controlMessage::loadString (const std::string &fromString)
{
    std::string idstring;
    std::string vstring;
    auto cc = fromString.find_first_of (':');
    std::string lstr;
    bool vstrid = false;
    if (cc != std::string::npos)
    {
        lstr = fromString.substr (0, cc - 1);
        vstring = fromString.substr (cc + 1, std::string::npos);
        cc = lstr.find_first_of ('(');
        if (cc != std::string::npos)
        {
            auto cp = lstr.find_first_of (')', cc + 1);
            idstring = lstr.substr (cc + 1, cp - 1);
            lstr = lstr.substr (0, cc - 1);
            trimString (lstr);
        }
    }
    if (lstr == "SET")
    {
        setMessageType (SET);
        if (idstring.empty ())
        {
            m_actionID = 0;
        }
        else
        {
            m_actionID = numeric_conversion<size_t> (idstring, 0);
        }
        auto svec = splitline (vstring, "=@");
        auto psep = splitline (svec[0], "()");
        if (psep.size () > 1)
        {
            m_units = psep[1];
        }
        m_field = psep[0];
        m_value = numeric_conversion (svec[1], kNullVal);
        if (svec.size () > 2)
        {
            m_time = numeric_conversion (svec[2], kNullVal);
        }
    }
    else if (lstr == "GET")
    {
        setMessageType (GET);
        if (idstring.empty ())
        {
            m_actionID = 0;
        }
        else
        {
            m_actionID = std::stoull (idstring);
        }
        auto svec = splitline (vstring, '@');
        auto psep = splitline (svec[0], "()");
        if (psep.size () > 1)
        {
            m_units = psep[1];
        }
        m_field = psep[0];

        if (svec.size () > 1)
        {
            m_time = numeric_conversion (svec[1], kNullVal);
        }
    }
    else if (lstr == "GET_PERIODIC")
    {
        setMessageType (GET_PERIODIC);
        if (idstring.empty ())
        {
            m_actionID = 0;
        }
        else
        {
            m_actionID = std::stoull (idstring);
        }
        auto svec = splitline (vstring, '@');
        auto psep = splitline (vstring, ',');
        if (psep.size () > 1)
        {
            m_units = psep[1];
        }
        m_field = psep[0];

        if (svec.size () > 1)
        {
            m_time = numeric_conversion (svec[1], kNullVal);
        }
    }
    else if (lstr == "GET_MULTIPLE")
    {
        setMessageType (GET_MULTIPLE);
        if (idstring.empty ())
        {
            m_actionID = 0;
        }
        else
        {
            m_actionID = std::stoull (idstring);
        }
        multiFields = splitline (vstring, ',');
    }
    else if (lstr == "GET_RESULT_MULTIPLE")
    {
        setMessageType (GET_RESULT_MULTIPLE);
        if (idstring.empty ())
        {
            m_actionID = 0;
        }
        else
        {
            m_actionID = std::stoull (idstring);
        }
        auto mf = splitline (vstring, ',');
        multiFields.resize (0);
        multiValues.resize (0);
        for (auto &mfl : mf)
        {
            auto fsep = splitline (mfl, '=');
            if (fsep.size () == 2)
            {
                multiFields.push_back (fsep[0]);
                multiValues.push_back (numeric_conversion (fsep[1], kNullVal));
            }
        }
    }
    else if (lstr == "SET SUCCESS")
    {
        setMessageType (SET_SUCCESS);
        vstrid = true;
    }
    else if (lstr == "SET FAIL")
    {
        setMessageType (SET_FAIL);
        vstrid = true;
    }
    else if (lstr == "GET RESULT")
    {
        setMessageType (GET_RESULT);
        if (idstring.empty ())
        {
            m_actionID = 0;
        }
        else
        {
            m_actionID = std::stoull (idstring);
        }
        auto svec = splitline (vstring, "=@");
        auto psep = splitline (svec[0], "()");
        if (psep.size () > 1)
        {
            m_units = psep[1];
        }
        m_field = psep[0];
        m_value = numeric_conversion (svec[1], kNullVal);
        if (svec.size () > 2)
        {
            m_time = numeric_conversion (svec[2], kNullVal);
        }
    }
    else if (lstr == "SET SCHEDULED")
    {
        setMessageType (SET_SCHEDULED);
        vstrid = true;
    }
    else if (lstr == "GET SCHEDULED")
    {
        setMessageType (GET_SCHEDULED);
        vstrid = true;
    }
    else if (lstr == "CANCEL")
    {
        setMessageType (CANCEL);
        vstrid = true;
    }
    else if (lstr == "CANCEL FAIL")
    {
        setMessageType (CANCEL_FAIL);
        vstrid = true;
    }
    else if (lstr == "CANCEL SUCCESS")
    {
        setMessageType (CANCEL_SUCCESS);
        vstrid = true;
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