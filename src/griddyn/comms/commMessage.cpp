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

#include "commMessage.h"
#include "utilities/stringConversion.h"

#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

using archiver = cereal::PortableBinaryOutputArchive;

using retriever = cereal::PortableBinaryInputArchive;

namespace griddyn
{

REGISTER_MESSAGE_TYPE (m1, "IGNORE", commMessage::ignoreMessageType);
REGISTER_MESSAGE_TYPE (m2, "ping", commMessage::pingMessageType);
REGISTER_MESSAGE_TYPE (m3, "reply", commMessage::replyMessageType);
REGISTER_MESSAGE_TYPE (m4, "NO EVENT", commMessage::NO_EVENT);
REGISTER_MESSAGE_TYPE (m5, "LOCAL FAULT", commMessage::LOCAL_FAULT_EVENT);
REGISTER_MESSAGE_TYPE (m6, "REMOTE FAULT", commMessage::REMOTE_FAULT_EVENT);
REGISTER_MESSAGE_TYPE (m7, "BREAKER TRIP", commMessage::BREAKER_TRIP_EVENT);
REGISTER_MESSAGE_TYPE (m8, "BREAKER CLOSE", commMessage::BREAKER_CLOSE_EVENT);
REGISTER_MESSAGE_TYPE (m9, "LOCAL FAULT CLEARED", commMessage::LOCAL_FAULT_CLEARED);
REGISTER_MESSAGE_TYPE (m10, "REMOTE FAULT CLEARED", commMessage::REMOTE_FAULT_CLEARED);
REGISTER_MESSAGE_TYPE (m11, "BREAKER TRIP COMMAND", commMessage::BREAKER_TRIP_COMMAND);
REGISTER_MESSAGE_TYPE (m12, "BREAKER CLOSE COMMAND", commMessage::BREAKER_CLOSE_COMMAND);
REGISTER_MESSAGE_TYPE (m13, "BREAKER OOS COMMAND", commMessage::BREAKER_OOS_COMMAND);
REGISTER_MESSAGE_TYPE (m14, "ALARM TRIGGER EVENT", commMessage::ALARM_TRIGGER_EVENT);
REGISTER_MESSAGE_TYPE (m15, "ALARM CLEARED EVENT", commMessage::ALARM_CLEARED_EVENT);


commMessage::commMessage(std::uint32_t type) : m_messageType(type)
{
    payload = corePayloadFactory::instance().createPayload(type);
    ptype = payloadType_t::shared;
}

commMessage::commMessage(std::uint32_t type, std::uint32_t messagecode) : m_messageType(type), code(messagecode)
{
    payload = corePayloadFactory::instance().createPayload(type);
    ptype = payloadType_t::shared;
}

std::string commMessage::to_string() const
{

    std::string message = MessageTypeRegistry::instance().getTypeString(m_messageType);

    if (code != 0xFFFF'FFFF)
    {
        message.push_back('[');
        appendInteger(message, code);
        message.push_back(']');
    }
    if (payload)
    {
        message.push_back(':');
        message.append(payload->to_string(m_messageType,code));
    }
    return message;
}
void commMessage::from_string(const std::string &fromString)
{
    auto ip = fromString.find_first_of(":[");
    if (ip == std::string::npos)
    {
        m_messageType = MessageTypeRegistry::instance().getType(fromString);
        code = 0xFFFF'FFFF;
        return;
    }
    m_messageType = MessageTypeRegistry::instance().getType(fromString.substr(0, ip - 1));
    if (fromString[ip] == '[')
    {
        auto end = fromString.find_first_of(']', ip + 1);
        code = numeric_conversion<std::uint32_t>(fromString.substr(ip + 1, end - ip - 1), 0xFFFFFFFF);
        ip = end + 1;
        if (ip >= fromString.size())
        {
            return;
        }
    }
    else
    {
        code = 0xFFFF'FFFF;
    }
    payload = corePayloadFactory::instance().createPayload(m_messageType);
    payload->from_string(m_messageType,code,fromString, ip + 1);
}

int commMessage::toByteArray(char *data, size_t buffer_size) const
{
    if ((data == nullptr) || (buffer_size == 0))
    {
        return -1;
    }
    boost::iostreams::basic_array_sink<char> sr(data, buffer_size);
    boost::iostreams::stream<boost::iostreams::basic_array_sink<char>> s(sr);

    archiver oa(s);
    try
    {
        save(oa);
        return static_cast<int> (boost::iostreams::seek(s, 0, std::ios_base::cur));
    }
    catch (const std::ios_base::failure &)
    {
        return -1;
    }
}

std::string commMessage::to_datastring() const
{
    std::string data;
    boost::iostreams::back_insert_device<std::string> inserter(data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
    archiver oa(s);

    save(oa);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush();
    return data;
}

std::vector<char> commMessage::to_vector() const
{
    std::vector<char> data;
    boost::iostreams::back_insert_device<std::vector<char>> inserter(data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::vector<char>>> s(inserter);
    archiver oa(s);

    save(oa);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush();
    return data;
}

void commMessage::to_vector(std::vector<char> &data) const
{
    data.clear();
    boost::iostreams::back_insert_device<std::vector<char>> inserter(data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::vector<char>>> s(inserter);
    archiver oa(s);

    save(oa);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush();
}

void commMessage::to_datastring(std::string &data) const
{
    data.clear();

    boost::iostreams::back_insert_device<std::string> inserter(data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
    archiver oa(s);

    save(oa);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush();
}

void commMessage::fromByteArray(const char *data, size_t buffer_size)
{
    boost::iostreams::basic_array_source<char> device(data, buffer_size);
    boost::iostreams::stream<boost::iostreams::basic_array_source<char>> s(device);
    retriever ia(s);
    try
    {
        load(ia);
    }
    catch (const cereal::Exception &ce)
    {
        m_messageType = unknownMessageType;
    }
}


void commMessage::from_datastring(const std::string &data) { fromByteArray(data.data(), data.size()); }

void commMessage::from_vector(const std::vector<char> &data) { fromByteArray(data.data(), data.size()); }

static std::map<std::string, std::uint32_t> alarmCodeMap{
    { "overcurrent", OVERCURRENT_ALARM },{ "undercurrent", UNDERCURRENT_ALARM },
    { "overvoltage", OVERVOLTAGE_ALARM },{ "undervoltage", UNDERVOLTAGE_ALARM },
    { "temperature_alarm1", TEMPERATURE_ALARM1 },{ "temperature", TEMPERATURE_ALARM1 },
    { "temperature_alarm2", TEMPERATURE_ALARM2 },{ "temperature2", TEMPERATURE_ALARM2 },
};

std::uint32_t getAlarmCode(const std::string &alarmStr)
{
    auto fnd = alarmCodeMap.find(alarmStr);
    if (fnd != alarmCodeMap.end())
    {
        return fnd->second;
    }
    return 0xFFFFFFFF;
}

MessageTypeRegistry& MessageTypeRegistry::instance()
{
    // can't use make shared because the constructor is private  note it is static so only created once
    static MessageTypeRegistry registry;
    return registry;
}

void MessageTypeRegistry::registerType(const std::string &name, std::uint32_t type)
{
    typeMapA[name] = type;
    typeMapB[type] = name;
}

uint32_t MessageTypeRegistry::getType(const std::string &name) const
{
    auto fnd = typeMapA.find(name);

    if (fnd != typeMapA.end())
    {
        return fnd->second;
    }
    if (name.compare(0, 5, "type_") == 0)
    {
        return std::stoul(name.substr(5));
    }
    return commMessage::unknownMessageType;
}

    std::string MessageTypeRegistry::getTypeString(int32_t type) const
    {
        auto fnd = typeMapB.find(type);

        if (fnd != typeMapB.end())
        {
            return fnd->second;
        }
        auto ret = std::string("type_");
        appendInteger(ret, type);
        return ret;
    }


    corePayloadFactory& corePayloadFactory::instance ()
{
    // can't use make shared because the constructor is private  note it is static so only created once
    static corePayloadFactory factory;
    return factory;
}

void corePayloadFactory::registerFactory (std::string name, payloadFactory *mf)
{
    auto ret = m_factoryMap.emplace (std::move (name), mf);
    if (!ret.second)
    {
        ret.first->second = mf;
    }
}

void corePayloadFactory::registerFactory (payloadFactory *mf)
{
    auto ret = m_factoryMap.emplace (mf->name, mf);
    if (!ret.second)
    {
        ret.first->second = mf;
    }
}

payloadFactory *corePayloadFactory::getFactory (const std::string &factoryName)
{
    auto mfind = m_factoryMap.find (factoryName);
    if (mfind != m_factoryMap.end ())
    {
        return mfind->second;
    }
    return nullptr;
}

// always find the narrowest range that valid
payloadFactory *corePayloadFactory::getFactory (std::uint32_t type)
{
    std::uint32_t crange = 0xFFFFFFFF;
    payloadFactory *cfact = nullptr;
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

std::vector<std::string> corePayloadFactory::getPayloadTypeNames ()
{
    std::vector<std::string> typeNames;
    typeNames.reserve (m_factoryMap.size ());
    for (auto typeName : m_factoryMap)
    {
        typeNames.push_back (typeName.first);
    }
    return typeNames;
}

std::shared_ptr<CommPayload> corePayloadFactory::createPayload (const std::string &messageType)
{
    auto mfind = m_factoryMap.find (messageType);
    if (mfind != m_factoryMap.end ())
    {
        auto obj = mfind->second->makePayload ();
        return obj;
    }
    return nullptr;
}

std::shared_ptr<CommPayload> corePayloadFactory::createPayload (const std::string &messageType, std::uint32_t type)
{
    auto mfind = m_factoryMap.find (messageType);
    if (mfind != m_factoryMap.end ())
    {
        auto obj = mfind->second->makePayload ();
        return obj;
    }
    return createPayload(type);
}

// always find the narrowest range that valid
std::shared_ptr<CommPayload> corePayloadFactory::createPayload (std::uint32_t type)
{
    std::uint32_t crange = 0xFFFFFFFF;
    payloadFactory *cfact = nullptr;
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
        return cfact->makePayload ();
    }

    return nullptr;
}

bool corePayloadFactory::isValidMessage (const std::string &messageType)
{
    auto mfind = m_factoryMap.find (messageType);
    return (mfind != m_factoryMap.end ());
}

}  // namespace griddyn