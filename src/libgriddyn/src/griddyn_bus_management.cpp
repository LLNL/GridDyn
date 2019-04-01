#include "griddyn_interface.hpp"

griddyn_value_t griddyn_bus_get_voltage(griddyn_bus const* bus)
{
  impl::vet_pointer(bus);
  auto const* bus_ptr = impl::bus_cast_const(bus);
  return bus_ptr->getVoltage();
}
