# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# file to include KLU
cmake_dependent_advanced_option(${PROJECT_NAME}_USE_SYSTEM_SUITESPARSE_ONLY
       "only search for system suitesparse libraries, bypass local build options" OFF "${PROJECT_NAME}_ENABLE_KLU"
        OFF)

if(MSVC)
    cmake_dependent_advanced_option(
        ${PROJECT_NAME}_SUITESPARSE_SUBPROJECT
        "enable Suitesparse to automatically download and include as a subproject"
        ON
        "NOT ${PROJECT_NAME}_USE_SYSTEM_SUITESPARSE_ONLY"
        OFF
    )
else()
    cmake_dependent_advanced_option(
        ${PROJECT_NAME}_SUITESPARSE_SUBPROJECT
        "enable Suitesparse to automatically download and include as a subproject"
        OFF
        "NOT ${PROJECT_NAME}_USE_SYSTEM_SUITESPARSE_ONLY"
        OFF
    )
endif()
cmake_dependent_advanced_option(
    ${PROJECT_NAME}_SUITESPARSE_FORCE_SUBPROJECT
    "force SUITESPARSE to automatically download and include as a subproject"
    OFF
    "NOT ${PROJECT_NAME}_USE_SYSTEM_SUITESPARSE_ONLY"
    OFF
)

cmake_dependent_advanced_option(${PROJECT_NAME}_USE_SUITESPARSE_STATIC_LIBRARY "use the suitesparse static library" OFF  "NOT ${PROJECT_NAME}_USE_SYSTEM_SUITESPARSE_ONLY"
        OFF)


if(${PROJECT_NAME}_USE_SYSTEM_SUITESPARSE_ONLY)
    find_package(KLU)
    set(${PROJECT_NAME}_SUITESPARSE_LOCAL_BUILD OFF CACHE INTERNAL "")
elseif(${PROJECT_NAME}_SUITESPARSE_FORCE_SUBPROJECT)
    include(addlibSuiteSparse)
else()

    show_variable(SuiteSparse_INSTALL_PATH PATH "path to the suitesparse libraries" "")

    mark_as_advanced(SuiteSparse_INSTALL_PATH)

    set(SUITESPARSE_CMAKE_SUFFIXES cmake/SuiteSparse cmake CMake/SuiteSparse lib/cmake)

    if(WIN32 AND NOT MSYS)
        find_package(
            KLU
            QUIET
            HINTS
            ${SuiteSparse_INSTALL_PATH}
            $ENV{SuiteSparse_INSTALL_PATH}
            PATH_SUFFIXES
            ${SUITESPARSE_CMAKE_SUFFIXES}
        )
    else()
        find_package(
            KLU
            QUIET
            HINTS
            ${SuiteSparse_INSTALL_PATH}
            $ENV{SuiteSparse_INSTALL_PATH}
            PATH_SUFFIXES
            ${SUITESPARSE_CMAKE_SUFFIXES}
            NO_SYSTEM_ENVIRONMENT_PATH
            NO_CMAKE_PACKAGE_REGISTRY
            NO_CMAKE_SYSTEM_PATH
            NO_CMAKE_SYSTEM_PACKAGE_REGISTRY
        )
    endif()

    if(NOT KLU_FOUND)
        find_package(KLU)
        if(NOT KLU_FOUND)
            if(${PROJECT_NAME}_SUITESPARSE_SUBPROJECT)
                include(addlibSuiteSparse)
            else()
                show_variable(KLU_DEBUG_LIBRARY FILEPATH
                              "path to the KLU debug library" "")
                show_variable(KLU_LIBRARY FILEPATH "path to the KLU library" "")
                show_variable(KLU_ROOT_DIR PATH "path to the KLU root directory"
                              "")
                if(${PROJECT_NAME}_USE_SUITESPARSE_STATIC_LIBRARY)
                    show_variable(KLU_STATIC_LIBRARY FILEPATH
                                  "path to the KLU static library" "")
                endif()
                show_variable(KLU_INCLUDE_DIR PATH
                              "path to the KLU include directory" "")
            endif()
        else()
            set(${PROJECT_NAME}_SUITESPARSE_LOCAL_BUILD OFF CACHE INTERNAL "")
        endif()
    endif()

endif() # ${PROJECT_NAME}_USE_SYSTEM_SUTIESPARSE_ONLY
hide_variable(SuiteSparse_DIR)

if(WIN32)
    if(TARGET libzmq)
        install(
            FILES $<TARGET_FILE:libzmq>
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
        )
    endif()
endif()
