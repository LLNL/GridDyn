
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

# - Check for the presence of TINYXML2
#
# The following variables are set when TINYXML2 is found:
#  TINYXML2_FOUND      = Set to true, if all components of TINYXML2 have been found.
#  TINYXML2_INCLUDES   = Include path for the header files of TINYXML2
#  TINYXML2_LIBRARIES  = Link these to use TINYXML2
#  TINYXML2_LFLAGS     = Linker flags (optional)

if (NOT TINYXML2_FOUND)

  if (NOT TINYXML2_ROOT_DIR)
    set (TINYXML2_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
  endif (NOT TINYXML2_ROOT_DIR)

  ##_____________________________________________________________________________
  ## Check for the header files

  find_path (TINYXML2_INCLUDES
    NAMES tinyxml2/tinyxml2.h
    HINTS ${TINYXML2_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES include
    )

  ##_____________________________________________________________________________
  ## Check for the release library

  find_library (TINYXML2_LIBRARIES_RELEASE
    NAMES tinyxml2
    HINTS ${TINYXML2_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib
    )

 ##_____________________________________________________________________________
  ## Check for a debug version

  find_library (TINYXML2_LIBRARIES_DEBUG 
    NAMES tinyxml2 tinyxml2d
    HINTS ${TINYXML2_ROOT_DIR}/debug ${CMAKE_INSTALL_PREFIX}/debug
    PATH_SUFFIXES lib
    )

IF(TINYXML2_LIBRARIES_DEBUG)
set(TINYXML2_LIBRARIES optimized ${TINYXML2_LIBRARIES_RELEASE} debug ${TINYXML2_LIBRARIES_DEBUG})
ELSEIF(TINYXML2_LIBRARIES_DEBUG)
set(TINYXML2_LIBRARIES ${TINYXML2_LIBRARIES_RELEASE})
ENDIF(TINYXML2_LIBRARIES_DEBUG)

  ##_____________________________________________________________________________
  ## Actions taken when all components have been found

  if (TINYXML2_INCLUDES AND TINYXML2_LIBRARIES)
    set (TINYXML2_FOUND TRUE)
  else (TINYXML2_INCLUDES AND TINYXML2_LIBRARIES)
    set (TINYXML2_FOUND FALSE)
    if (NOT TINYXML2_FIND_QUIETLY)
      if (NOT TINYXML2_INCLUDES)
	message (STATUS "Unable to find TINYXML2 header files!")
      endif (NOT TINYXML2_INCLUDES)
      if (NOT TINYXML2_LIBRARIES)
	message (STATUS "Unable to find TINYXML2 library files!")
      endif (NOT TINYXML2_LIBRARIES)
    endif (NOT TINYXML2_FIND_QUIETLY)
  endif (TINYXML2_INCLUDES AND TINYXML2_LIBRARIES)

  if (TINYXML2_FOUND)
    if (NOT TINYXML2_FIND_QUIETLY)
      message (STATUS "Found components for TINYXML2")
      message (STATUS "TINYXML2_ROOT_DIR  = ${TINYXML2_ROOT_DIR}")
      message (STATUS "TINYXML2_INCLUDES  = ${TINYXML2_INCLUDES}")
      message (STATUS "TINYXML2_LIBRARIES = ${TINYXML2_LIBRARIES}")
    endif (NOT TINYXML2_FIND_QUIETLY)
  else (TINYXML2_FOUND)
    if (TINYXML2_FIND_REQUIRED)
      message (FATAL_ERROR "Could not find TINYXML2!")
    endif (TINYXML2_FIND_REQUIRED)
  endif (TINYXML2_FOUND)

  ##_____________________________________________________________________________
  ## Mark advanced variables

  mark_as_advanced (
    TINYXML2_ROOT_DIR
    TINYXML2_INCLUDES
    TINYXML2_LIBRARIES
    )

endif (NOT TINYXML2_FOUND)