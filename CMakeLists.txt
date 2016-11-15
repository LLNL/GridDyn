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
project(GridDyn)

#states that Cmake version > 3.3
cmake_minimum_required(VERSION 3.3)
cmake_policy(VERSION 3.3)

#version number
set(GridDyn_VERSION_MAJOR 0)
set(GridDyn_VERSION_MINOR 6)

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
# value (hiding it merely makes it internal)

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



#include(mergestaticlibs)

# -------------------------------------------------------------
# Sundials
# -------------------------------------------------------------

#-------------------------------------------------------------
# Enable Cvode and Arkode modules
#-------------------------------------------------------------
OPTION(LOAD_CVODE "Enable CVODE Support" OFF)
OPTION(LOAD_ARKODE "Enable ARKODE support" OFF)

SHOW_VARIABLE(SUNDIALS_INSTALL_DIR PATH
  "SUNDIALS library directory" "${SUNDIALS_LIBRARY_DIR}")
find_package(SUNDIALS)

set(external_library_list
	${SUNDIALS_LIBRARIES}
	)
	
set(external_link_directories ${SUNDIALS_LIBRARY_DIR})

set(optional_component_test_files)
set(optional_system_test_files)

set(griddyn_optional_libraries )
# -------------------------------------------------------------
# Find (and test) the KLU libraries
# -------------------------------------------------------------

OPTION(KLU_ENABLE "Enable KLU support" ON)

SHOW_VARIABLE(KLU_INSTALL_DIR PATH
  "KLU library directory" "${KLU_INSTALL_DIR}")

set(SuiteSparse_FOUND NO)

if(KLU_ENABLE)
IF(MSVC)
set(SuiteSparse_USE_LAPACK_BLAS ON)
ENDIF(MSVC)
set(SuiteSparse_VERBOSE ON)
  find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF)

  if(SuiteSparse_FOUND) 
  INCLUDE_DIRECTORIES(SYSTEM ${SuiteSparse_INCLUDE_DIR})
  list(APPEND external_library_list ${SuiteSparse_LIBRARIES})
  #list(APPEND external_link_directories ${SuiteSparse_LIBRARY_DIR})
  else(SuiteSparse_FOUND)
	message( FATAL_ERROR "KLU not functional - support will not be provided")
	message( "Double check spelling specified libraries (search is case sensitive)")
  endif(SuiteSparse_FOUND)

endif(KLU_ENABLE)


set (optional_library_key_headers )
set (optional_library_functions )
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
	OPTION(SUNDIALS_OPENMP "Enable SUNDIALS NVector openMP implementation" ON)
	OPTION(GRIDDYN_OPENMP "Enable openmp internal to GridDyn--not used yet" OFF)
ENDIF(OPENMP_ENABLE)
OPTION(DISABLE_MULTITHREADING "disable multithreading in GridDyn libraries" OFF)

IF(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)

  set(Boost_USE_MULTITHREADED      OFF)   # Needed if MT libraries not built
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
ELSE(UNIX)
  IF(MINGW)
 add_compile_options(-Wall -pedantic)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  ELSE(MINGW)
  set(Boost_USE_STATIC_LIBS ON)
# -------------------------------------------------------------
# Extra definitions for visual studio
# -------------------------------------------------------------
IF(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  add_compile_options(-W4  /EHsc /wd4065 /wd4101 /wd4102 /wd4244 /wd4297 /wd4355 /wd4800 /wd4484)
ENDIF(MSVC)
  ENDIF(MINGW)
ENDIF(UNIX)

#check for clang 3.4 and the fact that CMAKE_CXX_STANDARD doesn't work yet for that compiler

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5)
	add_compile_options(-std=c++1y)
ELSE ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5)
set(CMAKE_CXX_STANDARD 14)
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.1)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wsuggest-override>)
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.1)

SHOW_VARIABLE(BOOST_ROOT PATH "Boost root directory" "${BOOST_ROOT}")

IF(FSKIT_ENABLE)
  find_package(Boost COMPONENTS program_options unit_test_framework filesystem serialization mpi system date_time REQUIRED)
ELSE(FSKIT_ENABLE)
  find_package(Boost COMPONENTS program_options unit_test_framework filesystem system date_time REQUIRED)
ENDIF(FSKIT_ENABLE)

message("Using Boost include files : ${Boost_INCLUDE_DIR}")
message("Using Boost libraries ${Boost_LIBRARY_DIRS}")

list(APPEND external_library_list ${Boost_LIBRARIES})
  list(APPEND external_link_directories ${Boost_LIBRARY_DIRS})

# -------------------------------------------------------------
# Enable FMI support?
# -------------------------------------------------------------

OPTION(FMI_ENABLE "Enable FMI support" OFF)
# If FMI is enabled try to locate the libraries
# link against them.

