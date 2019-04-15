#include <griddyn/libgriddyn.h>

#include "libgriddyn_test.h"

BOOST_AUTO_TEST_CASE (simulator_creation)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");
  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (simulator_parameter_retrieval)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_value_t base_power = griddyn_sim_get_base_power(sim);
  griddyn_value_t real_load = griddyn_sim_get_load_real(sim);
  griddyn_value_t reactive_load = griddyn_sim_get_load_reactive(sim);

  BOOST_CHECK(base_power == 100);

  // TODO: are these right? check them again post-run?
  BOOST_CHECK(real_load == 0);
  BOOST_CHECK(reactive_load == 0);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (simulator_clone)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");
  griddyn_sim* sim2 = griddyn_sim_clone(sim);

  griddyn_idx_t link_count = griddyn_sim_link_count(sim);
  griddyn_idx_t link_count2 = griddyn_sim_link_count(sim2);
  BOOST_ASSERT(link_count == 20);
  BOOST_ASSERT(link_count2 == 20);

  griddyn_link* link_0 = griddyn_sim_link_i(sim, 0);
  griddyn_link* link2_0 = griddyn_sim_link_i(sim2, 0);
  BOOST_CHECK(griddyn_link_get_status(link_0) == 1);
  BOOST_CHECK(griddyn_link_get_status(link2_0) == 1);

  griddyn_link_set_status(link_0, 0);
  BOOST_CHECK(griddyn_link_get_status(link_0) == 0);
  BOOST_CHECK(griddyn_link_get_status(link2_0) == 1);

  griddyn_sim_free(sim);
  griddyn_sim_free(sim2);
}
