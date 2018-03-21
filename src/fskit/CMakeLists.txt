# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(fskit_sources
  fskitCommunicator.cpp
  gridDynFederatedScheduler.cpp
  gridDynfskit.cpp
  zonalRelayLogicalProcess.cpp
  fskitRunner.cpp
  GridDynFederatedSimulator.cpp
)
	
set (fskit_headers
	fskitCommunicator.h
	gridDynFederatedScheduler.h
	gridDynfskit.h
	zonalRelayLogicalProcess.h
	fskitRunner.h
	GridDynFederatedSimulator.h
	griddyn-tracer.h
	)

add_library(fskitLibrary ${fskit_sources} ${fskit_headers})

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/libs/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)

target_link_libraries(fskitLibrary fileInput)

INSTALL(TARGETS fskitLibrary DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)

INSTALL(FILES ${fskit_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/fskit
  COMPONENT headers)


list(APPEND griddyn_optional_libraries fskitLibrary)
list(APPEND griddyn_optional_include_dirs ${PROJECT_SOURCE_DIR}/fskit)

set(griddyn_optional_libraries ${griddyn_optional_libraries} PARENT_SCOPE)
set(griddyn_optional_include_dirs ${griddyn_optional_include_dirs} PARENT_SCOPE)

list (APPEND optional_library_key_headers gridDynfskit.h)
list (APPEND optional_library_functions loadFskit)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)
