
# ---------------------------------------------------------------
# Programmer:  Philip Top @ LLNL
# ---------------------------------------------------------------
# -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
#
#LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
#LLNS Copyright End
#
# ---------------------------------------------------------------
# Find FMI library.
# 

set(HINTPATH /)



find_path (FMI_DIR include/fmilib.h HINTS ${HINTPATH} ${FMI_ROOT} DOC "FMI Directory")

set(FMI_INCLUDE_DIR ${FMI_DIR}/include)
set(FMI_LIBRARY_DIR ${FMI_DIR}/lib)

IF (USE_FMI_SHARED_LIBRARY)
set(FMI_LIBRARY_NAME fmilib_shared)
ELSE (USE_FMI_SHARED_LIBRARY)
set(FMI_LIBRARY_NAME fmilib)
ENDIF(USE_FMI_SHARED_LIBRARY)
    

# find library path using potential names for static and/or shared libs
set(temp_FMI_LIBRARY_DIR ${FMI_LIBRARY_DIR})
unset(FMI_LIBRARY_DIR CACHE)  
find_path(FMI_LIBRARY_DIR
  NAMES lib${FMI_LIBRARY_NAME}.so lib${FMI_LIBRARY_NAME}.a ${FMI_LIBRARY_NAME}.lib
  PATHS ${temp_FMI_LIBRARY_DIR}
  )

FIND_LIBRARY( FMI_LIBRARY ${FMI_LIBRARY_NAME} ${FMI_LIBRARY_DIR} NO_DEFAULT_PATH)

message(STATUS "searching for ${FMI_LIBRARY_NAME}")

set(FMI_LIBRARIES ${FMI_LIBRARY})

message(STATUS "fmi libraries= ${FMI_LIBRARIES}")

