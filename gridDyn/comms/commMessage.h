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

#ifndef COMM_MESSAGE_H_
#define COMM_MESSAGE_H_

#include "gridDynTypes.h"
#include <vector>
#include <cstddef>
#include <string>
#include <map>
#include <memory>
#include <type_traits>
#include <cstdint>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>



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
  virtual ~commMessage ()
  {
  }

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
  virtual ~messageFactory()
  {
  }

  virtual std::shared_ptr<commMessage> makeMessage () = 0;
  virtual std::shared_ptr<commMessage> makeMessage (std::uint32_t) = 0;
  
  virtual bool inRange (std::uint32_t) const
  {
    return true;
  }
  virtual std::uint32_t range () const
  {
    return 0xFFFFFFF0;
  }                                                            //return a very big range but leave a little room for special message codes
};
//component factory is a template class that inherits from cFactory to actually to the construction of a specific object


//TODO:: merge with the coreTypeFactory and other templates
//create a high level object factory for the coreObject class
typedef std::map<std::string, messageFactory*> mfMap;

class coreMessageFactory
{
public:
  ~coreMessageFactory ();
  static std::shared_ptr<coreMessageFactory> instance ();
  void registerFactory (const std::string  name, messageFactory *tf);
  void registerFactory (messageFactory *tf);
  std::vector<std::string> getMessageTypeNames ();
  std::shared_ptr<commMessage> createMessage (const std::string &messageType);
  std::shared_ptr<commMessage> createMessage (const std::string &messageType, std::uint32_t type);
  std::shared_ptr<commMessage> createMessage (std::uint32_t type);
  messageFactory * getFactory (const std::string &fname);
  messageFactory * getFactory (std::uint32_t type);
  bool isValidMessage (const std::string &messageType);
private:
  coreMessageFactory ();

  mfMap m_factoryMap;

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


  virtual std::shared_ptr<commMessage> makeMessage () override
  {
    std::shared_ptr<commMessage> cm = std::make_shared<Messagetype> ();
    return cm;
  }

  virtual std::shared_ptr<commMessage> makeMessage (std::uint32_t mtype) override
  {
    std::shared_ptr<commMessage> cm = std::make_shared<Messagetype> (mtype);
    return cm;
  }

  std::shared_ptr<Messagetype> makeTypeMessage () 
  {
    return(std::make_shared<Messagetype> ());
  }
  std::shared_ptr<Messagetype> makeTypeMessage (std::uint32_t mtype)
  {
    return(std::make_shared<Messagetype> (mtype));
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
