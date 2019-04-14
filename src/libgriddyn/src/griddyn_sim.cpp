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
  griddyn::loadFile(interface->get_simulation(), model_path);
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
  auto const* sim_ptr = impl::sim_cast_const(ctx);

  auto& bus_list = interface->get_bus_storage();
  sim_ptr->getBusVector(bus_list, 0);

  return bus_list[bus_idx];
}

griddyn_idx_t griddyn_sim_bus_count(griddyn_sim const* ctx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto const* sim_ptr = impl::sim_cast_const(ctx);

  auto& bus_list = interface->get_bus_storage();
  sim_ptr->getBusVector(bus_list, 0);

  return bus_list.size();
}

griddyn_link* griddyn_sim_link_i(griddyn_sim const* ctx, griddyn_idx_t link_idx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto const* sim_ptr = impl::sim_cast_const(ctx);

  auto& link_list = interface->get_link_storage();
  sim_ptr->getLinkVector(link_list, 0);

  return link_list[link_idx];
}

griddyn_idx_t griddyn_sim_link_count(griddyn_sim const* ctx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto const* sim_ptr = impl::sim_cast_const(ctx);

  auto& link_list = interface->get_link_storage();
  sim_ptr->getLinkVector(link_list, 0);

  return link_list.size();
}

void griddyn_sim_link_get_adjacency_list(
  griddyn_sim const* ctx,
  griddyn_idx_t* from,
  griddyn_idx_t* to,
  griddyn_idx_t capacity)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto const* sim_ptr = impl::sim_cast_const(ctx);

  // make sure bus_list and link_list are populated
  auto& bus_list = interface->get_bus_storage();
  sim_ptr->getBusVector(bus_list, 0);

  auto& link_list = interface->get_link_storage();
  sim_ptr->getLinkVector(link_list, 0);

  std::map<griddyn::gridBus*, griddyn_idx_t> idx_map;
  for (griddyn_idx_t bus_idx = 0; bus_idx < bus_list.size(); ++bus_idx)
  {
    idx_map[bus_list[bus_idx]] = bus_idx;
  }

  // ensure we're only writing to addresses the caller thinks are valid
  capacity = std::min(capacity, link_list.size());

  for (griddyn_idx_t link_idx = 0; link_idx < capacity; ++link_idx)
  {
    // "from" and "to" are rather meaningless...
    auto* bus_1 = link_list[link_idx]->getBus(1);
    auto idx_1 = idx_map[bus_1];
    from[link_idx] = idx_1;

    auto* bus_2 = link_list[link_idx]->getBus(2);
    auto idx_2 = idx_map[bus_2];
    to[link_idx] = idx_2;
  }
}
