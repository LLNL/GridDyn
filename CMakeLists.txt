# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project name
project(gridDyn)

#states that Cmake version > 2.8
cmake_minimum_required(VERSION 2.8)

#version number
set(GridDyn_VERSION_MAJOR 0)
set(GridDyn_VERSION_MINOR 4)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/config)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# -------------------------------------------------------------
# MACRO definitions
# -------------------------------------------------------------

# Macros to hide/show cached variables.
# These two macros can be used to "hide" or "show" in the
# list of cached variables various variables and/or options 
# that depend on other options.
# Note that once a variable is modified, it will preserve its
# value (hidding it merely makes it internal)

MACRO(HIDE_VARIABLE var)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE INTERNAL "")
  ENDIF(DEFINED ${var})
ENDMACRO(HIDE_VARIABLE)

MACRO(SHOW_VARIABLE var type doc default)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE "${type}" "${doc}" FORCE)
  ELSE(DEFINED ${var})
    SET(${var} "${default}" CACHE "${type}" "${doc}")
  ENDIF(DEFINED ${var})
ENDMACRO(SHOW_VARIABLE)


# Prohibit in-source build
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source build is not supported. Please, use an empty directory for building the project.")
ENDIF()


# -------------------------------------------------------------
# Find (and test) the KLU libraries
# -------------------------------------------------------------

OPTION(KLU_ENABLE "Enable KLU support" ON)

SHOW_VARIABLE(KLU_INSTALL_DIR PATH
  "KLU library directory" "${KLU_INSTALL_DIR}")

set(KLU_FOUND NO)

if(KLU_ENABLE)

  find_package(KLU)

  if(KLU_FOUND) 
  INCLUDE_DIRECTORIES(SYSTEM ${KLU_INCLUDE_DIR})
  else(KLU_FOUND)
	message( FATAL_ERROR "KLU not functional - support will not be provided")
	message( "Double check spelling specified libraries (search is case sensitive)")
  endif(KLU_FOUND)

endif(KLU_ENABLE)


# -------------------------------------------------------------
# Sundials
# -------------------------------------------------------------

#-------------------------------------------------------------
# Enable Cvode and Arkode modules
#-------------------------------------------------------------
OPTION(LOAD_CVODE "Enable CVODE Support" OFF)
OPTION(LOAD_ARKODE "Enable ARKODE support" OFF)

SHOW_VARIABLE(SUNDIALS_INSTALL_DIR PATH
  "sundials library directory" "${SUNDIALS_INSTALL_DIR}")
find_package(sundials)

# -------------------------------------------------------------
# FSKIT
# -------------------------------------------------------------

OPTION(FSKIT_ENABLE "Enable FSKIT support" OFF)

IF(FSKIT_ENABLE)
  find_package(FSKIT)
  INCLUDE_DIRECTORIES(SYSTEM ${FSKIT_INCLUDE_DIR})
  set(GRIDDYN_HAVE_FSKIT 1)
ENDIF(FSKIT_ENABLE)

# -------------------------------------------------------------
# Enable MPI support?
# -------------------------------------------------------------

OPTION(MPI_ENABLE "Enable MPI support" OFF)

IF(MPI_ENABLE)
  find_package(MPI)
  # SGS FIXME this should in configure.h somewhere perhaps?
  ADD_DEFINITIONS(-DHAVE_MPI)
ENDIF(MPI_ENABLE)

# -------------------------------------------------------------
# Enable OpenMP support?
# -------------------------------------------------------------
OPTION(OPENMP_ENABLE "Enable openMP support" ON)



IF(OPENMP_ENABLE)
  message(STATUS "****** finding OpenMP support")
  FIND_PACKAGE(OpenMP)
  IF(OPENMP_FOUND)
    ADD_DEFINITIONS(-DHAVE_OPENMP)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CDD_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
  ELSE(OPENMP_FOUND)
    message(STATUS "Disabling OpenMP support, could not determine compiler flags")
	set(OPENMP_ENABLE FALSE)
  ENDIF(OPENMP_FOUND)
ELSE(OPENMP_ENABLE)
  set(OPENMP_FOUND FALSE)
ENDIF(OPENMP_ENABLE)

IF (OPENMP_ENABLE)
	OPTION(SUNDIALS_OPENMP "Enable sundials NVector openMP implementation" ON)
	OPTION(GRIDDYN_OPENMP "Enable openmp internal to griddyn--not used yet" OFF)
ENDIF(OPENMP_ENABLE)
OPTION(THREAD_ENABLE "Enable multithreading support--not used yet" OFF)

IF(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)

  set(Boost_USE_MULTITHREADED      OFF)   # Needed if MT libraries not built
  add_definitions(-std=c++11 )
  add_definitions(-Wall -Wextra -Wshadow -pedantic -Wstrict-aliasing -Wunreachable-code)
ELSE(UNIX)
  IF(MINGW)
  add_definitions(-std=c++11)
  add_definitions(-Wall -Wextra -Wshadow -pedantic -Wstrict-aliasing -Wunreachable-code)
  ELSE(MINGW)
  set(Boost_USE_STATIC_LIBS ON)
  ENDIF(MINGW)
