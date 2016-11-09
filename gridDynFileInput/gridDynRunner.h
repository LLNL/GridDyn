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
 * -----------------------------------------------------------------
 */

#ifndef GRIDDYN_GRIDDYN_RUNNER_H
#define GRIDDYN_GRIDDYN_RUNNER_H

#include "griddyn-config.h"

#include <chrono>
#include <memory>

class gridDynSimulation;

#ifdef GRIDDYN_HAVE_FSKIT
namespace fskit {
class GrantedTimeWindowScheduler;
}
#endif

//forward declaration for boost::program_options::variables_map
namespace boost {
namespace program_options {
class variables_map;
}

}


/**
 * Build and run a GridDyn simulation.
 */
class GriddynRunner
{
public:
  /**
   * Initialize a simulation run from command line arguments.
   */
#ifdef GRIDDYN_HAVE_FSKIT
  int Initialize (int argc, char *argv[], std::shared_ptr<fskit::GrantedTimeWindowScheduler> scheduler);
#else
  int Initialize (int argc, char *argv[]);
#endif

  /**
   * Run simulation to completion
   */
  void Run (void);

  /**
   * Run simulation up to provided time.   Simulation may
   * return early.
   *
   * @param time maximum time simulation may advance to.
   * @return time simulation successfully advanced to.
   */
  double Step (double time);

  /**
   * Get the next GridDyn Event time
   *
   * @return the next event time
   */
  double getNextEvent () const;

  void Finalize (void);

private:
  /**
   * Get the next GridDyn Event time
   *
   * @return the next event time
   */
  void StopRecording (void);

  std::shared_ptr<gridDynSimulation> m_gds;

  decltype(std::chrono::high_resolution_clock::now ())m_startTime;
  decltype(std::chrono::high_resolution_clock::now ())m_stopTime;
  bool m_isMpiCountMode = false;
  bool eventMode = false;
};

class readerInfo;

int argumentParser (int argc, char *argv[], boost::program_options::variables_map &vm_map);

void loadXMLinfo (boost::program_options::variables_map &vm_map, readerInfo *ri);



int processCommandArguments (std::shared_ptr<gridDynSimulation> gds, readerInfo *ri, boost::program_options::variables_map &vm);
#endif

