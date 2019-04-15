#include <griddyn/libgriddyn.h>

#include "libgriddyn_test.h"

BOOST_AUTO_TEST_CASE (load_count)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);
  BOOST_ASSERT(bus_count == 14);

  std::vector<griddyn_idx_t> expected_load_counts = { 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1 };
  std::vector<griddyn_idx_t> actual_load_counts;

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_load_count = griddyn_bus_load_count(sim, current_bus);
    actual_load_counts.push_back(bus_load_count);
  }

  BOOST_CHECK(expected_load_counts == actual_load_counts);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (load_status)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);
  BOOST_ASSERT(bus_count == 14);

  std::vector<griddyn_bool_t> expected_load_status(11, 1);
  std::vector<griddyn_bool_t> actual_load_status;

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_load_count = griddyn_bus_load_count(sim, current_bus);
    for (griddyn_idx_t j = 0; j < bus_load_count; ++j)
    {
      griddyn_load* current_load = griddyn_bus_load_i(sim, current_bus, j);
      griddyn_bool_t load_status = griddyn_load_get_status(current_load);
      actual_load_status.push_back(load_status);
    }
  }

  BOOST_CHECK(expected_load_status == actual_load_status);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (load_status_change)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);
  BOOST_ASSERT(bus_count == 14);

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_load_count = griddyn_bus_load_count(sim, current_bus);
    for (griddyn_idx_t j = 0; j < bus_load_count; ++j)
    {
      griddyn_load* current_load = griddyn_bus_load_i(sim, current_bus, j);
      griddyn_load_set_status(current_load, 0);
    }
  }

  std::vector<griddyn_bool_t> expected_load_status(11, 0);
  std::vector<griddyn_bool_t> actual_load_status;

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_load_count = griddyn_bus_load_count(sim, current_bus);
    for (griddyn_idx_t j = 0; j < bus_load_count; ++j)
    {
      griddyn_load* current_load = griddyn_bus_load_i(sim, current_bus, j);
      griddyn_bool_t load_status = griddyn_load_get_status(current_load);
      actual_load_status.push_back(load_status);
    }
  }

  BOOST_CHECK(expected_load_status == actual_load_status);

  griddyn_sim_free(sim);
}

// TODO: pick a test case with an actual SVD
BOOST_AUTO_TEST_CASE (load_SVD_check)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);
  BOOST_ASSERT(bus_count == 14);

  std::vector<griddyn_bool_t> expected_load_is_svd(11, 0);
  std::vector<griddyn_bool_t> actual_load_is_svd;

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_load_count = griddyn_bus_load_count(sim, current_bus);
    for (griddyn_idx_t j = 0; j < bus_load_count; ++j)
    {
      griddyn_load* current_load = griddyn_bus_load_i(sim, current_bus, j);
      griddyn_bool_t load_status = griddyn_load_is_svd(current_load);
      actual_load_is_svd.push_back(load_status);
    }
  }

  BOOST_CHECK(expected_load_is_svd == actual_load_is_svd);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (load_shunt_detect)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t bus_count = griddyn_sim_bus_count(sim);
  BOOST_ASSERT(bus_count == 14);

  std::vector<griddyn_value_t> expected_load_conductance(11, 0);
  std::vector<griddyn_value_t> expected_load_susceptance = { 0, 0, 0, 0, 0, -0.19, 0, 0, 0, 0, 0 };

  std::vector<griddyn_value_t> actual_load_conductance;
  std::vector<griddyn_value_t> actual_load_susceptance;

  for (griddyn_idx_t i = 0; i < bus_count; ++i)
  {
    griddyn_bus* current_bus = griddyn_sim_bus_i(sim, i);
    griddyn_idx_t bus_load_count = griddyn_bus_load_count(sim, current_bus);
    for (griddyn_idx_t j = 0; j < bus_load_count; ++j)
    {
      griddyn_load* current_load = griddyn_bus_load_i(sim, current_bus, j);
      griddyn_value_t load_conductance = griddyn_load_get_conductance(current_load);
      griddyn_value_t load_susceptance = griddyn_load_get_susceptance(current_load);
      actual_load_conductance.push_back(load_conductance);
      actual_load_susceptance.push_back(load_susceptance);
    }
  }

  BOOST_CHECK(expected_load_conductance == actual_load_conductance);
  BOOST_CHECK(expected_load_susceptance == actual_load_susceptance);

  griddyn_sim_free(sim);
}
