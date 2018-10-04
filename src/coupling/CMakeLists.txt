# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

# set variable shared_sources with the sources for the coupling library
set(coupling_sources GhostSwingBusManager.cpp)

set(coupling_headers GhostSwingBusManager.h GhostSwingBusMessageTypes.h)
if(GRIDDYN_HAVE_MPI)
    list(APPEND coupling_sources MpiService.cpp)
    list(APPEND coupling_headers MpiService.h)
endif(GRIDDYN_HAVE_MPI)
# require MPI for this project

add_library(coupling_static_lib STATIC ${coupling_sources} ${coupling_headers})

target_link_libraries(coupling_static_lib PUBLIC griddyn_base)
set_target_properties(coupling_static_lib PROPERTIES FOLDER libraries)
install(
    TARGETS coupling_static_lib
    EXPORT griddyn-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries
)

install(
    FILES ${shared_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn
    COMPONENT headers
)
