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
*/

#ifndef GHOSTSWINGBUSMANAGER_H_
#define GHOSTSWINGBUSMANAGER_H_

#include "griddyn/griddyn-config.h"
#include "GhostSwingBusMessageTypes.h"

#ifndef GRIDDYN_HAVE_MPI
#include <functional>
#endif

#include <complex>
#include <vector>
#include <memory>


typedef int MPI_Request;

namespace griddyn
{
namespace mpi
{
    class MpiService;
}

class GhostSwingBusManager
{

public:
  /**
   * class destructor must be public so it can be used with shared_ptr
   */
  ~GhostSwingBusManager ();  //destructor

  using cvec= std::vector<std::complex<double> >;

  /**
   * Create a new instance of the GhostSwingBusManager.  This must be
   * called at the program launch with the command line arguments
   * since this method invoke MPI_Init.
   *
   * Note that argc and argv may NOT be the same on return, just as in
   * MPI_Init.
   */
  static std::shared_ptr<GhostSwingBusManager> Initialize (int *argc, char **argv[]); // only constructor that creates Instance

  /**
   * Return the current instance of the singleton.
   *
   * This will fail if Initialize has not been called.
   */
  static std::shared_ptr<GhostSwingBusManager> Instance ();


  /**
   * Returns true if instance of the manager exists.
   */
  static bool isInstance ();

  /**
   * Creates a new GridLab-D instance using
   * string sent as the command line to launch
   * the GridLAB-D instance.
   *
   * Returns taskId for the new instance.
   */
  int createGridlabDInstance (const std::string& arguments);

  /**
   * End the simulation.
   *
   * Must be called at the end of the simulation to ensure that
   * tasks launched by the manager are correctly ended.
   */
  void endSimulation ();

  /**
   * Advances time in distribution model by deltaTime with the
   * provided.N voltages on the swing bus.
   * This will asynchronously execute the distribution model.
   */
  void sendVoltageStep ( int taskId, cvec &voltage, unsigned int deltaTime);

  /**
   * Returns the computed currents from the distribution system for
   * deltaTime and voltages provided by the sendVoltageStep method
   * call.
   *
   * This is currently a blocking operation.  Likely will want to
   * have receiving of currents be processed in a non-blocking
   * way.
   */
  void getCurrent (int taskId, cvec &current);

  /**
   * Returns number of tasks.
   */
  int getNumTasks () const
  {
    return m_numTasks;
  }

  static void SetDebug (bool debug)
  {
    g_printStuff = debug;
  }

#ifndef GRIDDYN_HAVE_MPI
  /**
   * sets the dummy load function
   */
  void setDummyLoadFunction (int taskId, std::function<void(VoltageMessage* vm, CurrentMessage* cm)> dfunc)
  {
    dummy_load_eval[taskId] = dfunc;
  }
#endif

private:
  static bool g_printStuff;  //!< public boolean to change whether things are printed or not
#ifdef GRIDDYN_HAVE_MPI
  mpi::MpiService *servicer; //!< pointer to the global MpiService
#endif
  /**
   * Singleton so prevent external construction and copying of this
   * class.
   */
  GhostSwingBusManager ()
  {
  }

  /*
   * Private so cannot be called
   */
  GhostSwingBusManager (int *argc, char **argv[]);
  GhostSwingBusManager (GhostSwingBusManager const&) = delete;
  GhostSwingBusManager& operator= (GhostSwingBusManager const&) = delete;

  /**
   * End the distribution system model.
   *
   * Should be called for each distribution model.
   */
  void sendStopMessage ( int taskId);

  /**
   * Singleton instance.
   */
  static std::shared_ptr<GhostSwingBusManager> m_pInstance;

  int m_numTasks = 20;
#ifdef GRIDDYN_HAVE_MPI
  int m_taskId = 0;

  char m_hostname[256];

  /*
   * Async send requests
   */
  std::vector<MPI_Request> m_mpiSendRequests;
  std::vector<MPI_Request> m_mpiRecvRequests;

  /*
   * True if initialization send has completed.
   */
  std::vector<bool> m_initializeCompleted;

#else
  /**
   * a function call for a dummy load to execute on a windows system
   */
  std::vector < std::function < void(VoltageMessage * vm, CurrentMessage * cm) >> dummy_load_eval;

#endif

  /**
   * Next taskID to assign, keeps track of the
   * number of distribution models that have
   * been created.
   */
  int m_nextTaskId = 1;

  /**
   * Message buffer space for asynchronous MPI calls.
   */
  std::vector<VoltageMessage> m_voltSendMessage;
  std::vector<CurrentMessage> m_currReceiveMessage;


  /*
   * Did manager call MPI_Init.
   */
#ifdef GRIDDYN_HAVE_MPI
  bool m_mpiInitCalled = false;
#endif
};

}//namespace griddyn
#endif /* GHOSTSWINGBUSMANAGER_H_ */
