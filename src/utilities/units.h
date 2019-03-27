/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */
#pragma once

#include <units/units.hpp>
#include <string>

namespace gridUnits
{
const double badConversion = -1e48;
const double PI = 3.141592653589793;
/** @brief enumeration of common units in the power grid */

using units_t = units::precise_unit;

// pre-declared units
constexpr units_t Amp = units::precise::Ampere;
constexpr units_t C = units::precise::celsius;
constexpr units_t cost = units::precise::currency;
constexpr units_t defUnit = units::precise::defunit;
constexpr units_t deg = units::precise::deg;
constexpr units_t day = units::precise::day;
constexpr units_t F = units::precise::F;
constexpr units_t ft = units::precise::ft;
constexpr units_t hour = units::precise::hr;
constexpr units_t Hz = units::precise::Hz;
constexpr units_t K = units::precise::Kelvin;
constexpr units_t km = units::precise::km;
constexpr units_t kV = units::precise::kV;
constexpr units_t kW = units::precise::kW;
constexpr units_t meter = units::precise::meter;
constexpr units_t mile = units::precise::mile;
constexpr units_t min = units::precise::min;
constexpr units_t MVAR = units::precise::MW;
constexpr units_t MW = units::precise::MW;
constexpr units_t Ohm = units::precise::ohm;
constexpr units_t pu = units::precise::pu;
constexpr units_t puA = units::precise::puA;
constexpr units_t puHz = units::precise::puHz;
constexpr units_t puOhm = units::precise::puOhm;
constexpr units_t puV = units::precise::puV;
constexpr units_t puMW = units::precise::puMW;
constexpr units_t rad = units::precise::rad;
constexpr units_t rpm = units::precise::rpm;
constexpr units_t sec = units::precise::second;
constexpr units_t Volt = units::precise::volt;
constexpr units_t Watt = units::precise::watt;
constexpr units_t week = units::precise::week;

// Custom units
constexpr units_t puMVAR = MVAR * pu;

constexpr units_t Cph = cost / hour;
constexpr units_t CpMVARh = cost / MVAR / hour;
constexpr units_t CpMVAR2h = cost / MVAR / hour / hour;
constexpr units_t CpMWh = cost / MW / hour;
constexpr units_t CpMW2h = cost / MW / hour / hour;
constexpr units_t CppuMVARh = cost / puMVAR / hour;
constexpr units_t CppuMVAR2h = cost / puMVAR / hour / hour;
constexpr units_t CppuMWh = cost / puMW / hour;
constexpr units_t CppuMW2h = cost / puMW / hour / hour;
constexpr units_t MWph = MW / hour;
constexpr units_t MWpmin = MW / min;
constexpr units_t MWps = MW / sec;
constexpr units_t puMWph = puMW / hour;
constexpr units_t puMWpmin = puMW / min;
constexpr units_t puMWps = puMW / sec;
constexpr units_t rps = rad / sec;

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
 * should work in a cycle with getUnits function
 * @param[in] unitType  the unit to convert to a string
 * @return a string representing the units*/
std::string to_string (units_t unitType);

/** @brief convert a string into a units_t
 * should work in a cycle with to_string function
 * @param[in] unitString  the string containing a representation of the units
 * @param[in] defValue the default unit to use if the string conversion doesn't succeed
 * @return a unit
 */
units_t getUnits (const std::string &unitString, units_t defValue = defUnit);

/** @brief convert a number in one unit to another unit
 * @param[in] val  the value of the property in input units
 * @param[in] in the units of val
 * @param[in] out the units of the desired result
 * @param[in] basePower  the basePower when converting from pu values
 * @param[in] localBaseVoltage  the base Voltage to use when converting to and from pu values
 * @return the numerical value of the property in output units,  badConversion if unable to convert between the specified units
 */
double unitConversion (double val,
                       const units_t in,
                       const units_t out,
                       double basePower = 100,
                       double localBaseVoltage = 100);

/** @brief convert between units of Time
 * @param[in] val  the value of the property in input units
 * @param[in] in the units of val
 * @param[in] out the units of the desired result
 * @return the numerical value of the property in output units,  badConversion if unable to convert between the specified units
 */
double unitConversionTime (double val, units_t in, units_t out);

/** @brief convert between units used in power systems related to power, current, voltage, and resistance.
 * @param[in] val  the value of the property in input units
 * @param[in] in the units of val
 * @param[in] out the units of the desired result
 * @param[in] basePower  the basePower when converting from pu values
 * @param[in] localBaseVoltage  the base Voltage to use when converting to and from pu values
 * @return the numerical value of the property in output units,  badConversion if unable to convert between the
 * specified units
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
double unitConversionDistance (double val, units_t in, units_t out);
/** @brief convert a number between units of angle such as radians, deg
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionAngle (double val, units_t in, units_t out);

/** @brief convert a number between units of frequency
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@param[in] baseFreq  the base frequency to use when pu is in question
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionFreq (double val, units_t in, units_t out, double baseFreq = 60);
/** @brief convert a number between units of cost
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@param[in] basePower  the base power in MVA to use when converting between units involving pu notation
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionCost (double val, units_t in, units_t out, double basePower = 100);
/** @brief convert a number between units of Temperature
@param[in] val  the value of the property in input units
@param[in] in the units of val
@param[in] out the units of the desired result
@return the numerical value of the property in output units,  badConversion if unable to convert between the
specified units
*/
double unitConversionTemperature (double val, units_t in, units_t out);

}  // namespace gridUnits
