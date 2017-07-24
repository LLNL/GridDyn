# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(extra_sources
	extraModels.cpp
	txThermalModel.cpp
	txLifeSpan.cpp
	)
	
set(extra_headers
	extraModels.h
	txThermalModel.h
	txLifeSpan.h
	)

add_library(extraModelLibrary ${extra_sources} ${extra_headers})

INCLUDE_DIRECTORIES(.)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)

INSTALL(TARGETS extraModelLibrary
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.h")

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.hpp")

list(APPEND griddyn_optional_libraries extraModelLibrary)
list(APPEND griddyn_optional_include_dirs ${PROJECT_SOURCE_DIR}/src/extraModels)

set(griddyn_optional_libraries ${griddyn_optional_libraries} PARENT_SCOPE)
set(griddyn_optional_include_dirs ${griddyn_optional_include_dirs} PARENT_SCOPE)

list (APPEND optional_library_key_headers extraModels.h)
list (APPEND optional_library_functions loadExtraModels)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)
