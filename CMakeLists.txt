# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

# cmake >= 3.5 required, minimum_required sets policy
cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)

if (POLICY CMP0079)
cmake_policy(SET CMP0079 NEW)
endif()


# project name
project(GRIDDYN VERSION 0.9.1)

# version number
set(GRIDDYN_VERSION_BUILD)
set(GRIDDYN_DATE "7-11-2019")

set(
    GRIDDYN_VERSION_UNDERSCORE
    "${GRIDDYN_VERSION_MAJOR}_${GRIDDYN_VERSION_MINOR}_${GRIDDYN_VERSION_PATCH}"
)
if(GRIDDYN_VERSION_BUILD)
    set(GRIDDYN_VERSION ${GRIDDYN_VERSION}-${GRIDDYN_VERSION_BUILD})
    set(
        GRIDDYN_VERSION_UNDERSCORE
        "${GRIDDYN_VERSION_UNDERSCORE}-${GRIDDYN_VERSION_BUILD}"
    )
endif()


set(GRIDDYN_VERSION_STRING "${GRIDDYN_VERSION} ${GRIDDYN_DATE}")
set(GRIDDYN_VERSION_STRING_SHORT "${GRIDDYN_VERSION}")

if(POLICY CMP0079)
    # Policy CMP0079 introduced since CMake 3.13.2 changes a few things in target definitions with INTERFACE, use old until this is evaluated properly
    # condition.
    cmake_policy(SET CMP0079 NEW)
  endif()
  
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ------------------------------------------------------------
# set the module path and include some common macros
# ------------------------------------------------------------

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(GRIDDYN_ROOT_PROJ ON)
else()
    set(GRIDDYN_ROOT_PROJ OFF)
endif()

if (GRIDDYN_ROOT_PROJ)
set(CMAKE_MODULE_PATH
     ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/config/cmake/"
)
else()
	set(ORIGINAL_MODULE_PATH ${CMAKE_MODULE_PATH})
	set(CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/config/cmake/" ${CMAKE_MODULE_PATH}
)
endif()

include(extraMacros)
include(CMakeDependentOption)
# ------------------------------------------------------------
# set the install path to a local directory
# ------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(WIN32)
        if(MSYS OR CYGWIN)
            set(
                CMAKE_INSTALL_PREFIX
                "/usr/local/griddyn_${GRIDDYN_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path"
                FORCE
            )
        else()
            set(
                CMAKE_INSTALL_PREFIX
                "C:/local/griddyn_${GRIDDYN_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path"
                FORCE
            )
        endif()
    endif(WIN32)
endif()

show_variable(
    AUTOBUILD_INSTALL_PATH
    PATH
    "location to install the autobuild libraries and Headers"
    "${PROJECT_BINARY_DIR}/libs"
)

mark_as_advanced(AUTOBUILD_INSTALL_PATH)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)

# Prohibit in-source build
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(
        FATAL_ERROR
            "In-source build is not supported. Please, use an empty directory for building the project."
    )
endif()

# -------
# options
# -------

if (GRIDDYN_ROOT_PROJ)
if (DEFINED BUILD_TESTING)
option(ENABLE_GRIDDYN_TESTS "Enable the test executables to be built" ${BUILD_TESTING})
else()
option(ENABLE_GRIDDYN_TESTS "Enable the test executables to be built" ON)
endif()
endif()

option(
    ENABLE_FMI_EXPORT
    "Enable construction of a binary fmi shared library for GridDyn"
    OFF
)

option(
    BUILD_PYTHON_INTERFACE
    "Build Python extension"
    OFF
)

option(
    BUILD_PYTHON2_INTERFACE
    "Build Python2.7 extension (conflicts with \"PYTHON_INTERFACE\", requires swig)"
    OFF
)

option(
    BUILD_MATLAB_INTERFACE
    "Build Matlab Extension"
    OFF
)

option(
    BUILD_OCTAVE_INTERFACE
    "Build Octave extension (very experimental)"
    OFF
)

