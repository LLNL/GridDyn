# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project name
project(GridDyn)

#states that Cmake version > 3.5
cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)

#version number
set(GRIDDYN_VERSION_MAJOR 0)
set(GRIDDYN_VERSION_MINOR 9)
set(GRIDDYN_VERSION_PATCH 0)
set(GRIDDYN_VERSION ${GRIDDYN_VERSION_MAJOR}.${GRIDDYN_VERSION_MINOR}.${GRIDDYN_VERSION_PATCH})
set(GRIDDYN_DATE 3-20-2018)
set(GRIDDYN_VERSION_STRING "${GRIDDYN_VERSION} ${GRIDDYN_DATE}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/config/cmake)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
include(extraMacros)

FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)

# Prohibit in-source build
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source build is not supported. Please, use an empty directory for building the project.")
ENDIF()

#add a baseline library for underlying dependencies and flags
add_library(griddyn_base INTERFACE)

OPTION(ENABLE_EXTRA_COMPILER_WARNINGS "disable compiler warning for GridDyn build" ON)

SHOW_VARIABLE(COMPILER_OPTIMIZATION_LEVEL STRING
  "set the optimization level for the compiler (not implemented yet)" "normal")

  set(optimization_levels normal;high;machine;full)

set_property(CACHE COMPILER_OPTIMIZATION_LEVEL PROPERTY STRINGS ${optimization_levels})

OPTION(BUILD_SHARED_FMI_LIBRARY "Enable construction of a binary fmi shared library for GridDyn" OFF)

if (NOT CMAKE_DEBUG_POSTFIX)
set(CMAKE_DEBUG_POSTFIX d)
endif()


OPTION(BUILD_PYTHON_INTERFACE "Build Python extension" OFF)
OPTION(BUILD_MATLAB_INTERFACE "Build Matlab Extension" OFF)
OPTION(BUILD_OCTAVE_INTERFACE "Build Octave extension" OFF)
OPTION(BUILD_JAVA_INTERFACE "Build Java extension" OFF)

OPTION(DISABLE_SWIG "Disable the use of swig to generate interface code and use repo code" OFF)
OPTION(BUILD_GRIDDYN_SHARED_LIBRARY "Enable construction of griddyn binary shared library" OFF)

IF(BUILD_GRIDDYN_SHARED_LIBRARY OR BUILD_FMI_LIBRARY OR BUILD_PYTHON_INTERFACE OR BUILD_JAVA_INTERFACE OR BUILD_OCTAVE_INTERFACE OR BUILD_MATLAB_INTERFACE)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
ELSE()
OPTION(USE_POSITION_INDEPENDENT_CODE "Build the libraries with Position independent code Useful if only building the static library and it will be used later in a shared library" OFF)
IF(USE_POSITION_INDEPENDENT_CODE)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF()
ENDIF()

include(compiler_flags)

IF (VERSION_OPTION)
	IF (MSVC)
		target_compile_options(griddyn_base INTERFACE ${VERSION_OPTION})
	ELSE()
		target_compile_options(griddyn_base INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>)
	ENDIF (MSVC)
ENDIF(VERSION_OPTION)

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

IF (BUILD_MATLAB_INTERFACE OR BUILD_OCTAVE_INTERFACE OR USE_BOOST_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS ON)
  set(GRIDDYN_BOOST_STATIC ON)
	  
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
 
 include(GNUInstallDirs)
# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------

#message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${PROJECT_BINARY_DIR}/libs/include/griddyn/)
include(configGenerator)
INSTALL(FILES ${PROJECT_BINARY_DIR}/libs/include/griddyn/compiler-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn COMPONENT headers)

include(ExternalProject)




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
if (NOT WIN32)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
elseif (MSYS)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()
find_package(Threads REQUIRED)
target_link_libraries(griddyn_base INTERFACE Threads::Threads)
ENDIF(NOT DISABLE_MULTITHREADING)

# -------------------------------------------------------------
# finding MPI
# -------------------------------------------------------------

OPTION(MPI_ENABLE "Enable MPI networking library" OFF)
IF (MPI_ENABLE)
if(MSVC)
   #This is for finding MS-MPI.
   #set(_MPI_PREFIX_PATH)
  #list(APPEND _MPI_PREFIX_PATH
 #"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\MPI;Path]/..")
  # set(MPIEXEC "C:/Program Files/Microsoft MPI/Bin/mpiexec.exe")
   # For building MPI programs the selected Visual Studio compiler is used,
 #namely cl.exe.
  # So there is no need to set a specific MPI compiler.
  set(MPI_CXX_COMPILER "${CMAKE_CXX_COMPILER}")
  set(MPI_C_COMPILER "${CMAKE_CXX_COMPILER}")
  set(MPI_CXX_INCLUDE_PATH "C:/Program Files (x86)/Microsoft SDKs/MPI/Include")
   # Make sure the correct libraries (64-bit or 32-bit) are selected.
   # Decide between 32-bit and 64-bit libraries for Microsoft's MPI
   if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
     set(MS_MPI_ARCH_DIR x64)
  else()
    set(MS_MPI_ARCH_DIR x86)
   endif()
  set(MPI_CXX_LIBRARIES "C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/${MS_MPI_ARCH_DIR}/msmpi.lib")
   set(MPI_C_INCLUDE_PATH "${MPI_CXX_INCLUDE_PATH}")
   set(MPI_C_LIBRARIES "{${MPI_CXX_LIBRARIES}")
	set(MPI_C_FOUND YES)
	set(MPI_CXX_FOUND YES)
 else()
    find_package(MPI REQUIRED)
