# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project name
project(GridDyn)

#states that Cmake version > 3.4
cmake_minimum_required(VERSION 3.4)
cmake_policy(VERSION 3.4)

#version number
set(GRIDDYN_MAJOR 0)
set(GRIDDYN_MINOR 9)
set(GRIDDYN_PATCH 0)
set(GRIDDYN_DATE 2-2-2018)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/config/cmake)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)

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

OPTION(ENABLE_EXTRA_COMPILER_WARNINGS "disable compiler warning for GridDyn build" ON)

SHOW_VARIABLE(COMPILER_OPTIMIZATION_LEVEL STRING
  "set the optimization level for the compiler (not implemented yet)" "normal")

  set(optimization_levels normal;high;machine;full)

set_property(CACHE COMPILER_OPTIMIZATION_LEVEL PROPERTY STRINGS ${optimization_levels})

OPTION(BUILD_SHARED_FMI_LIBRARY "Enable construction of a binary fmi shared library for GridDyn" OFF)

if (NOT CMAKE_DEBUG_POSTFIX)
set(CMAKE_DEBUG_POSTFIX d)
endif()

OPTION(BUILD_GRIDDYN_SHARED_LIBRARY "Enable construction of griddyn binary shared library" OFF)

IF(BUILD_GRIDDYN_SHARED_LIBRARY OR BUILD_FMI_LIBRARY)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
ELSE()
OPTION(USE_POSITION_INDEPENDENT_CODE "Build the libraries with Position independent code Useful if only building the static library and it will be used later in a shared library" OFF)
IF(USE_POSITION_INDEPENDENT_CODE)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF()
ENDIF()

include(compiler_flags)

IF(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)
  set(Boost_USE_MULTITHREADED      OFF)   # Needed if MT libraries not built
  option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
ELSE(UNIX)
  IF(MINGW)
  option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
  ELSE(MINGW)
  #this would most likely be MSVC
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" ON)
  ENDIF(MINGW)
ENDIF(UNIX)

IF (BUILD_MATLAB OR USE_BOOST_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS ON)
  set(BOOST_STATIC ON)
	  
ENDIF ()

# -------------------------------------------------------------
# add coverage target
# -------------------------------------------------------------
 option (TEST_CODE_COVERAGE "Build a target for testing code coverage" OFF)
 if (TEST_CODE_COVERAGE)
 include(CodeCoverage)

 set(COVERAGE_EXCLUDES 'ThirdParty/*' 'tests/*' 'swig/*' 'examples/*')
 add_compile_options(${COVERAGE_COMPILER_FLAGS})
  SETUP_TARGET_FOR_COVERAGE(
     NAME griddyn_coverage                    # New target name
     EXECUTABLE CTest # Executable in PROJECT_BINARY_DIR
	)

 endif(TEST_CODE_COVERAGE)
 
# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------

#message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${PROJECT_BINARY_DIR}/libs/include/griddyn/)
include(configGenerator)
INSTALL(FILES ${PROJECT_BINARY_DIR}/libs/include/griddyn/compiler-config.h DESTINATION include/griddyn)

include(ExternalProject)


include(GNUInstallDirs)

# -------------------------------------------------------------
# setting the RPATH
# -------------------------------------------------------------
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/bin" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
ENDIF("${isSystemDir}" STREQUAL "-1")

if (NOT USE_BOOST_STATIC_LIBS)
list(APPEND CMAKE_INSTALL_RPATH ${Boost_LIBRARY_DIRS})
endif()

#########################################################################################

#include(mergestaticlibs)
# -------------------------------------------------------------
# Enable OpenMP support?
# -------------------------------------------------------------
OPTION(OPENMP_ENABLE "Enable openMP support" ON)


IF(OPENMP_ENABLE)
  #message(STATUS "****** finding OpenMP support")
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

IF (OPENMP_FOUND)
	OPTION(SUNDIALS_OPENMP "Enable SUNDIALS NVector openMP implementation" ON)
	OPTION(GRIDDYN_OPENMP "Enable openmp internal to GridDyn--not used yet" OFF)
