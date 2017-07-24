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
#include "commManager.h"
#include "comms/Communicator.h"
#include "core/propertyBuffer.h"

namespace griddyn
{
namespace comms
{
commManager::commManager () = default;
commManager::commManager (const commManager &cm)
{
    commName = cm.commName;
    commId = cm.commId;
    commType = cm.commType;
    commDestName = cm.commDestName;
    commDestId = cm.commDestId;
    if (cm.commLink)
    {
        commLink = cm.commLink->clone ();
    }
    if (cm.commPropBuffer)
    {
        commPropBuffer = std::make_unique<griddyn::propertyBuffer> (*cm.commPropBuffer);
    }
}

commManager::commManager (commManager &&cm) = default;

commManager &commManager::operator= (const commManager &cm)
{
    commName = cm.commName;
    commId = cm.commId;
    commType = cm.commType;
    commDestName = cm.commDestName;
    commDestId = cm.commDestId;
    if (cm.commLink)
    {
        commLink = cm.commLink->clone ();
    }

    if (cm.commPropBuffer)
    {
        commPropBuffer = std::make_unique<griddyn::propertyBuffer> (*(cm.commPropBuffer));
    }
    else
    {
        commPropBuffer = nullptr;
    }
    return *this;
}
commManager &commManager::operator= (commManager &&cm) = default;

commManager::~commManager () = default;

void commManager::setName (const std::string &name) { commName = name; }
bool commManager::set (const std::string &param, const std::string &val)
{
    if ((param == "commname") || (param == "name"))
    {
        setName (val);
    }
    else if (param == "commtype")
    {
        commType = val;
    }
    else if ((param == "commdest") || (param == "destination"))
    {
        if (val.front () == '#')
        {
            commDestId = std::stoull (val.substr (1, std::string::npos));
        }
        else
        {
            commDestName = val;
        }
    }
    else if (param.compare (0, 6, "comm::") == 0)
    {
        if (commLink)
        {
            commLink->set (param.substr (6), val);
        }
        else
        {
            if (!commPropBuffer)
            {
                commPropBuffer = std::make_unique<griddyn::propertyBuffer> ();
            }
            commPropBuffer->set (param.substr (6), val);
        }
    }
    else
    {
        return false;
    }
    return true;
}
bool commManager::set (const std::string &param, double val)
{
    if ((param == "commid") || (param == "id"))
    {
        commId = static_cast<std::uint64_t> (val);
    }
    else if ((param == "commdestid") || (param == "destid"))
    {
        commDestId = static_cast<uint64_t> (val);
    }
    else if (param.compare (0, 6, "comm::") == 0)
    {
        if (commLink)
        {
            commLink->set (param.substr (6), val);
        }
        else
        {
            if (!commPropBuffer)
            {
                commPropBuffer = std::make_unique<propertyBuffer> ();
            }
            commPropBuffer->set (param.substr (6), val);
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool commManager::setFlag (const std::string &flag, bool val)
{
    if (flag.compare (0, 6, "comm::") == 0)
    {
        if (commLink)
        {
            commLink->setFlag (flag.substr (6), val);
        }
        else
        {
            if (!commPropBuffer)
            {
                commPropBuffer = std::make_unique<griddyn::propertyBuffer> ();
            }
            commPropBuffer->setFlag (flag.substr (6), val);
        }
    }
    else
    {
        return false;
    }
    return true;
}

std::shared_ptr<Communicator> commManager::build ()
{
    commLink = makeCommunicator (commType, commName, commId);
    if (commPropBuffer)
    {
        commPropBuffer->apply (commLink);
        commPropBuffer = nullptr;
    }
    return commLink;
}

void commManager::send (std::shared_ptr<commMessage> m) const
{
    if (commDestId != 0)
    {
        commLink->transmit (commDestId, std::move (m));
    }
    else if (!commDestName.empty())
    {
        commLink->transmit (commDestName, std::move (m));
    }
    else
    {
        commLink->transmit (0, std::move (m));
    }
}

}  // namespace comms
}  // namespace griddyn
