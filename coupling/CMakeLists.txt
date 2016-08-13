# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

cmake_minimum_required (VERSION 2.6)

# set variable shared_sources with the sources for the coupling library
set(shared_sources
    GhostSwingBusManager.cpp
    )

set(shared_headers
    GhostSwingBusManager.h
	GhostSwingBusMessageTypes.h
    )
    
# require MPI for this project

set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})

include_directories(${PROJECT_SOURCE_DIR}/core)
include_directories(${MPI_INCLUDE_PATH})

add_library(coupling_static_lib STATIC ${shared_sources} ${shared_headers})

INSTALL(TARGETS coupling_static_lib 
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(FILES ${shared_headers} DESTINATION include/griddyn)



