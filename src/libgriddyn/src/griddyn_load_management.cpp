#include "griddyn_interface.hpp"
#include "griddyn/Load.h"
#include "griddyn/loads/svd.h"

griddyn_load* griddyn_bus_load_i(griddyn_sim const* ctx, griddyn_bus const* bus, griddyn_idx_t load_idx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto const* bus_ptr = impl::bus_cast_const(bus);

  auto map = impl::get_bus_map(interface);
  auto& load_storage = interface->get_load_storage();

  auto& load_vec = load_storage[map[bus_ptr]];

  return load_vec[load_idx];
}

griddyn_idx_t griddyn_bus_load_count(griddyn_sim const* ctx, griddyn_bus const* bus)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto const* bus_ptr = impl::bus_cast_const(bus);

  auto map = impl::get_bus_map(interface);
  auto& load_storage = interface->get_load_storage();

  auto& load_vec = load_storage[map[bus_ptr]];

  return load_vec.size();
}

griddyn_bool_t griddyn_load_is_svd(griddyn_load const* load)
{
  auto const* actual_load = impl::load_cast_const(load);
  return dynamic_cast<griddyn::loads::svd const*>(actual_load) != nullptr;
}

griddyn_bool_t griddyn_load_get_status(griddyn_load const* load)
{
  auto const* actual_load = impl::load_cast_const(load);
  return actual_load->isEnabled();
}

void griddyn_load_set_status(griddyn_load* load, griddyn_bool_t status)
{
  auto* actual_load = impl::load_cast(load);
  if (status)
  {
    actual_load->enable();
  }
  else
  {
    actual_load->disable();
  }
}

griddyn_value_t griddyn_load_get_conductance(griddyn_load const* load)
{
  auto const* actual_load = impl::load_cast_const(load);
  return actual_load->get("yp");
}

griddyn_value_t griddyn_load_get_susceptance(griddyn_load const* load)
{
  auto const* actual_load = impl::load_cast_const(load);
  return actual_load->get("yq");
}
