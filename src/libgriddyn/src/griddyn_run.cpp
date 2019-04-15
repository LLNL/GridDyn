#include "griddyn_interface.hpp"

void griddyn_sim_set_run_flags(griddyn_sim* ctx, griddyn_flag_t flags)
{
  griddyn::gridDynSimulation* sim_ptr = impl::sim_cast(ctx);
  {
    // TODO: There has got to be a better way of doing this
    // This is only the flags from gridDynSimulation.cpp, add others as needed
    if (flags & GRIDDYN_AUTOALLOCATE)                       sim_ptr->setFlag("autoallocate", true);
    if (flags & GRIDDYN_POWER_ADJUST)                       sim_ptr->setFlag("power_adjust", true);
    if (flags & GRIDDYN_USE_SPARSE_SOLVER)                  sim_ptr->setFlag("sparse", true);
    if (flags & GRIDDYN_USE_DENSE_SOLVER)                   sim_ptr->setFlag("dense", true);
    if (flags & GRIDDYN_NO_AUTO_AUTOGEN)                    sim_ptr->setFlag("no_auto_autogen", true);
    if (flags & GRIDDYN_AUTO_BUS_DISCONNECT)                sim_ptr->setFlag("auto_bus_disconnect", true);
    if (flags & GRIDDYN_ROOTS_DISABLED)                     sim_ptr->setFlag("roots_disabled", true);
    if (flags & GRIDDYN_VOLTAGECONSTRAINTS)                 sim_ptr->setFlag("voltageconstraints", true);
    if (flags & GRIDDYN_RECORD_ON_HALT)                     sim_ptr->setFlag("record_on_halt", true);
    if (flags & GRIDDYN_CONSTRAINTS_DISABLED)               sim_ptr->setFlag("constraints_disabled", true);
    if (flags & GRIDDYN_DC_MODE)                            sim_ptr->setFlag("dc_mode", true);
    if (flags & GRIDDYN_DCFLOW_INITIALIZATION)              sim_ptr->setFlag("dcFlow_initialization", true);
    if (flags & GRIDDYN_NO_LINK_ADJUSTMENTS)                sim_ptr->setFlag("no_link_adjustments", true);
    if (flags & GRIDDYN_DISABLE_LINK_ADJUSTMENTS)           sim_ptr->setFlag("disable_link_adjustments", true);
    if (flags & GRIDDYN_IGNORE_BUS_LIMITS)                  sim_ptr->setFlag("ignore_bus_limits", true);
    if (flags & GRIDDYN_POWERFLOW_ONLY)                     sim_ptr->setFlag("powerflow_only", true);
    if (flags & GRIDDYN_NO_POWERFLOW_ADJUSTMENTS)           sim_ptr->setFlag("no_powerflow_adjustments", true);
    if (flags & GRIDDYN_SAVEPOWERFLOW)                      sim_ptr->setFlag("savepowerflow", true);
    if (flags & GRIDDYN_LOW_VOLTAGE_CHECK)                  sim_ptr->setFlag("low_voltage_check", true);
    if (flags & GRIDDYN_NO_POWERFLOW_ERROR_RECOVERY)        sim_ptr->setFlag("no_powerflow_error_recovery", true);
    if (flags & GRIDDYN_DAE_INITIALIZATION_FOR_PARTITIONED) sim_ptr->setFlag("dae_initialization_for_partitioned", true);
    if (flags & GRIDDYN_FORCE_POWERFLOW)                    sim_ptr->setFlag("force_powerflow", true);
    if (flags & GRIDDYN_FORCE_EXTRA_POWERFLOW)              sim_ptr->setFlag("force_extra_powerflow", true);
    if (flags & GRIDDYN_DROOP_POWER_FLOW)                   sim_ptr->setFlag("droop_power_flow", true);
    if (flags & GRIDDYN_USE_THREADS)                        sim_ptr->setFlag("threads", true);
  }
}

void griddyn_sim_run(griddyn_sim* ctx)
{
  auto* interface = impl::interface_cast(ctx);
  auto* sim = interface->get_simulation();
  auto run_val = sim->run();

  // gridState_t is a enum class, linearize it
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
