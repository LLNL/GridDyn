#include <griddyn/libgriddyn.h>

#include "libgriddyn_test.h"

BOOST_AUTO_TEST_CASE (powerflow_run)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_value_t base_power = griddyn_sim_get_base_power(sim);
  griddyn_value_t real_load = griddyn_sim_get_load_real(sim);
  griddyn_value_t reactive_load = griddyn_sim_get_load_reactive(sim);

  BOOST_CHECK(base_power == 100);
  BOOST_CHECK(real_load == 0);
  BOOST_CHECK(reactive_load == 0);

  griddyn_sim_run(sim);

  BOOST_CHECK(griddyn_sim_get_run_result(sim) == 0);
  BOOST_CHECK(griddyn_sim_get_run_status(sim) == 2); // POWERFLOW_COMPLETE
  BOOST_CHECK(griddyn_sim_get_base_power(sim) == 100);
  BOOST_CHECK(griddyn_sim_get_load_real(sim) > 0);
  BOOST_CHECK(griddyn_sim_get_load_reactive(sim) > 0);

  griddyn_sim_free(sim);
}

BOOST_AUTO_TEST_CASE (powerflow_run_with_flags)
{
  griddyn_sim* sim = griddyn_sim_new(GRIDDYN_TEST_DIRECTORY "/IEEE_test_cases/ieee14.cdf");

  griddyn_value_t base_power = griddyn_sim_get_base_power(sim);
  griddyn_value_t real_load = griddyn_sim_get_load_real(sim);
  griddyn_value_t reactive_load = griddyn_sim_get_load_reactive(sim);

  BOOST_CHECK(base_power == 100);
  BOOST_CHECK(real_load == 0);
  BOOST_CHECK(reactive_load == 0);

  // TODO: find a flag that actually changes the solution, to verify that
  // setting flags actually works
  griddyn_sim_set_run_flags(sim, GRIDDYN_DC_MODE);
  griddyn_sim_run(sim);

  BOOST_CHECK(griddyn_sim_get_run_result(sim) == 0);
  BOOST_CHECK(griddyn_sim_get_run_status(sim) == 2); // POWERFLOW_COMPLETE
  BOOST_CHECK(griddyn_sim_get_base_power(sim) == 100);
  BOOST_CHECK(griddyn_sim_get_load_real(sim) > 0);
  BOOST_CHECK(griddyn_sim_get_load_reactive(sim) > 0);

  griddyn_sim_free(sim);
}
