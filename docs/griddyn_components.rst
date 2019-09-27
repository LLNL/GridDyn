
==================
GridDyn Components
==================

Components in GridDyn are divided into three categories: `primary`, `secondary`, and `submodel`. Primary components include `buses`,
`links`, `relays`, and `areas` and define the basic building blocks for power grid simulation. Secondary components are those which
tie into busses and consume or produce real and reactive power. The two component types in the secondary category are `loads` and
`generators`. Submodels are any other component in the system and can form the building blocks of other components. A majority of
the differential equations in the dynamic simulations are found in submodels. Submodels include things such as `exciters`, `governors`,
`generator models`, `control systems`, `sources`, as well as several others. There are a few other types of objects in GridDyn, but
they generally are used for specific purposes and do not take part in the equations unless interfaced through another object. The
component types currently available in GridDyn are detailed in another section.

The :ref:`basics_development_notes` section has information on the current development status of various components.

-----
Buses
-----

Buses form the nodes of a power system. They act as containers for secondary objects and attach to links. The default bus type is an
AC bus which in typical operation would have 2 states (voltage and angle). 4 types of bus operation are available, `PQ`, `PV`, `slack`,
and `fixed angle`. The practical value of fixed angle buses is unknown but was included for mathematical completeness and describes a
bus whose angle and reactive power are known.

The residual equation used in the bus model take one of two forms

.. math::

   f_v(X)=\sum_{i=0}^{gens} Qgen_i(V,\theta,f) + \sum_{i=0}^{loads} Qload_i(V,\theta,f) + \sum_{i=0}^{lines} Qline_i(V,\theta,f)

for PQ and afix type buses and

.. math::

   f_v(X)=V - V_{target}

for PV and SLK type buses. The equations for :math:`\theta` are very similar

.. math::

   f_{\theta}(X)=\sum_{i=0}^{gens} Pgen_i(V,\theta,f) + \sum_{i=0}^{loads} Pload_i(V,\theta,f) + \sum_{i=0}^{lines} Pline_i(V,\theta,f)

for PQ and PV type buses and

.. math::

   f_{\theta}(X)=\theta - \theta_{target}

for fixed angle and SLK type buses.

The frequency can be either extracted from an active generator attached to the bus or computed as a filtered derivative of the angle.
If it is computed the bus has an additional state as part of the dynamic calculations.

The bus model implemented in GridDyn also includes some ability to merge buses together to operate in node-breaker type configurations.
At present this is not well tested.

-----
Areas
-----

Areas define regions on the simulated grid. An area can contain other areas, buses, links, and relays. It principally acts as a container
for the other objects, though will eventually include controls such as AGC and other wide area controls. The simulation object itself is a
specialization of an area.

-----
Links
-----

In the most general form links connect buses together. As a primary object it can contain other objects, including state information. The
basic formulation is that of a standard AC transmission line model connecting two buses together. The code includes a number of possible
approximations.

------
Relays
------

Relays are perhaps the most interesting and unusual primary object included in GridDyn. The basic concept is that relays can take in
information from one object and act upon another. They add protection and control systems into the simulation environment. They exist
as primary objects since they can stand to operate on their own at the same level as buses and areas. They may contain states, other
objects, submodels, etc. They also act as gateways into communication simulations, functioning as measurement units and control
relays. And through relays a whole host of control and protection schemes can be implemented in simulation alongside normal power flow
and dynamic simulations. Examples of relays include fuses, breakers, differential relays, distance relays, and control relays, among
others.