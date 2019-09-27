
=========
XML Input
=========

The following section contains a description of the XML input file format and how to construct and specify an input file in the GridDyn XML format.
The XML format is intended to be used solely in GridDyn to enable full access to all the capabilities and models that may or may not be defined in
other formats. All the actual interpreters have been designed to use an element tree structure.  And as such the same reader code is used for the
XML interpreter and for a JSON interpreter, though there is some variance in the definitions of elements and attributes in those two contexts
meaning Json objects are somewhat more restricted in format. In the documentation, most of the examples will be in XML, but a few will be in JSON
for completeness.

---------------
Initial Example
---------------

A simple input case is as follows:

.. code:: XML

    <?xml version="1.0" encoding="utf-8"?>
    <GridDyn name="2bus_test" version="1">
   
    <bus name="bus1">
        <type>SLK</type>
        <angle>0</angle>
        <voltage>1.05</voltage>
        <generator name="gen1">
        </generator>
        <load name="load1">
            <P>1.05</P>
            <Q>0.31</Q>
        </load>
    </bus>
   
    <bus name="bus2">
        <load name="load2">
            <P>0.45</P>
            <Q>0.2</Q>
        </load>
    </bus>
    <link from="bus1" name="bus1_to_bus2" to="bus2">
        <b>0.127</b>
        <r>0.0839</r>
        <x>0.51833</x>
    </link>
   
    <flags>powerflow_only</flags>
    </GridDyn>

This small XML file defines a two bus system. There are 5 sections to this model description. The first line describes the standard XML header information
and is not used by GridDyn. The second line defines the simulation element and the name of the simulation. In general properties can be described in either
an element or as a property. There are certain aspects of parameters which can only be controlled in the element form, but for simple parameters either
works fine. Capitalization of properties also does not matter. All object properties in GridDyn are represented by lower case strings, the XML reader converts
all property names to lower case strings before input to GridDyn so capitalization doesn't matter in the XML input. The property values themselves preserve
capitalization and it is on a per property basis whether capitalization matters. For naming capitalization is preserved such that "object1" is distinct from
"Object1". For this XML file the simulation is given the name `2bus_test`. The version is for record keeping only and has no relevance to the simulation.

The second block defines a bus object with a name of `bus1`. The bus is a slack bus indicated by `<bustype>SLK</bustype>`. Other options for this parameter
include `PQ`, `PV`, `SLK`, and `afix`. The angle and voltage are specified. A generator object is included. The element `generator` is recognized as a
component and a new generator object is created with a name of `gen1`. Finally a load is created with a name of `load1` and a fixed real power of 1.05 and
a reactive power of 0.31.

The second bus is defined in a similar way, except it does not define a bustype which means it defaults to a PQ bus. The `link` is defined by:

.. code:: XML

    <link from="bus1" name="bus1_to_bus2" to="bus2">
        <b>0.127</b>
        <r>0.0839</r>
        <x>0.51833</x>
    </link>

The properties `b`, `r`, and `x` are defined in the XML as elements. The `to` and `from` fields are specified using the names of the buses. These properties
must be specified for the lines or the system will spit out a warning.

Finally, the last two lines specify that the simulation should stop after a power flow.

To add in dynamic modeling a few additional pieces of XML can be added. For our example, the powerflow_only flag at the bottom can be removed, and the
following lines can be added to the block for `gen1`:

.. code:: XML

    <generator name="gen1">
        <dynmodel>typical</dynmodel>
        <pmax>4</pmax>
    </generator>

This defines the generator to have a typical dynamic model, the meaning of which will be detailed in the section on model parameters for specific models [TODO]Add link to section[/TODO]. It
also specifies a `pmax` value of 4 per unit.

Next, an event can be added to the load attached to `bus2` to change a parameter with the code shown below:

.. code:: XML

    <bus name="bus2">
        <load name="load2">
            <P>0.45</P>
            <Q>0.2</Q>
            <event>@1|p=1.1</event>
        </load>
    </bus>