option(
    BUILD_JAVA_INTERFACE
    "Build Java extension"
    OFF
)

option(
    BUILD_GRIDDYN_SHARED_LIBRARY
    "Build a shared library of the C/C++ interface"
    OFF
)

option(
    ENABLE_CODE_COVERAGE_TEST
    "Build a target for testing code coverage"
    OFF
)

option(
    ENABLE_OPENMP
    "Enable openMP support"
    ON
)


option(
    # TODO this needs to be accounted for in the source code
    ENABLE_MULTITHREADING
    "disable multithreading in GridDyn libraries"
    ON
)

option(
    ENABLE_MPI
    "Enable MPI networking library"
    OFF
)

option(
    ENABLE_KLU
    "Option to disable KLU (not recommended [slow]; prefer to turn on AUTOBUILD_KLU)"
    ON
)

option(
    ENABLE_PLUGINS
    "Enable support for plugin modules"
    OFF
)
mark_as_advanced(ENABLE_PLUGINS)

option(
    ENABLE_EXTRA_MODELS
    "Compile and load extraModels"
    ON
)

option(
    ENABLE_EXTRA_SOLVERS
    "Compile and load extraSolvers"
    OFF
)

option(
    ENABLE_FMI
    "Enable FMI support"
    ON
)

option(
    ENABLE_GRIDDYN_LOGGING
    "enable all normal, debug, and trace logging in GridDyn"
    ON
)


option(
    ENABLE_GRIDDYN_DOXYGEN
    "Generate Doxygen doc target"
    OFF
)

option(
    ENABLE_HELICS_EXECUTABLE
    "Enable the HELICS executable to be built"
    OFF
)

option(
    ENABLE_NETWORKING_LIBRARY
    "Enable network based communication components"
    OFF
)

option(
    ENABLE_OPTIMIZATION_LIBRARY
    "Enable Optimization libraries"
    OFF
)

option(
    ENABLE_CLANG_TOOLS
    "if clang is found enable some custom targets for clang formatting and tidy"
    OFF
)

option(
    ENABLE_PACKAGE_BUILD
    "Add projects for making packages and installers for HELICS"
    OFF
)

option(
    FORCE_DEPENDENCY_REBUILD
    "Rebuild third party dependencies, even if already installed"
    OFF
)


