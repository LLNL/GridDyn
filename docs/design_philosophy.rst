=================
Design Philosophy
=================

GridDyn was formulated as a tool to aid in research in simulator coupling. Its use has expanded but it is primarily a research tool
into grid simulation and power grid related numeric methods, and is designed and constructed to enable that research. It is open source,
released under a BSD license. All included code will have a similarly permissive license. Any connections with software of other licenses
will require separate download and installation. It is intended to be fully cross platform, enabling use on all major operating systems,
all libraries used internally must support the same platforms. However interaction with other simulators, such as for distribution or
communication may impose additional platform restrictions. Optional components may not always abide by the same restrictions. Prior to
release 1.0 very little effort will be expended in backwards compatibility. GridDyn was written making extensive use of C++11 constructs,
and will shortly be making more use of C++14 standard constructs. Specifically allowing any features of the standard supported by GCC 4.9.x versions.
It is expected this will be the minimum version supported until newer compilers are much more widely accessible.

----------
Modularity
----------

GridDyn code makes heavy use of object oriented design and polymorphism and is intended to be modular and replaceable. The design intention
is to allow users to define a new object that meets a given component specification and have that be loaded into the simulation as easily as
any previously existing object, and require no knowledge of the implementation details of any other object in the simulation. Thus allowing
new and more complex models to be added to the system with no disruption to the rest of the system. Models also do not assume the precense
of any other object in the system, though they are allowed to check for the existence first. This is exemplified in the interaction of
generators with its subcomponents. Any combination of Generator Model, exciter, and governor should form a valid simulation even though some
combinations may not make much physical sense or be realistic.

-----------
Mathematics
-----------

The GridDyn code itself has only limited facilities for numeric solutions to the differential algebraic equations which define a dynamic power
system simulation. Instead it relies on external libraries interacting through a solver interface tailored for each individual solver. The models
are intended to be very flexible in support for an assortment of numeric approximations and solution models, and define the equations necessary
for model evaluation.

Initial development of dynamic simulation capability is done through a coupled differential algebraic solver with variable time stepping; the
primary solver used is IDA from the SUNDIALS package. It can use the dense solver or the KLU sparse solver which is much faster. Recent work
incorporates the use of a fixed time step solution mode, with a partitioned set of solvers separating the algebraic from the differential
components and solving them in alternating fashion. At present this is much less well tested. Initial formulations use CVODE for the differential
equations and KINSOL for the algebraic solution. Kinsol is also used to solve the power flow solution. ARKODE, an ODE solver using Runga-Kutta
methods is also available for solving the ODE portion of the partitioned solution.

In order to provide support for current and future models of grid components a decision was made to distribute the grid connectivity information
and not use a Y-bus matrix as is typical in power system simulation tools. This allows loads and tranmission lines to be modeled using arbitrary
equations. This decision alters the typical equations used to define a power flow solution at buses. Each bus simply sums the real and reactive
power produced or consumed by all connected loads, generators, and links. Those components are free to define the power as an arbitrary function
of bus voltage, angle, and frequency, provided that function is at least piecewise continuous.

.. note::

   Continuous functions work much better, piecewise continuous functions work but don't really play nicely with the variable timestepping.

Defining the problem in this way comes at a cost of complexity in the complementation and likely a performance hit but allows tremendous
flexibility for incorporating novel loads, generators, and other components into power flow and dynamic simulation solutions. The dependency
information is extracted through the Jacobian funciton call. Currently the solution always assumes the problem is non-linear even if the
approximations used are in fact linear. While GridDyn's interaction with the solvers comes exclusively through interface objects, there may
be some inherent biases in the interface definition due to primary testing with the SUNDIALS package. These will likely be exposed when
GridDyn is tested with alternative numerical solvers.

----------------
Model Definition
----------------

