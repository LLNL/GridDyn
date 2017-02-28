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
 * -----------------------------------------------------------------
 */

#ifndef GRIDDYN_GRIDDYN_RUNNER_H
#define GRIDDYN_GRIDDYN_RUNNER_H

#include "griddyn-config.h"
#include "gridDynDefinitions.h"
#include <chrono>
#include <memory>

class gridDynSimulation;

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
	/** constructor*/
	GriddynRunner();
	/** Destructor*/
	virtual ~GriddynRunner();
  /**
   * Initialize a simulation run from command line arguments.
   */

  virtual int Initialize (int argc, char *argv[]);
  /** initialization the simulation object so it is ready to run*/
  void simInitialize();
  /**
   * Run simulation to completion
   */
  virtual void Run (void);

  /**
   * Run simulation up to provided time.   Simulation may
   * return early.
   *
   * @param time maximum time simulation may advance to.
   * @return time simulation successfully advanced to.
   */
  virtual coreTime Step (coreTime time);

  /**
   * Get the next GridDyn Event time
   *
   * @return the next event time
   */
  coreTime getNextEvent () const;

  virtual void Finalize (void);
  /** get a pointer to the simulation object*/
  std::shared_ptr<gridDynSimulation> getSim() const
  {
	  return m_gds;
  }

  std::shared_ptr<gridDynSimulation> &getSim()
  {
	  return m_gds;
  }
protected:
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
/** process the command line arguments for GridDyn and load them in a program Variables Map
@param[in] argc the number arguments
@param[in] argv the arguments
@param[in] vm_map the variable map to store the input arguments
*/
int argumentParser (int argc, char *argv[], boost::program_options::variables_map &vm_map);

/** load information from the input arguments to the reader Info Structures for file interpretation
@param[in] vm_map the structure containing the input arguments
@param[in] ri the file reader informational structure*/
void loadXMLinfo (boost::program_options::variables_map &vm_map, readerInfo &ri);


/** process the command line arguments
@param[in] gds the simulation object
@param[in] ri the file reader informational structure
@param[in] vm the arguments structure*/
int processCommandArguments (std::shared_ptr<gridDynSimulation> &gds, readerInfo &ri, boost::program_options::variables_map &vm);
#endif