# -----------------
# dependent options
# -----------------
set(EXTENDED_INDEXING_ALLOWED "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
# TODO make sure this is adequately tested and consistently used
cmake_dependent_option(
    ENABLE_64_BIT_INDEXING
    "set all indexing and count variables to 64 bit unsigned (Usually not required)"
    OFF
    "EXTENDED_INDEXING_ALLOWED"
    OFF
)

cmake_dependent_option(
    ENABLE_TCP
    "Enable TCP connection library"
    ON
    "ENABLE_NETWORKING_LIBRARY"
    OFF
)

cmake_dependent_option(
    ENABLE_DIME
    "Enable connection with Dime"
    OFF
    "ENABLE_NETWORKING_LIBRARY"
    OFF
)

cmake_dependent_option(
    ENABLE_GRIDDYN_TRACE_LOGGING
    "enable trace logging"
    ON
    "ENABLE_GRIDDYN_LOGGING"
    OFF
)

cmake_dependent_option(
    ENABLE_GRIDDYN_DEBUG_LOGGING
    "enable debug logging"
    ON
    "ENABLE_GRIDDYN_LOGGING"
    OFF
)

mark_as_advanced(BUILD_GRIDDYN_SHARED_LIBRARY)

# add a baseline library for underlying dependencies and flags
add_library(griddyn_base INTERFACE)
add_library(griddyn_optional INTERFACE)

# -----------------------------------------------------------------------------
# General project wide configuration for debug postfix
# -----------------------------------------------------------------------------
if(NOT NO_DEBUG_POSFIX)
    if(NOT CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX d)
    endif()
endif()

if(
    BUILD_PYTHON_INTERFACE
    OR BUILD_PYTHON2_INTERFACE
    OR BUILD_MATLAB_INTERFACE
    OR BUILD_JAVA_INTERFACE
    OR BUILD_OCTAVE_INTERFACE
)
    set(INTERFACE_BUILD ON)
    hide_variable(BUILD_GRIDDYN_SHARED_LIBRARY)
    show_variable(
        DISABLE_SWIG
        BOOL
        "Disable the use of swig to generate interface code and use repo code"
        OFF
    )
else()
    set(INTERFACE_BUILD OFF)
    hide_variable(DISABLE_SWIG)
    show_variable(
        BUILD_GRIDDYN_SHARED_LIBRARY
        BOOL
        "Enable construction of GridDyn binary C based shared library"
        OFF
    )
endif()

if(
    INTERFACE_BUILD
    OR BUILD_GRIDDYN_SHARED_LIBRARY
    OR ENABLE_FMI_EXPORT
)
    set(BUILD_SHARED_LIBS ON)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if (NOT CMAKE_POSITION_INDEPENDENT_CODE)
    show_variable(
        CMAKE_POSITION_INDEPENDENT_CODE
        BOOL
        "Compile the static libraries so that they're relocatable"
        OFF
    )
endif()

if (NOT TARGET compile_flags_target)
   add_library(compile_flags_target INTERFACE)

endif()

if (GRIDDYN_ROOT_PROJECT)
    include(compiler_flags)

    message(STATUS "setting version build options to \"${CXX_STANDARD_FLAG}\"")
	if(CXX_STANDARD_FLAG)
		if(MSVC)
			target_compile_options(griddyn_base INTERFACE ${CXX_STANDARD_FLAG})
		else()
			target_compile_options(
				griddyn_base
				INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${CXX_STANDARD_FLAG}>
			)
		endif(MSVC)
	endif(CXX_STANDARD_FLAG)
	
endif()

   install(
    TARGETS compile_flags_target
    EXPORT griddyn-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if (GRIDDYN_ROOT_PROJ)
cmake_dependent_option(
    STATIC_STANDARD_LIB
    "Link against a static standard lib"
    OFF
    "NOT USE_LIBCXX"
    OFF
)
include(ucm)
mark_as_advanced(STATIC_STANDARD_LIB)
if(STATIC_STANDARD_LIB)
    ucm_set_runtime(STATIC)
else()
    ucm_set_runtime(DYNAMIC)
endif()

if(MSVC)
    option(EMBEDDED_DEBUG_INFO "embed debug info into obj files" OFF)
    mark_as_advanced(EMBEDDED_DEBUG_INFO)
    if(EMBEDDED_DEBUG_INFO)
        ucm_set_embedded_debug(EMBEDDED)
    else()
        ucm_set_embedded_debug(EXTERNAL)
    endif()
endif(MSVC)
endif (GRIDDYN_ROOT_PROJ)

# ------------------------------------------------------------
# BOOST  find the boost libraries
# ------------------------------------------------------------
# TODO check if boost works (e.g. libstdc++ vs libc++)
set(
    BOOST_REQUIRED_LIBRARIES
    filesystem
    system
    unit_test_framework
    date_time
)

include(addBoost)

target_link_libraries(griddyn_base INTERFACE Boostlibs::core)

set(GRIDDYN_BOOST_VERSION_LEVEL ${BOOST_VERSION_LEVEL})

# ------------------------------------------------------------
# add coverage target
# ------------------------------------------------------------
if(ENABLE_CODE_COVERAGE_TEST)
    if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
        include(CodeCoverage)

        set(
            COVERAGE_EXCLUDES
            'usr/*'
            'dependencies/*'
            'ThirdParty/*'
            'test/*'
            'tests/*'
            'swig/*'
            'examples/*'
        )
        setup_target_for_coverage(
            NAME
            griddyn_coverage # New target name
            EXECUTABLE
            ctest
            -R
            testShared # Executable in PROJECT_BINARY_DIR
        )
    else()
        message(
            FATAL_ERROR
                "CMAKE_BUILD_TYPE must be set to Coverage for testing code coverage"
        )
    endif()
endif(ENABLE_CODE_COVERAGE_TEST)

include(GNUInstallDirs)

# ------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# ------------------------------------------------------------
# message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${AUTOBUILD_INSTALL_PATH}/include/griddyn/)
include(configGenerator)
install(
    FILES ${AUTOBUILD_INSTALL_PATH}/include/griddyn/compiler-config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn
    COMPONENT headers
)

# ------------------------------------------------------------
# Enable OpenMP support?
# ------------------------------------------------------------
if(ENABLE_OPENMP)
    find_package(OpenMP QUIET)
    if(OPENMP_FOUND)
        if(NOT ${CMAKE_VERSION} VERSION_LESS "3.9")
            link_libraries(OpenMP::OpenMP_C OpenMP::OpenMP_CXX)
        else()
            add_compile_options(
                $<$<COMPILE_LANGUAGE:C>:${OpenMP_C_FLAGS}>
                $<$<COMPILE_LANGUAGE:CXX>:${OpenMP_CXX_FLAGS}>
            )
            link_libraries(${OpenMP_C_FLAGS})
        endif()
    else(OPENMP_FOUND)
        message(
            STATUS
                "Disabling OpenMP support, could not determine compiler flags"
        )
        set(ENABLE_OPENMP OFF)
    endif(OPENMP_FOUND)
endif(ENABLE_OPENMP)

cmake_dependent_option(
    ENABLE_OPENMP_SUNDIALS
    "Enable SUNDIALS NVector openMP implementation"
    ON
    "ENABLE_OPENMP"
    OFF
)

cmake_dependent_option(
    # TODO look at this
    ENABLE_OPENMP_GRIDDYN
    "Enable openmp internal to GridDyn--not used yet"
    OFF
    "ENABLE_OPENMP"
    OFF
)

# ------------------------------------------------------------
# Find multithreading headers and includes
# ------------------------------------------------------------
if(ENABLE_MULTITHREADING)
    if(NOT WIN32 OR MSYS)
        set(THREADS_PREFER_PTHREAD_FLAG ON)
    endif()
    find_package(Threads REQUIRED)
    target_link_libraries(griddyn_base INTERFACE Threads::Threads)
endif(ENABLE_MULTITHREADING)

# -------------------------------------------------------------
# Update git submodules
# -------------------------------------------------------------
include(updateGitSubmodules)
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/fmtlib/CMakeLists.txt")
	submod_update(ThirdParty/fmtlib)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp/CMakeLists.txt")
	submod_update(ThirdParty/jsoncpp)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/containers/gmlc/containers/BlockingQueue.hpp")
	submod_update(ThirdParty/containers)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/gmlc/concurrency/TriggerVariable.hpp")
	submod_update(ThirdParty/concurrency)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/sundials/include/ida/ida.h")
	submod_update(extern/sundials)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities/stringOps.h")
	submod_update(ThirdParty/utilities)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/units/units/units.hpp")
	submod_update(ThirdParty/units)
endif()


# ------------------------------------------------------------
# finding MPI
# ------------------------------------------------------------
if(ENABLE_MPI)
    include(addMPI)
    if(TARGET MPI::MPI_C)
        target_link_libraries(griddyn_base INTERFACE MPI::MPI_C)
    else()
        set(ENABLE_MPI FALSE)
    endif()
endif(ENABLE_MPI)

# just check some intenal options
if(BUILD_DEBUG_ONLY AND BUILD_RELEASE_ONLY)
    message(
        fatal
        "BUILD_DEBUG_ONLY AND BUILD_RELEASE_ONLY cannot be specified together"
    )
endif()

# ------------------------------------------------------------
# Find (and test) the KLU libraries
# ------------------------------------------------------------
if(ENABLE_KLU)
    include(addKLU_targets)
endif()

# ------------------------------------------------------------
# Sundials
# ------------------------------------------------------------
include(addSundials)

# ------------------------------------------------------------
# Enable HELICS executable
# ------------------------------------------------------------
# The order needs to be this -> ZMQ -> addHELICS due to knowledge dependencies
if(ENABLE_HELICS_EXECUTABLE)
    show_variable(
        HELICS_INSTALL_PATH
        PATH
        "path to the helics installation"
        "${PROJECT_BINARY_DIR}/libs"
    )
    set(ZMQ_NEEDED ON CACHE BOOL "ZMQ is needed" FORCE)
else(ENABLE_HELICS_EXECUTABLE)
    hide_variable(HELICS_INSTALL_PATH)
endif(ENABLE_HELICS_EXECUTABLE)

# ------------------------------------------------------------
# Enable ZMQ interfacing
# ------------------------------------------------------------
if(ENABLE_DIME)
    set(ZMQ_NEEDED ON CACHE BOOL "ZMQ is needed" FORCE)
endif(ENABLE_DIME)

hide_variable(ZMQ_NEEDED)

# If ZMQ library is enabled try to locate it and link against it
cmake_dependent_option(
    ENABLE_ZMQ
    "Enable ZMQ networking library"
    OFF
    "NOT ZMQ_NEEDED"
    OFF
)

if(ENABLE_ZMQ OR ZMQ_NEEDED)
    include(addZeroMQ)
    if(NOT ZeroMQ_FOUND)
        message(SEND_ERROR "unable to locate zmq library")
    endif()
endif()

# ------------------------------------------------------------
# setting the RPATH
# ------------------------------------------------------------
# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_MACOSX_RPATH 1)

set(
    CMAKE_BUILD_RPATH
    "${AUTOBUILD_INSTALL_PATH}/bin;${AUTOBUILD_INSTALL_PATH}/lib;${AUTOBUILD_INSTALL_PATH}/lib64"
)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(
    FIND
        CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}" isSystemDir
)

if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
endif("${isSystemDir}" STREQUAL "-1")

list(
    FIND
        CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir
)
if("${isSystemDir}" STREQUAL "-1")
    list(
        APPEND
            CMAKE_INSTALL_RPATH
            "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
    )
endif("${isSystemDir}" STREQUAL "-1")

if(NOT Boost_USE_STATIC_LIBS)
    list(APPEND CMAKE_INSTALL_RPATH ${Boost_LIBRARY_DIRS})
    list(APPEND CMAKE_BUILD_RPATH ${Boost_LIBRARY_DIRS})
endif()

# ------------------------------------------------------------
# global include directories
# ------------------------------------------------------------
target_include_directories(
    griddyn_base
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${AUTOBUILD_INSTALL_PATH}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/containers>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/concurrency>
)


# -----------------------------------------------------------------------------
# create the fmt header only targets
# -----------------------------------------------------------------------------
include(addfmt)

# --------------------------------------------------------------
#  Create the target for jsoncpp
# -----------------------------------------------------------
include(addJsoncpp)
set_target_properties(jsoncpp_lib PROPERTIES FOLDER Extern)

# --------------------------------------------------------------
#  Create the target for unitslib
# -----------------------------------------------------------
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14) # Supported values are ``11``, ``14``, and ``17``.
endif()
set(BUILD_UNITS_STATIC_LIBRARY ON CACHE INTERNAL "")
add_subdirectory("${PROJECT_SOURCE_DIR}/ThirdParty/units" "${PROJECT_BINARY_DIR}/ThirdParty/units")
set_target_properties(units-static PROPERTIES FOLDER Extern)

