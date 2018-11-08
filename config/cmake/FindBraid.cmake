# LLNS Copyright Start
# Copyright (c) 2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#-----------------------------------------------------------------------------
# - Find Braid library.
#
# This module finds if Braid is installed and determines where the
# include files and libraries are.  This code sets the following variables:
#  BRAID_FOUND         = Braid was found
#  BRAID_INCLUDE_DIR   = path to where header files can be found
#  BRAID_LIBRARY     = link library for Braid
#-----------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)

find_path (BRAID_DIR braid.h
    HINTS ${BRAID_INSTALL_DIR}
    DOC "BRAID Directory")

if (BRAID_DIR)
    set(BRAID_FOUND YES)

    set(BRAID_INCLUDE_DIR ${BRAID_DIR})

    find_library( BRAID_LIBRARY
        NAMES braid
        HINTS ${BRAID_DIR}
        NO_DEFAULT_PATH)

    add_library(braid::braid STATIC IMPORTED)
    set_property(TARGET braid::braid PROPERTY IMPORTED_LOCATION ${BRAID_LIBRARY})
    set_property(TARGET braid::braid PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${BRAID_INCLUDE_DIR})
else(BRAID_DIR)
    set(BRAID_FOUND FALSE)
endif(BRAID_DIR)

find_package_handle_standard_args(BRAID DEFAULT_MSG BRAID_LIBRARY BRAID_INCLUDE_DIR)
