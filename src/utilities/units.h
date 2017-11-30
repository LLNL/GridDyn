/*
* LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#ifndef GRIDDYN_UNITS_
#define GRIDDYN_UNITS_

#include <string>

namespace gridUnits
{
const double badConversion = -1e48;
const double PI = 3.141592653589793;
/** @brief enumeration of common units in the power grid */
enum units_t
{
    defUnit = 0,
    // Power system units
    MW = 1,
    kW = 2,
    Watt = 3,
    MVAR = 4,
    Volt = 5,
    kV = 6,
    Ohm = 7,
    Amp = 8,
    MWps = 9,  //!< MW/s
    MWpmin = 10,  //!<MW/min
    MWph = 11,  //!<MW/hr


    // rotational speed
    Hz = 51,
    rps = 52,  // radians per second
    rpm = 53,  // revolutions per minute

    // Per unit units
    pu = 100,
    puMW = 101,
    puV = 105,
    puOhm = 107,
    puA = 108,
    puMWps = 109,  //!< per unit MW/s
    puMWpmin = 110,  //!<per unit MW/min
    puMWph = 111,  //!<per unit MW/hr
    puHz = 151,


    // distance Units
    meter = 201,
    km = 202,
    mile = 203,
    ft = 204,

    // angle units
    deg = 301,
    rad = 302,


    // time units,
    sec = 401,
    min = 402,
    hour = 403,
    day = 404,
    week = 405,

    // cost units
    cost = 800,
    Cph = 801,  //!< cost per hour
    CpMWh = 802,  //!< cost per MWh
    CpMW2h = 803,  //!< cost per MWh^2
    CppuMWh = 804,  //!< cost per puMWh
    CppuMW2h = 805,  //!< cost per puMWh^2
    CpMVARh = 806,  //!< cost per MVARh
    CpMVAR2h = 807,  //!< cost per MVARh^2
    CppuMVARh = 808,  //!< cost per puMVARh
    CppuMVAR2h = 809,  //!< cost per puMVARh^2

    // temperature units
    F = 1001,
    C = 1002,
    K = 1003,
};

/** @brief enumeration of unit types related to the units*/
enum units_type_t
{
    distance,
    time,
    rotation,
    speed,
    angle,
    price,
    electrical,
    temperature,
    deftype,
};

/** @brief convert a units_t into a string
 should work in a cycle with getUnits function
@param[in] unitType  the unit to convert to a string
@return a string representing the units*/
std::string to_string (units_t unitType);

/** @brief convert a string into a units_t
 should work in a cycle with to_string function
@param[in] unitString  the string containing a representation of the units
@param[in] defValue the default unit to use if the string conversion doesn't succeed
@return a unit*/
units_t getUnits (const std::string &unitString, units_t defValue = defUnit);

/** @brief convert a number in one unit to another unit
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@param[in] basePower  the basePower when converting from pu values
@param[in] localBaseVoltage  the base Voltage to use when converting to and from pu values
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double
unitConversion (double val, const units_t in, const units_t out, double basePower = 100, double localBaseVoltage = 100);

/** @brief convert between units of Time
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionTime (double val, const units_t in, const units_t out);

/** @brief convert between units used in power systems related to power, current, voltage, and resistance.
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@param[in] basePower  the basePower when converting from pu values
@param[in] localBaseVoltage  the base Voltage to use when converting to and from pu values
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/

double unitConversionPower (double val,
                            const units_t in,
                            const units_t out,
                            double basePower = 100,
                            double localBaseVoltage = 100);

/** @brief convert a number between units of distance
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionDistance (double val, const units_t in, const units_t out);
/** @brief convert a number between units of angle such as radians, deg
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionAngle (double val, const units_t in, const units_t out);

/** @brief convert a number between units of frequency
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@param[in] baseFreq  the base frequency to use when pu is in question
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionFreq (double val, const units_t in, const units_t out, double baseFreq = 60);
/** @brief convert a number between units of cost
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@param[in] basePower  the base power in MVA to use when converting between units involving pu notation
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionCost (double val, const units_t in, const units_t out, double basePower = 100);
/** @brief convert a number between units of Temperature
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionTemperature (double val, const units_t in, const units_t out);
}//namespace gridUnits
#endif
