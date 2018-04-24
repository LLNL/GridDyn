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

#pragma once

#include "griddyn/griddyn-config.h"

#include <memory>
#include <mutex>
#include <string>

#include <mpi.h>

namespace griddyn
{
namespace mpi
{
/** service object to get a singular point for startup and shutdown of MPI*/
class MpiService
{
  public:
    /**
     * Create a new instance of the MpiService.  This should be
     * called at the program launch with the command line arguments
     * since this method invoke MPI_Init.
     *
     * Note that argc and argv may NOT be the same on return, just as in
     * MPI_Init.
     */
    static MpiService *instance (int *argc, char **argv[]);  // only constructor that creates Instance
    /**
     * Create a new instance of the MpiService.  This should be
     * called at the program launch with the command line arguments
     * since this method invoke MPI_Thread_Init.
     *
     * Note that argc and argv may NOT be the same on return, just as in
     * MPI_Init.
     */
    static MpiService *
    instance (int *argc, char **argv[], int threadingLevel);  // only constructor that creates Instance

    /**
     * Return the current instance of the singleton.
     */
    static MpiService *instance ();
    /**
     * Return the current instance of the singleton.
     */
    static MpiService *instance (int threadingLevel);

    int getRank () const { return commRank; };
    int getCommSize () const { return commSize; };
    int getThreadingLevel () const { return mpiThreadingLevel; };
    ~MpiService ();

  private:
    MpiService ();
    MpiService (int threadingLevel);
    MpiService (int *argc, char **argv[]);
    MpiService (int *argc, char **argv[], int threadingLevel);

    /**
     * Singleton instance.
     */
    static std::unique_ptr<MpiService> m_pInstance;
    static std::mutex startupLock;  //!< mutex protecting the instance
    MpiService (const MpiService &) = delete;
    MpiService &operator= (const MpiService &) = delete;

    int commRank = -1;
    int commSize = -1;
    int mpiThreadingLevel = -1;
    bool initialized_mpi = false;
};

}  // namespace mpi
}  // namespace griddyn
