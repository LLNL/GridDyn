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

#include "griddyn-config.h"
#ifdef GRIDDYN_HAVE_FSKIT

#include "gridDyn.h" // for gridDynSimulation
#include "fskitCommunicator.h"
#include "gridDynFederatedScheduler.h"
#include "eventQueue.h"
#include "gridEvent.h"
#include "comms/relayMessage.h"
#include "comms/controlMessage.h"
#include "comms/commMessage.h"
#include <fskit/granted-time-window-scheduler.h>

#include <iostream>

#define GRIDDYN_RANK 0
#define NS3_RANK 1
#define NS3_SIMULATOR_NAME "ns3"

FskitCommunicator::FskitCommunicator (std::string id)
  : gridCommunicator (id),
    LogicalProcess (
      fskit::GlobalLogicalProcessId (
        fskit::FederatedSimulatorId ("gridDyn"), GRIDDYN_RANK,
        fskit::LocalLogicalProcessId (id)))
{
  assert (GriddynFederatedScheduler::IsFederated ());

  // XXX: shared_from_this() cannot be used in a class
  // constructor because of the precondition that a
  // shared_ptr be created to manage this object prior
  // to calls to shared_from_this().
  //GriddynFederatedScheduler::GetScheduler ()->RegisterLogicalProcess (
  //  shared_from_this ());
}

FskitCommunicator::FskitCommunicator (std::string name, std::uint64_t id)  : gridCommunicator (name,id),
                                                                             LogicalProcess (
                                                                               fskit::GlobalLogicalProcessId (
                                                                                 fskit::FederatedSimulatorId ("gridDyn"), GRIDDYN_RANK,
                                                                                 fskit::LocalLogicalProcessId (name)))
{
  assert (GriddynFederatedScheduler::IsFederated ());

  // XXX: shared_from_this() cannot be used in a class
  // constructor because of the precondition that a
  // shared_ptr be created to manage this object prior
  // to calls to shared_from_this().
  //GriddynFederatedScheduler::GetScheduler ()->RegisterLogicalProcess (
  //  shared_from_this ());
}


void
FskitCommunicator::initialize ()
{
  assert (GriddynFederatedScheduler::GetScheduler () != 0);
  // XXX: This assumes that clients are using this class instance
  // as a shared_ptr.
  GriddynFederatedScheduler::GetScheduler ()->RegisterLogicalProcess (
    shared_from_this ());
}

void FskitCommunicator::disconnect()
{

}

// TODO RARS - This method needs to be refactored
void
FskitCommunicator::ProcessEventMessage (const fskit::EventMessage& eventMessage)
{
  auto gds = gridDynSimulation::getInstance ();

  // Convert fskit time (ns) to Griddyn time (s)
  double griddynTime = eventMessage.GetTime ().GetRaw () * 1.0E-9;

  switch (eventMessage.GetEventType ())
    {
    case commMessage::ignoreMessageType:
    case commMessage::pingMessageType:
    case commMessage::replyMessageType:
      {
        std::shared_ptr<commMessage> m = std::make_shared<commMessage> ();
        eventMessage.Unpack (*m);
        auto event = std::make_shared<functionEventAdapter> ([this, m]() {
        receive (0,getName (),m);
        return change_code::no_change;
      });

        event->m_nextTime = griddynTime;
        gds->add (event);
      }
      break;
    case controlMessage::GET:
    case controlMessage::GET_MULTIPLE:
    case controlMessage::GET_PERIODIC:
    case controlMessage::GET_RESULT:
    case controlMessage::GET_RESULT_MULTIPLE:
    case controlMessage::CANCEL:
    case controlMessage::CANCEL_SUCCESS:
    case controlMessage::SET_SUCCESS:
    case controlMessage::SET_FAIL:
    case controlMessage::SET_SCHEDULED:
    case controlMessage::GET_SCHEDULED:
    case controlMessage::CANCEL_FAIL:
      {
        std::shared_ptr<controlMessage> cm = std::make_shared<controlMessage> ();
        eventMessage.Unpack (*cm);
        std::shared_ptr<commMessage> m = std::dynamic_pointer_cast<commMessage> (cm);

        auto event = std::make_shared<functionEventAdapter> ([this, m]() {
        receive (0,getName (),m);
        return change_code::no_change;
      });

        event->m_nextTime = griddynTime;
        gds->add (event);
      }
      break;
    case controlMessage::SET:
      {
        std::shared_ptr<controlMessage> cm = std::make_shared<controlMessage> ();
        eventMessage.Unpack (*cm);
        std::shared_ptr<commMessage> m = std::dynamic_pointer_cast<commMessage> (cm);

        auto event = std::make_shared<functionEventAdapter> ([this, m]() {
        receive (0,getName (),m);
        return change_code::parameter_change;
      });

        event->m_nextTime = griddynTime;
        gds->add (event);
      }
      break;
    case relayMessage::NO_EVENT:
    case relayMessage::LOCAL_FAULT_EVENT:
    case relayMessage::REMOTE_FAULT_EVENT:
    case relayMessage::BREAKER_TRIP_EVENT:
    case relayMessage::BREAKER_CLOSE_EVENT:
    case relayMessage::LOCAL_FAULT_CLEARED:
    case relayMessage::REMOTE_FAULT_CLEARED:
    case relayMessage::BREAKER_CLOSE_COMMAND:
    case relayMessage::BREAKER_OOS_COMMAND:
      {
        std::shared_ptr<relayMessage> rm = std::make_shared<relayMessage> ();
        eventMessage.Unpack (*rm);
        std::shared_ptr<commMessage> m = std::dynamic_pointer_cast<commMessage> (rm);
        auto event = std::make_shared<functionEventAdapter> ([this, m]() {
        receive (0,getName (),m);
        return change_code::no_change;
      });

        event->m_nextTime = griddynTime;
        gds->add (event);
      }
      break;
    case relayMessage::BREAKER_TRIP_COMMAND:
      {
        std::shared_ptr<relayMessage> rm = std::make_shared<relayMessage> ();
        eventMessage.Unpack (*rm);
        std::shared_ptr<commMessage> m = std::dynamic_pointer_cast<commMessage> (rm);
        auto event = std::make_shared<functionEventAdapter> ([this, m]() {
        receive (0,getName (),m);
        return change_code::parameter_change;
      });

        event->m_nextTime = griddynTime;
        gds->add (event);
      }
      break;
    default:
      // TODO RARS - What do we do here?
      assert (false);
      break;
    }

}

