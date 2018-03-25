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
#include "fskitCommunicator.h"
#include "gridDynSimulation.h" // for gridDynSimulation
#include "gridDynFederatedScheduler.h"
#include "events/eventQueue.h"
#include "events/Event.h"
#include "comms/relayMessage.h"
#include "comms/controlMessage.h"
#include "comms/commMessage.h"
#include <fskit/granted-time-window-scheduler.h>

#include <iostream>

#define GRIDDYN_RANK 0
#define NS3_RANK 1
#define NS3_SIMULATOR_NAME "ns3"

// Why is default ctor needed?   
FskitCommunicator::FskitCommunicator () :
  LogicalProcess (
                  fskit::GlobalLogicalProcessId (
                                                 fskit::FederatedSimulatorId ("RAII-is-broken"), GRIDDYN_RANK,
                                                 fskit::LocalLogicalProcessId ("RAII-is-broken")))
{
}

FskitCommunicator::FskitCommunicator (std::string id)
  : Communicator (id),
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

FskitCommunicator::FskitCommunicator (std::string name, std::uint64_t id)  : Communicator (name,id),
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
    std::static_pointer_cast<FskitCommunicator>(Communicator::shared_from_this()));
}

void FskitCommunicator::disconnect()
{

}

// TODO RARS - This method needs to be refactored
void
FskitCommunicator::ProcessEventMessage (const fskit::EventMessage& eventMessage)
{
  auto gds = griddyn::gridDynSimulation::getInstance ();

  // Convert fskit time (ns) to Griddyn time (s)
  double griddynTime = eventMessage.GetTime ().GetRaw () * 1.0E-9;

  //auto m = coreMessageFactory::instance()->createMessage(eventMessage.GetEventType());
  auto m = std::make_shared<griddyn::comms::commMessage>(eventMessage.GetEventType());
  assert(m);
  eventMessage.Unpack(*m);
  std::string name = getName();
//PT put in #def switch for 4.9 vs future 
/*
  //using lambda capture to move the message to the lambda
  // unique ptr capture with message{std::move(m)} failed on gcc 4.9.3; build shared and capture the shared ptr.
  auto event = std::make_unique<functionEventAdapter>([this, message](){
      receive(0, getName(), message);
      return change_code::no_change;
  });
*/


  std::shared_ptr<griddyn::commMessage> message = std::move(m);
  //using lambda capture to move the message to the lambda
  // unique ptr capture with message{std::move(m)} failed on gcc 4.9.3; build shared and capture the shared ptr.
  auto event = std::make_unique<griddyn::functionEventAdapter>([this, message](){
      receive(0, getName(), message);
      return griddyn::change_code::no_change;
  });
  event->m_nextTime = griddynTime;
  gds->add(std::shared_ptr<griddyn::eventAdapter>(std::move(event)));

  /*
  switch (eventMessage.GetEventType ())
    {
    case commMessage::ignoreMessageType:
    case commMessage::pingMessageType:
    case commMessage::replyMessageType:
      {
        std::shared_ptr<commMessage> m = std::make_shared<commMessage> ();
        eventMessage.Unpack (*m);
        auto event = std::make_unique<functionEventAdapter> ([this, m]() {
        receive (0,getName (),m);
        return change_code::no_change;
      });

        event->m_nextTime = griddynTime;
        gds->add (std::move(event));
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

        auto event = std::make_unique<functionEventAdapter> ([this, m]() {
        receive (0,getName (),m);
        return change_code::no_change;
      });

        event->m_nextTime = griddynTime;
        gds->add (std::move(event));
      }
      break;
    case controlMessage::SET:
      {
        std::shared_ptr<controlMessage> cm = std::make_shared<controlMessage> ();
        eventMessage.Unpack (*cm);
        std::shared_ptr<commMessage> m = std::dynamic_pointer_cast<commMessage> (cm);

        auto event = std::make_unique<functionEventAdapter> ([this, m]() {
        receive (0,getName (),m);
        return change_code::parameter_change;
      });

        event->m_nextTime = griddynTime;
        gds->add (std::move(event));
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
        auto event = std::make_unique<functionEventAdapter> ([this, m]() {
        receive (0,getName (),m);
        return change_code::no_change;
      });

        event->m_nextTime = griddynTime;
        gds->add (std::move(event));
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
	*/
}

