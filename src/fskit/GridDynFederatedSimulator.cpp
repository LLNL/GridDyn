/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
/*
 * -----------------------------------------------------------------
 * LLNS Copyright Start
 * Copyright (c) 2014, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 * -----------------------------------------------------------------
 */

#include "GridDynFederatedSimulator.h"

#include <limits>

GriddynFederatedSimulator::GriddynFederatedSimulator (std::string name, int argc, char *argv[],
                                                      std::shared_ptr<fskit::GrantedTimeWindowScheduler> scheduler)
  : VariableStepSizeFederatedSimulator (fskit::FederatedSimulatorId (name)),
    m_name (name),
    m_currentFskitTime (0),
    m_currentGriddynTime (0)
{
  m_griddyn = std::make_shared<GriddynFskitRunner> ();

  m_griddyn->Initialize (argc, argv, scheduler);
}


bool GriddynFederatedSimulator::Initialize (void)
{
  m_griddyn->simInitialize ();
  return true;
}

void GriddynFederatedSimulator::StartCommunication (void)
{
}

bool GriddynFederatedSimulator::TestCommunication (void)
{
  return true;
}

fskit::Time GriddynFederatedSimulator::CalculateLocalGrantedTime (void)
{
  const double kBigNum (1e49);

  double griddynNextEventTime = m_griddyn->getNextEvent ();

  // Magic number that indicates there are no events on the event queue.
  if (griddynNextEventTime == kBigNum )
    {
      return fskit::Time::getMax ();
    }

  // This could be event time + lookahead.
  return fskit::Time (griddynNextEventTime * 1e9);
}

bool GriddynFederatedSimulator::Finalize (void)
{

  m_griddyn->Finalize ();

  return true;
}

std::tuple<fskit::Time,bool> GriddynFederatedSimulator::TimeAdvancement (const fskit::Time& time)
{
  griddyn::coreTime gdTime, gdRetTime;

  // Convert fskit time to coreTime used by Griddyn
  gdTime.setBaseTimeCode (time.GetRaw ());
  bool stopSimulation = false;

  // SGS this is not correct!! How to correctly handled lower resolution of Griddyn time?
  // Advance Griddyn if needed.
  if ( gdTime >= m_currentGriddynTime )
    {
      try
        {
          gdRetTime = m_griddyn->Step (gdTime);
          m_currentGriddynTime = gdRetTime;
          m_currentFskitTime = fskit::Time (gdRetTime.getBaseTimeCode ());
        }
      catch (...)
        {
          std::cout << "Griddyn exiting due to failure" << std::endl;
          stopSimulation = true;
        }
    }

  return std::make_tuple (m_currentFskitTime, stopSimulation);
}
