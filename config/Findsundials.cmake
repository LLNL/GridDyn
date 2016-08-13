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
# - Find Sundials includes and libraries.
#
# This module finds if Sundials is installed and determines where the
# include files and libraries are.  This code sets the following variables:
#  SUNDIALS_FOUND         = Sundials was found
#  SUNDIALS_LIBRARY_DIR   = path to where libraries can be found
#  SUNDIALS_INCLUDE_DIR   = path to where header files can be found
#  SUNDIALS_LIBRARIES     = link libraries for Sundials
#-----------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)

find_path (SUNDIALS_DIR include/sundials/sundials_config.h HINTS ${SUNDIALS_INSTALL_DIR} 
	  ENV PATHS $ENV{HOME}/sundials 
	  DOC "Sundials Directory")

message(STATUS "SUNDIALS_DIR=${SUNDIALS_DIR}")

if (SUNDIALS_DIR)
  SET(SUNDIALS_FOUND YES)

  SET(SUNDIALS_INCLUDE_DIR ${SUNDIALS_DIR}/include)
  set(SUNDIALS_LIBRARY_DIR ${SUNDIALS_DIR}/lib)

  # The set of required Sundials libraries
  set(SUNDIALS_REQUIRED_LIBS sundials_nvecserial sundials_ida sundials_kinsol)
  if (OPENMP_ENABLE)
    list(APPEND SUNDIALS_REQUIRED_LIBS sundials_nvecopenmp)
  endif(OPENMP_ENABLE)

  if (LOAD_CVODE)
	list(APPEND SUNDIALS_REQUIRED_LIBS sundials_cvode)
endif(LOAD_CVODE)
	
	 if (LOAD_ARKODE)
	list(APPEND SUNDIALS_REQUIRED_LIBS sundials_arkode)
endif(LOAD_ARKODE)
  
  foreach(TEST_LIB IN LISTS SUNDIALS_REQUIRED_LIBS)

    message(STATUS "Checking for TEST_LIB=${TEST_LIB}")

    # Need to make sure variable to search for isn't set
    unset(SUNDIALS_LIB CACHE)

    find_library(SUNDIALS_LIB
        NAMES ${TEST_LIB}
	HINTS ${SUNDIALS_LIBRARY_DIR}
	NO_DEFAULT_PATH)

    message(STATUS "SUNDIALS_LIB=${SUNDIALS_LIB}")
    if(SUNDIALS_LIB)
        list(APPEND SUNDIALS_LIBRARIES ${TEST_LIB})
    else(SUNDIALS_LIB)	    
        message(FATAL_ERROR "Could not find required Sundials library : ${TEST_LIB}")
    endif(SUNDIALS_LIB)
    
  endforeach(TEST_LIB)
  
  message(STATUS "CMAKE_FIND_LIBRARY_SUFFIXES=${CMAKE_FIND_LIBRARY_SUFFIXES}")
  message(STATUS "SUNDIALS_FOUND=${SUNDIALS_FOUND}")
  message(STATUS "SUNDIALS_LIBRARY_DIR=${SUNDIALS_LIBRARY_DIR}")
  message(STATUS "SUNDIALS_INCLUDE_DIR=${SUNDIALS_INCLUDE_DIR}")
  message(STATUS "SUNDIALS_LIBRARIES=${SUNDIALS_LIBRARIES}")

ELSE(SUNDIALS_DIR)
  SET(SUNDIALS_FOUND NO)
ENDIF(SUNDIALS_DIR)

find_package_handle_standard_args(SUNDIALS DEFAULT_MSG SUNDIALS_LIBRARIES SUNDIALS_INCLUDE_DIR)