The line `<event>@1|p=1.1</event>` defines an event such that at time 1.0 the `p` field of the load is set to 1.1, from the initial value of 0.45. More details
will be explained in the section on event specification [TODO]Add link to section[/TODO].

Finally, a block with a stop time and recorder can be added before the closing GridDyn tag:

.. code:: XML

    <stoptime>10</stoptime>
    <recorder period=0.5 field="auto">
        <file>twobusdynout.csv</file>
    </recorder>

This sets the simulation to run until a stoptime of 10 seconds. The recorder xml element defines a recorder to capture a set of automatic fields at a period of
0.05 seconds, and capture it to the file twobusdynout.csv upon completion of the scenario. More dtails on recorder specification are available later in this document [TODO]Add link to section[/TODO].

The final listing after these changes is:

.. code:: XML

    <?xml version="1.0" encoding="utf-8"?>
    <GridDyn name="2bus_test" version="1">
   
    <bus name="bus1">
        <type>SLK</type>
        <angle>0</angle>
        <voltage>1.05</voltage>
        <generator name="gen1">
            <dynmodel>typical</dynmodel>
            <pmax>4</pmax>
        </generator>
        <load name="load1">
            <P>1.05</P>
            <Q>0.31</Q>
        </load>
    </bus>
   
    <bus name="bus2">
        <load name="load2">
            <P>0.45</P>
            <Q>0.2</Q>
            <event>@1|p=1.1</event>
        </load>
    </bus>
    <link from="bus1" name="bus1_to_bus2" to="bus2">
        <b>0.127</b>
        <r>0.0839</r>
        <x>0.51833</x>
    </link>
   
    <stoptime>10</stoptime>
    <recorder period=0.5 field="auto">
        <file>twobusdynout.csv</file>
    </recorder>

    </GridDyn>

-----------------------
Parameter Specification
-----------------------

Simple parameters can be specified via elements or as attributes. Default units are in seconds for all times and time constants unless individual models
assume differently.  Power and impedance specifications are typically in PU values. Exceptions include `basepower` and `basevoltage` specifications which
are in `MW` and `KV` respectively. The default units on any rates are in units per second.  However, individual models are free to deviate from this
standard as makes sense for them so check with the individual model type specification for details.  Parameters in the XML can be specified in a number of
different forms that are useful in different contexts.  Below is an example showing the various methods.

.. code:: XML

    <?xml version="1.0" encoding="utf-8"?>
    <!--xml file to test parameter setting methods-->
    <GridDyn name="input_tests" version="0.0.1">
    <bus name="bus1">
        <load>
            <param name="P" value=0.4></param>
            <param field="q">0.3</param>
            <param field="ip" units="MW">55</param>
            <param>yq=0.11</param>
            <param name="iq(MW)" value=32/>
            <yp>0.5</yp>
        </load>

        <load yq=0.74 >
            <p units="puMW"> 0.31</p>
            <param>q(MW)=14.8</param>
            <param name="yp" unit="MW" value=127/>
        </load>
    </bus>
    </GridDyn>

The main variants involve varying how the units are placed. Units can be placed as an attribute named `unit` or `units` on the parameters
either in a `param` element or and element named after the model parameter. They can also be placed in parenthesis at the end of the parameter
name when the parameter name is a string contained in the elemental form. Values can be places in a value element, as the content of an
element, or following an equal sign when defined as a string like `<param>yq=0.11</param>`. Parameters assuming the default units are allowed
to be placed as attributes of the object.  

-------------------------------------
Functions and Mathematical Operations
-------------------------------------

GridDyn XML input allows mathematical operators and expressions in any parameter specification, including complex expressions. Supported
functions are shown in the tables that follow. In addition, most operators are supported including `+`, `-`, `*`, `/`, `^`, and `%`. Operator precedence is respected as are parenthesis.
String operations are not supported but the definition system has features that support some use cases for string operations.   


Zero argument mathematical expressions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. tabularcolumns:: |l|p{11cm}|

