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

#ifndef _GRID_COMMUNICATOR__
#define _GRID_COMMUNICATOR__

#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include <queue>

class commMessage;


/**
@brief base class for communicator object
*/
class gridCommunicator
{

public:
  bool autoPingEnabled = true;       //!< control for automatic ping enable
  gridCommunicator ();
  gridCommunicator (std::string name);
  gridCommunicator (std::string name, std::uint64_t id);
  gridCommunicator (std::uint64_t id);

  virtual ~gridCommunicator ()
  {
  }
  /** transmit a commmessage somewhere
  * transmits a data block to somewhere Else
  * @param[in] destName the identifier of the receiving location specificed as a string
  * @param[in] message  the message to send
  */
  virtual int transmit (const std::string &destName, std::shared_ptr<commMessage> message);
  /** transmit a commmessage somewhere
  * transmits a data block to somewhere Else
  * @param[in] destID the identifier of the receiving location specificed as a id code
  * @param[in] message  the message to send
  */
  virtual int transmit (std::uint64_t destID, std::shared_ptr<commMessage> message);
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
  * @param[in] destName the identifier of the receiving location specificed as a string
  * @param[in] message  the message to send
  */
  virtual void receive (std::uint64_t sourceID, const std::string &destName, std::shared_ptr<commMessage> message);

  //ping functions
  void ping (std::uint64_t destID);
  void ping (const std::string &destName);
  double getLastPingTime () const;

  void setName (std::string newName)
  {
    m_commName = newName;
  }
  void setID (std::uint64_t newID)
  {
    m_id = newID;
  }

  std::string getName () const
  {
    return m_commName;
  }
  std::uint64_t getID () const
  {
    return m_id;
  }
  typedef std::function<void (std::uint64_t, std::shared_ptr<commMessage>)> actionTypeMessage_t;
  void registerActionMessage (actionTypeMessage_t newAction)
  {
    m_rxCallbackMessage = newAction;
  }
  bool queuedMessages ();
  std::shared_ptr<commMessage> getMessage (std::uint64_t &source);
private:
  static std::uint64_t g_counterId;       //!< id counter
  std::uint64_t m_id;           //!< individual comm id
  std::string m_commName;       //!< name for a string
  actionTypeMessage_t m_rxCallbackMessage;       //!< call back action from parent object
  double lastPingSend = 0;
  double lastReplyRX = 0;
  std::queue<std::pair<std::uint64_t, std::shared_ptr<commMessage>>> messageQueue;
};

std::shared_ptr<gridCommunicator> makeCommunicator (const std::string &commType, const std::string &commName, const std::uint64_t id);
#endif
