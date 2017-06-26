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
#include "griddyn-config.h"

#include "Communicator.h"

#include "commMessage.h"
#include "communicationsCore.h"
#include "core/factoryTemplates.hpp"
#include <string>

namespace griddyn
{
static classFactory<Communicator> commFac (std::vector<std::string>{"comm", "simple", "basic"}, "basic");

Communicator::Communicator () : m_id (getID ()) { setName ("comm_" + std::to_string (m_id)); }
Communicator::Communicator (const std::string &name) : helperObject (name), m_id (getID ()) {}
Communicator::Communicator (const std::string &name, std::uint64_t id) : helperObject (name), m_id (id) {}
Communicator::Communicator (std::uint64_t id) : m_id (id) { setName ("comm_" + std::to_string (m_id)); }
std::shared_ptr<Communicator> Communicator::clone (std::shared_ptr<Communicator> comm) const
{
    if (!comm)
    {
        comm = std::make_shared<Communicator> (getName ());
    }
    else
    {
        comm->setName (getName ());
    }
    return comm;
}

void Communicator::transmit (std::uint64_t destID, std::shared_ptr<commMessage> message)
{
    communicationsCore::instance ()->send (m_id, destID, std::move (message));
}

void Communicator::transmit (const std::string &destName, std::shared_ptr<commMessage> message)
{
    communicationsCore::instance ()->send (m_id, destName, std::move (message));
}

void Communicator::receive (std::uint64_t sourceID, std::uint64_t destID, std::shared_ptr<commMessage> message)
{
    if ((destID == m_id) || (destID == 0))
    {
        if (autoPingEnabled)
        {
            if (message->getMessageType () == commMessage::pingMessageType)
            {
                communicationsCore::instance ()->send (m_id, sourceID, std::make_shared<commMessage> (
                                                                         commMessage::replyMessageType));
            }
            else if (message->getMessageType () == commMessage::replyMessageType)
            {
                lastReplyRX = communicationsCore::instance ()->getTime ();
                return;
            }
        }
        if (m_rxCallbackMessage)
        {
            m_rxCallbackMessage (sourceID, message);
        }
        else
        {
            messageQueue.emplace (sourceID, message);
        }
    }
}

void Communicator::receive (std::uint64_t sourceID,
                            const std::string &destName,
                            std::shared_ptr<commMessage> message)
{
    if (destName == getName ())
    {
        if (autoPingEnabled)
        {
            if (message->getMessageType () == commMessage::pingMessageType)
            {
                communicationsCore::instance ()->send (m_id, sourceID, std::make_shared<commMessage> (
                                                                         commMessage::replyMessageType));
            }
            else if (message->getMessageType () == commMessage::replyMessageType)
            {
                lastReplyRX = communicationsCore::instance ()->getTime ();
                return;
            }
        }
        if (m_rxCallbackMessage)
        {
            m_rxCallbackMessage (sourceID, message);
        }
        else
        {
            messageQueue.emplace (sourceID, message);
        }
    }
}

bool Communicator::messagesAvailable () const { return !(messageQueue.empty ()); }
std::shared_ptr<commMessage> Communicator::getMessage (std::uint64_t &source)
{
    if (messageQueue.empty ())
    {
        return nullptr;
    }
    auto msp = messageQueue.front ();
    source = msp.first;
    std::shared_ptr<commMessage> ms = msp.second;
    messageQueue.pop ();
    return ms;
}

// ping functions
void Communicator::ping (std::uint64_t destID)
{
    auto message = std::make_unique<commMessage> (commMessage::pingMessageType);
    auto ccore = communicationsCore::instance ();
    lastPingSend = ccore->getTime ();
    ccore->send (m_id, destID, std::move (message));
}

void Communicator::ping (const std::string &destName)
{
    auto message = std::make_unique<commMessage> (commMessage::pingMessageType);
    auto ccore = communicationsCore::instance ();
    lastPingSend = ccore->getTime ();
    ccore->send (m_id, destName, std::move (message));
}

coreTime Communicator::getLastPingTime () const { return lastReplyRX - lastPingSend; }
void Communicator::set (const std::string &param, const std::string &val)
{
    if ((param == "id") || (param == "name"))
    {
        setName (val);
    }
    else
    {
        helperObject::set (param, val);
    }
}
void Communicator::set (const std::string &param, double val)
{
    if ((param == "id") || (param == "name"))
    {
        setCommID (static_cast<uint64_t> (val));
    }
    else
    {
        helperObject::set (param, val);
    }
}

void Communicator::setFlag (const std::string &flag, bool val)
{
    if (flag == "autopingenabled")
    {
        autoPingEnabled = val;
    }
    else
    {
		helperObject::setFlag(flag, val);
    }
}

void Communicator::initialize () { communicationsCore::instance ()->registerCommunicator (this); }
void Communicator::disconnect () { communicationsCore::instance ()->unregisterCommunicator (this); }
std::unique_ptr<Communicator>
makeCommunicator (const std::string &commType, const std::string &commName, const std::uint64_t id)
{
    auto ret = coreClassFactory<Communicator>::instance ()->createObject (commType, commName);

    if (id != 0)
    {
        ret->setCommID (id);
    }

    return ret;
}
}  // namespace griddyn