IF (FMI_ENABLE)
	IF (Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
		set(FMI_ENABLE OFF)
		message("FMI support requires BOOST 1.61 or higher")
	ELSE(Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
	add_subdirectory(fmi)
	ENDIF(Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
ENDIF(FMI_ENABLE)


# -------------------------------------------------------------
# Enable ZMQ interfacing
# -------------------------------------------------------------

OPTION(ZMQ_ENABLE "Enable ZMQ networking library" OFF)
# If ZMQ library is enabled try to locate it and link against it

IF (ZMQ_ENABLE)

SHOW_VARIABLE(ZMQ_LIBRARY_PATH PATH
  "path to the zmq libraries" "${ZMQ_LIBRARY_PATH}")
  
SHOW_VARIABLE(ZMQ_INCLUDE_PATH PATH
  "path to the zmq headers" "${ZMQ_INCLUDE_PATH}")
find_package(ZMQ)

# ZMQ_INCLUDE_DIR, where to find zmq.h
# ZMQ_LIBRARY, the library needed to use ZMQ
# ZMQ_FOUND, if false, you cannot build anything that requires ZMQ.
	if (ZMQ_FOUND)
		add_subdirectory(zmqInterface)
	else(ZMQ_FOUND)
		message("unable to locate zmq library")
	endif(ZMQ_FOUND)
ENDIF(ZMQ_ENABLE)

# -------------------------------------------------------------
# Enable FNCS executable
# -------------------------------------------------------------

OPTION(FNCS_EXECUTABLE "Enable the FNCS executable to be built" OFF)
# If FNCS_EXECUTABLE is load the fncs directory
# link against them.

IF (FNCS_EXECUTABLE)
	SHOW_VARIABLE(USE_DUMMY_FNCS BOOL "use a dummy fncs implementation if none is available" OFF)
	IF(USE_DUMMY_FNCS)
		HIDE_VARIABLE(FNCS_INSTALL_PATH)
	ELSE(USE_DUMMY_FNCS)
		SHOW_VARIABLE(FNCS_INSTALL_PATH PATH "path to the fncs installation" "${FNCS_LIBRARY_PATH}")
  
		find_package(FNCS)
	ENDIF(USE_DUMMY_FNCS)
	add_subdirectory(fncs)
ELSE(FNCS_EXECUTABLE)
	HIDE_VARIABLE(FNCS_INSTALL_PATH)
	HIDE_VARIABLE(USE_DUMMY_FNCS)
ENDIF(FNCS_EXECUTABLE)


# -------------------------------------------------------------
# Enable support for Plugins
# -------------------------------------------------------------

OPTION(ENABLE_PLUGINS "Enable support for plugin modules" ON)

IF (ENABLE_PLUGINS)
	IF (Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
		set(ENABLE_PLUGINS OFF)
		message("Plugin support requires BOOST 1.61 or higher")
	ELSE(Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
		add_subdirectory(plugins)
	ENDIF(Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
ENDIF(ENABLE_PLUGINS)

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
 
#OPTION(BUILD_SERVERMODE "Build GridDyn Server" OFF)
IF (BUILD_SERVERMODE)
 #add_subdirectory(gridDynServer)
ENDIF(BUILD_SERVERMODE)



ADD_DEFINITIONS(-DTIXML_USE_TICPP)
# ------------------------------------------------------------
# Enable message logging
# -------------------------------------------------------------
OPTION(LOG_ENABLE "enable non-warning/error logging messages" ON)

IF(LOG_ENABLE)
OPTION(DEBUG_LOG_ENABLE "enable debug logging messages(Note: logging is still controlled via program options this is for program size and speed)" ON)
OPTION(TRACE_LOG_ENABLE "enable trace logging messages(Note: logging is still controlled via program options this is for program size and speed)" ON)
ENDIF(LOG_ENABLE)


option (GRIDDYN_GENERATE_DOXYGEN_DOC "Generate Doxygen doc target" OFF)

IF (GRIDDYN_GENERATE_DOXYGEN_DOC)
find_package(Doxygen)
if(DOXYGEN_FOUND)

	SHOW_VARIABLE(DOXYGEN_OUTPUT_DIR PATH "location to put Doxygen docs" "${CMAKE_CURRENT_SOURCE_DIR}/docs")
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

add_subdirectory(formatInterpreters)

add_subdirectory(core)

add_subdirectory(utilities)
#minizup is used as part of the utilities
add_subdirectory(ThirdParty/Minizip)

add_subdirectory(gridDyn)

OPTION(OPTIMIZATION_ENABLE "Enable Optimization libraries" OFF)
# If OPTIMIZATION is enabled try to locate the libraries
# link against them.
IF(OPTIMIZATION_ENABLE)
	add_subdirectory(gridDynOpt)
ENDIF(OPTIMIZATION_ENABLE)


add_subdirectory(gridDynFileInput)

add_subdirectory(gridDynCombined)

add_subdirectory(gridDynMain)

add_subdirectory(coupling)

# -------------------------------------------------------------
# Enable testCore construction?
# -------------------------------------------------------------

OPTION(TEST_ENABLE "Enable unit testing construction" ON)

IF (TEST_ENABLE)
add_subdirectory(test)
#enable_testing()
#add_test(NAME gridDynTest COMMAND testCore)

ENDIF(TEST_ENABLE)

FILE(GLOB efiles "examples/*")
INSTALL(FILES ${efiles} DESTINATION examples)

#SET(binfiles bin/configure.griddyn bin/pgriddyn bin/pgriddyn_debug)
#INSTALL(PROGRAMS ${binfiles} DESTINATION bin)