========   =======
function   details
========   =======
inf()      results in a large number between 0 and 1
nan()      uses nan("0")
pi()       pi
rand()     produces a uniform random number between 0 and 1
randn()    produces a normal random number with mean 0 and standard deviation of 1.0
randexp()  produces a random number from an exponential distribution with a mean of 1.0
randlogn() produces a random number from a log normal distribution
========== =======

One argument mathematical expressions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. tabularcolumns:: |l|p{11cm}|

========   =======
function   details
========   =======
sin(x)     sine of x
cos(x)     cosine of x
tan(x)     tangent of x
sinh(x)    hyperbolic sine of x
cosh(x)    hyperbolic cosine of x
tanh(x)    hyperbolic tangent of x
abs(x)     absolute value of x
sign(x)    return 1.0 if x>0 and -1.0 if x<0 and 0 if x==0
asin(x)    arcsin of x
acos(x)    arccosine of x
atan(x)    arctangent of x
sqrt(x)    the square root of x
cbrt(x)    the cube root of x
log(x)     the natural logarithm of x *log(exp(x))=x*
exp(x)     the exponential function :math:`e^x`
log10(x)   the base 10 logarithm of x
log2(x)    the base 2 logarithm of x
exp2(x)    evaluates :math:`2^x`
ceil(x)    the smallest integer value such that ceil(x)>=x
floor(x)   the largest integer value such that floor(x)<=x
round(x)   the nearest integer value to x
trunc(x)   the integer portion of x
none(x)    return x
dec(x)     the decimal portion of x *trunc(x)+dec(x)=x*
randexp(x) an exponential random variable with a mean of x
========== =======

Two argument mathematical expressions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. tabularcolumns:: |l|p{11cm}|

========      =======
function      details
========      =======
atan2(x,y)    the 4 quadrant arctangent function
pow(x,y)      evaluates :math:`x^y`
plus(x,y)     evaluates *x+y*
add(x,y)      evaluates *x+y*
minus(x,y)    evaluates *x-y*
subtract(x,y) evaluates *x-y*
mult(x,y)     evaluates *x\*y*
product(x,y)  evaluates *x\*y*
div(x,y)      evaluates *x/y*
max(x,y)      returns the greater of x or y
min(x,y)      returns the lesser of x or y
mod(x,y)      return the modulus of x and y e.g. mod(5,3)=2
hypot(x,y)    evaluates :math:`sqrt{x^2+y^2}`
rand(x,y)     return a random number between x and y
randn(x,y)    returns a random number from a normal distribution with mean x and variance y
randexp(x,y)  returns a random number from an exponential distribution with mean x and variance y
randlogn(x,y) returns a random number from a log normal distribution with mean x and variance y
randint(x,y)  returns a uniformly distributed random integer between x and y inclusive of x and y
============= =======

---------------------
Component Description
---------------------

Components are defined in elements matching the component name. For example

.. code:: XML

    <exciter name="ext1">
        <type>type1</type>
        <Aex>0</Aex>
        <Bex>0</Bex>
        <Ka>20</Ka>
        <Ke>1</Ke>
        <Kf>0.040</Kf>
        <Ta>0.200</Ta>
        <Te>0.700</Te>
        <Tf>1</Tf>
        <Urmax>50</Urmax>
        <Urmin>-50</Urmin>
    </exciter>

describes an exciter component as part of a generator. The name attribute or element is common for all objects. A `description` can also be defined
for all objects which is basically a string that can be added to any object. The `type` property is a keyword used to describe the detailed type of
the component. In the above example the specific type of the component is `type1`. GridDyn uses polymorphic objects for each of the components. The
type defined in the XML file for each component defines the specific object to instantiate. If type is not specified the default type of the component
is used.

Predefined components include:

area
    defines a region of the grid
bus
    the basic node of the system
link
    the basic object connecting buses together
relay
    primary object allowing control and triggers for other objects
sensor
    a form of a relay specifically targeted at sensing different parameters and allowing some direct signal processing on measurements before output
load
    the basic consumer of energy
generator
    the basic producer of electricity