ENDIF(UNIX)

SHOW_VARIABLE(BOOST_ROOT PATH "Boost root directory" "${BOOST_ROOT}")

IF(FSKIT_ENABLE)
  find_package(Boost COMPONENTS program_options unit_test_framework filesystem serialization mpi system date_time REQUIRED)
ELSE(FSKIT_ENABLE)
  find_package(Boost COMPONENTS program_options unit_test_framework filesystem system date_time REQUIRED)
ENDIF(FSKIT_ENABLE)

message("Using Boost include files : ${Boost_INCLUDE_DIR}")
message("Using Boost libraries ${Boost_LIBRARY_DIRS}")



# -------------------------------------------------------------
# Enable FMI support?
# -------------------------------------------------------------

OPTION(FMI_ENABLE "Enable FMI support" OFF)
# If FMI is enabled try to locate the libraries
# link against them.

if (FMI_ENABLE)
	set(FMI_ROOT ${PROJECT_SOURCE_DIR}/ThirdParty/FMILibrary-2.0.1/install)
	OPTION(USE_FMI_SHARED_LIBRARY "Use FMI shared library" ON)
	find_package(FMI)
	add_subdirectory(fmi)
ENDIF(FMI_ENABLE)


# -------------------------------------------------------------
# Enable compilation of extraModels?
# -------------------------------------------------------------

OPTION(LOAD_EXTRA_MODELS "Compile and load extraModels" ON)
# If extra models are used enabled try to locate the libraries
# link against them.

if (LOAD_EXTRA_MODELS)
  add_subdirectory(extraModels)
ENDIF(LOAD_EXTRA_MODELS)

# -------------------------------------------------------------
# Enable 64 bit indexing (enable to allow for more than 2^31 objects)--that would be a very big model
# -------------------------------------------------------------

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
  OPTION(ENABLE_64_BIT_INDEXING "set all indexing and count variables to 64 bit unsigned (Usually not required)" OFF)
endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
 
OPTION(BUILD_SERVERMODE "Build Griddyn Server" OFF)
IF (BUILD_SERVERMODE)
 add_subdirectory(gridDynServer)
ENDIF(BUILD_SERVERMODE)

# -------------------------------------------------------------
# Extra definitions
# -------------------------------------------------------------
IF(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-W4  /wd4065 /wd4101 /wd4102 /wd4244 /wd4297 /wd4355 /wd4800)
ENDIF(MSVC)

ADD_DEFINITIONS(-DTIXML_USE_TICPP)
# ------------------------------------------------------------
# Enable message logging
# -------------------------------------------------------------
OPTION(LOG_ENABLE "enable non-warning/error logging messages" ON)

IF(LOG_ENABLE)
OPTION(DEBUG_LOG_ENABLE "enable debug logging messages(Note: logging is still controlled via program options this is for program size and speed)" ON)
OPTION(TRACE_LOG_ENABLE "enable trace logging messages(Note: logging is still controlled via program options this is for program size and speed)" ON)
ENDIF(LOG_ENABLE)


option (GRIDDYN_GENERATE_DOXYGEN_DOC "Generate doxygen doc target" OFF)

IF (GRIDDYN_GENERATE_DOXYGEN_DOC)
find_package(Doxygen)
if(DOXYGEN_FOUND)

	SHOW_VARIABLE(DOXYGEN_OUTPUT_DIR PATH "location to put doxygen docs" "${CMAKE_CURRENT_SOURCE_DIR}/docs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
endif (GRIDDYN_GENERATE_DOXYGEN_DOC)



# -------------------------------------------------------------
# load the subdirectories
# -------------------------------------------------------------

add_subdirectory(ThirdParty/tinyxml)

add_subdirectory(core)

add_subdirectory(utilities)

add_subdirectory(gridDyn)

OPTION(OPTIMIZATION_ENABLE "Enable Optimization libraries" OFF)
# If OPTIMIZATION is enabled try to locate the libraries
# link against them.
IF(OPTIMIZATION_ENABLE)
	add_subdirectory(gridDynOpt)
ENDIF(OPTIMIZATION_ENABLE)


add_subdirectory(gridDynFileInput)

add_subdirectory(gridDynMain)

add_subdirectory(coupling)

# -------------------------------------------------------------
# Enable testCore construction?
# -------------------------------------------------------------

OPTION(TEST_ENABLE "Enable unit testing construction" ON)

IF (TEST_ENABLE)
add_subdirectory(test)
OPTION(ENABLE_EXPERIMENTAL_TEST_CASES "Enable experimental test cases" OFF)
#enable_testing()
#add_test(NAME gridDynTest COMMAND testCore)

ENDIF(TEST_ENABLE)

FILE(GLOB efiles "examples/*")
INSTALL(FILES ${efiles} DESTINATION examples)

SET(binfiles bin/configure.griddyn bin/pgriddyn bin/pgriddyn_debug)
INSTALL(PROGRAMS ${binfiles} DESTINATION bin)




