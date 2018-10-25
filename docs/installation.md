# Installation guide

GridDyn is written in C++ and makes use of a few external libraries not included in the released source code.  
External software packages needing installation prior to compilation of GridDyn include

- **Boost 1.61**  for most parts of GridDyn Boost 1.58 or greater will be sufficient but going forward some part of GridDyn will be making use of libraries only found in 1.61 or higher.  GridDyn currently compiles with Boost version >1.49.  
- **SUNDIALS 2.6.2** (GridDyn will be updated to use the new version of SUNDIALS when it is released later in 2016 at which time that will become the required version)
- **KLU** in the suitesparse package.  
- **cmake** for building
- **Doxygen** for building in source documentation.
- all other third party code is included in the release.  


Boost can be downloaded from [Boost](www.boost.org). Many of the features in GridDyn will work with older versions but going forward we will be making use of some features from Boost 1.61 and will use that as the baseline going forward. SUNDIALS can downloaded at [sundial](http://computation.llnl.gov/sundials). GridDyn requires version 2.6.2, but will be upgraded to the new version when it is released in the near future.  SUNDIALS should be built with KLU support enabled for reasonable performance.  KLU is part of SuiteSparse, on most Linux type systems it can be installed as a package. On Windows, cmake files can be found at [KLU](https://github.com/jlblancoc/suitesparse-metis-for-windows).

GridDyn uses a cmake build system to construct build files for whatever platform you happen to be on (assuming it is supported by cmake)
GridDyn uses C++11 extensively and will make use of some C++14 features in the near future.    Therefore, required future compilers are

- Visual Studio 2015
- gcc 4.9.3 or higher (4.8 works for the moment but will not in near future updates)
- clang 3.5 or higher (openMP must be turned off to use 3.4)
- Intel 16.0 (not thoroughly tested as of yet)

At present GridDyn will likely compile on gcc 4.8 and visual studio 2013 but that is expected to change with future updates.  

# cmake options

- **BOOST_ROOT**  if boost is not found in the system directories or a different version is desired the root location of boost can be specified in BOOST\_ROOT
- **BUILD_SERVERMODE**  the servermode for GridDyn is a work in progress, it is recommended that this option be disabled pending further progress
- **ENABLE_GRIDDYN\_DEBUG\_LOGGING** unselecting disables all DEBUG and TRACE log messages from getting compiled
- **ENABLE_GRIDDYN\_TRACE\_LOGGING** unselecting disables all TRACE log messages from getting compiled
- **DOXYGEN_OUTPUT\_DIR** location for the generated doxygen documentation
- **ENABLE_64\_BIT\_INDEXING**  select if you want support inside GridDyn for more than $2^{32}-2$ states or objects (I find this unlikely at this time)
- **ENABLE_EXPERIMENTAL\_TEST\_CASES**  select to enable some experiemental test cases in the test suite
- **ENABLE_FMI** enable experiemental support for FMI objects.  This is in development,  recommended to leave unselected unless you are developing
- **ENABLE_FMI\_EXPORT** build a FMI shared library for GridDyn
- **ENABLE_FSKIT** enable to build additional libraries and support for integration into FSKIT and PARGRID for tool coupling
- **ENABLE_GRIDDYN\_DOXYGEN**  select to create a case to build the doxygen docs
- **ENABLE_OPENMP\_GRIDDYN** (doesn't do any thing yet) eventually it will enable openMP in the GridDyn evaluation functions
- **ENABLE_KLU** this option may be removed in the future recommended to leave selected otherwise KLU support will not be built in
- **KLU_INSTALL\_DIR**  point to the installation dir for KLU if it was not found in the system directories
- **LOAD_ARKODE** build in support for ARKODE for solving differential equations
- **LOAD_CVODE** build in support for CVODE for solving differential equations Neither cvode or arkode are used at present but will be in the near future
- **ENABLE_EXTRA\_MODELS** select to build an additional library containing a few optional models-more will likely be added in the future
- **ENABLE_GRIDDYN\_LOGGING**  unselect to turn off all logging functions
- **ENABLE_MPI** select to build with MPI support using an MPI compatible compiler
- **ENABLE_OPENMP** option to build in support for openMP in both the solvers and in GridDyn
- **ENABLE_OPTIMIZATION\_LIBRARY**  enable building of the optimization extension.  This is a work in progress and doesn't do much yet,  recommended to leave unselected unless you are developing on that section.
- **SUNDIALS_INSTALLATION\_DIR** point to the installation location for SUNDIALS
- **ENABLE_OPENMP\_SUNDIALS**  select to enable OpenMP in SUNDIALS assumes SUNDIALS was built with openmp support
- **ENABLE_TESTS**  enable building of the testSuites
- **ENABLE_MULTITHREADING**  not used at preset but will eventually enable threaded execution in some models
- **ENABLE_EXTRA\_COMPILER\_WARNINGS** enable more compiler warnings (full list in config/cmake/compiler_flags.cmake)
- **BUILD_PYTHON\_INTERFACE** enable the python swig interface
- **BUILD_PYTHON2\_INTERFACE** enbale the newer python swig interface
- **BUILD_MATLAB\_INTERFACE** enable the matlab swig interface
- **BUILD_OCTAVE\_INTERFACE** enable the octave swig interface
- **BUILD_JAVA\_INTERFACE** enable the java swig interface
- **BUILD_CXX\_SHARED\_LIB** build `libgriddyn_shared_lib.so`, the c++ interface
- **ENABLE_CODE\_COVERAGE\_TEST** enable code coverage tools
- **ENABLE_PLUGINS** build libpluginLibrary
- **ENABLE_EXTRA\_SOLVERS** enable additional solvers (including braid, paradae)
- **ENABLE_HELICS\_EXECUTABLE**
- **ENABLE_NETWORKING\_LIBRARY** build in networking support
- **ENABLE_CLANG\_TOOLS** enable clang code cleanup tool support in cmake
- **ENABLE_PACKAGE\_BUILD** enable packaging with cpack
- **FORCE_DEPENDENCY\_REBUILD** force libraries in ThirdParty to rebuild every time cmake is called
- **ENABLE_TCP** enable TCP connection library. Depends on Networking
- **ENABLE_DIME** enable connection with DIME. Depends on Networking
- **ENABLE_ZMQ** enable ZMQ connection library. Depends on Networking

# Installation notes
##Mac
Example of successful build on a mac OS X
GridDyn on Mac OS X


	1. Install MacPorts
	2. Install cmake port
	3.	Download [SuiteSparse 4.2.1](http://faculty.cse.tamu.edu/davis/suitesparse.html)
	4. Build/install SuiteSparse
	5. Download [SUNDIALS 2.6.2](http://computation.llnl.gov/projects/SUNDIALS-suite-nonlinear-differential-algebraic-equation-solvers/download/SUNDIALS-2.6.2.tar.gz) you may have submit some information first- don't build dynamic libs because they caused issue with GridDyn runtime
	6. Configure/build/install SUNDIALS (cd SUNDIALS-2.6.2; mkdir build; ccmake ..; update paths for your install location (prefix, klu lib and include dirs), set KLU\_ENABLE to ON); make; make install)
	7. Download/configure/build/install BOOST
	8. Configure GridDyn (cd transmission; mkdir build; cd build; ccmake -DKLU\_DIR=$<\hdots>$ -DBoost\_INCLUDE\_DIR=$<\hdots>$ -DBoost\_LIBRARY\_DIRS=$<\hdots>$; ) or use cmake-gui


##Linux
Depending on the distribution, Boost or an updated version of it may need to be installed.  SUNDIALS and KLU may need to be installed as well.  Typically camke is used to generate makefiles thought it has been used to generate Eclipse projects.    BOOST\_ROOT, SUNDIALS\_INSTALLATION\_DIR, and KLU\_INSTALL\_DIR may need to be user specified if they are not in the system directories.  This can be done in the cmake-gui or through the command line tools.  Them running make will complile the program.   
Running make install will copy the executables and libraries to the install directory.  

##Windows
GridDyn has been successfully built with Visual Studio 2015 and on Msys2.  The msys2 build is much like building on linux.  This works fine with gcc,  the current clang version on msys2 has library incompatibilities with some of the boost libraries due to changes in gcc.  I don't fully follow what the exact issue is on but clang won't work on Msys2 to compile GridDyn unless SUNDIALS, boost, and KLU are compiled with the same compiler, I suspect the same issue is also present in some other linux platforms that use gcc 5.0 or greater as the default compiler.  The suitesparse version available through pacman on msys2 seems to work fine.  

For compilation with Visual Studio boost will need to built with the same version as is used to compile GridDyn.  Otherwise follow the same instructions.