endif()

if (MPI_C_FOUND)
	set(GRIDDYN_HAVE_MPI TRUE)
else()
	set(GRIDDYN_HAVE_MPI FALSE)
endif (MPI_C_FOUND)
	ELSE(MPI_ENABLE)
			set(GRIDDYN_HAVE_MPI FALSE)
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

set (optional_library_key_headers )
set (optional_library_functions )

# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------
include(addBoost)
set(GRIDDYN_BOOST_VERSION_LEVEL ${BOOST_VERSION_LEVEL})
message("Using Boost include files : ${Boost_INCLUDE_DIR}")
message("Using Boost libraries ${Boost_LIBRARY_DIRS}")


# -------------------------------------------------------------
# global include directories
# -------------------------------------------------------------
target_include_directories(griddyn_base INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/libs/include>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
	$<INSTALL_INTERFACE:include>
)

# -------------------------------------------------------------
# load the required subdirectories
# -------------------------------------------------------------

add_subdirectory(src/formatInterpreters)

add_subdirectory(src/core)
#required for utilities library

add_subdirectory(src/utilities)

add_subdirectory(src/griddyn)

# now go into the optional stuff

add_library(griddyn_optional INTERFACE)

# -------------------------------------------------------------
# Enable support for Plugins
# -------------------------------------------------------------

OPTION(ENABLE_PLUGINS "Enable support for plugin modules" OFF)

IF (ENABLE_PLUGINS)
	IF (BOOST_VERSION_LEVEL==0)
		set(ENABLE_PLUGINS OFF)
		message("Plugin support requires BOOST 1.61 or higher")
	ELSE()
		add_subdirectory(plugins)
		SET(DL_REQUIRED ON)
	ENDIF()
ENDIF(ENABLE_PLUGINS)

# -------------------------------------------------------------
# Enable compilation of extraModels?
# -------------------------------------------------------------

OPTION(LOAD_EXTRA_MODELS "Compile and load extraModels" ON)
# If extra models are used enabled try to locate the libraries
# link against them.

if (LOAD_EXTRA_MODELS)
  add_subdirectory(src/extraModels)
  target_link_libraries(griddyn_optional INTERFACE extraModelLibrary)
ENDIF(LOAD_EXTRA_MODELS)

# -------------------------------------------------------------
# Enable compilation of extraSolvers?
# -------------------------------------------------------------

OPTION(LOAD_EXTRA_SOLVERS "Compile and load extraSolvers" OFF)
# If extra solvers are used enabled try to locate the libraries
# link against them.

if (LOAD_EXTRA_SOLVERS)
  add_subdirectory(src/extraSolvers)
  target_link_libraries(griddyn_optional INTERFACE extraSolverLibrary)
ENDIF(LOAD_EXTRA_SOLVERS)

# -------------------------------------------------------------
# Enable FMI support?
# -------------------------------------------------------------

OPTION(FMI_ENABLE "Enable FMI support" ON)


IF (FMI_ENABLE)
	IF (BOOST_VERSION_LEVEL==0)
		set(FMI_ENABLE OFF CACHE BOOL "Enable FMI Support" FORCE)
		message("FMI support requires BOOST 1.61 or higher")
	ELSE()
		add_subdirectory(src/fmi)
		target_link_libraries(griddyn_optional INTERFACE fmiGDLibrary)
		SET(DL_REQUIRED ON)
	ENDIF()
ENDIF(FMI_ENABLE)


# -------------------------------------------------------------
# FSKIT
# -------------------------------------------------------------

OPTION(FSKIT_ENABLE "Enable FSKIT support" OFF)