GridDyn is intended to be flexible in its model definition allowing details to be defined through a number of common power system formats.
The most flexible definition is through a GridDyn specific XML format. Strictly speaking the most flexible XML cannot be defined by an XML
schema due to the fact that the readers allow element names to describe properties of which the complete set of which cannot be described
due to support for externally defined models. Alternate formulations exist which could be standardized in a schema but no attempt has been
made to do so. The XML formulation includes a variety of programming like concepts to allow construction of complex models quickly, including
arrays and conditionals, as well as limited support for equations and variable definitions. The file ingest library also supports importing
other files through the XML and defining a library of objects that can be referenced and copied elsewhere. The typical use case is expected to
be importing a file of another format that contains a majority of the desired simulation information and only defining the solver information
and any GridDyn specific models and adjustments in the XML. The general idea is to be as flexible and easy to use as possible for a text based
input format, as GridDyn develops support as many other formats as is practically possible. All the file ingest functionality is contained in
a separate library from the model bookkeeping and model evaluation functionality. Other types of input can be added as necessary and some
development is taking place towards a GUI which would interact through REST service commands and JSON objects. Included in GridDyn are
capabilities of searching through objects by name, index number, or userID.

-----------
Performance
-----------

GridDyn was designed for use in an HPC environment. What that means right now is that GridDyn can interoperate with other simulators in that
environment and some considerations were put in place in the design, but GridDyn on its own does not really take advantage of parallel processing.
As of release 0.5 the transmission power flow and dynamic solve is not itself parallel in any way. Considerable thought has been put into how that
might be accomplished in later versions but it is not presently in place. Initial steps will include adding in optional OpenMP pragmas to take
advantage of the inherent independence of the objects in calculation of the mathematical operations such as residual or Jacobian. OpenMP vector
operations can be enabled in SUNDIALS, though this is only expected to result in small performance gains and only for models over 5000 buses.
Further tests will be done to determine exact performance gains. 

Some effort has gone into improving the performance of the power flow solve and only incremental gains are expected at this point using the current
solve methodology. No effort has been expended on the dynamic simulation so some performance improvements can be expected in that area when examined.

The system has no inherent size limitations. Limited only by memory on any given system. Scalability studies have been carried out to solving a million
bus model, It could probably go higher but the practical value of such a single solve is unclear as of yet.

---------------
Model Libraries
---------------

The aim thus far in GridDyn has been the development of the interfaces. The models available are the result of programmatic needs or the need to ensure
the simulator is capable of dealing with specific kinds of model interactions. As a result the models presently available represent only a small subset
of those defined in power system libraries. More will be available as time goes on, but the idea is not to have a large collection internally but to enable
testing of new models, and to incorporate model definition libraries through the use of other tools and interfaces such as FMI, and possibly others as needed.

-------
Testing
-------

A suite of test cases is available and will continue to grow as more components and systems are thoroughly tested. The nature of the test suite is evolving
along with the code and will continue to do so. It makes use of the BOOST test suite of tools and if built creates 5 executable test programs that test the
various aspects of the system. While we are still a ways from that target 100% test coverage is a goal though likely not realistic in the near future. The
code is regularly compiled on at least 5 different compilers and multiple operating systems and strives for warning free operation.

-------------
Test Programs
-------------

If enabled 5 test programs are built. These programs execute the unit test suite for testing GridDyn. They are divided into 5 programs. testLibrary runs tests
aimed at testing operation of the various libraries used In GridDyn. The testComponents program executes test cases targeted at the individual model components
of GridDyn. The third, testSystem, runs system level tests and some performance and validation tests on GridDyn. The testSharedLibrary tests using GridDyn as a
shared library. The last, extraTests includes some longer running tests and performance tests. After installation these test programs are placed in the install
directory and can be executed by simply running the executable. Specific tests can be executed with command line parameters.

.. code:: bash
   
   > ./testComponents --run_test=block_tests
   > ./testComponents --run_test=block_tests/block_alg_diff_jac_test
   > ./testLibrary -h
