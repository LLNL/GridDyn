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

#include "commMessage.h"
#include "utilities/stringConversion.h"

namespace griddyn
{
static dMessageFactory<commMessage, 0, 0xFFFFFFF0> dmf ("message");

std::string commMessage::to_string (int modifiers) const
{
    std::string message = (modifiers == comm_modifiers::with_type) ? encodeTypeInString () : "";
    if (m_messageType == pingMessageType)
    {
        message += "ping";
    }
    else if (m_messageType == replyMessageType)
    {
        message += "reply";
    }
    else if (modifiers == comm_modifiers::none)
    {
        message = encodeTypeInString ();
    }
    return message;
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
        try
        {
            m_messageType = std::stoi (fromString);
        }
        catch (std::invalid_argument &)
        {
            m_messageType = unknownMessageType;
        }
    }
}

std::string commMessage::encodeTypeInString () const
{
    std::string retString ("[");
    appendInteger (retString, m_messageType);
    retString.push_back (']');
    return retString;
}

std::uint32_t commMessage::extractMessageType (const std::string &messageString)
{
    auto b1 = messageString.find_first_of ('[');
    if (b1 > 7)
    {
        return static_cast<std::uint32_t> (unknownMessageType);
    }
    auto b2 = messageString.find_first_of (']', b1);
    auto val = numeric_conversionComplete<std::uint32_t> (messageString.substr (b1 + 1, b2 - b1 - 1), 0xFFFFFFFF);
    return val;
}

std::shared_ptr<coreMessageFactory> coreMessageFactory::instance ()
{
    // can't use make shared because the constructor is private  note it is static so only created once
    static std::shared_ptr<coreMessageFactory> factory =
      std::shared_ptr<coreMessageFactory> (new coreMessageFactory ());
    return factory;
}

void coreMessageFactory::registerFactory (std::string name, messageFactory *mf)
{
    auto ret = m_factoryMap.emplace (std::move (name), mf);
    if (!ret.second)
    {
        ret.first->second = mf;
    }
}

void coreMessageFactory::registerFactory (messageFactory *mf)
{
    auto ret = m_factoryMap.emplace (mf->name, mf);
    if (!ret.second)
    {
        ret.first->second = mf;
    }
}

messageFactory *coreMessageFactory::getFactory (const std::string &factoryName)
{
    auto mfind = m_factoryMap.find (factoryName);
    if (mfind != m_factoryMap.end ())
    {
        return mfind->second;
    }
    return nullptr;
}

// always find the narrowest range that valid
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

std::vector<std::string> coreMessageFactory::getMessageTypeNames ()
{
    std::vector<std::string> typeNames;
    typeNames.reserve (m_factoryMap.size ());
    for (auto typeName : m_factoryMap)
    {
        typeNames.push_back (typeName.first);
    }
    return typeNames;
}

std::unique_ptr<commMessage> coreMessageFactory::createMessage (const std::string &messageType)
{
    auto mfind = m_factoryMap.find (messageType);
    if (mfind != m_factoryMap.end ())
    {
        auto obj = mfind->second->makeMessage ();
        return obj;
    }
    return nullptr;
}

std::unique_ptr<commMessage> coreMessageFactory::createMessage (const std::string &messageType, std::uint32_t type)
{
    auto mfind = m_factoryMap.find (messageType);
    if (mfind != m_factoryMap.end ())
    {
        auto obj = mfind->second->makeMessage (type);
        return obj;
    }
    return nullptr;
}

// always find the narrowest range that valid
std::unique_ptr<commMessage> coreMessageFactory::createMessage (std::uint32_t type)
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

    if (cfact != nullptr)
    {
        return cfact->makeMessage (type);
    }

    return nullptr;
}

bool coreMessageFactory::isValidMessage (const std::string &messageType)
{
    auto mfind = m_factoryMap.find (messageType);
    return (mfind != m_factoryMap.end ());
}

coreMessageFactory::~coreMessageFactory () = default;

}  // namespace griddyn