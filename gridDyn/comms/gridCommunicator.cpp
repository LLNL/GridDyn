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
#include "griddyn-config.h"

#include "gridCommunicator.h"

#include "communicationsCore.h"
#include "commMessage.h"
#include "core/factoryTemplates.h"
#include "core/coreExceptions.h"
#include <string>

static classFactory<gridCommunicator> commFac(std::vector<std::string>{ "comm", "simple", "basic" }, "basic");

gridCommunicator::gridCommunicator (): m_id(getID())
{
  setName("comm_" + std::to_string (m_id));
}

gridCommunicator::gridCommunicator (const std::string &name) : helperObject (name),m_id(getID())
{

}

gridCommunicator::gridCommunicator (const std::string &name, std::uint64_t id) :helperObject(name), m_id (id)
{
}

gridCommunicator::gridCommunicator (std::uint64_t id) : m_id (id)
{
  setName("comm_" + std::to_string (m_id));
}

std::shared_ptr<gridCommunicator> gridCommunicator::clone(std::shared_ptr<gridCommunicator> comm) const
{
	if (!comm)
	{
		comm = std::make_shared<gridCommunicator>(getName());
	}
	else
	{
		comm->setName(getName());
	}
	return comm;
}

void gridCommunicator::transmit (std::uint64_t destID, std::shared_ptr<commMessage> message)
{
  communicationsCore::instance ()->send (m_id, destID, std::move(message));
}

void gridCommunicator::transmit (const std::string &destName, std::shared_ptr<commMessage> message)
{
  communicationsCore::instance ()->send (m_id, destName, std::move(message));
}

void gridCommunicator::receive (std::uint64_t sourceID, std::uint64_t destID, std::shared_ptr<commMessage> message)
{
  if ((destID == m_id)||(destID==0))
    {
      if (autoPingEnabled)
        {
          if (message->getMessageType () == commMessage::pingMessageType)
            {
              communicationsCore::instance ()->send (m_id, sourceID, std::make_shared<commMessage> (commMessage::replyMessageType));
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

void gridCommunicator::receive (std::uint64_t sourceID, const std::string &destName, std::shared_ptr<commMessage> message)
{
  if (destName == getName())
    {
      if (autoPingEnabled)
        {
          if (message->getMessageType () == commMessage::pingMessageType)
            {
              communicationsCore::instance ()->send (m_id, sourceID, std::make_shared<commMessage> (commMessage::replyMessageType));
            }
          else if (message->getMessageType () == commMessage::replyMessageType)
            {
              lastReplyRX = communicationsCore::instance ()->getTime ();
              return;
            }
        }
      if (m_rxCallbackMessage)
        {
          m_rxCallbackMessage (sourceID,message);
        }
      else
        {
          messageQueue.emplace (sourceID, message);
        }
    }
}


bool gridCommunicator::messagesAvailable () const
{
  return !(messageQueue.empty ());
}

std::shared_ptr<commMessage> gridCommunicator::getMessage (std::uint64_t &source)
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


//ping functions
void gridCommunicator::ping (std::uint64_t destID)
{
  auto message = std::make_unique<commMessage> (commMessage::pingMessageType);
  auto ccore = communicationsCore::instance ();
  lastPingSend = ccore->getTime ();
  ccore->send (m_id, destID, std::move(message));
}

void gridCommunicator::ping (const std::string &destName)
{
  auto message = std::make_unique<commMessage> (commMessage::pingMessageType);
  auto ccore = communicationsCore::instance ();
  lastPingSend = ccore->getTime ();
  ccore->send (m_id, destName, std::move(message));
}

coreTime gridCommunicator::getLastPingTime () const
{
  return lastReplyRX - lastPingSend;
}


void gridCommunicator::set(const std::string &param, const std::string &val)
{
	if ((param == "id")||(param=="name"))
	{
		setName(val);
	}
	else
	{
		helperObject::set(param,val);
	}

}
void gridCommunicator::set(const std::string &param, double val)
{
	if ((param == "id") || (param == "name"))
	{
		setCommID(static_cast<uint64_t>(val));
	}
	else
	{
		helperObject::set(param, val);
	}

}


void gridCommunicator::setFlag(const std::string &flag, bool val)
{
	if (flag == "autopingenabled")
	{
		autoPingEnabled = val;
	}
	else
	{
		throw(unrecognizedParameter());
	}
	
}

void gridCommunicator::initialize()
{
	communicationsCore::instance()->registerCommunicator(this);
}


void gridCommunicator::disconnect()
{
	communicationsCore::instance()->unregisterCommunicator(this);
}

std::unique_ptr<gridCommunicator> makeCommunicator (const std::string &commType,const std::string &commName, const std::uint64_t id)
{
	auto ret = coreClassFactory<gridCommunicator>::instance()->createObject(commType, commName);

	if (id != 0)
	{
		ret->setCommID(id);
	}

	return ret;
}
