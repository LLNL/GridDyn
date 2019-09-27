.. _basics_development_notes:

==================
Development Notes
==================

GridDyn is very much a work in progress, development is proceeding on a number of different aspects from a number of directions and many components are
in states of partial operation or are awaiting development in other aspects of the code base. The notes in this section attempt to capture the development
status of various Griddyn components and note where active and planned development is taking place.

-------------------------
Interface and Executables
-------------------------

A gridDynServer executable is in development. This program will become the main means of interacting with simulations. The plan will be for it to support
multiple running simulations and allow users to interact through a set of interfaces. Planned interfaces include a RESTful service interface for ethernet
based interaction, which will eventually be the basis of interaction with a GUI, a command line interface, and a direct application interface through
TCP/UDP or MPI.

Also in development is a wrapper around the simulation engine into a Functional Mockup Interface to allow GridDyn to interact with other simulations
through the FMI for co-simulation framework.

------
Models
------

The models included in GridDyn are an evolving set. They have been added to address particular research questions or needs or test specific aspects of
GridDyn operation. The next several subsections talk about the state of development in the various components available in GridDyn.

Buses
^^^^^

The bus code is well tested but is constantly evolving to simplify the code or areas of responsibility, or to improve operation, even though the equations
used in the bus evaluation are quite straightforward. The bus itself is one of the more complex objects in GridDyn in order to handle the management of
loads and generators and the associated limits and controls. As well as the associated transition between powerflow and dynamic simulation. Currently
available are an ACbus, a DC bus for association with HVDC transission lines, a trivial bus, and an infinite bus. Some plans are in place for a 3-phase bus
but that has been low on the priority list. The DC bus is not thoroughly tested, particularly in dynamic contexts.

Area
^^^^

At present areas are primary used as a way to group objects. Ongoing development is taking place to add in area wide controls such as AGC. Some of these
structures are in place but have yet to be tied in with the Area model itself. There is work ongoing to do this and some form will be functional within the
next 3 months. Areas and subareas can be configured through the GridDyn XML format but none of the other available formats such as CDF or PTI currently make
use of the area information available in those formats. This will be added alongside the development of area controls.

Links
^^^^^

The basic AC link has been tested thoroughly in powerflow and dynamic simulations by comparison with standard test cases. Other link models such as DC links,
and an `adjustableTransformer` model have been tested in power flow simulations, but the dynamics of them are a work in progress. They operate fine in that
context but do not include the control dynamics, at least not at a level that is well-tested.

Relays
^^^^^^

The generic relay is one of the more complex objects to setup. Most use cases involve using one the specific relay types as they embody the information for
setting up a relay. There are no known issues with the relays though given their complexity it is likely there are many circumstances when they do not function
appropriately, or cause issues with interaction of the other parts of the system. The basic relay contains tremendous flexibility and it is not recommended that
beginning users attempt to directly instantiate it. You are of course welcome to try but the specification of conditions and actions is somewhat more complex
than most other system properties through the XML. Other relay types are in development as needed by specific usage requirements.

Loads
^^^^^

A number of types of loads are modeled in GridDyn. The basic model is a ZIP model. Extensions include ramps and a variety of other load shapes and others such
as an exponential load and a frequency dependent load. Also included are motor loads, including models of first order, 3rd order and 5th order induction models,
and include mechanisms for modeling motor stalling. The 5th order model has some potential issues during certain conditions that have not been fully debugged.
All work in powerflow and dynamic simulation. Code for loading a GridLab-D distribution system is included in the release but will not function without
corresponding alterations to a GridLab-D instance and operation with Pargrid, neither of which are included in this release, so for all practical purposes it
will revert to a debug mode with a simulated distribution simulation intended for debugging operations. The actual functionality necessary for coupling with a
distribution system will hopefully be released in the near future, though could be made available for partners. There is a composite load model available. This
is a more generic container for containing other load models. This is distinct and more general than the composite load model defined by FERC. Though an
instantiation of that model is planned and will make use of the generic composite model in GridDyn.

Generators

