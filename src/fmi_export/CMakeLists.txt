# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
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
target_include_directories(fmiGridDynSharedLib SYSTEM PRIVATE ${Boost_INCLUDE_DIR})

set_target_properties (fmiGridDynSharedLib PROPERTIES FOLDER fmi)

add_library(fmiBuilder STATIC ${fmiBuilder_sources} ${fmiBuilder_headers})
target_include_directories(fmiBuilder SYSTEM PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(fmiBuilder PUBLIC fileInput)

set_target_properties (fmiBuilder PROPERTIES FOLDER fmi)


target_link_libraries(fmiGridDynSharedLib PRIVATE gridDynCombined)

set(GRIDDYNFMILIBRARY_BINARY_LOC ${CMAKE_CURRENT_BINARY_DIR})

INSTALL(TARGETS fmiGridDynSharedLib EXPORT griddyn-targets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

INSTALL(FILES $<TARGET_FILE:fmiGridDynSharedLib> DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime EXCLUDE_FROM_ALL)
INSTALL(FILES $<TARGET_LINKER_FILE:fmiGridDynSharedLib> DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries EXCLUDE_FROM_ALL)
IF (WIN32)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		INSTALL(TARGETS fmiGridDynSharedLib RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/win64/ COMPONENT runtime)
		set(FMILIBRARY_TYPE win64)
	else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		INSTALL(TARGETS fmiGridDynSharedLib RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/win32/ COMPONENT runtime)
		set(FMILIBRARY_TYPE win32)
	endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
ELSE(WIN32)
 IF(UNIX)
   if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		INSTALL(TARGETS fmiGridDynSharedLib LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}/linux64/ COMPONENT runtime)
		set(FMILIBRARY_TYPE linux64 )
	else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		INSTALL(TARGETS fmiGridDynSharedLib LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}/linux32/ COMPONENT runtime)
		set(FMILIBRARY_TYPE linux32 )
	endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	ELSE(UNIX)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		INSTALL(TARGETS fmiGridDynSharedLib RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/darwin64/ COMPONENT runtime)
		set(FMILIBRARY_TYPE darwin64 )
	else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		INSTALL(TARGETS fmiGridDynSharedLib RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/darwin32/ COMPONENT runtime)
		set(FMILIBRARY_TYPE darwin32 )
	endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	ENDIF(UNIX)
ENDIF(WIN32)

target_compile_definitions(fmiBuilder PRIVATE -DGRIDDYNFMILIBRARY_BINARY_LOC="${GRIDDYNFMILIBRARY_BINARY_LOC}")
target_compile_definitions(fmiBuilder PRIVATE -DFMILIBRARY_TYPE="${FMILIBRARY_TYPE}")

list(APPEND optional_component_test_files ${PROJECT_SOURCE_DIR}/src/fmi_export/testFMIExport.cpp)
set (optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)

list(APPEND executable_only_components fmiBuilder)
set(executable_only_components ${executable_only_components} PARENT_SCOPE)

INSTALL(FILES ${fmiBuilder_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/fmi_export
  COMPONENT headers)
