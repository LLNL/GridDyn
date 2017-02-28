
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

# - Check for the presence of JSONCPP
#
# The following variables are set when JSONCPP is found:
#  JSONCPP_FOUND      = Set to true, if all components of JSONCPP have been found.
#  JSONCPP_INCLUDES   = Include path for the header files of JSONCPP
#  JSONCPP_LIBRARIES  = Link these to use JSONCPP
#  JSONCPP_LFLAGS     = Linker flags (optional)

if (NOT JSONCPP_FOUND)

  if (NOT JSONCPP_ROOT_DIR)
    set (JSONCPP_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
  endif (NOT JSONCPP_ROOT_DIR)

  ##_____________________________________________________________________________
  ## Check for the header files

  find_path (JSONCPP_INCLUDES
    NAMES json/json.h
    HINTS ${JSONCPP_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES include
    )

  ##_____________________________________________________________________________
  ## Check for the release library

  find_library (JSONCPP_LIBRARIES_RELEASE
    NAMES json jsoncpp
    HINTS ${JSONCPP_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib
    )

 ##_____________________________________________________________________________
  ## Check for a debug version

  find_library (JSONCPP_LIBRARIES_DEBUG 
    NAMES json jsoncpp jsoncppd
    HINTS ${JSONCPP_ROOT_DIR}/debug ${CMAKE_INSTALL_PREFIX}/debug
    PATH_SUFFIXES lib
    )

IF(JSONCPP_LIBRARIES_DEBUG)
set(JSONCPP_LIBRARIES optimized ${JSONCPP_LIBRARIES_RELEASE} debug ${JSONCPP_LIBRARIES_DEBUG})
ELSEIF(JSONCPP_LIBRARIES_DEBUG)
set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARIES_RELEASE})
ENDIF(JSONCPP_LIBRARIES_DEBUG)
  ##_____________________________________________________________________________
  ## Actions taken when all components have been found

  if (JSONCPP_INCLUDES AND JSONCPP_LIBRARIES)
    set (JSONCPP_FOUND TRUE)
  else (JSONCPP_INCLUDES AND JSONCPP_LIBRARIES)
    set (JSONCPP_FOUND FALSE)
    if (NOT JSONCPP_FIND_QUIETLY)
      if (NOT JSONCPP_INCLUDES)
	message (STATUS "Unable to find JSONCPP header files!")
      endif (NOT JSONCPP_INCLUDES)
      if (NOT JSONCPP_LIBRARIES)
	message (STATUS "Unable to find JSONCPP library files!")
      endif (NOT JSONCPP_LIBRARIES)
    endif (NOT JSONCPP_FIND_QUIETLY)
  endif (JSONCPP_INCLUDES AND JSONCPP_LIBRARIES)

  if (JSONCPP_FOUND)
    if (NOT JSONCPP_FIND_QUIETLY)
      message (STATUS "Found components for JSONCPP")
      message (STATUS "JSONCPP_ROOT_DIR  = ${JSONCPP_ROOT_DIR}")
      message (STATUS "JSONCPP_INCLUDES  = ${JSONCPP_INCLUDES}")
      message (STATUS "JSONCPP_LIBRARIES = ${JSONCPP_LIBRARIES}")
    endif (NOT JSONCPP_FIND_QUIETLY)
  else (JSONCPP_FOUND)
    if (JSONCPP_FIND_REQUIRED)
      message (FATAL_ERROR "Could not find JSONCPP!")
    endif (JSONCPP_FIND_REQUIRED)
  endif (JSONCPP_FOUND)

  ##_____________________________________________________________________________
  ## Mark advanced variables

  mark_as_advanced (
    JSONCPP_ROOT_DIR
    JSONCPP_INCLUDES
    JSONCPP_LIBRARIES
    )

endif (NOT JSONCPP_FOUND)