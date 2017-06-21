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
# - Find HELICS library.
# 
# This module finds if HELICS is installed and determines where the
# include files and libraries are.  This code sets the following variables:
#  HELICS_FOUND         = HELICS was found
#  HELICS_LIBRARY_DIR   = path to where libraries can be found
#  HELICS_INCLUDE_DIR   = path to where header files can be found
#  HELICS_LIBRARIES     = link libraries for HELICS
#-----------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)

find_path (HELICS_DIR include/helics/application_api/application_api.h
	  HINTS ${HELICS_INSTALL_PATH} 
	  DOC "HELICS Directory")

message(STATUS "${HELICS_INSTALL_PATH}  __${HELICS_DIR}")
if (HELICS_DIR)
  set(HELICS_FOUND YES)

  set(HELICS_INCLUDE_DIR ${HELICS_DIR}/include)
  set(HELICS_LIBRARY_DIR ${HELICS_DIR}/lib)

  set(HELICS_LIBRARY_NAME helics)
    
  find_library( HELICS_LIBRARY_RELEASE 
                NAMES helics
                HINTS ${HELICS_LIBRARY_DIR} 
		NO_DEFAULT_PATH)

 ##_____________________________________________________________________________
  ## Check for a debug version

find_library( HELICS_LIBRARY_DEBUG
                NAMES helics helicsd
                HINTS ${HELICS_LIBRARY_DIR}/debug ${CMAKE_INSTALL_PREFIX}/debug ${HELICS_DIR}/debug
				PATH_SUFFIXES lib
		NO_DEFAULT_PATH)
		

IF(HELICS_LIBRARY_DEBUG)
set(HELICS_LIBRARIES optimized ${HELICS_LIBRARY_RELEASE} debug ${HELICS_LIBRARY_DEBUG})
ELSE()
set(HELICS_LIBRARIES ${HELICS_LIBRARY_RELEASE})
ENDIF()

	find_program(HELICS_BROKER 
		NAMES bin/helics_broker helics_broker
		HINTS ${HELICS_INSTALL_PATH})
	find_program(HELICS_PLAYER
		NAMES bin/helics_player helics_player
		HINTS ${HELICS_INSTALL_PATH})
	find_program(HELICS_RECORDER
		NAMES bin/helics_recorder helics_recorder
		HINTS ${HELICS_INSTALL_PATH})
else(HELICS_DIR)
  set(HELICS_FOUND NO)
endif(HELICS_DIR)

find_package_handle_standard_args(HELICS DEFAULT_MSG HELICS_LIBRARIES HELICS_INCLUDE_DIR)

