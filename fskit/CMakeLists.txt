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
  GridDynFskitRunner.cpp
)
	
set (fskit_headers
	fskitCommunicator.h
	gridDynFederatedScheduler.h
	gridDynfskit.h
	zonalRelayLogicalProcess.h
	GridDynFskitRunner.h
	griddyn-tracer.h
	)

add_library(fskitLibrary ${fskit_sources} ${fskit_headers})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

INCLUDE_DIRECTORIES(${FMI_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/core)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynFileInput)

INSTALL(TARGETS fskitLibrary
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/gridDyn
  FILES_MATCHING PATTERN "*.h")

list(APPEND griddyn_optional_libraries fskitLibrary)
list(APPEND griddyn_optional_include_dirs ${PROJECT_SOURCE_DIR}/fskit)

set(griddyn_optional_libraries ${griddyn_optional_libraries} PARENT_SCOPE)
set(griddyn_optional_include_dirs ${griddyn_optional_include_dirs} PARENT_SCOPE)

list (APPEND optional_library_key_headers gridDynfskit.h)
list (APPEND optional_library_functions loadFskit)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)