IF(FSKIT_ENABLE)
  set(GRIDDYN_HAVE_FSKIT 1)
  add_subdirectory(src/fskit)
  target_link_libraries(griddyn_optional INTERFACE fskitLibrary)
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
OPTION(GRIDDYN_DISABLE_LOGGING "disable all normal, debug, and trace logging in GridDyn" OFF)
if (NOT DISABLE_LOGGING)
OPTION(GRIDDYN_DISABLE_TRACE_LOGGING "disable trace logging" OFF)
OPTION(GRIDDYN_DISABLE_DEBUG_LOGGING "disable debug logging" OFF)
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
# If HELICS_EXECUTABLE is load the helics directory
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
		if (DIME_ENABLE)
			target_link_libraries(griddyn_optional INTERFACE dimeLibrary)
		else(DIME_ENABLE)
			target_link_libraries(griddyn_optional INTERFACE zmqInterfaceLibrary)
		endif(DIME_ENABLE)
	else(ZeroMQ_FOUND)
		message(SEND_ERROR "unable to locate zmq library")
	endif(ZeroMQ_FOUND)
ENDIF((ZMQ_ENABLE) OR (ZMQ_NEEDED))


IF (HELICS_EXECUTABLE)
	include(addHELICS)
	add_subdirectory(src/helics)
	target_link_libraries(griddyn_optional INTERFACE helicsLib)
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
	target_link_libraries(griddyn_optional INTERFACE optimization)
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

INSTALL(TARGETS griddyn_base griddyn_optional 
	EXPORT griddyn-targets 
	DESTINATION ${CMAKE_INSTALL_LIBDIR} 
	COMPONENT libraries)
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


set(GRIDDYN_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for GriddynConfig.cmake")

FILE(GLOB efiles "examples/*")
INSTALL(FILES ${efiles} DESTINATION examples COMPONENT examples)

SET(binfiles bin/configure.griddyn bin/pgriddyn bin/pgriddyn_debug)
INSTALL(PROGRAMS ${binfiles} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime)

INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime)

install (EXPORT griddyn-targets 
	NAMESPACE Griddyn::
	DESTINATION ${GRIDDYN_CMAKECONFIG_INSTALL_DIR}
	COMPONENT libs
	)

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

#option(WITH_PYTHON3 "Use Python 3.*" OFF)

if (BUILD_PYTHON_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_JAVA_INTERFACE OR BUILD_OCTAVE_INTERFACE)
add_subdirectory(swig)
endif()



# -------------------------------------------------------------
# CPack 
# -------------------------------------------------------------

set(CPACK_PACKAGE_NAME "GridDyn")
set(CPACK_PACKAGE_VENDOR "Lawrence Livermore National Security LLC")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GridDyn Installer")
set(CPACK_PACKAGE_VERSION "${GRIDDYN_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR ${GRIDDYN_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${GRIDDYN_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${GRIDDYN_VERSION_PATCH})

set(CPACK_COMPONENTS_ALL applications headers libs runtime matlab python java octave examples)

if (WIN32)
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}\\\\LICENSE")
else(WIN32)
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif(WIN32)

set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Application")
 set(CPACK_COMPONENT_LIBS_DISPLAY_NAME "Libraries")
 set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Headers")
 set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime Libraries")
 set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples")
  
 set(CPACK_COMPONENT_MATLAB_GROUP interfaces)
 set(CPACK_COMPONENT_JAVA_GROUP interfaces)
 set(CPACK_COMPONENT_OCTAVE_GROUP interfaces)
 set(CPACK_COMPONENT_PYTHON_GROUP interfaces)
 
 set(CPACK_COMPONENT_APPLICATION_DESCRIPTION "Executables and helper applications for GridDyn")
 set(CPACK_COMPONENT_LIBS_DESCRIPTION "Libraries for compiling and linking with GridDyn")
 set(CPACK_COMPONENT_HEADERS_DESCRIPTION "Headers for linking and compiling with GridDyn")
 set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Runtime libraries for GrdDyn")
 set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Example files for GridDyn")

 set(CPACK_COMPONENT_GROUP_INTERFACES_DESCRIPTION "additional language interfaces for GridDyn")
 
 set(CPACK_COMPONENT_LIBS_DEPENDS headers)
 SET(CPACK_COMPONENT_RUNTIME_REQUIRED ON)
 if (WIN32)
	set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\docgen\\\\images\\\\griddyn.ico")
	set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/docgen/images/griddyn.ico")

	set(CPACK_NSIS_INSTALL_ROOT "C:\\\\local\\\\")
	set(CPACK_NSIS_URL_INFO_ABOUT "https://www.github.com/LLNL/GridDyn")
	set(CPACK_NSIS_MENU_LINKS 
		"https://www.github.com/LLNL/GridDyn" "GridDyn source code"
		"https://www.griddyn.org" "GridDyn Web Page")
	
		set(CPACK_NSIS_INSTALL_ROOT "C:\\\\local\\\\")
else(WIN32)
	set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/docgen/images/griddyn.ico")
endif(WIN32)
set(CPACK_SOURCE_IGNORE_FILES  "/Build*/;/build*/;/.git/")

#THIS LINE MUST BE LAST
include(CPack)

