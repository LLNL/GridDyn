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
 */

#include "workQueue.h"
#include "griddyn-config.h"
#include <iostream>

// static definitions in workQueue class
std::shared_ptr<workQueue> workQueue::pInstance;

std::mutex workQueue::instanceLock;

workQueue::workQueue (int threadCount) : numWorkers (threadCount)
{
#ifdef DISABLE_MULTITHREADING
    numWorkers = 0;
#else
    if (numWorkers < 0)
    {
        numWorkers = static_cast<int> (std::thread::hardware_concurrency ()) + 1;
    }
    threadpool.resize (numWorkers);
    for (int kk = 0; kk < numWorkers; ++kk)
    {
        threadpool[kk] = std::thread (&workQueue::workerLoop, this);
    }
#endif
}

workQueue::~workQueue ()
{
    if (!halt)
    {
        halt = true;
        queueCondition.notify_all ();
    }
    pInstance = nullptr;
    for (auto &thrd : threadpool)
    {
        if (thrd.joinable ())
        {
            thrd.join ();
        }
    }
}

void workQueue::addWorkBlock (std::shared_ptr<basicWorkBlock> newWork, workPriority priority)
{
    if ((!newWork) || (newWork->isFinished ()))
    {
        return;
    }
    if (numWorkers > 0)
    {
        size_t ccount;
        switch (priority)
        {
        case workPriority::high:
            ccount = workToDoHigh.size ();
            workToDoHigh.push (std::move (newWork));
            break;
        case workPriority::medium:
            ccount = workToDoMed.size ();
            workToDoMed.push (std::move (newWork));
            break;
        case workPriority::low:
        default:
            ccount = workToDoLow.size ();
            workToDoLow.push (std::move (newWork));
            break;
        }
        if (static_cast<int> (ccount) <= numWorkers)
        {
            queueCondition.notify_one ();
        }
    }
    else
    {
        newWork->execute ();
    }
}

void workQueue::addWorkBlock (std::vector<std::shared_ptr<basicWorkBlock>> &newWork, workPriority priority)
{
    if (numWorkers > 0)
    {
        switch (priority)
        {
        case workPriority::high:
            workToDoHigh.pushVector (newWork);
            break;
        case workPriority::medium:
            workToDoMed.pushVector (newWork);
            break;
        case workPriority::low:
        default:
            workToDoLow.pushVector (newWork);
            break;
        }
        queueCondition.notify_all ();
    }
    else
    {
        for (auto &wb : newWork)
        {
            wb->execute ();
        }
    }
}

std::shared_ptr<basicWorkBlock> workQueue::getWorkBlock ()
{
    std::shared_ptr<basicWorkBlock> wb;
    auto wbb = workToDoHigh.pop ();
    if (wbb)
    {
        return *wbb;
    }

    if (MedCounter >= priorityRatio)
    {
        if (!workToDoLow.empty ())
        {
            wbb = workToDoLow.pop ();
            if (wbb)
            {
                MedCounter = 0;
                return *wbb;
            }
        }
    }
    wbb = workToDoMed.pop ();
    if (wbb)
    {
        ++MedCounter;
        return *wbb;
    }
    wbb = workToDoLow.pop ();
    if (wbb)
    {
        return *wbb;
    }

    return wb;
}

std::shared_ptr<workQueue> workQueue::instance (int threadCount)
{
    if (!pInstance)
    {
        // this is to ensure we don't create it multiple times resulting in some weird condition or just a
        // waste of resources

        std::lock_guard<std::mutex> creationLock (instanceLock);
        if (!pInstance)
        {
            pInstance = std::shared_ptr<workQueue> (new workQueue (threadCount));
        }
    }
    return pInstance;
}

int workQueue::getWorkerCount ()
{
    if (!pInstance)
    {
        return 0;
    }
    return pInstance->numWorkers;
}

void workQueue::destroyWorkerQueue ()
{
    auto tempPtr = pInstance;
    if (tempPtr == pInstance)
    {
        std::lock_guard<std::mutex> creationLock (instanceLock);
        pInstance = nullptr;
    }
    halt = true;
    workToDoHigh.clear ();
    workToDoMed.clear ();
    workToDoLow.clear ();
    queueCondition.notify_all ();
}

void workQueue::workerLoop ()
{
    std::unique_lock<std::mutex> lv (queueLock, std::defer_lock);
    while (!halt)
    {
        if (isEmpty ())
        {
            // std::cout << std::this_thread::get_id << " sleeping\n";
            lv.lock ();
            queueCondition.wait (lv);
            lv.unlock ();
            // std::cout << std::this_thread::get_id << " awoken\n";
        }
        auto wb = getWorkBlock ();  // this will return empty if it is spurious and also sync the size if needed
        if ((wb) && (!wb->isFinished ()))
        {
            wb->execute ();
            //	std::cout << std::this_thread::get_id << " excuting\n";
        }
    }
    // std::cout << std::this_thread::get_id << " halting\n";
}