.. _reference_cmake_options:

=============
CMake Options
=============

The CMake build scripts for GridDyn support a number of configuration options that can be set via either the `cmake-gui` or the command 
line cmake command using `-D<VAR>=<VALUE>` arguments. The CMake manual available at https://cmake.org/cmake/help/latest/manual/cmake.1.html
 describes use of `-D` and other arguments in more detail.

BOOST_INSTALL_PATH
    Sets the root location of Boost. Can be used if Boost is not found in the system directories or if a different version is desired.

BUILD_SHARED_LIBS
    Turns on building of the GridDyn C and C++ shared libraries (``BUILD_GRIDDYN_C_SHARED_LIBRARY`` and ``BUILD_GRIDDYN_CXX_SHARED_LIBRARY`` options).

BUILD_TESTING
    Enable the test executables to be built.

ENABLE_GRIDDYN_LOGGING
    Enables all normal, debug, and trace logging in GridDyn.

ENABLE_GRIDDYN_DEBUG_LOGGING
    Unselecting disables all ``DEBUG`` and ``TRACE`` log messages from getting compiled.

ENABLE_GRIDDYN_TRACE_LOGGING
    Unselecting disables all ``TRACE`` log messages from getting compiled.

DOXYGEN_OUTPUT_DIR
    Location for the generated doxygen documentation.

ENABLE_64BIT_INDEXING
    Enables support inside GridDyn for more than 2:sup:`32`-2 states or objects.

ENABLE_FMI
    Enable support for FMI objects.

ENABLE_FMI_EXPORT
    Enable construction of a binary FMI shared library for GridDyn.

ENABLE_FSKIT
    Enable to build additional libraries and support for integration into FSKIT and PARGRID for tool coupling.

ENABLE_HELICS_EXECUTABLE
    Enable the HELICS executable to be built for tool coupling using HELICS for communication.

ENABLE_KLU
    Option to disable KLU (not recommended [slow]; prefer to turn on ``AUTOBUILD_KLU``)

ENABLE_MULTITHREADING
    Disable multithreading in GridDyn libraries.

ENABLE_MPI
    Enable MPI networking library.

ENABLE_OPENMP
    Enable OpenMP support.

ENABLE_OPENMP_GRIDDYN
    Enables OpenMP use internal to GridDyn.

ENABLE_OPENMP_SUNDIALS
    Enables the SUNDIALS NVector OpenMP implementation.

ENABLE_YAML
    Enables YAML file support in GridDyn.

ENABLE_EXTRA_MODELS
    Compile and load extraModels.

ENABLE_EXTRA_SOLVERS
    Compile and load extraSolvers (including braid, paradae).

ENABLE_NETWORKING_LIBRARY
    Enable network based communication components.

ENABLE_TCP
    Enable TCP connection library. Depends on Networking.

ENABLE_DIME
    Enable connection with DIME. Depends on Networking.

ENABLE_ZMQ
    Enable ZMQ connection library. Depends on Networking.

ENABLE_PLUGINS
    Build libpluginLibrary

ENABLE_OPTIMIZATION_LIBRARY
    Enable optimization libraries.

ENABLE_CODE_COVERAGE_TEST
    Build a target for testing code coverage.

ENABLE_GRIDDYN_DOXYGEN
    Generate Doxygen doc target.

ENABLE_CLANG_TOOLS
    If Clang is found, enable some custom targets for Clang formatting and tidy.

ENABLE_PACKAGE_BUILD
    Add projects for making packages and installers for GridDyn.

ENABLE_EXTRA_COMPILER_WARNINGS
    Enable more compiler warnings (full list in config/cmake/compiler_flags.cmake)

ENABLE_EXPERIMENTAL_TEST_CASES
    Enable some experimental test cases in the test suite.

FORCE_DEPENDENCY_REBUILD
    Rebuild third party dependencies, even if they're already installed.

LOAD_ARKODE
    Build in support for ARKODE for solving differential equations. Not used at present but will be in the near future.

LOAD_CVODE
    Build in support for CVODE for solving differential equations. Not used at present but will be in the near future.

SuiteSparse_INSTALL_PATH
    The location of the KLU installation if it was not found in the system directories.

SUNDIALS_INSTALL_PATH
    The location of the SUNDIALS installation if it wasn't found (or ``AUTOBUILD_SUNDIALS`` is disabled).