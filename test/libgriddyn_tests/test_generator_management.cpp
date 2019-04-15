#include <griddyn/libgriddyn.h>

#include "libgriddyn_test.h"

BOOST_AUTO_TEST_CASE (generator_count)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);
  BOOST_ASSERT(bus_count == 14);

  std::vector<griddyn_idx_t> expected_gen_counts = { 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 };
  std::vector<griddyn_idx_t> actual_gen_counts;

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_gen_count = griddyn_bus_generator_count(sim, current_bus);
    actual_gen_counts.push_back(bus_gen_count);
  }

  BOOST_CHECK(expected_gen_counts == actual_gen_counts);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (generator_status)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);
  BOOST_ASSERT(bus_count == 14);

  std::vector<griddyn_bool_t> expected_gen_status(5, 1);
  std::vector<griddyn_bool_t> actual_gen_status;

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_gen_count = griddyn_bus_generator_count(sim, current_bus);
    for (griddyn_idx_t j = 0; j < bus_gen_count; ++j)
    {
      griddyn_generator* current_generator = griddyn_bus_generator_i(sim, current_bus, j);
      griddyn_bool_t gen_status = griddyn_generator_get_status(current_generator);
      actual_gen_status.push_back(gen_status);
    }
  }

  BOOST_CHECK(expected_gen_status == actual_gen_status);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (generator_status_change)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);
  BOOST_ASSERT(bus_count == 14);

  std::vector<griddyn_bool_t> expected_gen_status(5, 0);
  std::vector<griddyn_bool_t> actual_gen_status;

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_gen_count = griddyn_bus_generator_count(sim, current_bus);
    for (griddyn_idx_t j = 0; j < bus_gen_count; ++j)
    {
      griddyn_generator* current_generator = griddyn_bus_generator_i(sim, current_bus, j);
      griddyn_generator_set_status(current_generator, 0);
    }
  }

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_gen_count = griddyn_bus_generator_count(sim, current_bus);
    for (griddyn_idx_t j = 0; j < bus_gen_count; ++j)
    {
      griddyn_generator* current_generator = griddyn_bus_generator_i(sim, current_bus, j);
      griddyn_bool_t gen_status = griddyn_generator_get_status(current_generator);
      actual_gen_status.push_back(gen_status);
    }
  }

  BOOST_CHECK(expected_gen_status == actual_gen_status);

  griddyn_sim_free(sim);
}
