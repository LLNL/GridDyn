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

#ifndef FSKIT_COMMUNICATOR_H
#define FSKIT_COMMUNICATOR_H

#include "comms/gridCommunicator.h"
#include <fskit/logical-process.h>
#include <fskit/event-message.h>
#include <boost/serialization/serialization.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

class FskitCommunicator
  : public gridCommunicator,
    public fskit::LogicalProcess,
    public std::enable_shared_from_this<FskitCommunicator>
{
public:
  class FskitCommunicatorMessage : public fskit::EventMessage
  {
public:
    FskitCommunicatorMessage ()
    {
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize (Archive & ar, const int version)
    {
    }
  };

  FskitCommunicator ();
  FskitCommunicator (std::string name);
  FskitCommunicator (std::string m_name, std::uint64_t id);

  virtual ~FskitCommunicator ()
  {
  }

  virtual void transmit (const std::string &destName, std::shared_ptr<commMessage> message) override;

  virtual void transmit (std::uint64_t destID, std::shared_ptr<commMessage> message) override;

  void ProcessEventMessage (const fskit::EventMessage& eventMessage);
  virtual void initialize () override; //!< XXX: Must be called by client
  virtual void disconnect() override;
private:
  void doTransmit (std::shared_ptr<commMessage> message);
};


#endif /* FSKIT_COMMUNICATOR_H */
