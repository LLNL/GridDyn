# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(extra_solver_sources
	extraSolvers.cpp
	)
	
set(extra_solver_headers
	extraSolvers.h
	)

OPTION(BUILD_BRAID_SOLVER "enable the braid parallel in time solver integration" OFF)
OPTION(BUILD_SUPERLUMT_SOLVER "enable a superLU solver" OFF)

if (BUILD_BRAID_SOLVER)
find_package(Braid)
list(APPEND extra_solver_sources )
list(APPEND extra_solver_headers )
endif()

add_library(extraSolverLibrary ${extra_solver_sources} ${extra_solver_headers})
set_target_properties (extraSolverLibrary PROPERTIES FOLDER libraries)

target_link_libraries(extraSolverLibrary PUBLIC griddyn)

if (BUILD_BRAID_SOLVER)
target_link_libraries(extraSolverLibrary PUBLIC braid::braid)
endif()

INSTALL(TARGETS extraSolverLibrary EXPORT griddyn-targets DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)

INSTALL(FILES ${extra_solver_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/extraSolver
 COMPONENT headers)


list (APPEND optional_library_key_headers extraSolvers/extraSolvers.h)
list (APPEND optional_library_functions loadExtraSolvers)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)
