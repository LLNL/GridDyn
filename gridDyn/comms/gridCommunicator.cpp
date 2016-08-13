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

#ifdef GRIDDYN_HAVE_FSKIT
#include "fskit/fskitCommunicator.h"
#endif

#include "gridCommunicator.h"

#include "communicationsCore.h"
#include "commMessage.h"
#include <string>
std::uint64_t gridCommunicator::g_counterId = 0;

gridCommunicator::gridCommunicator ()
{
  ++g_counterId;
  m_id = g_counterId;
  m_commName = "comm_" + std::to_string (m_id);
}

gridCommunicator::gridCommunicator (std::string name) : m_commName (name)
{
  ++g_counterId;
  m_id = g_counterId;
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


int gridCommunicator::transmit (std::uint64_t destID, std::shared_ptr<commMessage> message)
{
  return communicationsCore::instance ()->send (m_id, destID, message);
}

int gridCommunicator::transmit (const std::string &destName, std::shared_ptr<commMessage> message)
{
  return communicationsCore::instance ()->send (m_id, destName, message);
}

void gridCommunicator::receive (std::uint64_t sourceID, std::uint64_t destID, std::shared_ptr<commMessage> message)
{
  if (destID == m_id)
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


bool gridCommunicator::queuedMessages ()
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

std::shared_ptr<gridCommunicator> makeCommunicator (const std::string &commType,const std::string &commName, const std::uint64_t id)
{
  std::shared_ptr<gridCommunicator> ret = nullptr;
  if ((commType.empty ()) || (commType == "basic"))
    {
      if (id == 0)
        {
          ret = std::make_shared<gridCommunicator> (commName);
        }
      else
        {
          ret = std::make_shared<gridCommunicator> (commName, id);
        }
      communicationsCore::instance ()->registerCommunicator (ret);
    }
#ifdef GRIDDYN_HAVE_FSKIT
  else if (commType == "fskit")
    {
      // std::cout << "GridDyn Registering LP " << commName <<'\n';

      if (id == 0)
        {
          auto p = std::make_shared<FskitCommunicator> (commName);
          p->Register ();
          ret = p;

        }
      else
        {
          auto p = std::make_shared<FskitCommunicator> (commName,id);
          p->Register ();
          ret = p;
        }
      // return std::make_shared<FskitCommunicator>(commName);
    }
#endif
  return ret;
}
