
.. _reference_settable_object_properties:

==========================
Settable Object Properties
==========================

The tables here describe the parameters for each of the models present in GridDyn as of Version 0.5. The tables are automatically
generated via scripts so there are a few bugs and some missing information as of yet. Each table has 4 columns. The first column
specifies the string or strings that can be used to set this property, multiple strings that do the same thing are separated by a
comma. The second columns defines the type of parameter, number implies a numeric value, string implies a string field, and flag is
a flag or boolean variable which can be set to true with "true", or any number greater than 0.1 (typically 1), and set to false for
any number less than 0.1 or "false". The third column lists the default value if applicable and the fourth column is a description.
In many cases the default units will be described in [] at the beginning of the description, the default units are the units of the
default and the unit that is assumed if no units are given to the set command. All the set functions cascade to parent classes which
are identified in the table captions.

..
    .. _reference_gridCoreObject_setprops:

    .. csv-table:: gridCoreObject
        :file: inputTables/gridCoreObjects_setProps.csv
        :header: "string(s)", "type", "default", "description"
        :widths: "auto"
        
        "updateperiod, period", "number", "1e+48", "the update period"
        "updaterate, rate", "number", "0", ""
        "nextupdatetime", "number", "1e+48", "the next scheduled update"