int FskitCommunicator::transmit (const std::string & /*destName*/, std::shared_ptr<commMessage> message)
{
  doTransmit (message);
  return 0;
}

int FskitCommunicator::transmit (std::uint64_t /*destID*/, std::shared_ptr<commMessage> message)
{
  doTransmit (message);
  return 0;
}

void
FskitCommunicator::doTransmit ( std::shared_ptr<commMessage> message)
{
  std::shared_ptr<fskit::GrantedTimeWindowScheduler> scheduler (
    GriddynFederatedScheduler::GetScheduler ());

  gridDynSimulation* inst = gridDynSimulation::getInstance ();
  fskit::Time currentTime;
  if (inst != 0)
    {
      double currentTimeSeconds = inst->getCurrentTime ();
      //std::cout << "GridDyn currentTimeSeconds = " << currentTimeSeconds << std::endl;
      currentTime = fskit::Time (currentTimeSeconds * 1e9); // scale current time to nanoseconds
    }
  else
    {
      currentTime = scheduler->Next (); // Incorrect.
    }

  // XXX: The time increment needs to be retrieved from somewhere. Griddyn XML file?
  fskit::Time increment (1e7); // 1/100 of a second, in nanoseconds
                               // Think about what this delay is representing: microprocessor data transfer to comm layer, etc.

  //std::cout << "fskit currentTime = " << currentTime << std::endl;
  //std::cout << "fskit increment   = " << increment << std::endl;

  //std::cout << "FskitCommunicator::doTransmit() - sending messageType "
  //  << messageType << " to ns3" << std::endl;

  //std::cout << "GridDyn About to Send: MessageType = " << message->getMessageType () << std::endl;
  switch (message->getMessageType ())
    {
    case commMessage::ignoreMessageType:
    case commMessage::pingMessageType:
    case commMessage::replyMessageType:
      {
        //std::cout << "GridDyn About to Send: MessageType = " << message->getMessageType () << std::endl;

        scheduler->SendEventMessage (
          fskit::GlobalLogicalProcessId (
            fskit::FederatedSimulatorId (NS3_SIMULATOR_NAME),
            NS3_RANK,
            fskit::LocalLogicalProcessId (getName ())
            ),
          currentTime + increment,
          message->getMessageType (),
          *message
          );
      }
      break;
    case controlMessage::SET:
    case controlMessage::GET:
    case controlMessage::GET_MULTIPLE:
    case controlMessage::GET_PERIODIC:
    case controlMessage::SET_SUCCESS:
    case controlMessage::SET_FAIL:
    case controlMessage::GET_RESULT:
    case controlMessage::GET_RESULT_MULTIPLE:
    case controlMessage::SET_SCHEDULED:
    case controlMessage::GET_SCHEDULED:
    case controlMessage::CANCEL:
    case controlMessage::CANCEL_SUCCESS:
    case controlMessage::CANCEL_FAIL:
      {
        std::shared_ptr<controlMessage> m = std::dynamic_pointer_cast<controlMessage> (message);

        //std::cout << "GridDyn About to Send: MessageType = " << m->getMessageType ()
        //<< " Field = " << m->m_field << std::endl;

        scheduler->SendEventMessage (
          fskit::GlobalLogicalProcessId (
            fskit::FederatedSimulatorId (NS3_SIMULATOR_NAME),
            NS3_RANK,
            fskit::LocalLogicalProcessId (getName ())
            ),
          currentTime + increment,
          message->getMessageType (),
          *m
          );
      }
      break;
    case relayMessage::NO_EVENT:
    case relayMessage::LOCAL_FAULT_EVENT:
    case relayMessage::REMOTE_FAULT_EVENT:
    case relayMessage::BREAKER_TRIP_EVENT:
    case relayMessage::BREAKER_CLOSE_EVENT:
    case relayMessage::LOCAL_FAULT_CLEARED:
    case relayMessage::REMOTE_FAULT_CLEARED:
    case relayMessage::BREAKER_TRIP_COMMAND:
    case relayMessage::BREAKER_CLOSE_COMMAND:
    case relayMessage::BREAKER_OOS_COMMAND:
      {
        std::shared_ptr<relayMessage> m = std::dynamic_pointer_cast<relayMessage> (message);

        scheduler->SendEventMessage (
          fskit::GlobalLogicalProcessId (
            fskit::FederatedSimulatorId (NS3_SIMULATOR_NAME),
            NS3_RANK,
            fskit::LocalLogicalProcessId (getName ())
            ),
          currentTime + increment,
          message->getMessageType (),
          *m
          );
      }
      break;
    default:
      // TODO RARS - What do we do here?
      assert (false);
      break;
    }

}


#endif /* GRIDDYN_HAVE_FSKIT */
