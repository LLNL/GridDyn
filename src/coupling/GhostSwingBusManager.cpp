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

#include "GhostSwingBusManager.h"
#ifdef GRIDDYN_ENABLE_MPI
#    include "MpiService.h"
#    include <mpi.h>
#endif
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>

using namespace std;

namespace griddyn {
bool GhostSwingBusManager::g_printStuff = false;

// Global pointer to ensure single instance of class
std::shared_ptr<GhostSwingBusManager> GhostSwingBusManager::m_pInstance = nullptr;

// This constructor requires instance to exist
std::shared_ptr<GhostSwingBusManager> GhostSwingBusManager::Instance()
{
    if (!m_pInstance) {
#ifdef GRIDDYN_ENABLE_MPI
        throw(std::runtime_error("GhostSwingBusManager Instance does not exist!"));
#else  // mainly for convenience on a windows system for testing purposes the GhostSwingBus doesn't do anything \
    // without MPI
        m_pInstance =
            std::shared_ptr<GhostSwingBusManager>(new GhostSwingBusManager(nullptr, nullptr));
#endif
    }
    return m_pInstance;
}

// create Instance if doesn't exist...requires argc, argv for MPI
std::shared_ptr<GhostSwingBusManager> GhostSwingBusManager::Initialize(int* argc, char** argv[])
{
    if (!m_pInstance) {
        m_pInstance = std::shared_ptr<GhostSwingBusManager>(new GhostSwingBusManager(argc, argv));
    }
    return m_pInstance;
}

bool GhostSwingBusManager::isInstance()
{
    return static_cast<bool>(m_pInstance);
}

GhostSwingBusManager::GhostSwingBusManager(int* argc, char** argv[])
{
#ifdef GRIDDYN_ENABLE_MPI
    servicer = mpi::MpiService::instance(argc, argv);

    m_numTasks = servicer->getCommSize();
    m_mpiSendRequests.resize(m_numTasks);
    m_mpiRecvRequests.resize(m_numTasks);

    m_initializeCompleted.resize(m_numTasks);

#else
    (void)argc;
    (void)argv;
    dummy_load_eval.resize(m_numTasks);
#endif

    m_voltSendMessage.resize(m_numTasks);
    m_currReceiveMessage.resize(m_numTasks);
}

GhostSwingBusManager::~GhostSwingBusManager() = default;

// for Transmission
int GhostSwingBusManager::createGridlabDInstance(const string& arguments)
{
    assert(arguments.size() <=
           PATH_MAX * 4);  // there is a bug in Visual studio where the sizeof doesn't
    // compile

    int taskId = m_nextTaskId++;
    if (g_printStuff) {
        cout << "Task: " << taskId << " creating new GridLAB-D instance with args " << arguments
             << endl;
    }

#ifdef GRIDDYN_ENABLE_MPI
    auto* arguments_c = const_cast<char*>(arguments.c_str());
    m_initializeCompleted[taskId] = false;
    auto token = servicer->getToken();
    MPI_Isend(arguments_c,
              static_cast<int>(arguments.size()),
              MPI_CHAR,
              taskId,
              MODELSPECTAG,
              MPI_COMM_WORLD,
              &m_mpiSendRequests[taskId]);

#else
    if (taskId >= m_numTasks) {
        m_numTasks = m_numTasks + 50;
        dummy_load_eval.resize(m_numTasks);
        m_voltSendMessage.resize(m_numTasks);
        m_currReceiveMessage.resize(m_numTasks);
    }
#endif

    return taskId;
}

// Transmission
void GhostSwingBusManager::sendVoltageStep(int taskId, cvec& voltage, unsigned int deltaTime)
{
    // populate message structure
    if (g_printStuff) {
        cout << "taskId: " << taskId << " voltage[0]: " << endl
             << "** LEB: vector size = " << voltage.size() << endl;
    }

    // calculate number of ThreePhaseVoltages in voltage
    int numThreePhaseVoltage = static_cast<int>(voltage.size()) / 3;

    if (g_printStuff) {
        std::cout << "Sending voltage message deltaTime = " << deltaTime << " to task " << taskId
                  << '\n';
    }

    for (int i = 0; i < numThreePhaseVoltage; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (g_printStuff) {
                std::cout << "\tvoltage[" << i << "][" << j << "] = " << voltage[(i * 3) + j]
                          << " abs = " << std::abs(voltage[(i * 3) + j])
                          << " arg = " << std::arg(voltage[(i * 3) + j]) << endl;
            }
            m_voltSendMessage[taskId].voltage[i].real[j] = voltage[(i * 3) + j].real();
            m_voltSendMessage[taskId].voltage[i].imag[j] = voltage[(i * 3) + j].imag();
        }
    }
    m_voltSendMessage[taskId].numThreePhaseVoltage = numThreePhaseVoltage;
    m_voltSendMessage[taskId].deltaTime = deltaTime;

#ifdef GRIDDYN_ENABLE_MPI
    auto token = servicer->getToken();
    if (!m_initializeCompleted[taskId]) {
        MPI_Status status;

        // SGS TODO add timer for this.

        // Make sure async initialize Send has completed.
        MPI_Wait(&m_mpiSendRequests[taskId], &status);

        m_initializeCompleted[taskId] = true;
    }