genmodel
    the dynamic model of an electrical generator
governor
    a generator governor
exciter
    an exciter for a generator
pss
    a power system stabilizer*
controlblock
    a basic control block
source
    a signal generator in GridDyn
simulation
    a simulation object
agc
    describes an automatic generation control*
reserveddispatcher
    describes a reserve dispatcher*
scheduler
    a scheduling controller*

* these objects are in development and do not work consistently

Several components are also defined that map onto the more general components, in some cases these define specific types, in others they are
simply maps. Examples include `fuse=>relay`, `breaker=>relay`, `transformer=>line`, `block=>controlblock`, `control=>relay`, and `tie=>line`.
Custom definitions can also be defined if desired through a `translate` element.

---------------------
Object Identification
---------------------

There are many instances where it is necessary to identify an object for purposes of creating links or to extract a property from another.
Internally there is a hierarchy of objects starting with the root simulation object. This allows a path like specification of the objects.
There are 2 different notations for describing an object path, one based on colons, the other more similar to the URI specification for WEB
like services, and both allow properties to be specified in a similar fashion. Objects can be referred to by 4 distinct patterns. The first
is by the name of the object. The name should be unique within any given parent object. The second uses an object component name and index
number, for example `bus\#0` would refer to the bus in index location 0;  using `bus!0` will also work the same as `bus\#0` but can be used
in settings where the '\#' is not allowed. All indices are 0 based. The fourth makes use of the user id of an object, to use this objects
would be identified by `load\$2` to locate the load with userID of 2.

When searching for an object the system starts in the current directory for the search. If it is not found it traverses to the root object
and starts the search from there.

An example of a path specified with the ':' notation is `area45::bus#6::load#0:p`. The single colon marks that the final string represents
a property. The same object in the URI notation would be `area45/bus#6/load#0?p`.

In some cases mixed notation might work but it is not recommended. The property indication can be left off when referencing an entire object.
Starting the object identification string with an '@' or a beginning '/' implies searching starting in the root object, otherwise the search
starts at whatever the current object of interest is.

----------------
Special Elements
----------------

In addition to the components elements, several element names have special purposes.

translate
^^^^^^^^^

The translate element is used to create a custom object definition.

.. code:: XML

    <translate name="special" component="exciter" type="type1"/> 

Putting this command in the XML file will allow objects using "special" as the element name instead of specifying "exciter" in the element name
and a specific type. Translations, unlike definitions, are global and are only allowed in the root element of an XML file. IF you wish to specify
a default type for a component or other defined component translation then the name or component can be left out.

define
^^^^^^

The GridDyn XML file allows specification of definition strings that can be used as parameter values or in other definitions.

.. code:: XML

    <define name="constant1" value=5/>
    <define name="constant2" value=46 locked=1/>
    <define name="constant3 value="constant1*constant2" eval=1/>

The above snippet of code defines 3 constants. Internally constants are stored as strings. If the `eval` attribute is specified the value string
is evaluated before storing it as a string, otherwise it will be stored as a string and evaluated on use. The `locked` attribute defines a global
parameter that cannot be overridden by another `define` command. The mechanisms allow programmatic or command line overrides of any internal
definitions in the XML file. Inside the XML file they cross scope boundaries like a global variable. Regular definitions are only valid in elements
they were defined in and subelements. So if a definition is used, define it in the root scope or it will only be applicable in a subsection of the XML.  
 
When using definitions they can be used as a variable in other languages wherever a string or numerical value would be used. They can also be used
in string replacements like the following code.

.. code:: XML

    <bus name="bus_$#rowindex$_$#colindex$">

When evaluating the expression, the parts of the string between the \$ signs gets evaluated first. In this case "\#rowindex" and "\#colindex" are
part of an array structure which is described in the subsection on arrays.[TODO]Add ref to array subheading[/TODO]
 
After the substring replacement, the entire expression is evaluated again for other definitions. There is a small set of predefined definitions
including \%date, \%datetime, and \%time, which contain strings of the expected values at the time of file input.

