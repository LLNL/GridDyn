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

#ifndef _GRID_COMMUNICATOR__
#define _GRID_COMMUNICATOR__
#pragma once

#include "core/helperObject.h"
#include "core/coreDefinitions.hpp"
#include <cstdint>
#include <functional>
#include <queue>
#include <memory>

namespace griddyn
{
class commMessage;

/**
@brief base class for communicator object
*/
class Communicator:public griddyn::helperObject, public std::enable_shared_from_this<Communicator>
{

public:
  bool autoPingEnabled = true;       //!< control for automatic ping enable
  Communicator ();
  explicit Communicator (const std::string &name);
  Communicator (const std::string &name, std::uint64_t id);
  explicit Communicator (std::uint64_t id);

  /** function to clone the communicator
  @param[in] obj an object to copy data to
  @return a shared ptr to a new communicator
  */
  virtual std::shared_ptr<Communicator> clone(std::shared_ptr<Communicator> comm = nullptr) const;

  /** transmit a commMessage somewhere
  * transmits a data block to somewhere Else
  * @param[in] destName the identifier of the receiving location specified as a string
  * @param[in] message  the message to send
  */
  virtual void transmit (const std::string &destName, std::shared_ptr<commMessage> message);
  /** transmit a commMessage somewhere
  * transmits a data block to somewhere Else
  * @param[in] destID the identifier of the receiving location specified as a id code
  * @param[in] message  the message to send
  */
  virtual void transmit (std::uint64_t destID, std::shared_ptr<commMessage> message);
  /** receive data
  * received a data block and takes the appropriate action
  * @param[in] sourceID the identifier of the transmit location
  * @param[in] destID the identifier of the receiving location
  * @param[in] message  the message to send
  */
  virtual void receive (std::uint64_t sourceID, std::uint64_t destID, std::shared_ptr<commMessage> message);
  /** receive data
  * received a data block and takes the appropriate action
  * @param[in] sourceID the identifier of the transmit location
  * @param[in] destName the identifier of the receiving location specified as a string
  * @param[in] message  the message to send
  */
  virtual void receive (std::uint64_t sourceID, const std::string &destName, std::shared_ptr<commMessage> message);

  //ping functions
  void ping (std::uint64_t destID);
  void ping (const std::string &destName);
  griddyn::coreTime getLastPingTime () const;

  
  void setCommID (std::uint64_t newID)
  {
    m_id = newID;
  }

  std::uint64_t getCommID () const
  {
    return m_id;
  }
  using rxMessageCallback_t=std::function<void (std::uint64_t, std::shared_ptr<commMessage>)> ;
  void registerReceiveCallback (rxMessageCallback_t newAction)
  {
    m_rxCallbackMessage = newAction;
  }
  bool messagesAvailable () const;
  std::shared_ptr<commMessage> getMessage (std::uint64_t &source);

  virtual void initialize();

  virtual void disconnect();
  virtual void set(const std::string &param, const std::string &val) override;
  virtual void set(const std::string &param, double val) override;
  virtual void setFlag(const std::string &flag, bool val) override;
private:
  std::uint64_t m_id;           //!< individual comm id
  rxMessageCallback_t m_rxCallbackMessage;       //!< call back action from parent object
  griddyn::coreTime lastPingSend = griddyn::timeZero;
  griddyn::coreTime lastReplyRX = griddyn::timeZero;
  std::queue<std::pair<std::uint64_t, std::shared_ptr<commMessage>>> messageQueue;
};

std::unique_ptr<Communicator> makeCommunicator (const std::string &commType, const std::string &commName, const std::uint64_t id);
}//namespace griddyn
#endif
