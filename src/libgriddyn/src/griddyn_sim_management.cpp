#include "griddyn_interface.hpp"

griddyn_value_t griddyn_sim_get_base_power(griddyn_sim const* ctx)
{
  impl::vet_pointer(ctx);
  auto const* sim = impl::sim_cast_const(ctx);
  return sim->get("basepower");
}

griddyn_value_t griddyn_sim_get_load_real(griddyn_sim const* ctx)
{
  impl::vet_pointer(ctx);
  auto const* sim = impl::sim_cast_const(ctx);
  return sim->getLoadReal();
}

griddyn_value_t griddyn_sim_get_load_reactive(griddyn_sim const* ctx)
{
  impl::vet_pointer(ctx);
  auto const* sim = impl::sim_cast_const(ctx);
  return sim->getLoadReactive();
}
