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
#include "griddyn-config.h"

#include "gridCommunicator.h"

#include "communicationsCore.h"
#include "commMessage.h"
#include "core/factoryTemplates.h"
#include "core/gridDynExceptions.h"
#include <string>

std::atomic<std::uint64_t> gridCommunicator::g_counterId(0);

static classFactory<gridCommunicator> commFac(std::vector<std::string>{ "comm", "simple", "basic" }, "basic");

gridCommunicator::gridCommunicator ()
{
  m_id = ++g_counterId;
  m_commName = "comm_" + std::to_string (m_id);
}

gridCommunicator::gridCommunicator (std::string name) : m_commName (name)
{
	m_id=++g_counterId;
}

gridCommunicator::gridCommunicator (std::string name, std::uint64_t id) : m_id (id),m_commName (name)
{
  ++g_counterId;
}

gridCommunicator::gridCommunicator (std::uint64_t id) : m_id (id)
{
  ++g_counterId;
  m_commName = "comm_" + std::to_string (m_id);
}

std::shared_ptr<gridCommunicator> gridCommunicator::clone(std::shared_ptr<gridCommunicator> comm) const
{
	if (!comm)
	{
		comm = std::make_shared<gridCommunicator>(m_commName);
	}
	else
	{
		comm->m_commName = m_commName;
	}
	return comm;
}

void gridCommunicator::transmit (std::uint64_t destID, std::shared_ptr<commMessage> message)
{
  communicationsCore::instance ()->send (m_id, destID, message);
}

void gridCommunicator::transmit (const std::string &destName, std::shared_ptr<commMessage> message)
{
  communicationsCore::instance ()->send (m_id, destName, message);
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
          messageQueue.push (std::make_pair (sourceID, message));
        }
    }
}

void gridCommunicator::receive (std::uint64_t sourceID, const std::string &destName, std::shared_ptr<commMessage> message)
{
  if (destName == m_commName)
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
          messageQueue.push (std::make_pair (sourceID, message));
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
  auto message = std::make_shared<commMessage> (commMessage::pingMessageType);
  auto ccore = communicationsCore::instance ();
  lastPingSend = ccore->getTime ();
  ccore->send (m_id, destID, message);
}

void gridCommunicator::ping (const std::string &destName)
{
  auto message = std::make_shared<commMessage> (commMessage::pingMessageType);
  auto ccore = communicationsCore::instance ();
  lastPingSend = ccore->getTime ();
  ccore->send (m_id, destName, message);
}

double gridCommunicator::getLastPingTime () const
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
		throw(unrecognizedParameter());
	}

}
void gridCommunicator::set(const std::string &param, double val)
{
	if ((param == "id") || (param == "name"))
	{
		setID(static_cast<uint64_t>(val));
	}
	else
	{
		throw(unrecognizedParameter());
	}

}


void gridCommunicator::setFlag(const std::string & /*param*/, bool /*val*/)
{
	throw(unrecognizedParameter());
}

void gridCommunicator::initialize()
{
	communicationsCore::instance()->registerCommunicator(this);
}


void gridCommunicator::disconnect()
{
	communicationsCore::instance()->unregisterCommunicator(this);
}

std::shared_ptr<gridCommunicator> makeCommunicator (const std::string &commType,const std::string &commName, const std::uint64_t id)
{
	std::shared_ptr<gridCommunicator> ret = coreClassFactory<gridCommunicator>::instance()->createObject(commType, commName);

	if (id != 0)
	{
		ret->setID(id);
	}

	return ret;
}
