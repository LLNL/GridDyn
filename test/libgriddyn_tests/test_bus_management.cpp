#include <griddyn/libgriddyn.h>

#include "libgriddyn_test.h"

BOOST_AUTO_TEST_CASE (bus_count)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);

  BOOST_CHECK(bus_count == 14);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (bus_voltage)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);
  BOOST_ASSERT(bus_count == 14);

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_value_t current_voltage = griddyn_bus_get_voltage(current_bus);
    BOOST_CHECK(current_voltage > 1);
    BOOST_CHECK(current_voltage < 2);
  }

  griddyn_sim_free(sim);
}
