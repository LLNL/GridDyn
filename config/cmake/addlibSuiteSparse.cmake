# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#
# This file is used to add suitesparse to a project
#

if(${PROJECT_NAME}_USE_SUITESPARSE_STATIC_LIBRARY)
    set(suitesparse_static_build ON)
    set(suitesparse_shared_build OFF)
else()
    set(suitesparse_static_build OFF)
    set(suitesparse_shared_build ON)
endif()

if(MINGW)
    set(${PROJECT_NAME}_SUITESPARSE_VERSION v4.3.1)
else()
    set(${PROJECT_NAME}_SUITESPARSE_VERSION v4.3.2)
endif()

string(TOLOWER "suitesparse" lcName)

if(NOT CMAKE_VERSION VERSION_LESS 3.11)
    include(FetchContent)

    mark_as_advanced(FETCHCONTENT_BASE_DIR)
    mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
    mark_as_advanced(FETCHCONTENT_QUIET)
    mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)

    fetchcontent_declare(
        suitesparse
        GIT_REPOSITORY https://github.com/jlblancoc/suitesparse-metis-for-windows.git
        GIT_TAG ${${PROJECT_NAME}_SUTIESPARSE_VERSION}
    )

    fetchcontent_getproperties(suitesparse)

    if(NOT ${lcName}_POPULATED)
        # Fetch the content using previously declared details
        fetchcontent_populate(suitesparse)

    endif()

    hide_variable(FETCHCONTENT_SOURCE_DIR_SUITESPARSE)
    hide_variable(FETCHCONTENT_UPDATES_DISCONNECTED_SUITESPARSE)
else() # CMake <3.11

    # create the directory first
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/_deps)

    include(GitUtils)
    git_clone(
        PROJECT_NAME
        ${lcName}
        GIT_URL
        https://github.com/jlblancoc/suitesparse-metis-for-windows.git
        GIT_TAG
        ${${PROJECT_NAME}_SUITESPARSE_VERSION}
        DIRECTORY
        ${PROJECT_BINARY_DIR}/_deps
    )

    set(${lcName}_BINARY_DIR ${PROJECT_BINARY_DIR}/_deps/${lcName}-build)

endif()

# Set custom variab
if (MSVC OR MINGW)
    set(LAPACK_DIR ${${lcName}_SOURCE_DIR}/lapack_windows/x64 CACHE INTERNAL "")
    endif()

set(${PROJECT_NAME}_SUITESPARSE_LOCAL_BUILD
    ON
    CACHE INTERNAL ""
)
set(KLU_INCLUDE_DIR "${${lcName}_SOURCE_DIR}/Suitesparse/KLU/Include" 
					"${${lcName}_SOURCE_DIR}/Suitesparse/AMD/Include" 
                    "${${lcName}_SOURCE_DIR}/Suitesparse/SuiteSparse_config" 
					"${${lcName}_SOURCE_DIR}/Suitesparse/COLAMD/Include"
					"${${lcName}_SOURCE_DIR}/Suitesparse/BTF/Include" 
					CACHE INTERNAL "")

if (NOT EXISTS ${${lcName}_SOURCE_DIR}/checkGetSuiteSparse-old.cmake)
    file(RENAME ${${lcName}_SOURCE_DIR}/checkGetSuiteSparse.cmake
             ${${lcName}_SOURCE_DIR}/checkGetSuiteSparse-old.cmake
        )
    file(COPY ${PROJECT_SOURCE_DIR}/config/cmake/checkGetSuiteSparse-griddyn.cmake 
              ${PROJECT_SOURCE_DIR}/config/cmake/CMakeLists-suitesparse.txt
              DESTINATION
             ${${lcName}_SOURCE_DIR})
             
     file(RENAME ${${lcName}_SOURCE_DIR}/CMakeLists.txt
             ${${lcName}_SOURCE_DIR}/CMakeLists.old
        )
     file(RENAME ${${lcName}_SOURCE_DIR}/checkGetSuiteSparse-griddyn.cmake 
             ${${lcName}_SOURCE_DIR}/checkGetSuiteSparse.cmake
             )
             
       file(RENAME ${${lcName}_SOURCE_DIR}/CMakeLists-suitesparse.txt
             ${${lcName}_SOURCE_DIR}/CMakeLists.txt
             )
endif()

add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR} EXCLUDE_FROM_ALL)

set(SuiteSparse_FOUND ON CACHE INTERNAL "")

set(SuiteSparse_LIBRARIES klu btf amd colamd suitesparseconfig)

set_target_properties(${SuiteSparse_LIBRARIES} PROPERTIES FOLDER "klu")

# hide a bunch of local variables and options


if(${PROJECT_NAME}_USE_SUITESPARSE_STATIC_LIBRARY)
    set(klu_primary_target klu)
else()
    set(klu_primary_target klu)
endif()

if(${PROJECT_NAME}_BUILD_CXX_SHARED_LIB OR NOT ${PROJECT_NAME}_DISABLE_C_SHARED_LIB)

    if(NOT ${PROJECT_NAME}_USE_SUITESPARSE_STATIC_LIBRARY)
        set_target_properties(${klu_primary_target} PROPERTIES PUBLIC_HEADER "")
        #[[ if(NOT CMAKE_VERSION VERSION_LESS "3.13")
            install(
                TARGETS ${klu_primary_target}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                FRAMEWORK DESTINATION "Library/Frameworks"
            )
        elseif(WIN32)
            install(
                FILES $<TARGET_FILE:${klu_primary_target}>
                DESTINATION ${CMAKE_INSTALL_BINDIR}
                COMPONENT libs
            )
        else()
            message(
                WARNING
                    "Update to CMake 3.13+ or enable the ${PROJECT_NAME}_USE_SUITESPARSE_STATIC_LIBRARY CMake option to install when using suitesparse as a subproject"
            )
        endif()
        if(MSVC
           AND NOT EMBEDDED_DEBUG_INFO
           AND NOT ${PROJECT_NAME}_BINARY_ONLY_INSTALL
        )
            install(
                FILES $<TARGET_PDB_FILE:${klu_primary_target}>
                DESTINATION ${CMAKE_INSTALL_BINDIR}
                OPTIONAL
                COMPONENT libs
            )
        endif()
        if(MSVC AND NOT ${PROJECT_NAME}_BINARY_ONLY_INSTALL)
            install(
                FILES $<TARGET_LINKER_FILE:${klu_primary_target}>
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
                COMPONENT libs
            )
        endif()
]]
    endif()

endif()