    MPI_Isend(&m_voltSendMessage[taskId],
              sizeof(VoltageMessage),
              MPI_BYTE,
              taskId,
              VOLTAGESTEPTAG,
              MPI_COMM_WORLD,
              &m_mpiSendRequests[taskId]);

    MPI_Irecv(&m_currReceiveMessage[taskId],
              sizeof(CurrentMessage),
              MPI_BYTE,
              taskId,
              CURRENTTAG,
              MPI_COMM_WORLD,
              &m_mpiRecvRequests[taskId]);

#else

    try {
        dummy_load_eval[taskId](&(m_voltSendMessage[taskId]), &(m_currReceiveMessage[taskId]));
    }
    catch (const std::bad_function_call&) {
    }

#endif
}

void GhostSwingBusManager::sendStopMessage(int taskId)
{
    if (g_printStuff) {
        cout << "Sending STOP message to Distribution task " << taskId << endl;
    }
#ifdef GRIDDYN_ENABLE_MPI
    // Blocking send to gridlabd task
    auto token = servicer->getToken();
    MPI_Send(&m_voltSendMessage[taskId], 1, MPI_BYTE, taskId, STOPTAG, MPI_COMM_WORLD);
#endif
}

// Transmission
void GhostSwingBusManager::getCurrent(int taskId, cvec& current)
{
    int numThreePhaseCurrent;
    if (g_printStuff) {
        cout << "Transmission *waiting* to get current from Task: " << taskId << endl;
    }

#ifdef GRIDDYN_ENABLE_MPI
    {
        MPI_Status status;
        auto token = servicer->getToken();
        // Make sure async Send has completed.
        MPI_Wait(&m_mpiSendRequests[taskId], &status);

        // Make sure async Recv has completed.
        MPI_Wait(&m_mpiRecvRequests[taskId], &status);
    }
#endif

    numThreePhaseCurrent = m_currReceiveMessage[taskId].numThreePhaseCurrent;
    if (g_printStuff) {
        cout << "Current received from Task:" << taskId
             << ", numThreePhaseCurrent = " << numThreePhaseCurrent << endl;
    }
    current.resize(numThreePhaseCurrent *
                   3);  // resize vector to number of three phase currents received.
    for (int i = 0; i < numThreePhaseCurrent; ++i) {
        for (int j = 0; j < 3; ++j) {
            current[(i * 3) + j].real(m_currReceiveMessage[taskId].current[i].real[j]);
            current[(i * 3) + j].imag(m_currReceiveMessage[taskId].current[i].imag[j]);
            if (g_printStuff) {
                cout << "\tcurrReceiveMessage, current[" << (i * 3) + j
                     << "] = " << current[(i * 3) + j]
                     << " abs = " << std::abs(current[(i * 3) + j])
                     << " arg = " << std::arg(current[(i * 3) + j]) << endl;
            }
        }
    }
}

// must cleanup MPI
void GhostSwingBusManager::endSimulation()
{
    for (int i = 1; i < getNumTasks(); ++i) {
        sendStopMessage(i);
    }

#ifdef GRIDDYN_ENABLE_MPI

    if (g_printStuff) {
        cout << "end task : " << m_taskId << endl;
    }
#endif
    // clear the shared_ptr, the object will probably get deleted at this point and will be unable
    // to be called
    m_pInstance.reset();
}

}  // namespace griddyn
