# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
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
	fmi_models/CymeDistLoad.cpp
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
	fmi_models/CymeDistLoad.h
	fmiGDinfo.h
	)
set(fmiImport_headers
	fmi_import/fmiInfo.h
	fmi_import/fmiImport.h
	fmi_import/fmiObjects.h
	fmi_import/fmiEnumDefinitions.h
	fmi_import/fmiLibraryManager.h
	)

add_library(fmiGDLibrary STATIC ${fmiGD_sources} ${fmiImport_sources} ${fmiImport_headers} ${fmiGD_headers})

source_group("gridDynFMU" FILES ${fmiGD_sources} ${fmiGD_headers})
source_group("FMUImport" FILES ${fmiImport_sources} ${fmiImport_headers})

set_target_properties (fmiGDLibrary PROPERTIES FOLDER fmi)

target_link_libraries(fmiGDLibrary griddyn formatInterpreter)

INSTALL(TARGETS fmiGDLibrary
		EXPORT griddyn-targets
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT libraries)


list (APPEND optional_library_key_headers fmi/fmiGDinfo.h)
list (APPEND optional_library_functions loadFmiLibrary)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)


list(APPEND optional_component_test_files ${PROJECT_SOURCE_DIR}/test/componentTests/testFMI.cpp)
set (optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)

INSTALL(FILES ${fmiImport_headers} ${fmiGD_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn
  COMPONENT headers)
