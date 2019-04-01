set(libgriddyn_Sources
  griddyn_interface.hpp

  griddyn_bus_management.cpp
  griddyn_generator_management.cpp
  griddyn_link_management.cpp
  griddyn_load_management.cpp
  griddyn_run.cpp
  griddyn_sim.cpp
  griddyn_sim_management.cpp
)

list(TRANSFORM libgriddyn_Sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(libgriddyn PRIVATE ${libgriddyn_Sources})
