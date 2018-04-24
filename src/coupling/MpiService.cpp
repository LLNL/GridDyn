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

#include "MpiService.h"

namespace griddyn
{
namespace mpi
{

MpiService *MpiService::instance(int *argc, char **argv[], int threadingLevel)
{
    std::lock_guard<std::mutex> lock(startupLock);
    if (!m_pInstance)
    {
        m_pInstance = std::unique_ptr<MpiService>(new MpiService(argc, argv, threadingLevel));
    }
    return m_pInstance.get();
}

MpiService *MpiService::instance(int threadingLevel)
{
    std::lock_guard<std::mutex> lock(startupLock);
    if (!m_pInstance)
    {
        m_pInstance = std::unique_ptr<MpiService>(new MpiService(threadingLevel));
    }
    return m_pInstance.get();
}

MpiService *MpiService::instance(int *argc, char **argv[])
{
    std::lock_guard<std::mutex> lock(startupLock);
    if (!m_pInstance)
    {
        m_pInstance = std::unique_ptr<MpiService>(new MpiService(argc, argv));
    }
    return m_pInstance.get();
}

MpiService *MpiService::instance ()
{
    std::lock_guard<std::mutex> lock(startupLock);
    if (!m_pInstance)
    {
        m_pInstance = std::unique_ptr<MpiService>(new MpiService());
    }
    return m_pInstance.get();
}

MpiService::MpiService(int *argc, char **argv[])
{
    // Initialize MPI with MPI_THREAD_FUNNELED  
    int mpi_initialized;

    MPI_Initialized(&mpi_initialized);

    if (!mpi_initialized)
    {
        MPI_Init(argc, argv);

        MPI_Initialized(&mpi_initialized);
        if (!mpi_initialized)
        {
            throw(std::runtime_error("unable to initialize MPI"));
        }
        initialized_mpi = true;
    }
    MPI_Query_thread(&mpiThreadingLevel);

    // get number of tasks
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);

    // get taskId of this task
    MPI_Comm_rank(MPI_COMM_WORLD, &commRank);
}

MpiService::MpiService (int *argc, char **argv[], int threadingLevel)
{
    // Initialize MPI with MPI_THREAD_FUNNELED  
    int mpi_initialized;

    MPI_Initialized(&mpi_initialized);

    if (!mpi_initialized)
    {
        MPI_Init_thread(argc, argv, threadingLevel, &mpiThreadingLevel);

        MPI_Initialized(&mpi_initialized);
        if (!mpi_initialized)
        {
            throw(std::runtime_error("unable to initialize MPI"));
        }
        initialized_mpi = true;
    }
    else
    {
        MPI_Query_thread(&mpiThreadingLevel);
    }

    if (mpiThreadingLevel < threadingLevel)
    {
        throw(std::runtime_error("MPI not available for requested threading level"));
    }
    // get number of tasks
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);

    // get taskId of this task
    MPI_Comm_rank(MPI_COMM_WORLD, &commRank);
}

MpiService::MpiService(int threadingLevel):MpiService(nullptr,nullptr,threadingLevel)
{
    
}

MpiService::MpiService():MpiService(nullptr,nullptr)
{
   
}

MpiService::~MpiService ()
{
    if (initialized_mpi)
    {
        // Finalize MPI
        int mpi_initialized;
        MPI_Initialized(&mpi_initialized);

        // Probably not a necessary check, a user using MPI should have also initialized it themselves
        if (mpi_initialized)
        {
          //  std::cout << "About to finalize MPI for rank " << commRank << std::endl;
            MPI_Finalize();
          //  std::cout << "MPI Finalized for rank " << commRank << std::endl;
        }
    }
}

}  // namespace mpi
}  // namespace helics
