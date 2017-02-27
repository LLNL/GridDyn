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

#ifndef COMM_MESSAGE_H_
#define COMM_MESSAGE_H_

#include "gridDynDefinitions.h"
#include <vector>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <cstdint>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>


/** basic message class */
class commMessage
{
public:
	enum comm_message_type_t
	{
		ignoreMessageType = 0,
		pingMessageType = 1,
		replyMessageType = 2,
		unknownMessageType = 0xFFFFFFFF,
  };
  commMessage ()
  {
  }
  commMessage (std::uint32_t type) : m_messageType (type)
  {
  }
  virtual ~commMessage() = default;

  std::uint32_t getMessageType(void) const
  {
	  return m_messageType;
  }
  virtual void setMessageType (std::uint32_t nType)
  {
    m_messageType = nType;
  }
  enum comm_modifiers
  {
	  none = 0,
	  with_type=1,
  };
  virtual std::string toString (int modifiers=comm_modifiers::none) const;
  virtual void loadString (const std::string &fromString);

  static std::uint32_t extractMessageType(const std::string &messageString);
protected:
	std::string encodeTypeInString() const;
private:
  std::uint32_t m_messageType = ignoreMessageType;

  //add the boost serialization stuff
  friend class boost::serialization::access;
  template <class Archive>
  void serialize (Archive & ar, const unsigned int /*version*/)
  {
    ar & m_messageType;
  }
};


// class definitions for the message factories that can create the message
//cFactory is a virtual base class for message Construction functions
class messageFactory
{
public:
  std::string  name;
  messageFactory (const std::string & typeName) : name (typeName)
  {
  }
  virtual ~messageFactory() = default;

  virtual std::unique_ptr<commMessage> makeMessage () = 0;
  virtual std::unique_ptr<commMessage> makeMessage (std::uint32_t) = 0;
  
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
  ~coreMessageFactory ();
  static std::shared_ptr<coreMessageFactory> instance ();
  void registerFactory (std::string name, messageFactory *tf);
  void registerFactory (messageFactory *tf);
  std::vector<std::string> getMessageTypeNames ();
  std::unique_ptr<commMessage> createMessage (const std::string &messageType);
  std::unique_ptr<commMessage> createMessage (const std::string &messageType, std::uint32_t type);
  std::unique_ptr<commMessage> createMessage (std::uint32_t type);
  messageFactory * getFactory (const std::string &fname);
  messageFactory * getFactory (std::uint32_t type);
  bool isValidMessage (const std::string &messageType);
private:
	/** private constructor defined in a singleton class*/
  coreMessageFactory ();

  mfMap m_factoryMap; //!< the map containing the factories from a string

};

/** template for making a specific message from the factory
*/
template <class Messagetype, std::uint32_t minCodeValue, std::uint32_t maxCodeValue>
class dMessageFactory : public messageFactory
{
  static_assert (std::is_base_of<commMessage, Messagetype>::value, "factory class must have commMessage as base");
public:
  dMessageFactory (const std::string &typeName) : messageFactory (typeName)
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

  std::unique_ptr<Messagetype> makeTypeMessage () 
  {
    return(std::make_unique<Messagetype> ());
  }
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



#endif
