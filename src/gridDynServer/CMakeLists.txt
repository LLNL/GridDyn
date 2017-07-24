# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(griddynServer_sources
	gridDynServerMain.cpp
	gridDynServer.cpp
	)
	
set(griddynServer_headers
	gridDynServer.h
	)

link_directories(${external_link_directories})


set(GRIDYNSERVER_BINARY_LOC ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

add_executable(griddynServer ${griddynServer_sources} ${griddynServer_headers})

INCLUDE_DIRECTORIES(.)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/formatInterpreter)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gridDynFileInput)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/utilities)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/coupling)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gridDynCombined)


target_link_libraries(griddynServer gridDynCombined )

INSTALL(TARGETS griddynServer RUNTIME DESTINATION bin)
