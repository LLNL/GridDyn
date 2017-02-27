
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

# - Check for the presence of ZLIB
#
# The following variables are set when ZLIB is found:
#  ZLIB_FOUND      = Set to true, if all components of ZLIB have been found.
#  ZLIB_INCLUDES   = Include path for the header files of ZLIB
#  ZLIB_LIBRARIES  = Link these to use ZLIB
#  ZLIB_LFLAGS     = Linker flags (optional)

if (NOT ZLIB_FOUND)

  if (NOT ZLIB_ROOT_DIR)
    set (ZLIB_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
  endif (NOT ZLIB_ROOT_DIR)

  ##_____________________________________________________________________________
  ## Check for the header files

  find_path (ZLIB_INCLUDES
    NAMES zlib.h
    HINTS ${ZLIB_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES include
    )

  ##_____________________________________________________________________________
  ## Check for the library

set(lib_names libzlib zlib)
if(UNIX)
list(APPEND lib_names libz z)
endif(UNIX)

  find_library (ZLIB_LIBRARIES
    NAMES ${lib_names}
    HINTS ${ZLIB_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib
    )

  ##_____________________________________________________________________________
  ## Actions taken when all components have been found

  if (ZLIB_INCLUDES AND ZLIB_LIBRARIES)
    set (ZLIB_FOUND TRUE)
  else (ZLIB_INCLUDES AND ZLIB_LIBRARIES)
    set (ZLIB_FOUND FALSE)
    if (NOT ZLIB_FIND_QUIETLY)
      if (NOT ZLIB_INCLUDES)
	message (STATUS "Unable to find ZLIB header files!")
      endif (NOT ZLIB_INCLUDES)
      if (NOT ZLIB_LIBRARIES)
	message (STATUS "Unable to find ZLIB library files!")
      endif (NOT ZLIB_LIBRARIES)
    endif (NOT ZLIB_FIND_QUIETLY)
  endif (ZLIB_INCLUDES AND ZLIB_LIBRARIES)

  if (ZLIB_FOUND)
    if (NOT ZLIB_FIND_QUIETLY)
      message (STATUS "Found components for ZLIB")
      message (STATUS "ZLIB_ROOT_DIR  = ${ZLIB_ROOT_DIR}")
      message (STATUS "ZLIB_INCLUDES  = ${ZLIB_INCLUDES}")
      message (STATUS "ZLIB_LIBRARIES = ${ZLIB_LIBRARIES}")
    endif (NOT ZLIB_FIND_QUIETLY)
  else (ZLIB_FOUND)
    if (ZLIB_FIND_REQUIRED)
      message (FATAL_ERROR "Could not find ZLIB!")
    endif (ZLIB_FIND_REQUIRED)
  endif (ZLIB_FOUND)

  ##_____________________________________________________________________________
  ## Mark advanced variables

  mark_as_advanced (
    ZLIB_ROOT_DIR
    ZLIB_INCLUDES
    ZLIB_LIBRARIES
    )

endif (NOT ZLIB_FOUND)
