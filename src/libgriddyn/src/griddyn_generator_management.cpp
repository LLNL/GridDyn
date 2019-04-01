#include "griddyn_interface.hpp"

griddyn_generator* griddyn_bus_generator_i(griddyn_sim const* ctx, griddyn_bus const* bus, griddyn_idx_t generator_idx)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto const* bus_ptr = impl::bus_cast_const(bus);

  auto map = impl::get_bus_map(interface);
  auto& generator_storage = interface->get_generator_storage();

  auto& generator_vec = generator_storage[map[bus_ptr]];

  griddyn::Generator* last = bus_ptr->getGen(0);
  for (griddyn_idx_t i = 1; last != nullptr; ++i)
  {
    generator_vec.push_back(last);
    last = bus_ptr->getGen(i);
  }
  return generator_vec[generator_idx];
}

griddyn_idx_t griddyn_bus_generator_count(griddyn_sim const* ctx, griddyn_bus const* bus)
{
  auto const* interface = impl::interface_cast_const(ctx);
  auto const* bus_ptr = impl::bus_cast_const(bus);

  auto map = impl::get_bus_map(interface);
  auto& generator_storage = interface->get_generator_storage();

  auto& generator_vec = generator_storage[map[bus_ptr]];

  griddyn::Generator* last = bus_ptr->getGen(0);
  for (griddyn_idx_t i = 1; last != nullptr; ++i)
  {
    generator_vec.push_back(last);
    last = bus_ptr->getGen(i);
  }
  return generator_vec.size();
}

griddyn_bool_t griddyn_generator_get_status(griddyn_generator const* generator)
{
  auto const* generator_ptr = impl::generator_cast_const(generator);
  return generator_ptr->isEnabled();
}

void griddyn_generator_set_status(griddyn_generator* generator, griddyn_bool_t status)
{
  auto* generator_ptr = impl::generator_cast(generator);
  if (status)
  {
    generator_ptr->enable();
  }
  else
  {
    generator_ptr->disable();
  }
}
