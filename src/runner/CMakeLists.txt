# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(runner_sources gridDynRunner.cpp)

set(runner_headers gridDynRunner.h)

add_library(runner STATIC ${runner_sources} ${runner_headers})

target_link_libraries(runner PUBLIC fileInput)

# TODO might not need this
target_link_libraries(runner PUBLIC utilities)

set_target_properties(runner PROPERTIES FOLDER libraries)

install(
    TARGETS runner
    EXPORT griddyn-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries
)

install(
    FILES ${runner_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/runner
    COMPONENT headers
)
