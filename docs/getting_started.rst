===============
Getting Started
===============

-------------
Prerequisites
-------------

GridDyn is written in C++ and makes use of a few external libraries not included in the released source code. External software packages needing
installation prior to compilation of GridDyn include:

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

See :ref:`reference_cmake_options` for a list of CMake configuration options to turn on/off different features.

------------------
Installation Notes
------------------

Mac
^^^

For building on macOS, use Homebrew and make sure git, CMake, suite-sparse, Boost, and OpenMP are installed.

Linux
^^^^^

Depending on the distribution, Boost or an updated version of it may need to be installed (the package in the package manager may be significantly outdated).
SuiteSparse/KLU may need to be installed as well. Typically CMake is used to generate Makefiles, but it can also be used to generate Eclipse projects.
``BOOST_INSTALL_PATH`` and ``SuiteSparse_INSTALL_PATH`` may need to be user specified if they are not in the system directories. This can be done with
the ``cmake-gui`` or the command line ``cmake``. Then running ``make`` will compile the program. Running ``make install`` will copy the executables and
libraries to the install directory.

Windows
^^^^^^^

GridDyn has been built with Visual Studio 2015 and MSYS2. The MSYS2 build is like building on Linux, and works fine with GCC, though the current Clang
version on MSYS2 has library incompatibilities with some of the Boost libraries due to changes in GCC. I don't fully follow what the exact issue is but
Clang won't work on MSYS2 to compile GridDyn unless SUNDIALS, Boost, and KLU are compiled with the same compiler, I suspect the same issue is also present
in some other Linux platforms that use GCC 5.0 or greater as the default compiler. The SuiteSparse version available through pacman on MSYS2 seems to work fine.

For compilation with Visual Studio, Boost will need to be built with the same version as is used to compile GridDyn. Otherwise, follow the same instructions.

---------------
Running GridDyn
---------------

The main executable for GridDyn is built as `gridDynMain` and is intended to load and run a single simulation. The executables `testSystem`, `testComponents`,
`testLibrary`, `testSharedLibrary`, and `extraTests` are test programs for the unit testing of GridDyn. A server mode for interactive sessions is a work in
progress, but is not operational at the time of this release.

.. code:: bash

   > ./gridDynMain --version

will display the version information.

.. code:: bash

   > ./gridDynMain --h

will display available command line options.

Typical usage is:

.. code:: bash

   > ./gridDynMain [options] inputFile [options]

The primary input file can be specified with the flag `--input` or a single flagless argument. Additional input files should be specified using `-i` or
`--import` flags.

Command line only options:

--help
    Print the help message
--config-file arg
    Specify a config file to use
--config-file-output arg
    File to store current config options
--mpicount
    Setup for an MPI run, prints out a string listing the number of MPI tasks that are required to execute the specified scenario, then halts execution
--helics
    Setup for a HELICS run for a coupled co-simulation
--version
    Print version string

Configuration options:

-o, --powerflow-output filename
    File output for the powerflow solution. Extension specifies a type (.csv, .xml, .dat, .bin, .txt), unrecognized extensions default to the same format
    as .txt
-P, --param arg
    Override simulation file parameters, `-param ParamName=<val>`
-D, --dir directory
    Add search directory for input files
-i, --import filename
    Add import files loaded after the main input file
--powerflow-only
    Set the solver to stop after the power flow solution
--state-output filename
    File for saving states, corresponds to `--save-state-period`
--save-state-period arg
    Save state every N ms, -1 for saving only at the end
--log-file filename
    Log file output
-q, --quiet
    Set verbosity to zero and printing to none
--jac-output arg
    Powerflow Jacobian file output
--v, --verbose arg
    Specify verbosity output, 3=verbose, 2=normal, 1=summary, 0=none
-f, --flags arg
    Specify flags to feed to the solver, e.g. `--flags=flag1,flag2,flag3` no spaces between flags if multiple flags are specified or enclose in quotes
-w, --warn arg
    Specify warning level output for input file processing, 2=all, 1=important, 0=none
--auto-capture filename
    Automatically capture a set of parameters from a dynamic simulation to the specified file format determined by extension. Either .csv or .txt will
    record the output in csv format, all others will record in the binary format. The filename must be specified with `--auto-capture-period` if used.
--auto-capture-period arg
    Specifies the automatic capture period in seconds. If specified without a corresponding `--auto-capture` file, a file named `auto_capture.bin` is created.
--auto-capture-field arg
    Specify the fields ot be captured through the auto capture. Defaults to `auto`. Can be a comma or semicolon separated list, no spaces unless enclosed
    in quotes.

The configuration routine will look for and load a file named `gridDynConfig.ini` if it is available. It will also load any command line specified config
file. The order of precedence is command line, user specified config file, then system config file (if available).