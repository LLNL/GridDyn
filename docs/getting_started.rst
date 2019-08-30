
===============
Getting Started
===============

-------------
Prerequisites
-------------

GridDyn is written in C++ and makes use of a few external libraries not included in the released source code. External software packages needing installation prior to compilation of GridDyn include:

#. A modern C/C++ compiler for building
#. The ``cmake`` command for building
#. The ``git`` command for getting the code
#. Boost 1.61 or greater
#. Doxygen for building in-source documentation
#. KLU in SuiteSparse - on Linux the SuiteSparse package typically has KLU, on macOS the Homebrew suite-sparse package can be used

Currently supported compilers are:

* Visual Studio 2015
* GCC 4.9.3 or higher
* Clang 3.5 or higher (OpenMP must be off to use 3.4)
* Intel 16.0 (not well tested as of yet)

-------------
CMake Options
-------------

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
    Enable the HELICS executable to be build for tool coupling using HELICS for communication.

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
    Compile and load extraSolvers.

ENABLE_NETWORKING_LIBRARY
    Enable network based communication components.

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

FORCE_DEPENDENCY_REBUILD
    Rebuild third party dependencies, even if they're already installed.

SuiteSparse_INSTALL_PATH
    The location of the KLU installation if it was not found in the system directories.

SUNDIALS_INSTALL_PATH
    The location of the SUNDIALS installation if it wasn't found (or ``AUTOBUILD_SUNDIALS`` is disabled).

------------------
Installation Notes
------------------

Mac
^^^

For building on macOS, use Homebrew and make sure git, CMake, suite-sparse, Boost, and OpenMP are installed.

Linux
^^^^^

Depending on the distribution, Boost or an updated version of it may need to be installed (the package in the package manager may be significantly outdated). SuiteSparse/KLU may need to be installed as well. Typically CMake is used to generate Makefiles, but it can also be used to generate Eclipse projects. ``BOOST_INSTALL_PATH`` and ``SuiteSparse_INSTALL_PATH`` may need to be user specified if they are not in the system directories. This can be done with the ``cmake-gui`` or the command line ``cmake``. Then running ``make`` will compile the program. Running ``make install`` will copy the executables and libraries to the install directory.

Windows
^^^^^^^

GridDyn has been built with Visual Studio 2015 and MSYS2. The MSYS2 build is like building on Linux, and works fine with GCC, thgouh the current Clang version on MSYS2 has library incompatibilities with some of the Boost libraries due to changes in GCC. I don't fully follow what the exact issue is but Clang won't work on MSYS2 to compile GridDyn unless SUNDIALS, Boost, and KLU are compiled with the same compiler, I suspect the same issue is also present in some other Linux platforms that use GCC 5.0 or greater as the default compiler. The SuiteSparse version available through pacman on MSYS2 seems to work fine.

For compilation with Visual Studio, Boost will need to be built with the same version as is used to compile GridDyn. Otherwise, follow the same instructions.

