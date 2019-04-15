#include "griddyn_interface.hpp"

#include "fileInput/fileInput.h"

#include <string>
#include <sstream>
#include <vector>
#include <map>

griddyn_sim* griddyn_sim_new(char const* model_path)
{
  auto sim_name = impl::get_next_sim_name();
  auto* interface = new impl::sim_interface(sim_name);

  // Build caches for links and such in the interface, after we've loaded a model
  {
    griddyn::loadFile(interface->get_simulation(), model_path);
    interface->invalidate();
  }

  return interface;
}

void griddyn_sim_free(griddyn_sim* ctx)
{
  auto* interface = impl::interface_cast(ctx);
  delete interface;
}

griddyn_sim* griddyn_sim_clone(griddyn_sim const* ctx)
{
  auto const* sim = impl::interface_cast_const(ctx);
  return new impl::sim_interface(*sim);
}

griddyn_bus* griddyn_sim_bus_i(griddyn_sim const* ctx, griddyn_idx_t bus_idx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto& bus_list = interface->get_bus_storage();
  return bus_list[bus_idx];
}

griddyn_idx_t griddyn_sim_bus_count(griddyn_sim const* ctx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto& bus_list = interface->get_bus_storage();
  return bus_list.size();
}

griddyn_link* griddyn_sim_link_i(griddyn_sim const* ctx, griddyn_idx_t link_idx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto& link_list = interface->get_link_storage();
  return link_list[link_idx];
}

griddyn_idx_t griddyn_sim_link_count(griddyn_sim const* ctx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto& link_list = interface->get_link_storage();
  return link_list.size();
}
