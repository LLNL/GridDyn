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

#include "gridDyn.h"
#include "solvers/solverInterface.h"
#include "powerFlowErrorRecovery.h"

#include <algorithm>


powerFlowErrorRecovery::powerFlowErrorRecovery (gridDynSimulation *gds, std::shared_ptr<solverInterface> sd) : sim (gds), solver (sd)
{

}
powerFlowErrorRecovery::~powerFlowErrorRecovery ()
{
}

powerFlowErrorRecovery::recovery_return_codes powerFlowErrorRecovery::attemptFix (int error_code)
{
  if ((error_code == -11) && (sim->currentProcessState () != gridDynSimulation::gridState_t::INITIALIZED))
    {        //something possibly went wrong in the initial setup try a full reinit
      sim->reInitpFlow (solver->getSolverMode (), change_code::state_count_change);
      return (attempt_number > 3) ? recovery_return_codes::out_of_options : recovery_return_codes::more_options;
    }
  if (error_code == SOLVER_INVALID_STATE_ERROR)
    {
      lowVoltageFix ();
    }
  ++attempt_number;
  switch (attempt_number)
    {
    case 1:
      if (powerFlowFix1 ())
        {
          return recovery_return_codes::more_options;
        }
      //if we didn't do anything just move to the next fix method
      return attemptFix ();
    case 2:
      if (powerFlowFix2 ())
        {
          return recovery_return_codes::more_options;
        }
      //if we didn't do anything just move to the next fix method
      return attemptFix ();
    case 3:
      if (powerFlowFix3 ())
        {
          return recovery_return_codes::more_options;
        }
      //if we didn't do anything just move to the next fix method
      return attemptFix ();
    case 4:
      if (powerFlowFix4 ())
        {
          return recovery_return_codes::more_options;
        }
      //if we didn't do anything just move to the next fix method
      return attemptFix ();
    case 5:
      powerFlowFix5 ();
      return recovery_return_codes::more_options;
    default:
      break;
    }
  return recovery_return_codes::out_of_options;
}

void powerFlowErrorRecovery::reset ()
{
  attempt_number = 0;
}

void powerFlowErrorRecovery::updateInfo (std::shared_ptr<solverInterface> sd)
{
  solver = sd;
}
int powerFlowErrorRecovery::attempts () const
{
  return attempt_number;
}
//Try any non-reversable adjustments which might be out there
bool powerFlowErrorRecovery::powerFlowFix1 ()
{
  if (!sim->opFlags[has_powerflow_adjustments])
    {
      return false;
    }
  sim->updateLocalCache ();
  change_code eval = sim->powerFlowAdjust (lower_flags (sim->controlFlags), check_level_t::full_check);
  if (eval > change_code::non_state_change)
    {
      sim->checkNetwork (gridDynSimulation::network_check_type::simplified);
      sim->reInitpFlow (solver->getSolverMode (), eval);
      return true;
    }
  return false;
}

// try a few rounds of Guass Seidel like convergence
bool powerFlowErrorRecovery::powerFlowFix2 ()
{
  sim->guess (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode ());
  sim->converge (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode (),converge_mode::block_iteration,0.1);
  sim->setState (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode ());
  if (sim->opFlags[has_powerflow_adjustments])
    {
      sim->updateLocalCache ();
      change_code eval = sim->powerFlowAdjust (lower_flags (sim->controlFlags), check_level_t::reversable_only);
      sim->reInitpFlow (solver->getSolverMode (), eval);
    }
  return true;
}

//check for some low voltage condtions and change the low voltage load conditions
bool powerFlowErrorRecovery::powerFlowFix3 ()
{
  std::vector<double> v;
  sim->getVoltage (v);
  if (std::any_of (v.begin (), v.end (), [](double a) {
    return (a < 0.7);
  }))
    {
      sim->guess (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode ());
      sim->converge (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode (),converge_mode::single_iteration,0);
      sim->setState (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode ());
      if (!sim->opFlags[prev_setall_pqvlimit])
        {
          sim->setAll ("load", "pqlowvlimit", 1.0);
          sim->controlFlags.set (voltage_constraints_flag);
          sim->opFlags.set (prev_setall_pqvlimit);
          sim->log (sim, GD_DEBUG_PRINT, "pq adjust on load");
        }
      sim->updateLocalCache ();
      sim->powerFlowAdjust (lower_flags (sim->controlFlags), check_level_t::reversable_only);
      sim->reInitpFlow (solver->getSolverMode (), change_code::state_count_change);
      sim->guess (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode ());
      sim->converge (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode (), converge_mode::block_iteration, 0.1);
      sim->setState (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode ());
      sim->updateLocalCache ();
      change_code eval = sim->powerFlowAdjust (lower_flags (sim->controlFlags), check_level_t::reversable_only);
      while (eval > change_code::no_change)
        {
          sim->reInitpFlow (solver->getSolverMode (), eval);
          sim->guess (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode ());
          sim->converge (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode (), converge_mode::single_iteration, 0);
          sim->setState (sim->getCurrentTime (), solver->state_data (), nullptr, solver->getSolverMode ());
          sim->updateLocalCache ();
          eval = sim->powerFlowAdjust (lower_flags (sim->controlFlags), check_level_t::reversable_only);
        }
      return true;
    }
  return false;
}

//Try to disconnect very low voltage buses
bool powerFlowErrorRecovery::powerFlowFix4 ()
{
  std::vector<double> v;
  sim->getVoltage (v);
  if (std::any_of (v.begin (), v.end (), [](double a) {
    return (a < 0.1);
  }))
    {
      sim->setAll ("bus", "lowvdisconnect", 0.03);
      sim->reInitpFlow (solver->getSolverMode ());
      return true;
    }
  return false;
}


bool powerFlowErrorRecovery::lowVoltageFix ()
{
  change_code eval = sim->powerFlowAdjust (lower_flags (sim->controlFlags), check_level_t::low_voltage_check);
  if (eval > change_code::no_change)
    {
      sim->checkNetwork (gridDynSimulation::network_check_type::simplified);
      sim->reInitpFlow (solver->getSolverMode (), eval);
      return true;
    }
  return false;
}

//Don't know what to do here yet
bool powerFlowErrorRecovery::powerFlowFix5 ()
{
  return false;
}
