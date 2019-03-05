/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 * -----------------------------------------------------------------
 */

#pragma once

#include "griddyn/gridDynDefinitions.hpp"
#include "griddyn/griddyn-config.h"
#include <chrono>
#include <future>
#include <memory>

#ifndef GRIDDYN_PENDING
#define GRIDDYN_PENDING (25)
#endif
// forward declaration for CLI::App
namespace CLI
{
class App;
}  // namespace CLI

namespace griddyn
{
class gridDynSimulation;
class readerInfo;
/**
 * Build and run a GridDyn simulation.
 */
class GriddynRunner
{
  public:
    /** constructor*/
    GriddynRunner ();
    GriddynRunner (std::shared_ptr<gridDynSimulation> sim);
    /** Destructor*/
    virtual ~GriddynRunner ();
    int InitializeFromString (const std::string &cmdargs);

    /**
     * Initialize a simulation run from command line arguments.
     @param[in] argc the number of console arguments
     @param[in] argv the actual console arguments
     @param[in] allowUnrecognized set to true to indicate that the unrecognized arguments should be allowed
     @return >0 normal stop,  0 normal, <0 error
     */
    int Initialize (int argc, char *argv[], bool allowUnrecognized = false);
    /**
    * Initialize a simulation run from command line arguments using a given readerInfo structure
    @param[in] argc the number of console arguments
    @param[in] argv the actual console arguments
    @param[in] ri the readerInfo structure that contains any additional reader information
    @return >0 normal stop,  0 normal, <0 error
    */
    int Initialize (int argc, char *argv[], readerInfo &ri, bool allowUnrecognized = false);
    /** initialization the simulation object so it is ready to run*/
    virtual void simInitialize ();
    /**
     * Run simulation to completion
     @return the final time of the simulation
     */
    virtual coreTime Run ();

    /**
    * Run simulation to completion but return immediately
    @details @see getStatus to query the status and result of the asynchronous call
    */
    virtual void RunAsync ();

    /**
     * Run simulation up to provided time.   Simulation may
     * return early.
     *
     * @param time maximum time simulation may advance to.
     * @return time simulation successfully advanced to.
     */
    virtual coreTime Step (coreTime time);

    /**
     * Run simulation up to provided time.   Simulation may
     * return early.
     *
     * @param time maximum time simulation may advance to.
     * @return time simulation successfully advanced to.
     */
    virtual void StepAsync (coreTime time);

    /** get the current execution status of the simulation
    @param[out] timeReturn the current simulation time
    @return GRIDDYN_PENDING if an asyncrhonous operation is ongoing otherwise returns the current state of the
    simulation*/
    virtual int getStatus (coreTime &timeReturn);
    /**
     * Get the next GridDyn Event time
     *
     * @return the next event time
     */
    coreTime getNextEvent () const;

    virtual void Finalize ();
    virtual int Reset ();
    virtual int Reset (readerInfo &ri);
    /** reset the underlying simulation of a runner*/
    void resetSim (std::shared_ptr<gridDynSimulation> sim) { m_gds = std::move (sim); }
    /** get a pointer to the simulation object*/
    std::shared_ptr<const gridDynSimulation> getSim () const { return m_gds; }

    std::shared_ptr<gridDynSimulation> &getSim ();

    virtual bool isReady () const;

  protected:
    /**
     * Get the next GridDyn Event time
     *
     * @return the next event time
     */
    void StopRecording (void);

    std::shared_ptr<gridDynSimulation> m_gds;

    decltype (std::chrono::high_resolution_clock::now ()) m_startTime;
    decltype (std::chrono::high_resolution_clock::now ()) m_stopTime;
    bool eventMode = false;

    virtual std::shared_ptr<CLI::App> generateCommandLineParser ();

  private:
    std::unique_ptr<boost::program_options::variables_map> vm;
    std::future<coreTime> async_ret;  //!< future code for the asynchronous operations
};

/** process the command line arguments for GridDyn and load them in a program Variables Map
@param[in] argc the number arguments
@param[in] argv the arguments
@param[in] vm_map the variable map to store the input arguments
@param[in] allowUnrecognized set to true to allow unrecognized options without throwing an error
@return 1 if an arguments were included for information purposes and no execution is requested, 0 if everything is
normal, <0 if there was an invalid argument
*/
int argumentParser (int argc,
                    char *argv[],
                    boost::program_options::variables_map &vm_map,
                    bool allowUnrecognized = false);

/** load information from the input arguments to the reader Info Structures for file interpretation
@param[in] vm_map the structure containing the input arguments
@param[in] ri the file reader informational structure*/
void loadXMLinfo (boost::program_options::variables_map &vm_map, readerInfo &ri);

/** process the command line arguments
@param[in] gds the simulation object
@param[in] ri the file reader informational structure
@param[in] vm the arguments structure*/
int processCommandArguments (std::shared_ptr<gridDynSimulation> &gds,
                             readerInfo &ri,
                             boost::program_options::variables_map &vm);

}  // namespace griddyn
#endif
