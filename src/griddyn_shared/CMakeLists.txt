# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

IF(UNIX)
  add_compile_options(-fvisibility=hidden)
ELSE(UNIX)
  IF(MINGW)
 add_compile_options(-fvisibility=hidden)
  ENDIF(MINGW)
ENDIF(UNIX)


set(griddyn_lib_headers
griddyn.h
griddyn_export.h
griddyn_export_advanced.h
griddyn_export_internal.h
)
 
set(griddyn_lib_sources
griddyn_exportObjects.cpp
griddyn_exportSimulation.cpp
griddyn_exportQueries.cpp
griddyn_exportMathFunctions.cpp
griddyn_exportObjectMathFunctions.cpp
griddyn_exportEvents.cpp
)

add_library(griddyn_shared_lib SHARED ${griddyn_lib_sources} ${griddyn_lib_headers})
add_definitions(-DBUILD_DLL=1)
link_directories(${external_link_directories})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gridDynCombined)
INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})

IF(WITH_MATLAB)
  target_link_libraries(griddyn_shared_lib gridDynCombined -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export.txt)
ELSE(WITH_MATLAB)
  target_link_libraries(griddyn_shared_lib gridDynCombined)
ENDIF(WITH_MATLAB)


INSTALL(TARGETS griddyn_shared_lib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.h")

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.hpp")
