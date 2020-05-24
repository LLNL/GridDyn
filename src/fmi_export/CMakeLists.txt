# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(fmiShared_headers fmiRunner.h fmiCoordinator.h fmiCollector.h fmiEvent.h loadFMIExportObjects.h)

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

set_target_properties(
    fmiGridDynSharedLib PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden
)

set_target_properties(fmiGridDynSharedLib PROPERTIES FOLDER fmi)

add_library(fmiBuilder STATIC ${fmiBuilder_sources} ${fmiBuilder_headers})

target_link_libraries(fmiBuilder PUBLIC runner)

set_target_properties(fmiBuilder PROPERTIES FOLDER fmi)

target_link_libraries(fmiGridDynSharedLib PRIVATE runner)

install(
    TARGETS fmiGridDynSharedLib ${GRIDDYN_EXPORT_COMMAND}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(fmiGridDynSharedLib PROPERTIES NO_SONAME ON PREFIX "")
# Todo when minimum CMake version becomes 3.6: re-add EXCLUDE_FROM_ALL to these two install FILES
# commands for fmiGridDynSharedLib
install(FILES $<TARGET_FILE:fmiGridDynSharedLib> DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT RUNTIME
)
install(FILES $<TARGET_LINKER_FILE:fmiGridDynSharedLib> DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT libraries
)
if(WIN32)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        install(TARGETS fmiGridDynSharedLib RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/win64/
                                                    COMPONENT RUNTIME
        )
        set(FMILIBRARY_TYPE win64)
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        install(TARGETS fmiGridDynSharedLib RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/win32/
                                                    COMPONENT RUNTIME
        )
        set(FMILIBRARY_TYPE win32)
    endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
else(WIN32)
    if(UNIX)
        if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            install(TARGETS fmiGridDynSharedLib LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}/linux64/
                                                        COMPONENT RUNTIME
            )
            set(FMILIBRARY_TYPE linux64)
        else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            install(TARGETS fmiGridDynSharedLib LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}/linux32/
                                                        COMPONENT RUNTIME
            )
            set(FMILIBRARY_TYPE linux32)
        endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    else(UNIX)
        if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            install(TARGETS fmiGridDynSharedLib
                    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/darwin64/ COMPONENT RUNTIME
            )
            set(FMILIBRARY_TYPE darwin64)
        else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            install(TARGETS fmiGridDynSharedLib
                    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/darwin32/ COMPONENT RUNTIME
            )
            set(FMILIBRARY_TYPE darwin32)
        endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    endif(UNIX)
endif(WIN32)

target_compile_definitions(
    fmiBuilder PRIVATE -DGRIDDYNFMILIBRARY_LOC="$<TARGET_FILE_DIR:fmiGridDynSharedLib>"
)

target_compile_definitions(
    fmiBuilder PRIVATE -DGRIDDYNFMILIBRARY_NAME="$<TARGET_FILE_NAME:fmiGridDynSharedLib>"
)

target_compile_definitions(
    fmiBuilder PRIVATE -DGRIDDYNFMILIBRARY_BASE_NAME="$<TARGET_FILE_BASE_NAME:fmiGridDynSharedLib>"
)

target_compile_definitions(fmiBuilder PRIVATE -DFMILIBRARY_TYPE="${FMILIBRARY_TYPE}")

list(APPEND optional_component_test_files ${CMAKE_CURRENT_SOURCE_DIR}/testFMIExport.cpp)
set(optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)

install(FILES ${fmiBuilder_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/fmi_export
        COMPONENT headers
)
