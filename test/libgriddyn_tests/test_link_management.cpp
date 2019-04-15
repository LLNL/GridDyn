#include <griddyn/libgriddyn.h>

#include "libgriddyn_test.h"

BOOST_AUTO_TEST_CASE (link_count)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t link_count = griddyn_sim_link_count(sim);

  BOOST_CHECK(link_count == 20);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (link_status)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t link_count = griddyn_sim_link_count(sim);
  BOOST_ASSERT(link_count == 20);

  std::vector<griddyn_bool_t> expected_link_status(20, 1);
  std::vector<griddyn_bool_t> actual_link_status;

  for (griddyn_idx_t i = 0; i < link_count; ++i)
  {
    griddyn_link* current_link = griddyn_sim_link_i(sim, i);
    griddyn_bool_t link_status = griddyn_link_get_status(current_link);
    actual_link_status.push_back(link_status);
  }

  BOOST_CHECK(expected_link_status == actual_link_status);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (link_status_change)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t link_count = griddyn_sim_link_count(sim);
  BOOST_ASSERT(link_count == 20);

  for (griddyn_idx_t i = 0; i < link_count; ++i)
  {
    griddyn_link* current_link = griddyn_sim_link_i(sim, i);
    griddyn_link_set_status(current_link, 0);
  }

  std::vector<griddyn_bool_t> expected_link_status(20, 0);
  std::vector<griddyn_bool_t> actual_link_status;

  for (griddyn_idx_t i = 0; i < link_count; ++i)
  {
    griddyn_link* current_link = griddyn_sim_link_i(sim, i);
    griddyn_bool_t link_status = griddyn_link_get_status(current_link);
    actual_link_status.push_back(link_status);
  }

  BOOST_CHECK(expected_link_status == actual_link_status);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (link_is_transformer)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t link_count = griddyn_sim_link_count(sim);
  BOOST_ASSERT(link_count == 20);

  std::vector<griddyn_bool_t> expected_transformers =
  {
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  std::vector<griddyn_bool_t> actual_transformers;

  for (griddyn_idx_t i = 0; i < link_count; ++i)
  {
    griddyn_link* current_link = griddyn_sim_link_i(sim, i);
    griddyn_bool_t is_transformer = griddyn_link_is_transformer(current_link);
    actual_transformers.push_back(is_transformer);
  }

  BOOST_CHECK(expected_transformers == actual_transformers);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (link_current_rating_inspection)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t link_count = griddyn_sim_link_count(sim);
  BOOST_ASSERT(link_count == 20);

  std::vector<griddyn_value_t> expected_current(20, 0);
  // expect ratings to all be very high

  std::vector<griddyn_value_t> actual_current;
  std::vector<griddyn_value_t> actual_rating;

  for (griddyn_idx_t i = 0; i < link_count; ++i)
  {
    griddyn_link* current_link = griddyn_sim_link_i(sim, i);
    griddyn_value_t link_current = griddyn_link_get_current(current_link);
    griddyn_value_t link_rating = griddyn_link_get_rating(current_link);
    actual_current.push_back(link_current);
    actual_rating.push_back(link_rating);
  }

  BOOST_CHECK(expected_current == actual_current);
  BOOST_CHECK(
    std::all_of(
      actual_rating.begin(),
      actual_rating.end(),
      [](auto x){ return x > 1e48; }
  ));

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (link_connectome)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_idx_t link_count = griddyn_sim_link_count(sim);
  BOOST_ASSERT(link_count == 20);

  std::map<griddyn_value_t, size_t> bus_voltages;

  // TODO: actually check connectivity, not just that the buses are okay
  for (griddyn_idx_t i = 0; i < link_count; ++i)
  {
    griddyn_link* current_link = griddyn_sim_link_i(sim, i);
    griddyn_bus* bus_1;
    griddyn_bus* bus_2;

    griddyn_link_get_buses(current_link, &bus_1, &bus_2);

    griddyn_value_t bus_1_voltage = griddyn_bus_get_voltage(bus_1);
    griddyn_value_t bus_2_voltage = griddyn_bus_get_voltage(bus_2);

    BOOST_CHECK(bus_1_voltage > 1);
    BOOST_CHECK(bus_1_voltage < 2);
    BOOST_CHECK(bus_2_voltage > 1);
    BOOST_CHECK(bus_2_voltage < 2);
  }

  griddyn_sim_free(sim);
}
