# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Copyright (c) 2014-2020, Lawrence Livermore National Security
# See the top-level NOTICE for additional details. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


IF (MSVC)
    set(HELICS_PATH_HINTS
        C:/local/helics_2_3_0
        )
ENDIF(MSVC)

SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")


set(HELICS_CMAKE_SUFFIXES
    lib/cmake/HELICS/
            cmake/HELICS/)

find_package(HELICS 2.3
    HINTS
        ${HELICS_INSTALL_PATH}
        $ENV{HELICS_INSTALL_PATH}
        ${HELICS_PATH_HINTS}
    PATH_SUFFIXES ${HELICS_CMAKE_SUFFIXES}
    )

if (NOT HELICS_FOUND)
        message(FATAL_ERROR "unable to locate HELICS for linking")
endif()