# -----------------------------------------------------------------------------
# create utilities target
# -----------------------------------------------------------------------------
add_subdirectory(ThirdParty/utilities)
set_target_properties(gmlc_utilities PROPERTIES FOLDER Extern)
target_include_directories(gmlc_utilities SYSTEM PRIVATE $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>)

# ------------------------------------------------------------
# load the required subdirectories
# ------------------------------------------------------------
add_subdirectory(src)


# ------------------------------------------------------------
# Enable Doxygen
# ------------------------------------------------------------
if(ENABLE_GRIDDYN_DOXYGEN)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)

        show_variable(
            DOXYGEN_OUTPUT_DIR
            PATH
            "location to put Doxygen docs"
            "${CMAKE_CURRENT_SOURCE_DIR}/docs"
        )
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/config/Doxyfile.in
            ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
        )
        add_custom_target(
            doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif(DOXYGEN_FOUND)
endif(ENABLE_GRIDDYN_DOXYGEN)

file(GLOB KEY_LIBRARY_FILES ${PROJECT_BINARY_DIR}/libs/bin/*)
message(STATUS "key files ${KEY_LIBRARY_FILES}")

# TODO this should probably be in a different file
# ------------------------------------------------------------
# Enable clang analysis and formatting tools
# ------------------------------------------------------------
if(ENABLE_CLANG_TOOLS)
    include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)

# TODO this should be under tests/
# ------------------------------------------------------------
# Enable testCore construction?
# ------------------------------------------------------------
if(ENABLE_GRIDDYN_TESTS)
    # message(STATUS "otcf:${optional_component_test_files}")
    add_subdirectory(test)
    if(BUILD_SHARED_LIBS)
        add_subdirectory(test/testSharedLibrary)
    endif(BUILD_SHARED_LIBS)
    enable_testing()
    # add_test(NAME gridDynTest COMMAND testCore)
endif(ENABLE_GRIDDYN_TESTS)

set(
    GRIDDYN_CMAKECONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    CACHE STRING "install path for GriddynConfig.cmake"
)

# TODO group the installs better
install(DIRECTORY examples/ DESTINATION examples COMPONENT examples)
# file(GLOB ebraidfiles "examples/braid_examples/*.*")
# install(FILES ${ebraidfiles} DESTINATION examples/braid_examples COMPONENT examples)

set(binfiles bin/configure.griddyn bin/pgriddyn bin/pgriddyn_debug)
install(
    PROGRAMS ${binfiles}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT binaries
)

install(
    FILES ${KEY_LIBRARY_FILES}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT binaries
)

install(
    EXPORT griddyn-targets
    NAMESPACE Griddyn::
    DESTINATION ${GRIDDYN_CMAKECONFIG_INSTALL_DIR}
    COMPONENT libs
)

# ------------------------------------------------------------
# Future Additions
# ------------------------------------------------------------
# adding dlls
# install(FILES ${LOCATION_OF_FILES} DESTINATION bin)
# file(GLOB docs "docs/manuals/*")
# install(FILES ${docs} DESTINATION docs)

# TODO this should be under tests/ too
# ------------------------------------------------------------
# CTest
# ------------------------------------------------------------
if(ENABLE_GRIDDYN_TESTS)
    include(CTest)
    find_program(CTEST_MEMORYCHECK_COMMAND valgrind)

    add_test(
        NAME Example179BusDynamicTest
        COMMAND
            gridDynMain
            ${PROJECT_SOURCE_DIR}/test/test_files/IEEE_test_cases/ieee300.cdf
    )
    set_property(
        TEST Example179BusDynamicTest
        PROPERTY
            LABELS
            Quick
            Continuous
            Nightly
    )

    add_test(NAME testComponents COMMAND testComponents)
    add_test(NAME testLibrary COMMAND testLibrary)
    add_test(NAME testSystem COMMAND testSystem)
    add_test(NAME testExtra COMMAND testExtra)
    set_property(
        TEST testComponents testLibrary testSystem
        PROPERTY LABELS Nightly Release
    )
    set_property(TEST testExtra PROPERTY LABELS TESTLABEL Release)

    # Runs a subset of the overall tests
    add_test(NAME testComponentsQuick COMMAND testComponents --run_test=@quick)
    add_test(NAME testLibraryQuick COMMAND testLibrary --run_test=@quick)
    add_test(NAME testSystemQuick COMMAND testSystem --run_test=@quick)
    set_property(
        TEST testComponentsQuick testSystemQuick
        PROPERTY LABELS Quick Continuous
    )
    set_property(
        TEST testLibraryQuick
        PROPERTY
            LABELS
            Quick
            Continuous
            Valgrind
    )

    if(BUILD_GRIDDYN_SHARED_LIBRARY)
        add_test(NAME testSharedLibrary COMMAND shared_library_tests)
        set_property(TEST testSharedLibrary PROPERTY LABELS Nightly Release)

        add_test(
            NAME testSharedLibraryQuick
            COMMAND shared_library_tests --run_test=@quick
        )
        set_property(
            TEST testSharedLibraryQuick
            PROPERTY
                LABELS
                Quick
                Continuous
                Valgrind
        )
    endif()
endif()

# ------------------------------------------------------------
# swig interface builds
# ------------------------------------------------------------
if(INTERFACE_BUILD)
    add_subdirectory(interfaces)
endif()

# TODO make a cpack file, put this there
# ------------------------------------------------------------
# CPack
# ------------------------------------------------------------
if(ENABLE_PACKAGE_BUILD)
    set(CPACK_PACKAGE_NAME "GridDyn")
    set(CPACK_PACKAGE_VENDOR "Lawrence Livermore National Security LLC")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GridDyn Installer")
    set(CPACK_PACKAGE_VERSION "${GRIDDYN_VERSION}")
    set(CPACK_PACKAGE_VERSION_MAJOR ${GRIDDYN_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${GRIDDYN_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${GRIDDYN_VERSION_PATCH})

    set(
        CPACK_COMPONENTS_ALL
        applications
        headers
        libs
        binaries
        matlab
        python
        java
        octave
        examples
    )

    if(WIN32)
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

    set(
        CPACK_COMPONENT_APPLICATION_DESCRIPTION
        "Executables and helper applications for GridDyn"
    )
    set(
        CPACK_COMPONENT_LIBS_DESCRIPTION
        "Libraries for compiling and linking with GridDyn"
    )
    set(
        CPACK_COMPONENT_HEADERS_DESCRIPTION
        "Headers for linking and compiling with GridDyn"
    )
    set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Runtime libraries for GrdDyn")
    set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Example files for GridDyn")

    set(
        CPACK_COMPONENT_GROUP_INTERFACES_DESCRIPTION
        "additional language interfaces for GridDyn"
    )

    set(CPACK_COMPONENT_LIBS_DEPENDS headers)
    set(CPACK_COMPONENT_RUNTIME_REQUIRED ON)
    if(WIN32)
        set(
            CPACK_PACKAGE_ICON
            "${CMAKE_SOURCE_DIR}\\\\docgen\\\\images\\\\griddyn.ico"
        )
        set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/docgen/images/griddyn.ico")

        set(CPACK_NSIS_INSTALL_ROOT "C:\\\\local\\\\")
        set(CPACK_NSIS_URL_INFO_ABOUT "https://www.github.com/LLNL/GridDyn")
        set(
            CPACK_NSIS_MENU_LINKS
            "https://www.github.com/LLNL/GridDyn"
            "GridDyn source code"
            "https://www.griddyn.org"
            "GridDyn Web Page"
        )

        set(CPACK_NSIS_INSTALL_ROOT "C:\\\\local\\\\")
    else(WIN32)
        set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/docgen/images/griddyn.ico")
    endif(WIN32)
    set(CPACK_SOURCE_IGNORE_FILES "/Build*/;/build*/;/.git/")

    # THIS LINE MUST BE LAST
    include(CPack)
endif(ENABLE_PACKAGE_BUILD)

if (NOT GRIDDYN_ROOT_PROJ)
	#restore the original module path
	set(CMAKE_MODULE_PATH ${ORIGINAL_MODULE_PATH})
endif()