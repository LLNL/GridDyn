#ifndef GRIDDYN_GRIDDYN_H
#define GRIDDYN_GRIDDYN_H

#include <griddyn/bits/griddyn_constants.h>
#include <griddyn/bits/griddyn_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** GridDyn simulation administration functionality */
///@{
LIBGRIDDYN_EXPORT griddyn_sim*    griddyn_sim_new(char const* model_path);
LIBGRIDDYN_EXPORT void            griddyn_sim_free();

LIBGRIDDYN_EXPORT griddyn_bus*    griddyn_sim_bus_list(griddyn_sim const* ctx);
LIBGRIDDYN_EXPORT griddyn_idx_t   griddyn_sim_bus_count(griddyn_sim const* ctx);

LIBGRIDDYN_EXPORT griddyn_link*   griddyn_sim_link_list(griddyn_sim const* ctx);
LIBGRIDDYN_EXPORT griddyn_idx_t   griddyn_sim_link_count(griddyn_sim const* ctx);
LIBGRIDDYN_EXPORT void            griddyn_sim_link_get_adjacency_list(
                                    griddyn_sim const* ctx,
                                    griddyn_idx_t* from,
                                    griddyn_idx_t* to);
///@}

/** GridDyn simulation run functionality */
///@{
LIBGRIDDYN_EXPORT void            griddyn_sim_set_run_flags(griddyn_sim* ctx, griddyn_flags_t flags);

LIBGRIDDYN_EXPORT void            griddyn_sim_run(griddyn_sim* ctx);
LIBGRIDDYN_EXPORT griddyn_result_t  griddyn_sim_get_run_result(griddyn_sim const* ctx);
LIBGRIDDYN_EXPORT griddyn_status_t  griddyn_sim_get_run_status(griddyn_sim const* ctx);
///@}

/** GridDyn simulation management */
///@{
LIBGRIDDYN_EXPORT griddyn_value_t griddyn_sim_get_base_power(griddyn_sim const* ctx);

LIBGRIDDYN_EXPORT griddyn_value_t griddyn_sim_get_load_real(griddyn_sim const* ctx);
LIBGRIDDYN_EXPORT griddyn_value_t griddyn_sim_get_load_reactive(griddyn_sim const* ctx);
///@}

/** GridDyn bus management */
///@{
LIBGRIDDYN_EXPORT griddyn_value_t griddyn_bus_get_voltage(griddyn_bus const* bus);
///@}

/** GridDyn load management */
///@{
LIBGRIDDYN_EXPORT griddyn_load*   griddyn_bus_load_list(griddyn_bus const* bus);
LIBGRIDDYN_EXPORT griddyn_idx_t   griddyn_bus_load_count(griddyn_bus const* bus);

LIBGRIDDYN_EXPORT griddyn_bool_t  griddyn_load_is_svd(griddyn_load const* load);

LIBGRIDDYN_EXPORT griddyn_bool_t  griddyn_load_get_status(griddyn_load const* load);
LIBGRIDDYN_EXPORT void            griddyn_load_set_status(griddyn_load* load, griddyn_bool_t status);

LIBGRIDDYN_EXPORT griddyn_value_t griddyn_load_get_conductance(griddyn_load const* load);
LIBGRIDDYN_EXPORT griddyn_value_t griddyn_load_get_susceptance(griddyn_load const* load);
///@}

/** GridDyn generator management */
///@{
LIBGRIDDYN_EXPORT griddyn_generator*  griddyn_bus_generator_list(griddyn_bus const* bus);
LIBGRIDDYN_EXPORT griddyn_idx_t   griddyn_bus_generator_count(griddyn_bus const* bus);

LIBGRIDDYN_EXPORT griddyn_bool_t  griddyn_generator_get_status(griddyn_generator const* generator);
LIBGRIDDYN_EXPORT void            griddyn_generator_set_status(griddyn_generator* generator, griddyn_bool_t status);
///@}

/** GridDyn link management */
///@{
LIBGRIDDYN_EXPORT griddyn_bool_t  griddyn_link_get_status(griddyn_link const* link);
LIBGRIDDYN_EXPORT void            griddyn_link_set_status(griddyn_link* link, griddyn_bool_t status);

LIBGRIDDYN_EXPORT griddyn_bool_t  griddyn_link_is_transformer(griddyn_link const* link);

LIBGRIDDYN_EXPORT griddyn_value_t griddyn_link_get_current(griddyn_link const* link);
LIBGRIDDYN_EXPORT griddyn_value_t griddyn_link_get_rating(griddyn_link const* link);
///@}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GRIDDYN_GRIDDYN_H
