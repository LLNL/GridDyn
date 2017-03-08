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

set(fmiShared_headers
fmiRunner.h
fmiCoordinator.h
fmiCollector.h
fmiEvent.h
loadFMIExportObjects.h
)

set(fmiShared_sources
fmiGridDynExport.cpp
fmiRunner.cpp
fmiCoordinator.cpp
fmiCollector.cpp
fmiEvent.cpp
loadFMIExportObjects.cpp
)

set(fmiBuilder_headers
fmiCoordinator.h
fmiCollector.h
fmiEvent.h
loadFMIExportObjects.h
fmuBuilder.h
fmiRunner.h
)

set(fmiBuilder_sources
fmiCoordinator.cpp
fmiCollector.cpp
fmiEvent.cpp
loadFMIExportObjects.cpp
fmuBuilder.cpp
fmiRunner.cpp
fmiGridDynExport.cpp

)

add_library(fmiGridDynSharedLib SHARED ${fmiShared_sources} ${fmiShared_headers})

add_library(fmiBuilder STATIC ${fmiBuilder_sources} ${fmiBuilder_headers})
link_directories(${external_link_directories})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynCombined)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})



target_link_libraries(fmiGridDynSharedLib gridDynCombined)

set(GRIDYNFMILIBRARY_BINARY_LOC ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

INSTALL(TARGETS fmiGridDynSharedLib RUNTIME DESTINATION bin)

list(APPEND optional_component_test_files ${PROJECT_SOURCE_DIR}/fmi_export/testFMIExport.cpp)
set (optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)

list(APPEND executable_only_components fmiBuilder)
set(executable_only_components ${executable_only_components} PARENT_SCOPE)