custom
^^^^^^

Translations are useful for readability, library elements allow duplication of objects with only minor modifications. For library elements the
object is constructed once from XML and duplicated. Custom objects allow duplication of objects or sets of objects from the XML. A reference to
the actual element source is stored and reprocessed at the time the custom object is encountered. This also allows a set of object to be defined
in one input form to be imported by a different form and used to create objects described in the first. For instance you could create a library
of different object sections and import that into another XML file and only use a few of the custom definitions you are interested in. Custom
objects can use other custom objects but cannot define new custom sections. Custom objects can define a set of required arguments and default values. 
When calling the custom element arguments can be defined. 

A brief example using custom elements is shown below.

.. literalinclude:: ../test/test_files/xml_tests/test_custom_element1.xml
    :language: XML

Another example using arguments is shown below.

.. literalinclude:: ../test/test_files/xml_tests/test_custom_element2.xml
    :language: XML

configuration
^^^^^^^^^^^^^

A `configuration` element can define some parameters and operations for the XML reader itself. There are currently 2 parameters that can be specified,
`printlevel` and `match\_type`. The `printlevel` controls the verbosity of the output. The `match\_type` controls the default match capitalization for
searching for specified fields. The following table shows valid values for these fields.

+-------------+----------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| parameter   | value                | details                                                                                                                                                                                |
+=============+======================+========================================================================================================================================================================================+
| printlevel  | none(0)              | print only warnings and errors                                                                                                                                                         |
|             | summary(1)           | print only summary info                                                                                                                                                                |
|             | detailed(2)          | print detailed information                                                                                                                                                             |
+-------------+----------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| match\_type | capital\_case\_match | match on specified, lower case, and upper case values (this is the default value)                                                                                                      |
|             | all                  | match to all cases [lex]                                                                                                                                                               |
|             | exact                | only match to the given string. Ths operation can be used to process files where every component and XML description field is in lower case in the xml to speed up processing slightly |
+-------------+----------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

event
^^^^^

Events are changes that take place during the course of the GridDyn simulation execution. They can be as simple as the example used previously or contain
more complex specifications and multiple times and values. The events are likely to be updated significantly in the near future, and while much of the
specification will remain the same some new capabilities will be added.  

Parameters for events are specified like those in the components. The available element or attribute names are shown in this table.

=========  ==============================================
parameter  description
=========  ==============================================
target     the object to extract data from
field      the field of the target object to capture
time, t    the time the event is scheduled to occur
units      the desired units of the output
value      the value associated with the event
period     for periodic events sets the period
file       the input file for a player type event
column     the column in a data file to use for the event
=========  ==============================================

The `field` option of event specification is by far the most flexible. Any text directly in the `event` element is captured as the field.

recorder
^^^^^^^^

Recorders are the primary data output system for GridDyn dynamic simulations. Like events, recorders have a set of parameters associated with them.
The details are in the table below. Multiple recorder elements can be specified and the recorder for a single file can have multiple elements that
get merged even if they are in different objects. They are keyed by recorder name and/or filename. Certain properties like the sampling period are
specified on a recorder basis. Others are for the properties and data to record.  

=================  ==============  ===========================================================================
parameter          default         description
=================  ==============  ===========================================================================
file               outputfile.csv  the file to save the data to
name               recorder\_\#    the name of the recorder for easy reference later
description                        description that gets put in the header of the output file
column             -1              the column of the recorder in which to place the requested data
target                             the object to extract data from
field                              the field of the target object to capture
units              defUnit         the desired units of the output
offset             0               an offset index for a particular state
gain               1.0             a multiplier on the measurement
bias               0.0             a measurement bias
precision          7               the number of digits of precision to print for string formats
frequency          1.0             set the frequency of recording
period             1.0             set the measurement period
starttime          -inf            set a start time for the recorder
stoptime           inf             set a stop time for the recorder
autosave           0               set the recorder to save every N samples; 0 for off
reserve            0               reserve space for N samples
period_resolution  0               set the minimum resolution for any time period (not usually user specified)
=================  ==============  ===========================================================================