ENDIF(OPENMP_FOUND)

OPTION(DISABLE_MULTITHREADING "disable multithreading in GridDyn libraries" OFF)


# -------------------------------------------------------------
# Find multithreading headers and includes
# -------------------------------------------------------------

IF (NOT DISABLE_MULTITHREADING)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND external_library_list Threads::Threads)
ENDIF(NOT DISABLE_MULTITHREADING)

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
# Find (and test) the KLU libraries
# -------------------------------------------------------------

include(addKLU)



# -------------------------------------------------------------
# Sundials
# -------------------------------------------------------------

include(addSundials)
set(optional_component_test_files)
set(optional_system_test_files)

set(griddyn_optional_libraries )




set (optional_library_key_headers )
set (optional_library_functions )

# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------
include(addBoost)

message("Using Boost include files : ${Boost_INCLUDE_DIR}")
message("Using Boost libraries ${Boost_LIBRARY_DIRS}")

list(APPEND external_library_list ${Boost_LIBRARIES_core})
  list(APPEND external_link_directories ${Boost_LIBRARY_DIRS})

# -------------------------------------------------------------
# global include directories
# -------------------------------------------------------------
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/ThirdParty)
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})
#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/swig)
# -------------------------------------------------------------
# load the required subdirectories
# -------------------------------------------------------------

add_subdirectory(src/formatInterpreters)

add_subdirectory(src/core)
#required for utilities library

add_subdirectory(src/utilities)

add_subdirectory(src/griddyn)

# now go into the optional stuff


# -------------------------------------------------------------
# Enable support for Plugins
# -------------------------------------------------------------

OPTION(ENABLE_PLUGINS "Enable support for plugin modules" OFF)

