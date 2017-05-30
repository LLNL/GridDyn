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
#ifndef GRIDDYN_FEDERATED_SCHEDULER_H_
#define GRIDDYN_FEDERATED_SCHEDULER_H_


#include <memory>
#include <cassert>

namespace fskit {
class GrantedTimeWindowScheduler;
}

/**
 * Singleton for accessing federated scheduler.
 *
 * Singleton is initialized in GriddynRunner.
 */
class GriddynFederatedScheduler
{
public:
  static bool IsFederated ()
  {
    return g_scheduler != nullptr;
  }

  /**
   * Return singleton federated scheduler.
   */
  static std::shared_ptr<fskit::GrantedTimeWindowScheduler> GetScheduler ()
  {
    assert (g_scheduler);
    return g_scheduler;
  }

  /* Make non-copyable since it is this class is used to access a singleton
   * via static methods.
   */
  GriddynFederatedScheduler () = default;
  GriddynFederatedScheduler (const GriddynFederatedScheduler&) = delete;
  GriddynFederatedScheduler& operator= (const GriddynFederatedScheduler&) = delete;

  /*
   * GriddynRunnder initializes the federated scheduler.
   */
  friend class GriddynFskitRunner;

private:
  /**
   * Initialize singleton.
   */
  static void Initialize (std::shared_ptr<fskit::GrantedTimeWindowScheduler> scheduler)
  {
	  //TODO:: make this thread safe probably with a mutex lock
    g_scheduler = scheduler;
  }

  static std::shared_ptr<fskit::GrantedTimeWindowScheduler> g_scheduler;
};

#endif // GRIDDYN_FEDERATED_SCHEDULER_H_
