/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "griddyn/griddyn-config.h"
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <mpi.h>

namespace helics
{
namespace mpi
{
/** service for using MPI to communicate*/
class MpiService
{
  public:
    static std::shared_ptr<MpiService> getInstance ();
    static void setMpiCommunicator (MPI_Comm communicator);

    int getRank ();
    ~MpiService();
  private:
    MpiService ();
    
    MpiService (const MpiService &) = delete;
    MpiService &operator= (const MpiService &) = delete;

    int commRank;
    static MPI_Comm mpiCommunicator;

    std::atomic<int> comms_connected;
    std::atomic<bool> startup_flag;
    std::atomic<bool> stop_service;


    void startService ();
    void serviceLoop ();

    bool initMPI ();
};

} // namespace mpi
}  // namespace helics

