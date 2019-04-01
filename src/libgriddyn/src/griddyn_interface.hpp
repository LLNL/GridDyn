#ifndef LIBGRIDDYN_SRC_INTERFACE_HPP
#define LIBGRIDDYN_SRC_INTERFACE_HPP

#include <griddyn/libgriddyn.h>
#include <stdlib.h>

#include "griddyn/gridDynSimulation.h"
#include "griddyn/Load.h"
#include "griddyn/gridBus.h"
#include "griddyn/Link.h"
#include "griddyn/Generator.h"

#include <sstream>
#include <map>

namespace impl
{

/**
 * This class holds a gridDynSimulation and permanent buffers to bus/link vectors
 * If the gridDynSimulation's internal bus/link vectors change (i.e. if
 * bus_storage_/link_storage_'s pointers are invalidated) I full expect this to
 * crash and burn
 * However, after modifying the simulation, calling get_* _again_ should give a
 * new, valid, pointer/array
 */
class sim_interface : public griddyn_sim
{
public:
  template<typename... Ts>
  sim_interface(Ts&&... args) :
    sim_(
      std::make_unique<griddyn::gridDynSimulation>(
        std::forward<Ts...>(args)...)) {}

  sim_interface(sim_interface const& other) :
    last_sim_result_(other.last_sim_result_),
    last_sim_status_(other.last_sim_status_),
    sim_(dynamic_cast<griddyn::gridDynSimulation*>(other.sim_->clone())),
    bus_storage_(),
    link_storage_(),
    load_storage_(),
    generator_storage_()
    {}

  // TODO: add cache_links, cache_busses, cache_loads_at_bus and delete the
  // associated re-caching code scattered throughout

  griddyn::gridDynSimulation* get_simulation() { return sim_.get(); }
  griddyn::gridDynSimulation const* get_simulation() const { return sim_.get(); }
  std::vector<griddyn::gridBus*>& get_bus_storage() const { return bus_storage_; }
  std::vector<griddyn::Link*>& get_link_storage() const { return link_storage_; }
  std::vector<std::vector<griddyn::Load*>>& get_load_storage() const { return load_storage_; }
  std::vector<std::vector<griddyn::Generator*>>& get_generator_storage() const { return generator_storage_; }

  void set_run_result(griddyn_result_t run_result, griddyn_status_t run_status)
  {
    last_sim_result_ = run_result;
    last_sim_status_ = run_status;
  }

  griddyn_result_t get_run_result() const
  {
    return last_sim_result_;
  }

  griddyn_status_t get_run_status() const
  {
    return last_sim_status_;
  }

private:
  griddyn_result_t last_sim_result_ = 0;
  griddyn_status_t last_sim_status_ = 0;
  std::unique_ptr<griddyn::gridDynSimulation> sim_;

private:
  /**
   * These are caches of their current values in griddyn, so updating them
   * on a const sim_interface is okay
   */
  mutable std::vector<griddyn::gridBus*> bus_storage_;
  mutable std::vector<griddyn::Link*> link_storage_;
  mutable std::vector<std::vector<griddyn::Load*>> load_storage_;
  mutable std::vector<std::vector<griddyn::Generator*>> generator_storage_;
};

/**
 * Get the next default simulation name
 * Starts with "Griddyn_Sim_1" and counts upwards
 */
inline std::string get_next_sim_name()
{
  static auto sim_init_count = 1;
  std::stringstream ss;
  ss << "GridDyn_Sim_" << sim_init_count++;
  return ss.str();
}

/**
 * Ensure a pointer passed in is safe to use
 * Currently, this only checks for null, but it should also be possible to
 * verify against all of the pointers we've ever given out
 * Note: this should only be used in the various *_cast[_const]() functions
 */
template<typename T>
inline void vet_pointer(T const* ptr)
{
  if (ptr == nullptr) abort();
  // TODO(1): this should also assert based on some of the ideas in the
  // previous comment
}

namespace detail {
template<typename InT, typename OutT>
OutT* impl_cast(InT* chunk)
{
  vet_pointer(chunk);
  return static_cast<OutT*>(chunk);
}

template<typename InT, typename OutT>
OutT const* impl_cast_const(InT const* chunk)
{
  vet_pointer(chunk);
  return static_cast<OutT const*>(chunk);
}
} // namespace detail

/**
 * Convert a griddyn_sim pointer back to a sim_interface pointer
 */
inline sim_interface* interface_cast(griddyn_sim* interface)
{
  return detail::impl_cast<griddyn_sim, sim_interface>(interface);
}

/**
 * Like interface_cast, but const
 */
inline sim_interface const* interface_cast_const(griddyn_sim const* interface)
{
  return detail::impl_cast_const<griddyn_sim, sim_interface>(interface);
}

/**
 * Convert a griddyn_sim pointer to a pointer to the underlying gridDynSimulation
 */
inline griddyn::gridDynSimulation* sim_cast(griddyn_sim* sim)
{
  auto* interface = detail::impl_cast<griddyn_sim, sim_interface>(sim);
  return interface->get_simulation();
}

/**
 * Like sim_cast, but const
 */
inline griddyn::gridDynSimulation const* sim_cast_const(griddyn_sim const* sim)
{
  auto const* interface = detail::impl_cast_const<griddyn_sim, sim_interface>(sim);
  return interface->get_simulation();
}

inline griddyn::gridBus* bus_cast(griddyn_bus* bus)
{
  return detail::impl_cast<griddyn_bus, griddyn::gridBus>(bus);
}

inline griddyn::gridBus const* bus_cast_const(griddyn_bus const* bus)
{
  return detail::impl_cast_const<griddyn_bus, griddyn::gridBus>(bus);
}

inline griddyn::Link* link_cast(griddyn_link* link)
{
  return detail::impl_cast<griddyn_link, griddyn::Link>(link);
}

inline griddyn::Link const* link_cast_const(griddyn_link const* link)
{
  return detail::impl_cast_const<griddyn_link, griddyn::Link>(link);
}

inline griddyn::Load* load_cast(griddyn_load* load)
{
  return detail::impl_cast<griddyn_load, griddyn::Load>(load);
}

inline griddyn::Load const* load_cast_const(griddyn_load const* load)
{
  return detail::impl_cast_const<griddyn_load, griddyn::Load>(load);
}

inline griddyn::Generator* generator_cast(griddyn_generator* generator)
{
  return detail::impl_cast<griddyn_generator, griddyn::Generator>(generator);
}

inline griddyn::Generator const* generator_cast_const(griddyn_generator const* generator)
{
  return detail::impl_cast_const<griddyn_generator, griddyn::Generator>(generator);
}

inline std::map<griddyn::gridBus const*, griddyn_idx_t> get_bus_map(griddyn_sim const* ctx)
{
  auto const* interface = interface_cast_const(ctx);
  auto const* sim_ptr = sim_cast_const(ctx);

  // make sure bus_list and link_list are populated
  auto& bus_list = interface->get_bus_storage();
  sim_ptr->getBusVector(bus_list, 0);

  std::map<griddyn::gridBus const*, griddyn_idx_t> idx_map;
  for (griddyn_idx_t bus_idx = 0; bus_idx < bus_list.size(); ++bus_idx)
  {
    idx_map[bus_list[bus_idx]] = bus_idx;
  }
  return idx_map;
}

} // namespace impl

#endif // LIBGRIDDYN_SRC_INTERFACE_HPP