IF (ENABLE_PLUGINS)
	IF (Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
		set(ENABLE_PLUGINS OFF)
		message("Plugin support requires BOOST 1.61 or higher")
	ELSE(Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
		add_subdirectory(plugins)
		SET(DL_REQUIRED ON)
	ENDIF(Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
ENDIF(ENABLE_PLUGINS)

# -------------------------------------------------------------
# Enable compilation of extraModels?
# -------------------------------------------------------------

OPTION(LOAD_EXTRA_MODELS "Compile and load extraModels" ON)
# If extra models are used enabled try to locate the libraries
# link against them.

if (LOAD_EXTRA_MODELS)
  add_subdirectory(src/extraModels)
ENDIF(LOAD_EXTRA_MODELS)

# -------------------------------------------------------------
# Enable FMI support?
# -------------------------------------------------------------

OPTION(FMI_ENABLE "Enable FMI support" ON)


IF (FMI_ENABLE)
	IF (Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
		set(FMI_ENABLE OFF CACHE BOOL "Enable FMI Support" FORCE)
		message("FMI support requires BOOST 1.61 or higher")
	ELSE(Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
		add_subdirectory(src/fmi)
		SET(DL_REQUIRED ON)
	ENDIF(Boost_MAJOR_VERSION==1 AND Boost_MINOR_VERSION<61)
ENDIF(FMI_ENABLE)


# -------------------------------------------------------------
# FSKIT
# -------------------------------------------------------------

OPTION(FSKIT_ENABLE "Enable FSKIT support" OFF)

IF(FSKIT_ENABLE)
  find_package(FSKIT)
  INCLUDE_DIRECTORIES(SYSTEM ${FSKIT_INCLUDE_DIR})
  set(GRIDDYN_HAVE_FSKIT 1)
  add_subdirectory(src/fskit)
ENDIF(FSKIT_ENABLE)

# -------------------------------------------------------------
# Enable 64 bit indexing (enable to allow for more than 2^31 objects)--that would be a very big model
# -------------------------------------------------------------

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
  OPTION(ENABLE_64_BIT_INDEXING "set all indexing and count variables to 64 bit unsigned (Usually not required)" OFF)
endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")


# ------------------------------------------------------------
# Enable message logging
# -------------------------------------------------------------
OPTION(DISABLE_LOGGING "disable all normal, debug, and trace logging in GridDyn" OFF)
if (NOT DISABLE_LOGGING)
OPTION(DISABLE_TRACE_LOGGING "disable trace logging" OFF)
OPTION(DISABLE_DEBUG_LOGGING "disable debug logging" OFF)
endif()

option (GRIDDYN_GENERATE_DOXYGEN_DOC "Generate Doxygen doc target" OFF)

IF (GRIDDYN_GENERATE_DOXYGEN_DOC)
find_package(Doxygen)
if(DOXYGEN_FOUND)

	SHOW_VARIABLE(DOXYGEN_OUTPUT_DIR PATH "location to put Doxygen docs" "${CMAKE_CURRENT_SOURCE_DIR}/docs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
endif (GRIDDYN_GENERATE_DOXYGEN_DOC)



# -------------------------------------------------------------
# Enable HELICS executable
# -------------------------------------------------------------

OPTION(HELICS_EXECUTABLE "Enable the HELICS executable to be built" OFF)
# If HELICS_EXECUTABLE is load the fncs directory
# link against them.

IF (HELICS_EXECUTABLE)
	SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")
	set(ZMQ_NEEDED ON CACHE BOOL "ZMQ is needed" FORCE)
ELSE(HELICS_EXECUTABLE)
	HIDE_VARIABLE(HELICS_INSTALL_PATH)
ENDIF(HELICS_EXECUTABLE)

# -------------------------------------------------------------
# Enable ZMQ interfacing
# -------------------------------------------------------------

OPTION(DIME_ENABLE "Enable connection with Dime" OFF)

IF (DIME_ENABLE)
set(ZMQ_NEEDED ON CACHE BOOL "ZMQ is needed" FORCE)
ENDIF(DIME_ENABLE)

OPTION(ZMQ_ENABLE "Enable ZMQ networking library" ${ZMQ_NEEDED})
# If ZMQ library is enabled try to locate it and link against it

IF ((ZMQ_ENABLE) OR (ZMQ_NEEDED))
	include(addZeroMQ)
	# ZMQ_INCLUDE_DIR, where to find zmq.h
# ZMQ_LIBRARY, the library needed to use ZMQ
# ZMQ_FOUND, if false, you cannot build anything that requires ZMQ.
	if (ZeroMQ_FOUND)
		add_subdirectory(src/zmqlib)
	else(ZeroMQ_FOUND)
		message(SEND_ERROR "unable to locate zmq library")
	endif(ZeroMQ_FOUND)
ENDIF((ZMQ_ENABLE) OR (ZMQ_NEEDED))


IF (HELICS_EXECUTABLE)
	SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")
	if (EXISTS ${HELICS_INSTALL_PATH}/cmake/HELICS/HELICSImport.cmake)
		IF (AUTOBUILD_HELICS)
			OPTION(FORCE_HELICS_REBUILD "force rebuild of helics" OFF)
			IF(FORCE_HELICS_REBUILD)
				include(buildHELICS)
				build_helics()
				set(HELICS_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
				set(FORCE_HELICS_REBUILD OFF CACHE BOOL "force rebuild of helics" FORCE)
			ENDIF(FORCE_HELICS_REBUILD)
		ENDIF(AUTOBUILD_HELICS)
		message(status "loading ${HELICS_INSTALL_PATH}/cmake/HELICS/HELICSImport.cmake")
		include(${HELICS_INSTALL_PATH}/cmake/HELICS/HELICSImport.cmake)
	else()
		message(status "couldn't find ${HELICS_INSTALL_PATH}/cmake/HELICS/HELICSImport.cmake")
		OPTION(AUTOBUILD_HELICS "enable HELICS to automatically download and build" OFF)
		IF(AUTOBUILD_HELICS)
			include(buildHELICS)
			build_helics()
			include(${HELICS_INSTALL_PATH}/cmake/HELICS/HELICSImport.cmake)
		ENDIF(AUTOBUILD_HELICS)
	endif()
	add_subdirectory(src/helics)
ENDIF(HELICS_EXECUTABLE)

FILE(GLOB KEY_LIBRARY_FILES  ${PROJECT_BINARY_DIR}/libs/bin/*)
message(STATUS "key files ${KEY_LIBRARY_FILES}")
# -------------------------------------------------------------
# load the subdirectories
# -------------------------------------------------------------



OPTION(OPTIMIZATION_ENABLE "Enable Optimization libraries" OFF)
# If OPTIMIZATION is enabled try to locate the libraries
# link against them.
IF(OPTIMIZATION_ENABLE)
	add_subdirectory(src/optimization)
ENDIF(OPTIMIZATION_ENABLE)

add_subdirectory(src/fileInput)

add_subdirectory(src/coupling)

add_subdirectory(src/gridDynCombined)

IF(BUILD_SHARED_FMI_LIBRARY)
	add_subdirectory(src/fmi_export)
ENDIF(BUILD_SHARED_FMI_LIBRARY)

OPTION(BUILD_GRIDDYN_SHARED_LIBRARY "Enable construction of griddyn binary shared library" OFF)

IF(BUILD_GRIDDYN_SHARED_LIBRARY)
	add_subdirectory(src/griddyn_shared)
ENDIF(BUILD_GRIDDYN_SHARED_LIBRARY)

add_subdirectory(src/gridDynMain)


# -------------------------------------------------------------
# Enable clang analysis and formatting tools
# -------------------------------------------------------------

OPTION(ENABLE_CLANG_TOOLS "if clang is found enable some custom targets for clang formatting and tidy" OFF)

if (ENABLE_CLANG_TOOLS)
include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)
# -------------------------------------------------------------
# Enable testCore construction?
# -------------------------------------------------------------

OPTION(TEST_ENABLE "Enable unit testing construction" ON)

IF (TEST_ENABLE)
	#message("otcf:${optional_component_test_files}")
	add_subdirectory(test)
IF(BUILD_GRIDDYN_SHARED_LIBRARY)
	add_subdirectory(test/testSharedLibrary)
ENDIF(BUILD_GRIDDYN_SHARED_LIBRARY)
	#enable_testing()
	#add_test(NAME gridDynTest COMMAND testCore)
ENDIF(TEST_ENABLE)

FILE(GLOB efiles "examples/*")
INSTALL(FILES ${efiles} DESTINATION examples)

SET(binfiles bin/configure.griddyn bin/pgriddyn bin/pgriddyn_debug)
INSTALL(PROGRAMS ${binfiles} DESTINATION bin)

INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION bin)


# -------------------------------------------------------------
# Future Additions
# -------------------------------------------------------------

#adding dlls
# INSTALL(FILES ${LOCATION_OF_FILES} DESTINATION bin)
#FILE(GLOB docs "docs/manuals/*")
#INSTALL(FILES ${docs} DESTINATION docs)

# -------------------------------------------------------------
# CTest
# -------------------------------------------------------------

enable_testing()
add_test(NAME testComponents COMMAND testComponents)

add_test(NAME testLibrary COMMAND testLibrary)

add_test(NAME testSystem COMMAND testSystem)
#####################################################
######################### swig ######################
#####################################################
option(WITH_PYTHON "Compile the Python front-end" OFF)
option(WITH_MATLAB "Compile the MATLAB front-end" OFF)
option(WITH_PYTHON3 "Use Python 3.*" OFF)
if(WITH_PYTHON OR WITH_MATLAB)
    add_subdirectory(swig)
ENDIF()

# -------------------------------------------------------------
# CPack for NSIS Installer
# -------------------------------------------------------------

set(CPACK_PACKAGE_NAME "GridDyn")
set(CPACK_PACKAGE_VENDOR "Lawrence Livermore National Security")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GridDyn Installer")
set(CPACK_PACKAGE_VERSION "${GridDyn_MAJOR_VERSION}.${GridDyn_MINOR_VERSION}.${GridDyn_PATCH_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR ${GridDyn_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${GridDyn_MINOR_VERSION})
#set(CPACK_PACKAGE_VERSION_PATCH ${GridDyn_VERSION_PATCH})

#THIS LINE MUST BE LAST
include(CPack)
