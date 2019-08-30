###########
Style Guide
###########

Naming Styles
=============

Classes
-------

Camel case names starting with a Capital letter

e.g. ``GridDynClass``

Class Methods
-------------

Camel case names starting with a lower case letter

e.g. ``gridDynMethod``

Class Static Members
--------------------

Camel case names starting with lower case and preceded by a ``s_``

e.g. ``s_staticMember``

Class Members
-------------

Camel case names starting with lower case and preceded by a ``m_``

e.g. ``m_classMember``

Model Parameters
^^^^^^^^^^^^^^^^

A subset of class members specifically referring to settable model parameters.

Engineering reference model parameters are preceded by ``mp_``, ``K`` is used for gains, ``T`` for time constants, ``R`` for resistances, ``X`` for impedances, and others are used as appropriate, typically using a capital letter first followed by a number of other lower case letters.

e.g. ``mp_K1``, ``mp_T3``, ``mp_Rs``

Pointers
^^^^^^^^

Camel case starting with a lower case and preceded by ``p_``

e.g. ``p_classMemberPointer``

Function Names
--------------

Camel case starting with lower case

e.g. ``functionName``

Function Arguments
------------------

Camel case starting with lower case

e.g. ``functionName(type functionArgument1, type2 functionArgument2)``

Enumeration Names
-----------------

Lower case separated by ``_`` and followed by ``_t``

e.g. ``enumeration_name_t``

Enumeration Fields
------------------

Lower case separated by ``_``

e.g. ``enumeration_field``

Global Constants
----------------

Capital letters preceded by a lower case ``k``

e.g. ``kCONSTANT``

Macros
------

Capital letters with words separated by ``_``

e.g. ``MY_MACRO``
