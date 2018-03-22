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

#pragma once

#include "../gridDynDefinitions.hpp"
#include <vector>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <cstdint>

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/portable_binary.hpp>
//#include <cereal/archives/json.hpp>

namespace griddyn
{
/** basic message class */
class commMessage
{
public:
	/** define the most basic of message types*/
	enum comm_message_type_t:std::uint32_t
	{
		ignoreMessageType = 0, //!< a message that can be ignored
		pingMessageType = 1,	//!< a message sending a ping
		replyMessageType = 2,	//!< a message responding to a ping
		unknownMessageType = 0xFFFFFFFF,	//!< I don't know what this message means
  };
	//default constructor
	commMessage() noexcept = default;
  /**constructor from a message type */
  commMessage (std::uint32_t type) : m_messageType (type)
  {
  }
  /** virtual destructor*/
  virtual ~commMessage() = default;
  /** get the message type*/
  std::uint32_t getMessageType() const
  {
	  return m_messageType;
  }
  /** explicitly set the message type of a message object*/
  virtual void setMessageType (std::uint32_t type)
  {
    m_messageType = type;
  }
  /** an enumeration describing some options for converting a message to a string*/
  enum comm_modifiers
  {
	  none = 0,
	  with_type=1,
  };
  /** generate a string describing the message*/
  virtual std::string to_string (int modifiers=comm_modifiers::none) const;
  /** load a message definition from a string*/
  virtual void loadString (const std::string &fromString);

  /** convert a command to a raw data bytes
  @param[out] data pointer to memory to store the command
  @param[in] buffer_size-- the size of the buffer
  @return the size of the buffer actually used
  */
  int toByteArray(char *data, size_t buffer_size) const;
  /** covert to a byte vector using a reference*/
  void to_vector(std::vector<char> &data) const;
  /** convert a command to a byte vector*/
  std::vector<char> to_vector() const;
  /** generate a command from a raw data stream*/
  virtual void fromByteArray(const char *data, size_t buffer_size);
  /** load a command from a packetized stream /ref packetize
  @return the number of bytes used
  */
  void from_vector(const std::vector<char> &data);

  /** extract the type of the message from a string
  @param[in] messageString the message string to extract the type from*/
  static std::uint32_t extractMessageType(const std::string &messageString);
protected:
	/** do an encoding on the type to place it into a string*/
	std::string encodeTypeInString() const;
private:
  std::uint32_t m_messageType = ignoreMessageType; //!< the actual type of the message

  friend class cereal::access;
  template <class Archive>
  void serialize (Archive & ar)
  {
    ar(m_messageType);
  }
};


/** class definitions for the message factories that can create the message
messageFactory is a virtual base class for message Construction functions
*/
class messageFactory
{
public:
  std::string  name; //!< the name of the factory
  /** constructor taking the name as an argument*/
  explicit messageFactory (const std::string & typeName) : name (typeName)
  {
  }
  /** virtual destructor*/
  virtual ~messageFactory() = default;

  /** build a default message*/
  virtual std::unique_ptr<commMessage> makeMessage () = 0;
  /** make a message of a specific type*/
  virtual std::unique_ptr<commMessage> makeMessage (std::uint32_t) = 0;
  /** check if a given message type is within the valid range of a specific message object*/
  virtual bool inRange (std::uint32_t) const
  {
    return true;
  }

  virtual std::uint32_t range () const
  {
    return 0xFFFFFFF0;
  }            //return a very big range but leave a little room for special message codes
};
//component factory is a template class that inherits from cFactory to actually to the construction of a specific object


//TODO:: merge with the coreTypeFactory and other templates May not be able to with the extra functions required
//create a high level object factory for the coreObject class
typedef std::unordered_map<std::string, messageFactory*> mfMap;
/** core message factory class for building messages of a specified type
*/
class coreMessageFactory
{
public:
	/** destructor*/
  ~coreMessageFactory ();
  /** get a pointer to the underlying factory*/
  static std::shared_ptr<coreMessageFactory> instance ();
  /** insert a factory in the coreMessageFactory
  @param[in] name the string used to find the message factory in subsequent operations
  @param[in] mf pointer to a message factory to store in the core factory*/
  void registerFactory (std::string name, messageFactory *mf);
  /** insert a factory in the coreMessageFactory
  @param[in] mf pointer to a message factory to store in the core factory*/
  void registerFactory (messageFactory *mf);
  /** get a list of all the valid message types*/
  std::vector<std::string> getMessageTypeNames ();
  /** build a default message of the type defined in messageType*/
  std::unique_ptr<commMessage> createMessage (const std::string &messageType);
  /** build a default message of the type defined in messageType
  @param messageType string describing the class of messages
  @param type the specific message code for the message*/
  std::unique_ptr<commMessage> createMessage (const std::string &messageType, std::uint32_t type);
  /** build a message of the specific type, deriving the general type from the valid ranges of specific types defining in the factory
  @param type the specific message code for the message*/
  std::unique_ptr<commMessage> createMessage (std::uint32_t type);
  /** get a pointer to a specific factory*/
  messageFactory * getFactory (const std::string &factoryName);
  /** get a pointer to a factory that builds a specific type of message*/
  messageFactory * getFactory (std::uint32_t type);
  /** check if a string represents a valid message class*/
  bool isValidMessage (const std::string &messageType);
private:
	/** private constructor defined in a singleton class*/
  coreMessageFactory ()=default;

  mfMap m_factoryMap; //!< the map containing the factories from a string

};

/** template for making a specific message from the factory
*/
template <class Messagetype, std::uint32_t minCodeValue, std::uint32_t maxCodeValue>
class dMessageFactory : public messageFactory
{
  static_assert (std::is_base_of<commMessage, Messagetype>::value, "factory class must have commMessage as base");
public:
  explicit dMessageFactory (const std::string &typeName) : messageFactory (typeName)
  {
    coreMessageFactory::instance ()->registerFactory (typeName, this);
  }


  virtual std::unique_ptr<commMessage> makeMessage () override
  {
    std::unique_ptr<commMessage> cm = std::make_unique<Messagetype> ();
    return cm;
  }

  virtual std::unique_ptr<commMessage> makeMessage (std::uint32_t mtype) override
  {
    std::unique_ptr<commMessage> cm = std::make_unique<Messagetype> (mtype);
    return cm;
  }
  /** default generator for a specific instantiation of a message factory*/
  std::unique_ptr<Messagetype> makeTypeMessage ()
  {
    return(std::make_unique<Messagetype> ());
  }
  /** make a message of the specific type*/
  std::unique_ptr<Messagetype> makeTypeMessage (std::uint32_t mtype)
  {
    return(std::make_unique<Messagetype> (mtype));
  }

  virtual bool inRange (std::uint32_t code) const override
  {
    return ((code >= minCodeValue) && (code <= maxCodeValue));
  }
  virtual std::uint32_t range () const override
  {
    return (maxCodeValue - minCodeValue);
  }
};

}//namespace griddyn