^^^^^^^^^^
Two basic generators are available, a regular generator and a variable generator intended as a base for modeling renewable generation. The `generator` model
itself contains very little in the way of mathematical functions, instead it acts as a manager for the various submodels that make up a power system generator.
These include governors, exciters, generator models, and power system stabilizers. The variable generator also has mechanisms for including sources which are
data generators, and filters. The combination of which creates a mechanism for feeding weather data to a solar or wind plant and converting that into power. The
generator is specifically formulated to allow any/all/none of the subcomponents to be present and still operate. A default generator model is put into place if
none is specified and a dynamic simulation is required. A third generator which includes a notion of energy storage is in planning stages.

Generator Models
^^^^^^^^^^^^^^^^

A wide assortment of `genModels` are included. Most have been debugged and tested. The classical generator model includes a notion of a stabilizer due to inherent
instabilities under fault conditions when attached to an exciter and/or governor. Not that the classical generator model is an appropriate model to use for such
circumstances, but nonetheless a stabilizer was incorporated to make the model stable. The incorporation of saturation into the models is not complete. The models
accept the parameters but are not included in the calculation. GENROU and GENSAL models are being developed but are not complete as of release 0.5.

Exciters
^^^^^^^^

Available exciters include simple, IEEE type1, IEEE type2, DC1A, and DC2A. The DC2A model has some undiagnosed issue in particular situations and is not recommended
for use at present.

Governors
^^^^^^^^^

The basic governor and TGOV1 models are operational, others are not completed and further work is being delayed until a more general control system model is in
place which will greatly simplify governor construction as well as other control systems. The deadband is not working in TGOV1.

Power System Stabilizers
^^^^^^^^^^^^^^^^^^^^^^^^

The current PSS code is a placeholder for future work. No PSS model is currently available, though some initial design work has taken place. The work has been
delayed until the control system code is operational.

Control Blocks
^^^^^^^^^^^^^^

Control blocks are a building block for other models and a number of them are used in other models throughout Griddyn. Development on the generic transfer function
block is not finished but the others are working and tested. These will form the building blocks of a set of general control system modules which could be used to
build other types of more complex models.

------
Others
------

Other components in Griddyn include `sources` which are operational but not well tested in practice, `schedulers` which are used to control generator
scheduling, and other types of controllers for AGC, dispatch, and other sorts of controls. Most of these are in various states of development and not
well tested.

Events
^^^^^^

Griddyn supports a notion of events which can be scheduled in a simulation and can basically alter any property of the system with the exception of some
models prohibiting changing of certain properties after simulation has begun, in this case the event will still be valid, it just won't do anything.
Support for more complex events involving multiple devices in a more straightforward fashion is planned.

Recorders
^^^^^^^^^

Support for extracting any calculated field or property from an object is supported through `grabber` objects. This can be done directly via the state
arrays or from the objects themselves. The files can be saved periodically or at the end of the simulation in a binary format or in CSV. Readers for
the binary format are available in C++, Matlab, and Python. If a large amount of data is captured frequently for dynamic simulations there is currently
a performance hit. There are ideas for mitigating this that will be addressed when the performance of the dynamic simulation is studied and addressed.

Simulation
^^^^^^^^^^

Some of the mechanics and interfacing of the planned optimization extension are in place but nothing actually works yet, so don't use it.

FMU Interaction
^^^^^^^^^^^^^^^

This works in some cases but is a little more complex to set up than the rest of the code as it is under significant active development, therefore it
is not recommended for use at this time.

----------
File Input
----------

GridDyn is capable of reading XML and Json files defining the GridDyn data directly and these formats can take advantage of all GridDyn capabilities.
Json is not as well tested and was targeted mainly for the server interface, but it should work as a file format just fine. A fairly flexible CSV input
file reader is also available for inputing larger datasets in a more workable format. CDF files are read though the area and a few other properties not
important for powerflow are not loaded into GridDyn yet. Most of the common elements in raw and pti ﬁles are also loaded properly. Some of the more exotic
elements such as multiterminal DC lines and 3-way transformers are not yet, mainly since we have no examples of such things in example files. EPC files
for PSLF are the same though used less extensively than raw files. Matlab files from Matpower and PSAT can also be loaded. Not all dynamic models from PSAT
are available, for DYR files models that match those available are loaded and some others are translated to available models. The library of models in
GridDyn is much smaller than those available in commercial tools. Support for other formats is added as needed by projects.