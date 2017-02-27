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
#pragma once
#ifndef GRIDDYN_COMMUNICATIONS_CORE_
#define GRIDDYN_COMMUNICATIONS_CORE_

#include "gridDynDefinitions.h"
#include <memory>
#include <unordered_map>
#include <cstdint>

class gridCommunicator;
class commMessage;

typedef std::unordered_map<std::string, gridCommunicator *> commMapString;
typedef std::unordered_map<std::uint64_t, gridCommunicator * > commMapID;

#define SEND_SUCCESS (0)
#define DESTINATION_NOT_FOUND (-1);
/** communicationCore object represents a very basic communications router for internal use inside GridDyn
@details it maintains a table and sends messages to the appropriate destination using callbacks
*/
class communicationsCore
{
public:
	/** get an instance of the singleton core */
  static std::shared_ptr<communicationsCore> instance (); 
  virtual ~communicationsCore() = default;
  /** register a communicator*/
  virtual void registerCommunicator (gridCommunicator *comm); 
  /** unregister a communicator */
  virtual void unregisterCommunicator(gridCommunicator *comm);
  /** send a message to a specified destination
  @param[in] source -the identity of the source of the message
  @param[in] dest the name of the destination
  @param[in] message - the message being sent
  @return SEND_SUCCESS if the message send was successful DESTINATION_NOT_FOUND otherwise
  */
  virtual int send (std::uint64_t source, const std::string &dest, std::shared_ptr<commMessage> message);
  /** send a message to a specified destination
  @param[in] source -the identity of the source of the message
  @param[in] dest the name of the destination
  @param[in] message - the message being sent
  @return SEND_SUCCESS if the message send was successful DESTINATION_NOT_FOUND otherwise
  */
  virtual int send (std::uint64_t source, std::uint64_t, std::shared_ptr<commMessage> message);
  coreTime getTime () const
  {
    return m_time;
  }
  void setTime (coreTime nTime)
  {
    m_time = nTime;
  }
  /** lookup an id by name
  @param[in] commName the name of the communicator
  @return the id associated with the communicator*/
  virtual std::uint64_t lookup (const std::string &commName);
  /** lookup an name by id
  @param[in] did the id associated with a communicator
  @the name of the communicator*/
  virtual std::string lookup (std::uint64_t did);

private:
	/** private constructor*/
  communicationsCore ();
  commMapString m_stringMap; //!< map containing the strings
  commMapID m_idMap; //!< map containing the id
  coreTime m_time; //!< current time of the communicator
};

#endif