Recorder fields define which property of an object to capture. This includes all properties and calculations involving properties. All functions
and expressions defined in the func section [TODO]Add ref to func section[/TODO] are valid in recorder expressions.

solver
^^^^^^

Solvers can be defined through the XML file. There are some default solvers defined but the `solver` element allows the definition of custom solvers
applied to specific problem types. This allows specification of specific approximations or other configuration options for the solvers to use for
solving various specific problems. Solver properties are shown in the table below.

==========  ==========  ====================================================================================================================
parameter   default     description
==========  ==========  ====================================================================================================================
printlevel  error(1)    may be specified with a string or number, "debug"(2), "error"(1), "none"(0)", "errno" only prints out error messages
approx      "none"      see the table below for details on possible options [TODO]Add ref to the table on solver options[/TODO]
flags                   see the table for details on possible options [TODO]Add ref to table on flags[/TODO]
tolerance   1e-8        the residual tolerance to use
name        solver\_\#  the name of the solver
index       automatic   the specified index of the solver
file                    log file for the solver
==========  ==========  ====================================================================================================================

Solvers have a set of options used to define what types of problems they are intended to solve. And another set of intended approximations. This
information gets passed to the models whenever a solve is attempted. A listing of the possible modes is shown in the solver modes table below. In
some cases multiple modes can be combined, in other cases they are mutually exclusive and the second will override the earlier specification. A
number of approximations are also specified mainly targeting approximations to transmission lines. These approximations are suggestions rather
than directives and models are free to ignore them. There are 3 independent approximations that can be used in various combinations and several
descriptions which turn on the simplifications in a convenient form. Most approximations target the acline models, but future approximations can
be added specifically looking at other models. 

======================  ======================================================================================================
mode/approximation      description
======================  ======================================================================================================
local                   used for local solutions
dae                     solver is intended for solving a set of coupled differential algebraic equations
differential            solver is intended to solve a set of coupled differential equations
algebraic               solver is intended to solve algebraic equations only
dynamic                 solver is intended for dynamic simulations
powerflow               solver is intended for powerflow problems (implies !dynamic and algebraic)
extended                instructs the model to use an extended state formulation mainly targetted at state estimation problems
primary                 opposite of extend
ac                      solve for both voltage and angle on the buses
dc                      solve only for the angle on AC buses, assume the voltage is fixed
r, small_r              assume the resistance of transmission lines is small
small_angle             use the small angle approximation (assume :math:`sin(\theta)=\theta`)
coupling                assume there is no coupling between the :math:`V` and :math:`\theta` states
normal                  use full detailed calculations
simple, simplified      use the small_r approximation
decoupled               use the coupling approximation
small_angle_decoupled   use the small angle and decoupled approximations
small_angle_simplified  use the small angle and small r approximations
simplified_decoupled    use the small r and decoupling approximations
fast_decoupled          use all 3 approximations
linear                  linear
======================  ======================================================================================================

Solvers can also include a number of options that are common across all solvers (though specific solvers may not implement them). Often
specific solvers also include other options specific to that numerical solver.

=================  ===========================================================================================================================================================================================================
mode               description
=================  ===========================================================================================================================================================================================================
dense, sparse      set the solver to use a dense matrix solve or a sparse matrix (default)
parallel, serial   set the solver to use parallel or serial (default) arrays, this is in the form of openMP array
constant_Jacobian  tell the solver to assume the Jacobian is constant
mask               tell the solver to use a masking element to shield specific variables from the solution. This functionality is used in some cases of initial condition generation and probably shouldn't be used externally
=================  ===========================================================================================================================================================================================================

import
^^^^^^

Import statements are used to add an external file into the simulation. The file can be of a type capable of being read by GridDyn. Import
statements are typically single element statements though they can have subelements if desired. A couple examples are shown in this example.

.. code:: XML

    <import>sep_lib.xml</import>
    <import prefix="A1">subnetwork.csv</import>
    <import final=true ext="xml">last_elements.odx</import>

