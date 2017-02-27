
#--------------------------------------------------------------------------------
# Copyright (c) 2012-2013, Lars Baehren <lbaehren@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#--------------------------------------------------------------------------------

# - Check for the presence of TICPP
#
# The following variables are set when TICPP is found:
#  TICPP_FOUND      = Set to true, if all components of TICPP have been found.
#  TICPP_INCLUDES   = Include path for the header files of TICPP
#  TICPP_LIBRARIES  = Link these to use TICPP
#  TICPP_LFLAGS     = Linker flags (optional)

if (NOT TICPP_FOUND)

  if (NOT TICPP_ROOT_DIR)
    set (TICPP_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
  endif (NOT TICPP_ROOT_DIR)

  ##_____________________________________________________________________________
  ## Check for the header files

  find_path (TICPP_INCLUDES
    NAMES ticpp/ticpp.h
    HINTS ${TICPP_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES include
    )

  ##_____________________________________________________________________________
  ## Check for the release library

  find_library (TICPP_LIBRARIES_RELEASE
    NAMES ticpp
    HINTS ${TICPP_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib
    )

 ##_____________________________________________________________________________
  ## Check for a debug version

  find_library (TICPP_LIBRARIES_DEBUG 
    NAMES ticpp ticppd
    HINTS ${TICPP_ROOT_DIR}/debug ${CMAKE_INSTALL_PREFIX}/debug
    PATH_SUFFIXES lib
    )

IF(TICPP_LIBRARIES_DEBUG)
set(TICPP_LIBRARIES optimized ${TICPP_LIBRARIES_RELEASE} debug ${TICPP_LIBRARIES_DEBUG})
ELSEIF(TICPP_LIBRARIES_DEBUG)
set(TICPP_LIBRARIES ${TICPP_LIBRARIES_RELEASE})
ENDIF(TICPP_LIBRARIES_DEBUG)
  ##_____________________________________________________________________________
  ## Actions taken when all components have been found

  if (TICPP_INCLUDES AND TICPP_LIBRARIES)
    set (TICPP_FOUND TRUE)
  else (TICPP_INCLUDES AND TICPP_LIBRARIES)
    set (TICPP_FOUND FALSE)
    if (NOT TICPP_FIND_QUIETLY)
      if (NOT TICPP_INCLUDES)
	message (STATUS "Unable to find TICPP header files!")
      endif (NOT TICPP_INCLUDES)
      if (NOT TICPP_LIBRARIES)
	message (STATUS "Unable to find TICPP library files!")
      endif (NOT TICPP_LIBRARIES)
    endif (NOT TICPP_FIND_QUIETLY)
  endif (TICPP_INCLUDES AND TICPP_LIBRARIES)

  if (TICPP_FOUND)
    if (NOT TICPP_FIND_QUIETLY)
      message (STATUS "Found components for TICPP")
      message (STATUS "TICPP_ROOT_DIR  = ${TICPP_ROOT_DIR}")
      message (STATUS "TICPP_INCLUDES  = ${TICPP_INCLUDES}")
      message (STATUS "TICPP_LIBRARIES = ${TICPP_LIBRARIES}")
    endif (NOT TICPP_FIND_QUIETLY)
  else (TICPP_FOUND)
    if (TICPP_FIND_REQUIRED)
      message (FATAL_ERROR "Could not find TICPP!")
    endif (TICPP_FIND_REQUIRED)
  endif (TICPP_FOUND)

  ##_____________________________________________________________________________
  ## Mark advanced variables

  mark_as_advanced (
    TICPP_ROOT_DIR
    TICPP_INCLUDES
    TICPP_LIBRARIES
    )

endif (NOT TICPP_FOUND)