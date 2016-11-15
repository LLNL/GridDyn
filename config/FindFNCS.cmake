# LLNS Copyright Start
# Copyright (c) 2014, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#-----------------------------------------------------------------------------
# - Find FNCS library.
# 
# This module finds if FNCS is installed and determines where the
# include files and libraries are.  This code sets the following variables:
#  FNCS_FOUND         = FNCS was found
#  FNCS_LIBRARY_DIR   = path to where libraries can be found
#  FNCS_INCLUDE_DIR   = path to where header files can be found
#  FNCS_LIBRARIES     = link libraries for FNCS
#-----------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)

find_path (FNCS_DIR src/fncs.hpp 
	  HINTS ${FNCS_INSTALL_DIR} 
	  DOC "FNCS Directory")

if (FNCS_DIR)
  set(FNCS_FOUND YES)

  set(FNCS_INCLUDE_DIR ${FNCS_DIR}/include)
  set(FNCS_LIBRARY_DIR ${FNCS_DIR}/lib)

  set(FNCS_LIBRARY_NAME fncs)
    
  find_library( FNCS_LIBRARY 
                NAMES fncs
                HINTS ${FNCS_LIBRARY_DIR} 
		NO_DEFAULT_PATH)

  set(FNCS_LIBRARIES ${FNCS_LIBRARY})

else(FNCS_DIR)
  set(FNCS_FOUND NO)
endif(FNCS_DIR)

find_package_handle_standard_args(FNCS DEFAULT_MSG FNCS_LIBRARIES FNCS_INCLUDE_DIR)

