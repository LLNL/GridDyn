# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(fmiImport_sources
	fmi_import/fmiImport.cpp
	fmi_import/fmiInfo.cpp
	fmi_import/fmi2Object.cpp
	fmi_import/fmi2ModelExchangeObject.cpp
	fmi_import/fmi2CoSimObject.cpp
	fmi_import/fmiVariableSet.cpp
	fmi_import/fmiEnumDefinitions.cpp
	fmi_import/fmiLibraryManager.cpp
	
	)

set(fmiGD_sources
	fmiGDinfo.cpp
	fmi_models/fmiMESubModel.cpp
	fmi_models/fmiCoSimSubModel.cpp
	fmi_models/fmiMELoad.cpp
	fmi_models/fmiCoSimLoad.cpp
	fmi_models/fmiMELoad3phase.cpp
	fmi_models/fmiCoSimLoad3phase.cpp
	fmi_models/fmiExciter.cpp
	fmi_models/outputEstimator.cpp
	fmi_models/fmiGovernor.cpp
	fmi_models/fmiGenModel.cpp
	)
	
set(fmiGD_headers
	fmi_models/fmiMESubModel.h
	fmi_models/fmiCoSimSubModel.h 
	fmi_models/fmiMELoad.h
	fmi_models/fmiCoSimLoad.h
	fmi_models/fmiMELoad3phase.h
	fmi_models/fmiCoSimLoad3phase.h
	fmi_models/fmiExciter.h
	fmi_models/fmiGovernor.h
	fmi_models/fmiGenModel.h
	fmi_models/outputEstimator.h
	fmi_models/fmiSupport.h
	fmi_models/fmiMEWrapper.hpp
	fmi_models/fmiCoSimWrapper.hpp
	fmi_models/fmiWrapper.hpp
	fmiGDinfo.h
	)	
set(fmiImport_headers
	fmi_import/fmiInfo.h
	fmi_import/fmiImport.h
	fmi_import/fmiObjects.h
	fmi_import/fmiEnumDefinitions.h
	fmi_import/fmiLibraryManager.h
	)

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

IF(UNIX)
  add_compile_options(-fvisibility=hidden)
ELSE(UNIX)
  IF(MINGW)
 add_compile_options(-fvisibility=hidden)
  ENDIF(MINGW)
ENDIF(UNIX)



add_library(fmiGDLibrary ${fmiGD_sources} ${fmiImport_sources} ${fmiImport_headers} ${fmiGD_headers})

source_group("gridDynFMU" FILES ${fmiGD_sources} ${fmiGD_headers})
source_group("FMUImport" FILES ${fmiImport_sources} ${fmiImport_headers})

INCLUDE_DIRECTORIES(.)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/plugins)


list(APPEND griddyn_optional_libraries fmiGDLibrary)
list(APPEND griddyn_optional_include_dirs ${PROJECT_SOURCE_DIR}/src/fmi)

set(griddyn_optional_libraries ${griddyn_optional_libraries} PARENT_SCOPE)
set(griddyn_optional_include_dirs ${griddyn_optional_include_dirs} PARENT_SCOPE)


list (APPEND optional_library_key_headers fmiGDinfo.h)
list (APPEND optional_library_functions loadFmiLibrary)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)


list(APPEND optional_component_test_files ${PROJECT_SOURCE_DIR}/test/componentTests/testFMI.cpp)
set (optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.h")

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.hpp")
