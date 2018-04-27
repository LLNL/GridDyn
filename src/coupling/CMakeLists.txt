# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

cmake_minimum_required (VERSION 3.5)

# set variable shared_sources with the sources for the coupling library
set(coupling_sources
    GhostSwingBusManager.cpp
	MpiService.cpp
    )

set(coupling_headers
    GhostSwingBusManager.h
	GhostSwingBusMessageTypes.h
	MpiService.h
    )
    
# require MPI for this project

add_library(coupling_static_lib STATIC ${coupling_sources} ${coupling_headers})
IF (GRIDDYN_HAVE_MPI)
target_compile_definitions(coupling_static_lib PRIVATE ${MPI_CXX_COMPILE_FLAGS})
target_include_directories(coupling_static_lib PRIVATE ${MPI_CXX_INCLUDE_PATH})
#target_link_libraries(coupling_static_lib PUBLIC ${MPI_CXX_LINK_FLAGS})
target_link_libraries(coupling_static_lib PUBLIC ${MPI_CXX_LIBRARIES})
ENDIF()
target_link_libraries(coupling_static_lib PUBLIC griddyn_base)
set_target_properties (coupling_static_lib PROPERTIES FOLDER libraries)
INSTALL(TARGETS coupling_static_lib EXPORT griddyn-targets  DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)

INSTALL(FILES ${shared_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn COMPONENT headers)



