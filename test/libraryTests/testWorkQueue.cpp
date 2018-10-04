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

// test case for workQueue

#include "../testHelper.h"
#include "utilities/workQueue.h"
#include <boost/test/unit_test.hpp>
#include <chrono>
#include <iostream>
BOOST_AUTO_TEST_SUITE (workQueue_tests, * boost::unit_test::label("quick"))

// some tests only make sense if multithreading is enabled

#ifdef ENABLE_MULTITHREADING
BOOST_AUTO_TEST_CASE (workQueue_test1)
{
    int check = workQueue::getWorkerCount ();
    BOOST_CHECK_EQUAL (check, 0);

    auto wq = workQueue::instance (1);

    BOOST_CHECK ((wq));
    check = workQueue::getWorkerCount ();
    BOOST_CHECK_EQUAL (check, 1);

    wq->destroyWorkerQueue ();

    wq.reset ();  // reset to make get rid of shared ptr reference
    check = workQueue::getWorkerCount ();
    BOOST_CHECK_EQUAL (check, 0);

    auto fk = [] {
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
        return std::hash<std::thread::id> () (std::this_thread::get_id ());
    };
    wq = workQueue::instance (5);
    check = workQueue::getWorkerCount ();
    BOOST_CHECK_EQUAL (check, 5);
    std::vector<decltype (make_shared_workBlock (fk))> blocks (10);
    std::vector<std::shared_ptr<basicWorkBlock>> bblocks (10);
    for (size_t kk = 0; kk < 10; ++kk)
    {
        blocks[kk] = make_shared_workBlock (fk);
        bblocks[kk] = blocks[kk];
    }
    wq->addWorkBlock (bblocks);
    std::vector<decltype (fk ())> res (10);
    for (size_t kk = 0; kk < 10; ++kk)
    {
        res[kk] = blocks[kk]->getReturnVal ();
    }
    std::sort (res.begin (), res.end ());
    auto last = std::unique (res.begin (), res.end ());
    res.erase (last, res.end ());
    BOOST_CHECK_EQUAL (res.size (), 5u);
    wq->destroyWorkerQueue ();
}

#endif

BOOST_AUTO_TEST_CASE (workQueue_test2)
{
    // Test a zero worker count
    auto wq = workQueue::instance (0);
    std::function<void()> fk = [] { std::this_thread::sleep_for (std::chrono::milliseconds (110)); };

    auto b1 = make_shared_workBlock (fk);
    auto start_t = std::chrono::high_resolution_clock::now ();
    wq->addWorkBlock (b1);
    auto stop_t = std::chrono::high_resolution_clock::now ();
    std::chrono::duration<double> elapsed_time = stop_t - start_t;
    // verifying the block was run immediately
    BOOST_CHECK_GE (elapsed_time.count (), 0.1);
    // verify the block won't run again until reset
    start_t = std::chrono::high_resolution_clock::now ();
    wq->addWorkBlock (b1);
    stop_t = std::chrono::high_resolution_clock::now ();
    elapsed_time = stop_t - start_t;
    // verifying the block was run immediately
    BOOST_CHECK_LT (elapsed_time.count (), 0.05);
    // verify function update works properly
    fk = [] { std::this_thread::sleep_for (std::chrono::milliseconds (130)); };
    b1->updateWorkFunction (fk);

    start_t = std::chrono::high_resolution_clock::now ();
    wq->addWorkBlock (b1);
    stop_t = std::chrono::high_resolution_clock::now ();
    elapsed_time = stop_t - start_t;
    BOOST_CHECK_GE (elapsed_time.count (), 0.125);

    wq->destroyWorkerQueue ();
#ifdef ENABLE_MULTITHREADING
    wq = workQueue::instance (1);
    b1->reset ();
    start_t = std::chrono::high_resolution_clock::now ();
    wq->addWorkBlock (b1);
    stop_t = std::chrono::high_resolution_clock::now ();
    elapsed_time = stop_t - start_t;
    // verifying the block was run immediately
    BOOST_CHECK_LT (elapsed_time.count (), 0.05);
    wq->destroyWorkerQueue ();
#endif
}

#ifdef ENABLE_MULTITHREADING
BOOST_AUTO_TEST_CASE (workQueue_test3)
{
    // Test a queue priority mechanisms

    auto wq = workQueue::instance (1);
    // a sleeper work block to give us time to set up the rest
    auto fk = [] { std::this_thread::sleep_for (std::chrono::milliseconds (300)); };

    auto b1 = make_workBlock (fk);
    // only 1 worker thread so don't worry about locking
    std::vector<int> order;

    auto hp = [&order] { order.push_back (1); };
    auto mp = [&order] { order.push_back (2); };
    auto lp = [&order] { order.push_back (3); };
    wq->setPriorityRatio (3);
    wq->addWorkBlock (std::move (b1), workQueue::workPriority::high);

    wq->addWorkBlock (make_workBlock (lp), workQueue::workPriority::low);
    wq->addWorkBlock (make_workBlock (lp), workQueue::workPriority::low);
    wq->addWorkBlock (make_workBlock (lp), workQueue::workPriority::low);

    wq->addWorkBlock (make_workBlock (mp), workQueue::workPriority::medium);
    wq->addWorkBlock (make_workBlock (mp), workQueue::workPriority::medium);
    wq->addWorkBlock (make_workBlock (mp), workQueue::workPriority::medium);
    wq->addWorkBlock (make_workBlock (mp), workQueue::workPriority::medium);
    wq->addWorkBlock (make_workBlock (mp), workQueue::workPriority::medium);
    wq->addWorkBlock (make_workBlock (mp), workQueue::workPriority::medium);
    wq->addWorkBlock (make_workBlock (mp), workQueue::workPriority::medium);
    wq->addWorkBlock (make_workBlock (mp), workQueue::workPriority::medium);
    wq->addWorkBlock (make_workBlock (mp), workQueue::workPriority::medium);

    wq->addWorkBlock (make_workBlock (hp), workQueue::workPriority::high);
    wq->addWorkBlock (make_workBlock (hp), workQueue::workPriority::high);
    std::this_thread::sleep_for (std::chrono::milliseconds (340));
    while (!wq->isEmpty ())
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (40));
    }
    BOOST_CHECK_EQUAL (order.size (), 14u);
    std::vector<int> orderCorrect = {1, 1, 2, 2, 2, 3, 2, 2, 2, 3, 2, 2, 2, 3};
    int cdiff = 0;
    for (size_t kk = 0; kk < 14; ++kk)
    {
        if (order[kk] != orderCorrect[kk])
        {
            ++cdiff;
        }
    }
    BOOST_CHECK_MESSAGE (cdiff == 0, "Execution out of order");
}

#endif

BOOST_AUTO_TEST_SUITE_END ()
