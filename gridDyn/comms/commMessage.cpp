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

#include "commMessage.h"

static dMessageFactory<commMessage,0,0xFFFFFFF0> dmf ("message");


std::string commMessage::toString ()
{
  if (m_messageType == pingMessageType)
    {
      return "ping";
    }
  else if (m_messageType == replyMessageType)
    {
      return "reply";
    }
  else
    {
      return std::to_string (m_messageType);
    }

}
void commMessage::loadString (const std::string &fromString)
{
  if (fromString == "ping")
    {
      m_messageType = pingMessageType;
    }
  else if (fromString == "reply")
    {
      m_messageType = replyMessageType;
    }
  else
    {
      m_messageType = std::stoi (fromString);
    }

}


std::shared_ptr<coreMessageFactory> coreMessageFactory::instance ()
{
  //can't use make shared because the constructor is private  note it is static so only created once
  static std::shared_ptr<coreMessageFactory> factory = std::shared_ptr<coreMessageFactory> (new coreMessageFactory ());
  return factory;
}

void coreMessageFactory::registerFactory (const std::string  name, messageFactory *mf)
{
  auto ret = m_factoryMap.insert (std::pair < std::string,  messageFactory *> (name, mf));
  if (ret.second == false)
    {
      ret.first->second = mf;
    }
}

void coreMessageFactory::registerFactory (messageFactory *mf)
{
  auto ret = m_factoryMap.insert (std::pair < std::string, messageFactory *> (mf->name, mf));
  if (ret.second == false)
    {
      ret.first->second = mf;
    }
}

messageFactory *coreMessageFactory::getFactory (const std::string &fname)
{
  auto mfind = m_factoryMap.find (fname);
  if (mfind != m_factoryMap.end ())
    {
      return m_factoryMap[fname];
    }
  else
    {
      return nullptr;
    }
}

//always find the narrowest range that valid
messageFactory *coreMessageFactory::getFactory (std::uint32_t type)
{
  std::uint32_t crange = 0xFFFFFFFF;
  messageFactory *cfact = nullptr;
  for (auto &fact : m_factoryMap)
    {
      if (fact.second->inRange (type))
        {
          if (fact.second->range () < crange)
            {
              crange = fact.second->range ();
              cfact = fact.second;
            }
        }
    }

  return cfact;
}

stringVec coreMessageFactory::getMessageTypeNames ()
{
  stringVec fnames;
  fnames.reserve (m_factoryMap.size ());
  for (auto fname : m_factoryMap)
    {
      fnames.push_back (fname.first);
    }
  return fnames;
}



std::shared_ptr<commMessage> coreMessageFactory::createMessage (const std::string &messType)
{
  auto mfind = m_factoryMap.find (messType);
  if (mfind != m_factoryMap.end ())
    {
      auto obj = m_factoryMap[messType]->makeMessage ();
      return obj;
    }
  else
    {
      return nullptr;
    }
}

std::shared_ptr<commMessage> coreMessageFactory::createMessage (const std::string &messType, std::uint32_t code)
{
  auto mfind = m_factoryMap.find (messType);
  if (mfind != m_factoryMap.end ())
    {
      auto obj = m_factoryMap[messType]->makeMessage (code);
      return obj;
    }
  else
    {
      return nullptr;
    }
}

//always find the narrowest range that valid
std::shared_ptr<commMessage> coreMessageFactory::createMessage (std::uint32_t type)
{
  std::uint32_t crange = 0xFFFFFFFF;
  messageFactory *cfact = nullptr;
  for (auto &fact:m_factoryMap)
    {
      if (fact.second->inRange (type))
        {
          if (fact.second->range () < crange)
            {
              crange = fact.second->range ();
              cfact = fact.second;
            }
        }
    }

  if (cfact)
    {
      return cfact->makeMessage (type);
    }
  else
    {
      return nullptr;
    }
}


bool coreMessageFactory::isValidMessage (const std::string &messType)
{
  auto mfind = m_factoryMap.find (messType);
  return (mfind != m_factoryMap.end ());
}


coreMessageFactory::coreMessageFactory ()
{

}

coreMessageFactory::~coreMessageFactory ()
{

}