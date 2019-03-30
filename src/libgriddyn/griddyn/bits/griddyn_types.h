#ifndef GRIDDYN_BITS_GRIDDYN_TYPES_H
#define GRIDDYN_BITS_GRIDDYN_TYPES_H

#include <stddef.h> // size_t

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t    griddyn_idx_t;
typedef double    griddyn_value_t;
typedef int       griddyn_bool_t;

typedef struct griddyn_sim_   griddyn_sim;
typedef struct griddyn_bus_   griddyn_bus;
typedef struct griddyn_link_  griddyn_link;
typedef struct griddyn_load_  griddyn_load;
typedef struct griddyn_generator_ griddyn_generator;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GRIDDYN_BITS_GRIDDYN_TYPES_H
