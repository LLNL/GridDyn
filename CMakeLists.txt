# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

# cmake >= 3.5 required
cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)

# project name
project(GRIDDYN VERSION 0.9.0)

# -------
# options
# -------
include(CMakeDependentOption)

option(
    # TODO this happens in cmake/compiler_flags.cmake and cmake/installer/cmakelists
    ENABLE_EXTRA_COMPILER_WARNINGS
    "disable compiler warning for GridDyn build"
    ON
)

option(
    # TODO this needs a better name
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
    BUILD_CXX_SHARED_LIB
    "Build a Shared Libraries of the CXX interface"
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

cmake_dependent_option(
    ENABLE_OPENMP_SUNDIALS
    "Enable SUNDIALS NVector openMP implementation"
    ON
    "ENABLE_OPENMP"
    OFF
)

cmake_dependent_option(
    ENABLE_OPENMP_GRIDDYN
    "Enable openmp internal to GridDyn--not used yet"
    OFF
    "ENABLE_OPENMP"
    OFF
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

option(
    # ENABLE_EXTRA_MODELS?
    LOAD_EXTRA_MODELS
    "Compile and load extraModels"
    ON
)

option(
    LOAD_EXTRA_SOLVERS
    "Compile and load extraSolvers"
    OFF
)

option(
    ENABLE_FMI
    "Enable FMI support"
    ON
)

option(
    ENABLE_FSKIT
    "Enable FSKIT support"
    OFF
)


set(EXTENDED_INDEXING_ALLOWED "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
# TODO make sure this is adequately tested and consistently used
cmake_dependent_option(
    ENABLE_64_BIT_INDEXING
    "set all indexing and count variables to 64 bit unsigned (Usually not required)"
    OFF
    "EXTENDED_INDEXING_ALLOWED"
    OFF
)

option(
    ENABLE_GRIDDYN_LOGGING
    "enable all normal, debug, and trace logging in GridDyn"
    ON
)

cmake_dependent_option(
    ENABLE_GRIDDYN_TRACE_LOGGING
    "enable trace logging"
    ON
    "ENABLE_GRIDDYN_LOGGING"
    OFF
)

option(
    ENABLE_GRIDDYN_DEBUG_LOGGING
    "enable debug logging"
    ON
    "ENABLE_GRIDDYN_LOGGING"
    OFF
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
    "Enable network based communiation components"
    OFF
)

cmake_dependent_option(
    DIME_ENABLE
    "Enable connection with Dime"
    OFF
    "ENABLE_NETWORKING_LIBRARY"
    OFF
)

cmake_dependent_option(
    TCP_ENABLE
    "Enable TCP connection library"
    ON
    "ENABLE_NETWORKING_LIBRARY"
    OFF
)

cmake_dependent_option(
    ZMQ_ENABLE
    "Enable ZMQ networking library"
    OFF
    "NOT ZMQ_NEEDED"
    OFF
)

option(
    ENABLE_OPTIMIZATION
    "Enable Optimization libraries"
    OFF
)

option(
    ENABLE_CLANG_TOOLS
    "if clang is found enable some custom targets for clang formatting and tidy"
    OFF
)

option(
    ENABLE_TESTS
    "Enable unit testing construction"
    ON
)

option(
    ENABLE_PACKAGE_BUILD
    "Add projects for making packages and installers for HELICS"
    OFF
)

# version number
set(GRIDDYN_VERSION_BUILD)
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

set(GRIDDYN_DATE 4-28-2018)
set(GRIDDYN_VERSION_STRING "${GRIDDYN_VERSION} ${GRIDDYN_DATE}")
set(GRIDDYN_VERSION_STRING_SHORT "${GRIDDYN_VERSION}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ------------------------------------------------------------
# set the module path and include some common macros
# ------------------------------------------------------------
set(
    CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/config/cmake/"
)

include(extraMacros)
# include(BuildType)

# ------------------------------------------------------------
# set the install path to a local directory
# ------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(WIN32)
        if(MSYS)
            set(
                CMAKE_INSTALL_PREFIX
                "/usr/local/griddyn_${GRIDDYN_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path"
                FORCE
            )
        else(MSYS)
            set(
                CMAKE_INSTALL_PREFIX
                "C:/local/griddyn_${GRIDDYN_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path"
                FORCE
            )
        endif(MSYS)

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
include(extraMacros)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)

# Prohibit in-source build
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(
        FATAL_ERROR
            "In-source build is not supported. Please, use an empty directory for building the project."
    )
endif()

# add a baseline library for underlying dependencies and flags
add_library(griddyn_base INTERFACE)


show_variable(
    COMPILER_OPTIMIZATION_LEVEL
    STRING
    "set the optimization level for the compiler (not implemented yet)"
    "normal"
)

set(optimization_levels normal;high;machine;full)

set_property(
    CACHE COMPILER_OPTIMIZATION_LEVEL
    PROPERTY STRINGS ${optimization_levels}
)


if(NOT CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX d)
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

mark_as_advanced(BUILD_CXX_SHARED_LIB)

if(
    INTERFACE_BUILD
    OR BUILD_GRIDDYN_SHARED_LIBRARY
    OR BUILD_CXX_SHARED_LIB
    OR ENABLE_FMI_EXPORT
)
    set(BUILD_SHARED_LIBS ON)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(compiler_flags)

if(VERSION_OPTION)
    if(MSVC)
        target_compile_options(griddyn_base INTERFACE ${VERSION_OPTION})
    else()
        target_compile_options(
            griddyn_base
            INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>
        )
    endif(MSVC)
endif(VERSION_OPTION)

# ------------------------------------------------------------
# BOOST  find the boost libraries
# ------------------------------------------------------------
set(
    BOOST_REQUIRED_LIBRARIES
    program_options
    unit_test_framework
    filesystem
    system
    date_time
    timer
    chrono
)
include(addBoost)
set(GRIDDYN_BOOST_VERSION_LEVEL ${BOOST_VERSION_LEVEL})
target_link_libraries(griddyn_base INTERFACE Boostlibs::core)

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

include(ExternalProject)

# include(mergestaticlibs)
# ------------------------------------------------------------
# Enable OpenMP support?
# ------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # TODO look at ENABLE_OPENMP
endif()

if(ENABLE_OPENMP)
    find_package(OpenMP)
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
        set(ENABLE_OPENMP_SUNDIALS OFF)
        set(ENABLE_OPENMP_GRIDDYN OFF)
    endif(OPENMP_FOUND)
endif(ENABLE_OPENMP)

# ------------------------------------------------------------
# Find multithreading headers and includes
# ------------------------------------------------------------

if(ENABLE_MULTITHREADING)
    if(NOT WIN32)
        set(THREADS_PREFER_PTHREAD_FLAG ON)
    elseif(MSYS)
        set(THREADS_PREFER_PTHREAD_FLAG ON)
    endif()
    find_package(Threads REQUIRED)
    target_link_libraries(griddyn_base INTERFACE Threads::Threads)
endif(ENABLE_MULTITHREADING)

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
else(ENABLE_MPI)
    set(ENABLE_MPI FALSE)
endif(ENABLE_MPI)

# just check some intenal options
if(BUILD_DEBUG_ONLY AND BUILD_RELEASE_ONLY)
    message(
        fatal
        "BOTH BUILD_DEBUG_ONLY AND BUILD_RELEASE_ONLY cannot be specified together otherwise nothing works"
    )
endif()

# ------------------------------------------------------------
# Find (and test) the KLU libraries
# ------------------------------------------------------------
if(ENABLE_KLU)
    # TODO this is KLU_ENABLE in a couple places
    include(addKLU_targets)
endif()

# ------------------------------------------------------------
# Sundials
# ------------------------------------------------------------

include(addSundials)
set(optional_component_test_files)
set(optional_system_test_files)

set(optional_library_key_headers)
set(optional_library_functions)

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
)

target_include_directories(
    griddyn_base SYSTEM
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
        $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
)

# ------------------------------------------------------------
# load the required subdirectories
# ------------------------------------------------------------

add_subdirectory(src/formatInterpreters)

add_subdirectory(src/core)
# required for utilities library

add_subdirectory(src/utilities)

add_subdirectory(src/griddyn)

# now go into the optional stuff

add_library(griddyn_optional INTERFACE)

# ------------------------------------------------------------
# Enable support for Plugins
# ------------------------------------------------------------


if(ENABLE_PLUGINS)
    if(BOOST_VERSION_LEVEL==0)
        set(ENABLE_PLUGINS OFF)
        message(WARNING "Plugin support requires BOOST 1.61 or higher")
    else()
        add_subdirectory(src/plugins)
        set(DL_REQUIRED ON)
    endif()
endif(ENABLE_PLUGINS)

# ------------------------------------------------------------
# Enable compilation of extraModels?
# ------------------------------------------------------------

# If extra models are used enabled try to locate the libraries
# link against them.

if(LOAD_EXTRA_MODELS)
    add_subdirectory(src/extraModels)
    target_link_libraries(griddyn_optional INTERFACE extraModelLibrary)
endif(LOAD_EXTRA_MODELS)

# ------------------------------------------------------------
# Enable compilation of extraSolvers?
# ------------------------------------------------------------

# If extra solvers are used enabled try to locate the libraries
# link against them.

if(LOAD_EXTRA_SOLVERS)
    add_subdirectory(src/extraSolvers)
    target_link_libraries(griddyn_optional INTERFACE extraSolverLibrary)
endif(LOAD_EXTRA_SOLVERS)

# ------------------------------------------------------------
# Enable FMI support?
# ------------------------------------------------------------

if(ENABLE_FMI)
    if(BOOST_VERSION_LEVEL==0)
        set(ENABLE_FMI OFF CACHE BOOL "Enable FMI Support" FORCE)
        message(WARNING "FMI support requires BOOST 1.61 or higher, disabling it")
    else()
        add_subdirectory(src/fmi)
        target_link_libraries(griddyn_optional PUBLIC INTERFACE fmiGDLibrary)
        set(DL_REQUIRED ON)
    endif()
endif(ENABLE_FMI)

# ------------------------------------------------------------
# FSKIT
# ------------------------------------------------------------
if(ENABLE_FSKIT)
    # TODO this needs to do a find_package(fskit) of some sort
    add_subdirectory(src/fskit)
    target_link_libraries(griddyn_optional INTERFACE fskitLibrary)
endif(ENABLE_FSKIT)

# ------------------------------------------------------------
# Enable message logging
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

if(DIME_ENABLE)
    set(ZMQ_NEEDED ON CACHE BOOL "ZMQ is needed" FORCE)
endif(DIME_ENABLE)

hide_variable(ZMQ_NEEDED)


# If ZMQ library is enabled try to locate it and link against it

if((ZMQ_ENABLE) OR (ZMQ_NEEDED))
    include(addZeroMQ)
    if(NOT ZeroMQ_FOUND)
        message(SEND_ERROR "unable to locate zmq library")
    endif()
endif()

if(ENABLE_NETWORKING_LIBRARY)
    add_subdirectory(src/networking)
    target_link_libraries(griddyn_optional INTERFACE networking)
endif()

if(ENABLE_HELICS_EXECUTABLE)
    include(addHELICS)
    add_subdirectory(src/helics)
    target_link_libraries(griddyn_optional INTERFACE helicsLib)
endif(ENABLE_HELICS_EXECUTABLE)

file(GLOB KEY_LIBRARY_FILES ${PROJECT_BINARY_DIR}/libs/bin/*)
message(STATUS "key files ${KEY_LIBRARY_FILES}")
# ------------------------------------------------------------
# load the subdirectories
# ------------------------------------------------------------

# If OPTIMIZATION is enabled try to locate the libraries
# link against them.
if(ENABLE_OPTIMIZATION)
    add_subdirectory(src/optimization)
    target_link_libraries(griddyn_optional INTERFACE optimization)
endif(ENABLE_OPTIMIZATION)

add_subdirectory(src/coupling)

add_subdirectory(src/gridDynLoader)

add_subdirectory(src/fileInput)

add_subdirectory(src/runner)

if(ENABLE_FMI_EXPORT)
    add_subdirectory(src/fmi_export)
endif(ENABLE_FMI_EXPORT)

if(BUILD_SHARED_LIBS)
    add_subdirectory(src/griddyn_shared)
endif(BUILD_SHARED_LIBS)

add_subdirectory(src/gridDynMain)

install(
    TARGETS griddyn_base griddyn_optional
    EXPORT griddyn-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries
)
# ------------------------------------------------------------
# Enable clang analysis and formatting tools
# ------------------------------------------------------------


if(ENABLE_CLANG_TOOLS)
    include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)
# ------------------------------------------------------------
# Enable testCore construction?
# ------------------------------------------------------------


if(ENABLE_TESTS)
    # message(STATUS "otcf:${optional_component_test_files}")
    add_subdirectory(test)
    if(BUILD_SHARED_LIBS)
        add_subdirectory(test/testSharedLibrary)
    endif(BUILD_SHARED_LIBS)
    enable_testing()
    # add_test(NAME gridDynTest COMMAND testCore)
endif(ENABLE_TESTS)

set(
    GRIDDYN_CMAKECONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    CACHE STRING "install path for GriddynConfig.cmake"
)

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

# ------------------------------------------------------------
# CTest
# ------------------------------------------------------------

if(ENABLE_TESTS)
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
    add_subdirectory(swig)
endif()

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