The optional attributes/elements are descripted in the table below.

+-----------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| parameter | valid values                | description                                                                                                                                                                                                                                                                                        |
+===========+=============================+====================================================================================================================================================================================================================================================================================================+
| prefix    | string                      | a string to prefix all object names from the imported file                                                                                                                                                                                                                                         |
+-----------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| final     | "true(1)"                   | if set to true the import is delayed until after all other non-final imports and the local file have been loaded                                                                                                                                                                                   |
|           | "false(0)"                  | if set to false or not included the import is processed before any locally defined objects and in the order imports are specified                                                                                                                                                                  |
+-----------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| file      | string                      | the file name, can also be interpreted from the element text                                                                                                                                                                                                                                       |
+-----------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| filetype  | string                      | the extension to use for interpreting the import file; if not specified the extension is determined from the file name                                                                                                                                                                             |
+-----------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| flags     | ignore_step_up_transformers | the flags option is to add in additional options, it will likely be expanded as needed, currently the only option available is to ignore step up transformers in some formats of model input. As the file readers improve and become more integrated and consistent more options will be available |
+-----------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

directory
^^^^^^^^^

The directory element allows the user to specify additional search paths for GridDyn to locate any files without an absolute path.

.. code:: XML

    <directory>/home/usr/user1/GridDyn</directory>
    <directory>

library
^^^^^^^

GridDyn file input can include a library of predefined objects. This section is defined through a library element. Any of the components described
above can be included as a library element. These library objects get stored in a separate holding area and are copied when any object uses a `ref`
fields with a value of the library element name. The `ref` field can be either an element or an attribute. If `type` and `ref` are specified the type
definition takes priority and the library object is cloned to the newly created object, if only `ref` is specified a new object is cloned directly
from the library object. There can be multiple library sections, they simply get merged. By using import statements libraries can be defined in a
separate file. A simple example using libraries and references is shown below. The code describes 4 objects and generator model, an exciter and a
governor, and a generator that uses the 3 previously defined submodels to make up the dynamic components of the generator.

.. code:: XML

    <library>
        <model name="mod1">
            <type>fourthOrder</type>
            <D>0.040</D>
            <H>5</H>
            <Tdop>8</Tdop>
            <Tqop>1</Tqop>
            <Xd>1.050</Xd>
            <Xdp>0.350</Xdp>
            <Xq>0.850</Xq>
            <Xqp>0.350</Xqp>
        </model>
        <exciter name="ext1">
            <type>type1</type>
            <Aex>0</Aex>
            <Bex>0</Bex>
            <Ka>20</Ka>
            <Ke>1</Ke>
            <Kf>0.040</Kf>
            <Ta>0.200</Ta>
            <Te>0.700</Te>
            <Tf>1</Tf>
            <Urmax>50</Urmax>
            <Urmin>-50</Urmin>
        </exciter>
        <governor name="gov1">
            <type>basic</type>
            <K>16.667</K>
            <T1>0.100</T1>
            <T2>0.150</T2>
            <T3>0.050</T3>
        </governor>
        <generator name="gen1">
            <model ref="mod1"/>
            <exciter ref="ext1"/>
            <governor ref="gov1"/>
        </generator>
    </library>

Libraries are only allowed to be defined at the root object level, they are not allowed in any element that is a part of the root element
so they are directly processed by the interpreter. 

array
^^^^^

Arrays and the `if` statement make up the control structures in the XML file. Arrays allow objects and sets of objects to be generated in a loop,
they can even contain other loops. An example file used for building some scalability tests is shown below. This file uses many of the concepts
discussed previously.  

.. literalinclude:: ../test/test_files/performance_tests/block_grid2.xml
    :language: XML

Arrays can have several attributes which define how the array is handled.

============  =======  =============================================================
attribute     default  description
============  =======  =============================================================
start         1        the index to start the array counter
stop          X        the last index to use, either stop or count must be specified
count         X        the number of loops, either stop or count must be specified
loopvariable  \#index  the name of the definitions to store the loop variable
interval      1.0      the interval between each iteration of the loop counter
============  =======  =============================================================

