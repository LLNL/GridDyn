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
*/

#ifndef GRIDDYN_FEDERATED_SIMULATOR_H
#define GRIDDYN_FEDERATED_SIMULATOR_H

#include "fskit/GridDynFskitRunner.h"

#include <fskit/granted-time-window-scheduler.h>
#include <fskit/variable-step-size-federated-simulator.h>
#include <fskit/time.h>

#include <random>
#include <iostream>

class GriddynFskitRunner;

/**
 * Example variable step size simulator implementation.
 */
class GriddynFederatedSimulator : public fskit::VariableStepSizeFederatedSimulator
{
public:
  GriddynFederatedSimulator (std::string name, int argc, char *argv[],
                             std::shared_ptr<fskit::GrantedTimeWindowScheduler> scheduler);

  bool Initialize (void);

  void StartCommunication (void);

  bool TestCommunication (void);

  fskit::Time CalculateLocalGrantedTime (void);

  bool Finalize (void);

  std::tuple<fskit::Time,bool> TimeAdvancement (const fskit::Time& time);

private:
  std::string m_name;

  fskit::Time m_currentFskitTime;
  double m_currentGriddynTime;

  std::shared_ptr<GriddynFskitRunner> m_griddyn;
};

#endif
