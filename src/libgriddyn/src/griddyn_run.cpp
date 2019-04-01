#include "griddyn_interface.hpp"

void griddyn_sim_set_run_flags(griddyn_sim* ctx, griddyn_flag_t flags)
{
  griddyn::gridDynSimulation* sim_ptr = impl::sim_cast(ctx);
  {
    // TODO: There has got to be a better way of doing this
    if (flags & GRIDDYN_DENSE_SOLVER)                       sim_ptr->setFlag("dense_solver", true);
    if (flags & GRIDDYN_POWER_ADJUST_ENABLED)               sim_ptr->setFlag("power_adjust_enabled", true);
    if (flags & GRIDDYN_DCFLOW_INITIALIZATION)              sim_ptr->setFlag("dcFlow_initialization", true);
    if (flags & GRIDDYN_PARALLEL_RESIDUAL_ENABLED)          sim_ptr->setFlag("parallel_residual_enabled", true);
    if (flags & GRIDDYN_PARALLEL_JACOBIAN_ENABLED)          sim_ptr->setFlag("parallel_jacobian_enabled", true);
    if (flags & GRIDDYN_PARALLEL_CONTINGENCY_ENABLED)       sim_ptr->setFlag("parallel_contingency_enabled", true);
    if (flags & GRIDDYN_MPI_CONTINGENCY_ENABLED)            sim_ptr->setFlag("mpi_contingency_enabled", true);
    if (flags & GRIDDYN_FIRST_RUN_LIMITS_ONLY)              sim_ptr->setFlag("first_run_limits_only", true);
    if (flags & GRIDDYN_NO_RESET)                           sim_ptr->setFlag("no_reset", true);
    if (flags & GRIDDYN_VOLTAGE_CONSTRAINTS_FLAG)           sim_ptr->setFlag("voltage_constraints_flag", true);
    if (flags & GRIDDYN_RECORD_ON_HALT_FLAG)                sim_ptr->setFlag("record_on_halt_flag", true);
    if (flags & GRIDDYN_NO_AUTO_SLACK_BUS)                  sim_ptr->setFlag("no_auto_slack_bus", true);
    if (flags & GRIDDYN_NO_AUTO_DISCONNECT)                 sim_ptr->setFlag("no_auto_disconnect", true);
    if (flags & GRIDDYN_SINGLE_STEP_MODE)                   sim_ptr->setFlag("single_step_mode", true);
    if (flags & GRIDDYN_DC_MODE)                            sim_ptr->setFlag("dc_mode", true);
    if (flags & GRIDDYN_FORCE_POWER_FLOW)                   sim_ptr->setFlag("force_power_flow", true);
    if (flags & GRIDDYN_POWER_FLOW_ONLY)                    sim_ptr->setFlag("power_flow_only", true);
    if (flags & GRIDDYN_NO_POWERFLOW_ADJUSTMENTS)           sim_ptr->setFlag("no_powerflow_adjustments", true);
    if (flags & GRIDDYN_SAVE_POWER_FLOW_DATA)               sim_ptr->setFlag("save_power_flow_data", true);
    if (flags & GRIDDYN_NO_POWERFLOW_ERROR_RECOVERY)        sim_ptr->setFlag("no_powerflow_error_recovery", true);
    if (flags & GRIDDYN_DAE_INITIALIZATION_FOR_PARTITIONED) sim_ptr->setFlag("dae_initialization_for_partitioned", true);
    if (flags & GRIDDYN_FORCE_EXTRA_POWERFLOW)              sim_ptr->setFlag("force_extra_powerflow", true);
    if (flags & GRIDDYN_DROOP_POWER_FLOW)                   sim_ptr->setFlag("droop_power_flow", true);
    if (flags & GRIDDYN_DCJACCOMP_FLAG)                     sim_ptr->setFlag("dcJacComp_flag", true);
    if (flags & GRIDDYN_RESET_VOLTAGE_FLAG)                 sim_ptr->setFlag("reset_voltage_flag", true);
    if (flags & GRIDDYN_PREV_SETALL_PQVLIMIT)               sim_ptr->setFlag("prev_setall_pqvlimit", true);
    if (flags & GRIDDYN_INVALID_STATE_FLAG)                 sim_ptr->setFlag("invalid_state_flag", true);
    if (flags & GRIDDYN_CHECK_RESET_VOLTAGE_FLAG)           sim_ptr->setFlag("check_reset_voltage_flag", true);
    if (flags & GRIDDYN_POWERFLOW_SAVED)                    sim_ptr->setFlag("powerflow_saved", true);
    if (flags & GRIDDYN_LOW_BUS_VOLTAGE)                    sim_ptr->setFlag("low_bus_voltage", true);
  }
}

void griddyn_sim_run(griddyn_sim* ctx)
{
  auto* interface = impl::interface_cast(ctx);
  auto* sim = interface->get_simulation();
  auto run_val = sim->run();
  // gridState_t is a enum class, so linearize it
  // TODO: put the values in the constants header file
  auto state_val = static_cast<griddyn_status_t>(sim->currentProcessState());
  interface->set_run_result(run_val, state_val);
}

griddyn_result_t griddyn_sim_get_run_result(griddyn_sim const* ctx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  return interface->get_run_result();
}

griddyn_status_t griddyn_sim_get_run_status(griddyn_sim const* ctx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  return interface->get_run_status();
}