if
^^

If elements create a conditional inclusion. Most often used for conditional inclusion based on fixed parameters to allow a single file to do a few
different scenarios. However, they can be tied in with random function generators and arrays to generate random distributions of elements. Any element
component along with `import` and `define` statements are allowed in an `if` element. 

The `if` element must have an element or attribute named `condition`. The condition is a string specifying a value or two values and a comparison operator. 
If a single expression is given, the elements in the `if` statement are evaluated as long as the expression does not result in a 0. Otherwise both sides of
the expression are evaluated and the comparison is checked. If both sides evaluate to strings, a string comparison is done, otherwise a numerical comparison
if both sides result in numerical values. Depending on the file type and reader '>' and '<' may need to be replaced with the XML character codes of *\&gt;* and
*\&lt;*. These codes are interpreted properly. Compound expressions are not yet supported. Eventually the goal will be to support conditions based on object
values instead of values that can be evaluated in the element reader itself, but this capability is not yet allowed.

econ
^^^^

The `econ` element describes data related to the costs and values of an object. It will be used for interaction with optimization solvers and the root object
must be an optimization type simulation. While the element works fine, it doesn't do anything with the data.

position
^^^^^^^^

A `position` element describes data related to the geophysical (or relative) position of an object. The element is ignored but will be further developed at a later time.  

actions
^^^^^^^

The gridDynSimulation object can execute a number of types of actions. These can be controlled through the API but also through an action queue. 
The actions are defined and stored in a queue and executed when the run function is called. If no actions are defined some logic is in place to
do something sensible, typically run a power flow then a dynamic simulation if dynamic components were instantiated. Actions allow a much finer
grained control over this process. These actions can be loaded through the XML file and eventually in a type of script (not enabled yet). Actions
are specified through an `action` element containing the action string. The string is translated into an action and stored in a queue.  

.. code:: XML

    <action>run 23.7</action>

The list of available commands is shown in the table below. For all lines in the table `(s)` implies string parameter, `(d)` implies double parameter,
`(i)` integer parameter, `(X)\*` optional, `(s|d|i)` string or doulbe or int, and for a given line everything following a \# at the beginning of a word
is considered a comment and ignored.

=============================================  ====================================================================
action string                                  description
=============================================  ====================================================================
ignore XXXXXX                                  do nothing
set parameter(s) value(d)                      set a particular parameter; the parameter can include an object path
setall objecttype(s) parameter(s) value(d)     set a parameter on all objects of a particular type
setsolver mode(s) solver(s|i)                  set the solver to use for a particular mode of operation
settime newtime(d)                             set the simulation time
print parameter(s) setstring(s)                print a parameter (can include path)
powerflowstep solutionType(s)\*                take a single step of the specified solution type
eventmode stop(d)* step(d)\*                   run in event driven power flow mode until stop with step
initialize                                     run the initialization routine
dynamic solutionType(s)\* stop(d)\* step(d)\*  run a dynamic simulation
dynamicdae stop(d)\*                           run a dynamic simulation using DAE solver
dynamicpart stop(d)* step(d)\*                 run a dynamic simulation using the partitioned solver
dynamicdecoupled stop(d)\* step(d)\*           run a dynamic simulation using the decoupled solver
reset level(i)                                 reset the simulation to the specified level
iterate interval(d)\* stop(d)\*                run an iterative power flow with the given interval
run time(d)\*                                  run the simulation using the default mode to the given time
save subject(s) file(s)                        save a particular type of file
load subject(s) file(s)                        load a particular type of file
add addstring(s)                               add something to the simulation
rollback point(s|d)                            rollback to a saved checkpoint (not implemented yet)
checkpoint name(s)                             save a named checkpoint (not implemented yet)
contingency ????                               run a contingency analysis (not implemented yet)
continuation ????                              run a continuation analysis (not implemented yet)
=============================================  ====================================================================