void FskitCommunicator::transmit (const std::string & /*destName*/, std::shared_ptr<griddyn::commMessage> message)
{
  doTransmit (message);
}

void FskitCommunicator::transmit (std::uint64_t /*destID*/, std::shared_ptr<griddyn::commMessage> message)
{
  doTransmit (message);
}

void
FskitCommunicator::doTransmit ( std::shared_ptr<griddyn::commMessage> message)
{
  std::shared_ptr<fskit::GrantedTimeWindowScheduler> scheduler (
    GriddynFederatedScheduler::GetScheduler ());

  griddyn::gridDynSimulation* inst = griddyn::gridDynSimulation::getInstance ();
  fskit::Time currentTime;
  if (inst != 0)
    {
      double currentTimeSeconds = inst->getSimulationTime ();
      currentTime = fskit::Time (currentTimeSeconds * 1e9); // scale current time to nanoseconds
    }
  else
    {
      currentTime = scheduler->Next (); // Incorrect.
    }

  // XXX: The time increment needs to be retrieved from somewhere. Griddyn XML file?
  fskit::Time increment (1e7); // 1/100 of a second, in nanoseconds
                               // Think about what this delay is representing: microprocessor data transfer to comm layer, etc.

  switch (message->getMessageType ())
    {
    case griddyn::commMessage::ignoreMessageType:
    case griddyn::commMessage::pingMessageType:
    case griddyn::commMessage::replyMessageType:
      {
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
    case griddyn::comms::controlMessage::SET:
    case griddyn::comms::controlMessage::GET:
    case griddyn::comms::controlMessage::GET_MULTIPLE:
    case griddyn::comms::controlMessage::GET_PERIODIC:
    case griddyn::comms::controlMessage::SET_SUCCESS:
    case griddyn::comms::controlMessage::SET_FAIL:
    case griddyn::comms::controlMessage::GET_RESULT:
    case griddyn::comms::controlMessage::GET_RESULT_MULTIPLE:
    case griddyn::comms::controlMessage::SET_SCHEDULED:
    case griddyn::comms::controlMessage::GET_SCHEDULED:
    case griddyn::comms::controlMessage::CANCEL:
    case griddyn::comms::controlMessage::CANCEL_SUCCESS:
    case griddyn::comms::controlMessage::CANCEL_FAIL:
      {
        std::shared_ptr<griddyn::comms::controlMessage> m = std::dynamic_pointer_cast<griddyn::comms::controlMessage> (message);

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
    case griddyn::comms::relayMessage::NO_EVENT:
    case griddyn::comms::relayMessage::LOCAL_FAULT_EVENT:
    case griddyn::comms::relayMessage::REMOTE_FAULT_EVENT:
    case griddyn::comms::relayMessage::BREAKER_TRIP_EVENT:
    case griddyn::comms::relayMessage::BREAKER_CLOSE_EVENT:
    case griddyn::comms::relayMessage::LOCAL_FAULT_CLEARED:
    case griddyn::comms::relayMessage::REMOTE_FAULT_CLEARED:
    case griddyn::comms::relayMessage::BREAKER_TRIP_COMMAND:
    case griddyn::comms::relayMessage::BREAKER_CLOSE_COMMAND:
    case griddyn::comms::relayMessage::BREAKER_OOS_COMMAND:
      {
        std::shared_ptr<griddyn::comms::relayMessage> m = std::dynamic_pointer_cast<griddyn::comms::relayMessage> (message);

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

