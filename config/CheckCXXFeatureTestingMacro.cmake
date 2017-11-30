#.rst:
# CheckCXXFeatureTestingMacro
# -----------------
#
# Check if a CXX feature testing macro exists, and its value
#
# CHECK_CXX_FEATURE_TESTING_MACRO(<feature> <files> <variable> [<min_version>])
#
# Check that the <symbol> is available after including given header
# <files>, compare the result to <min_version> (or 0), and return <variable>.
# Specify the list of files in one argument as a semicolon-separated list.
# <variable> will be created as an internal cache variable.
#
# The following variables may be set before calling this macro to modify
# the way the check is run:
#
# ::
#
#   CMAKE_REQUIRED_FLAGS = string of compile command line flags
#   CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#   CMAKE_REQUIRED_INCLUDES = list of include directories
#   CMAKE_REQUIRED_LIBRARIES = list of libraries to link
#   CMAKE_REQUIRED_QUIET = execute quietly without messages

#=============================================================================
# Copyright 2003-2011 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

macro(CHECK_CXX_FEATURE_TESTING_MACRO SYMBOL FILES VARIABLE)
  set(extra_macro_args ${ARGN})
  list(LENGTH extra_macro_args num_extra_args)
  if(${num_extra_args} GREATER 0)
    list(GET extra_macro_args 0 EXPECTED_VALUE) 
  else()
    set(EXPECTED_VALUE 0)
  endif()
  if(CMAKE_CXX_COMPILER_LOADED)
    _CHECK_PREPROCESSOR_MACRO("${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckFeatureTestingMacro.cxx" "${SYMBOL}" "${FILES}" "${VARIABLE}" "${EXPECTED_VALUE}")
  else()
    message(FATAL_ERROR "CHECK_CXX_FEATURE_TESTING_MACRO needs CXX language enabled")
  endif()
endmacro()

macro(_CHECK_PREPROCESSOR_MACRO SOURCEFILE SYMBOL FILES VARIABLE EXPECTED_VALUE)
  if(NOT DEFINED "${VARIABLE}" OR "x${${VARIABLE}}" STREQUAL "x${VARIABLE}")
    set(CMAKE_CONFIGURABLE_FILE_CONTENT "/* */\n")
    set(MACRO_CHECK_SYMBOL_EXISTS_FLAGS ${CMAKE_REQUIRED_FLAGS})
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_SYMBOL_EXISTS_LIBS
        LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    else()
      set(CHECK_SYMBOL_EXISTS_LIBS)
    endif()
    if(CMAKE_REQUIRED_INCLUDES)
      set(CMAKE_SYMBOL_EXISTS_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    else()
      set(CMAKE_SYMBOL_EXISTS_INCLUDES)
    endif()
    foreach(FILE ${FILES})
      if(NOT "${FILE}" STREQUAL "")
        set(CMAKE_CONFIGURABLE_FILE_CONTENT
          "${CMAKE_CONFIGURABLE_FILE_CONTENT}#include <${FILE}>\n")
      endif()
    endforeach()
    set(CMAKE_CONFIGURABLE_FILE_CONTENT
      "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n#include <cstdlib>\nint main(int argc, char** argv)\n{\n  return atoi(argv[1]) > __cpp_${SYMBOL};\n}\n")

    configure_file("${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
      "${SOURCEFILE}" @ONLY)

    if(NOT CMAKE_REQUIRED_QUIET)
      message(STATUS "Looking for ${SYMBOL}")
    endif()
    try_run(${VARIABLE}_EXITCODE ${VARIABLE}_COMPILED
      ${CMAKE_BINARY_DIR}
      "${SOURCEFILE}"
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      ${CHECK_SYMBOL_EXISTS_LIBS}
      CMAKE_FLAGS
        -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_SYMBOL_EXISTS_FLAGS}
        -DCMAKE_SKIP_RPATH:BOOL=${CMAKE_SKIP_RPATH}
      "${CMAKE_SYMBOL_EXISTS_INCLUDES}"
      OUTPUT_VARIABLE OUTPUT
      ARGS "${EXPECTED_VALUE}")
    if(${VARIABLE}_COMPILED)
     if("${${VARIABLE}_EXITCODE}" EQUAL 0)
       if(NOT CMAKE_REQUIRED_QUIET)
          message(STATUS "Looking for ${SYMBOL} - found")
        endif()
        set(${VARIABLE} 1 CACHE INTERNAL "Have symbol ${SYMBOL}")
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
          "Determining if the ${SYMBOL} "
          "exist passed with the following output:\n"
          "${OUTPUT}\nFile ${SOURCEFILE}:\n"
          "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
      else()
        if(NOT CMAKE_REQUIRED_QUIET)
          message(STATUS "Looking for ${SYMBOL} - old version")
        endif()
        set(${VARIABLE} "" CACHE INTERNAL "Have symbol ${SYMBOL}")
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
          "Determining if the ${SYMBOL} "
          "exist failed with the following output:\n"
          "${OUTPUT}\nFile ${SOURCEFILE}:\n"
          "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
      endif()
    else()
      if(NOT CMAKE_REQUIRED_QUIET)
        message(STATUS "Looking for ${SYMBOL} - not found")
      endif()
      set(${VARIABLE} "" CACHE INTERNAL "Have symbol ${SYMBOL}")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if the ${SYMBOL} "
        "exist failed with the following output:\n"
        "${OUTPUT}\nFile ${SOURCEFILE}:\n"
        "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
    endif()
  endif()
endmacro()
