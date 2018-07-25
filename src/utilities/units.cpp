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

#include "units.h"

#include "stringOps.h"

#include "mapOps.hpp"
#include <utility>

// disable a funny warning (bug in visual studio 2015)
#ifdef _MSC_VER
#if _MSC_VER >= 1900
#pragma warning(disable : 4592)
#endif
#endif
/*
defUnit = 0,
// Power system units
MW		= 1,
MVAR	= 2,
V		= 3,
kW		= 4,
kV		= 6,
Ohms	= 7,
Hz		= 8,
A		= 9,

// Per unit units
puMW	= 101,
puV		= 103,
puHz	= 108,
puOhm	= 107,
puA		= 109,

// distance Units
meter	= 201,
km		= 202,
mile	= 203,
foot	= 204

// angle units
deg		= 301,
rad		= 302,

// time units,
sec		= 401,
min		= 402,
hour	= 403,
day		= 404,
week	= 405


*/

namespace gridUnits
{

static std::map<std::string, units_t> name2Unit{
  {"mw", MW},
  {"mws", MW},
  {"mvar", MVAR},
  {"mva", MVAR},
  {"mvars", MVAR},
  {"amp", Amp},
  {"a", Amp},
  {"amps", Amp},
  {"v", Volt},
  {"volt", Volt},
  {"volts", Volt},
  {"kv", kV},
  {"kw", kW},
  {"w", Watt},
  {"watt", Watt},
  {"watts", Watt},
  {"ohm", Ohm},
  {"ohms", Ohm},
  {"hz", Hz},
  {"1/s", Hz},
  {"cps", Hz},
  {"rad/s", rps},
  {"rps", rps},
  {"rpm", rpm},
  {"rev/min", rpm},
  {"pumw", puMW},
  {"mw/s", MWps},
  {"mw/min", MWpmin},
  {"mw/hr", MWph},
  {"pumw/s", puMWps},
  {"pumw/min", puMWpmin},
  {"pumw/hr", puMWph},
  {"mw/s", MWps},
  {"puhz", puHz},
  {"pu", pu},
  {"puohm", puOhm},
  {"pua", puA},
  {"m", meter},
  {"meter", meter},
  {"meters", meter},
  {"mi", mile},
  {"mile", mile},
  {"miles", mile},
  {"ft", ft},
  {"feet", ft},
  {"foot", ft},
  {"km", km},
  {"kilometer", km},
  {"kilometers", km},
  {"deg", deg},
  {"degree", deg},
  {"degrees", deg},
  {"rad", rad},
  {"radian", rad},
  {"radians", rad},
  {"s", sec},
  {"sec", sec},
  {"second", sec},
  {"seconds", sec},
  {"min", min},
  {"minute", min},
  {"minutes", min},
  {"hr", hour},
  {"hour", hour},
  {"hours", hour},
  {"day", day},
  {"days", day},
  {"week", week},
  {"wk", week},
  {"weeks", week},
  {"$", cost},
  {"cost", cost},
  {"$/hr", Cph},
  {"cph", Cph},
  {"$/mwh", CpMWh},
  {"cpmwh", CpMWh},
  {"$/mvarh", CpMVARh},
  {"$/mvah", CpMVARh},
  {"cpmvarh", CpMVARh},
  {"$/mw^2h", CpMW2h},
  {"cpmw2h", CpMW2h},
  {"$/mw2h", CpMW2h},
  {"$/mvar^2h", CpMVAR2h},
  {"$/mvah^2", CpMVAR2h},
  {"cpmvar2h", CpMVAR2h},
  {"$/mva2h", CpMVAR2h},
  {"$/pumwh", CppuMWh},
  {"cppumwh", CpMWh},
  {"$/pumvarh", CpMVARh},
  {"$/pumvah", CppuMVARh},
  {"cppumvarh", CppuMVARh},
  {"$/pumw^2h", CpMW2h},
  {"cppumw2h", CppuMW2h},
  {"$/mwh2h", CppuMW2h},
  {"$/pumvar^2h", CpMVAR2h},
  {"$/pumvar^2h", CppuMVAR2h},
  {"cpmvar2h", CppuMVAR2h},
  {"$/pumvar2h", CppuMVAR2h},
  {"F", F},
  {"fahrenheit", F},
  {"C", C},
  {"Celsius", C},
  {"K", K},
  {"Kelvin", C},
};

const static std::map<units_t, std::string> unit2Name{
  {MW, "MW"},
  {MVAR, "MVAR"},
  {Amp, "A"},
  {Volt, "V"},
  {kV, "kV"},
  {kW, "kW"},
  {Watt, "W"},
  {Ohm, "Ohm"},
  {Hz, "Hz"},
  {rps, "rps"},
  {rpm, "rpm"},
  {MWps, "MW/s"},
  {MWpmin, "MW/min"},
  {MWph, "MW/hr"},

  {puMW, "puMW"},
  {puMWps, "puMW/s"},
  {puMWpmin, "puMW/min"},
  {puMWph, "puMW/hr"},
  {puHz, "puHz"},
  {puOhm, "puOhm"},
  {puA, "puA"},
  {pu, "pu"},
  {meter, "m"},
  {mile, "mi"},
  {ft, "ft"},
  {km, "km"},
  {deg, "deg"},
  {rad, "rad"},
  {sec, "s"},
  {min, "min"},
  {hour, "hr"},
  {day, "day"},
  {week, "wk"},
  {defUnit, ""},
  {cost, "$"},
  {Cph, "$/h"},
  {CpMWh, "$/MWh"},
  {CpMVARh, "$/MVARh"},
  {CpMW2h, "$/MW^2h"},
  {CpMVAR2h, "$/MVAR^2h"},
  {CppuMWh, "$/puMWh"},
  {CppuMVARh, "$/puMVARh"},
  {CppuMW2h, "$/puMW^2h"},
  {CppuMVAR2h, "$/puMVAR^2h"},
  {F, "F"},
  {C, "C"},
  {K, "K"},
};


// function to convert a string to a unit enumeration value

std::string to_string (units_t unitType) { return mapFind (unit2Name, unitType, std::string ("unknown")); }
units_t getUnits (const std::string &unitString, units_t defValue)
{
    if (unitString.empty ())
    {
        return defValue;
    }
    auto unitName = convertToLowerCase (unitString);
    return mapFind (name2Unit, unitName, defValue);
}

static const std::map<units_t, units_type_t> unit2Type{std::make_pair (MW, electrical),
                                                       std::make_pair (MVAR, electrical),
                                                       std::make_pair (Amp, electrical),
                                                       std::make_pair (Volt, electrical),
                                                       std::make_pair (kV, electrical),
                                                       std::make_pair (kW, electrical),
                                                       std::make_pair (Watt, electrical),
                                                       std::make_pair (Ohm, electrical),
                                                       std::make_pair (Hz, rotation),
                                                       std::make_pair (rps, rotation),
                                                       std::make_pair (rpm, rotation),
                                                       std::make_pair (puMW, electrical),
                                                       std::make_pair (puHz, rotation),
                                                       std::make_pair (puOhm, electrical),
                                                       std::make_pair (MWps, electrical),
                                                       std::make_pair (MWpmin, electrical),
                                                       std::make_pair (MWph, electrical),
                                                       std::make_pair (puMWps, electrical),
                                                       std::make_pair (puMWpmin, electrical),
                                                       std::make_pair (puMWph, electrical),
                                                       std::make_pair (MWps, electrical),
                                                       std::make_pair (puA, electrical),
                                                       std::make_pair (puV, electrical),
                                                       std::make_pair (pu, electrical),
                                                       std::make_pair (meter, distance),
                                                       std::make_pair (mile, distance),
                                                       std::make_pair (ft, distance),
                                                       std::make_pair (km, distance),
                                                       std::make_pair (deg, angle),
                                                       std::make_pair (rad, angle),
                                                       std::make_pair (sec, time),
                                                       std::make_pair (min, time),
                                                       std::make_pair (hour, time),
                                                       std::make_pair (day, time),
                                                       std::make_pair (week, time),
                                                       std::make_pair (defUnit, electrical),
                                                       std::make_pair (cost, price),
                                                       std::make_pair (Cph, price),
                                                       std::make_pair (CpMWh, price),
                                                       std::make_pair (CpMVARh, price),
                                                       std::make_pair (CpMW2h, price),
                                                       std::make_pair (CpMVAR2h, price),
                                                       std::make_pair (CppuMWh, price),
                                                       std::make_pair (CppuMVARh, price),
                                                       std::make_pair (CppuMW2h, price),
                                                       std::make_pair (CppuMVAR2h, price),
                                                       std::make_pair (C, temperature),
                                                       std::make_pair (F, temperature),
                                                       std::make_pair (K, temperature)};

inline bool isPu(const units_t in)
{
	return ((in >= pu) && (static_cast<int>(in) < 200));
}
inline bool conversionNotNeeded (const units_t in, const units_t out)
{
    return ((in == defUnit) || (in == out) || (out == defUnit)||(isPu(in)&&(out==pu)));
}

double unitConversionPower (double val, const units_t in, const units_t out, double basePower, double localBaseVoltage)
{
    // check if no conversion is needed
    if (conversionNotNeeded (in, out))
    {
        return val;
    }
    double ret = badConversion;
    switch (in)
    {
    case Watt:
        val = val / 1000.0;  // I know this looks funny, there really shouldn't be a break here convert to KW
		FALLTHROUGH
        // FALLTHROUGH
    case kW:
        val = val / 1000.0;  // then convert to MW
		FALLTHROUGH
        // FALLTHROUGH
    case MW:
    case MVAR:
        val = val / basePower;  // now convert to puMW
		FALLTHROUGH
        // FALLTHROUGH
    case puMW:
    case pu:
        switch (out)
        {
        case Watt:
            ret = val * 1000000.0 * basePower;
            break;
        case MW:
        case MVAR:
            ret = val * basePower;
            break;
        case kW:
            ret = val * 1000.0 * basePower;
            break;
        case Ohm:
            ret = localBaseVoltage * localBaseVoltage /
                  (val * basePower);  // the factors of 1000000 cancel out on top and bottomw
            break;
        case Amp:
            ret = 1000 * val * basePower / localBaseVoltage;
            break;
        case puMW:
        case pu:
        case puOhm:
        case puA:
        case puV:
            ret = val;
            break;
        case kV:
            ret = val * localBaseVoltage;
            break;
        case Volt:
            ret = val * localBaseVoltage * 1000.0;
            break;
        default:
            break;
        }
        break;
    case puV:
        val = val * localBaseVoltage;  // convert to kV --localBaseVoltage is defined in kV
		FALLTHROUGH
        // FALLTHROUGH
    case kV:
        val = val * 1000.0;  // convert to V
		FALLTHROUGH
        // FALLTHROUGH
    case Volt:
        switch (out)
        {
        case Volt:
            ret = val;
            break;
        case kV:
            ret = val / 1000.0;
            break;
        case puV:
		case pu:
            ret = val / localBaseVoltage / 1000.0;
            break;
        default:
            break;
        }
        break;
    case Ohm:
        val = val / (basePower * 1000000.0 / localBaseVoltage / localBaseVoltage);  // convert to puOhms
		FALLTHROUGH
        // FALLTHROUGH
    case puOhm:

        switch (out)
        {
        case Ohm:
            ret = val * (localBaseVoltage * localBaseVoltage / basePower);
            break;
        case MW:
            ret = basePower / val;
            break;
        case kW:
            ret = basePower / val * 1000.0;
            break;
        case Watt:
            ret = basePower / val * 1000000.0;
            break;
        case Amp:
            ret = 1 / val * (basePower * 1000000.0 / localBaseVoltage);
            break;
        case puMW:
        case pu:
            ret = 1 / val;  // V^2/R assuming voltage=1.0 pu;
            break;
        case puOhm:  // this is here due to cascading cases:
            ret = val;
            break;
        case puA:
            ret = 1 / val;  // same as puMW since it assumes v=1 and ohms law I=V/R;
            break;
        default:
            break;
        }
        break;
    case Amp:
        val = val * localBaseVoltage / basePower / 1000.0;  // convert to puA
		FALLTHROUGH
        // FALLTHROUGH
    case puA:

        switch (out)
        {
        case MW:
        case kW:
        case Watt:
        case Ohm:
        case Amp:
            ret = 1000.0 * val * (basePower / localBaseVoltage);
            break;
        case puMW:
        case pu:
        case puOhm:
        case puA:
            ret = val;
            break;
        default:
            break;
        }
        break;
    case MWph:
        val = val / 60.0;
		FALLTHROUGH
        // FALLTHROUGH
    case MWpmin:
        val = val / 60.0;
		FALLTHROUGH
        // FALLTHROUGH
    case MWps:
        switch (out)
        {
        case puMWps:
            ret = val / basePower;
            break;
        case puMWph:
            ret = val / basePower * 3600.0;
            break;
        case puMWpmin:
            ret = val / basePower * 60.0;
            break;
        case MWps:
            ret = val;
            break;
        case MWpmin:
            ret = val * 60.0;
            break;
        case MWph:
            ret = val * 3600.0;
            break;
        default:
            break;
        }
        break;
    case puMWph:
        val = val / 60.0;
		FALLTHROUGH
        // FALLTHROUGH
    case puMWpmin:
        val = val / 60.0;
		FALLTHROUGH
        // FALLTHROUGH
    case puMWps:
        switch (out)
        {
        case MWph:
            ret = val * basePower * 3600;
            break;
        case MWpmin:
            ret = val * basePower * 60;
            break;
        case MWps:
            ret = val * basePower;
            break;
        case puMWps:
            ret = val;
            break;
        case puMWph:
            ret = val * 3600;
            break;
        case puMWpmin:
            ret = val * 60;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return ret;
}

double unitConversionTemperature (double val, const units_t in, const units_t out)
{
    if (conversionNotNeeded (in, out))
    {
        return val;
    }
    double ret = badConversion;
    switch (in)
    {
    case F:
        if (out == C)
        {
            ret = (val - 32.0) * (5.0 / 9.0);
        }
        else  // out==K
        {
            ret = (val + 459.67) * (5.0 / 9.0);
        }
        break;
    case C:
        if (out == F)
        {
            ret = val * (9.0 / 5.0) + 32.0;
        }
        else  // out==K
        {
            ret = val + 273.15;
        }
        break;
    case K:
        if (out == F)
        {
            ret = val * (9.0 / 5.0) - 459.67;
        }
        else  // out==C
        {
            ret = val - 273.15;
        }
        break;
    default:
        break;
    }
    return ret;
}

double unitConversionTime (double val, const units_t in, const units_t out)
{
    // check if no conversion is needed
    if (conversionNotNeeded (in, out))
    {
        return val;
    }
    double ret = badConversion;
    switch (in)
    {
    // time units,
    case sec:
        switch (out)
        {
        case min:
            ret = val / 60.0;
            break;
        case hour:
            ret = val / 3600.0;
            break;
        case day:
            ret = val / 3600.0 / 24.0;
            break;
        case week:
            ret = val / 3600.0 / 24.0 / 7.0;
            break;
        default:
            break;
        }
        break;
    case min:
        switch (out)
        {
        case sec:
            ret = val * 60.0;
            break;
        case hour:
            ret = val / 60.0;
            break;
        case day:
            ret = val / 60.0 / 24.0;
            break;
        case week:
            ret = val / 60.0 / 24.0 / 7.0;
            break;
        default:
            break;
        }
        break;
    case hour:
        switch (out)
        {
        case sec:
            ret = val * 36000.0;
            break;
        case min:
            ret = val * 60.0;
            break;
        case day:
            ret = val / 24.0;
            break;
        case week:
            ret = val / 24.0 / 7.0;
            break;
        default:
            break;
        }
        break;
    case day:
        switch (out)
        {
        case sec:
            ret = val * 3600.0 * 24.0;
            break;
        case min:
            ret = val * 60.0 * 24.0;
            break;
        case hour:
            ret = val * 24.0;
            break;
        case week:
            ret = val / 7.0;
            break;
        default:
            break;
        }
        break;
    case week:
        switch (out)
        {
        case sec:
            ret = val * 3600.0 * 24.0 * 7.0;
            break;
        case min:
            ret = val * 60.0 * 24.0 * 7.0;
            break;
        case hour:
            ret = val * 7.0 * 24.0;
            break;
        case day:
            ret = val * 7.0;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return ret;
}

double unitConversionAngle (double val, const units_t in, const units_t out)
{
    // check if no conversion is needed

    if (conversionNotNeeded (in, out))
    {
        return val;
    }
    double ret = badConversion;
    // angle units
    if (in == deg)
    {
        if (out == rad)
        {
            ret = val * PI / 180.0;
        }
    }
    else if (in == rad)
    {
        if (out == deg)
        {
            ret = val * 180.0 / PI;
        }
    }
    return ret;
}
double unitConversionDistance (double val, const units_t in, const units_t out)
{
    // check if no conversion is needed
    if (conversionNotNeeded (in, out))
    {
        return val;
    }
    double ret = badConversion;
    switch (in)
    {
    case km:
        val = val * 1000;
		FALLTHROUGH
// FALLTHROUGH
    case meter:
        switch (out)
        {
        case meter:
            ret = val;
            break;
        case km:
            ret = val / 1000;
            break;
        case ft:
            ret = val * 100 / 2.54;
            break;
        case mile:
            ret = val * 100 / 2.54 / 5280;
            break;
        default:
            break;
        }
        break;
    case mile:
        ret = val * 5280;
		FALLTHROUGH
        // FALLTHROUGH
    case ft:
        switch (out)
        {
        case meter:
        case km:
        case ft:
        case mile:
        default:
            break;
        }
    default:
        break;
    }
    return ret;
}

double unitConversionFreq (double val, const units_t in, const units_t out, double baseFreq)
{
    // check if no conversion is needed
    if (conversionNotNeeded (in, out))
    {
        return val;
    }
    double ret = badConversion;
    switch (in)
    {
    case Hz:
        switch (out)
        {
        case puHz:
        case pu:
            ret = val / baseFreq;
            break;
        case rps:
            ret = 2.0 * PI * val;
            break;
        case rpm:
            ret = val * 60.0;
            break;
        default:
            break;
        }
        break;
    case puHz:
    case pu:
        switch (out)
        {
        case Hz:
            ret = val * baseFreq;
            break;
        case rps:
            ret = 2.0 * PI * val * baseFreq;
            break;
        case rpm:
            ret = val * 60.0 * baseFreq;
            break;
        default:
            break;
        }
        break;
    case rps:
        switch (out)
        {
        case puHz:
            ret = val / 2.0 / PI / baseFreq;
            break;
        case Hz:
            ret = val / 2.0 / PI;
            break;
        case rpm:
            ret = val / 2.0 / PI * 60.0;
            break;
        default:
            break;
        }
        break;
    case rpm:
        switch (out)
        {
        case puHz:
            ret = val / 60.0 / baseFreq;
            break;
        case Hz:
            ret = val / 60.0;
            break;
        case rps:
            ret = 2.0 * PI * val / 60.0;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return ret;
}

double unitConversionCost (double val, const units_t in, const units_t out, double basePower)
{
    // check if no conversion is needed
    if (conversionNotNeeded (in, out))
    {
        return val;
    }
    double ret = badConversion;
    switch (in)
    {
    case CpMWh:
    case CpMVARh:
        switch (out)
        {
        case CppuMWh:
        case CppuMVARh:
            ret = val * basePower;
            break;
        default:
            break;
        }
        break;
    case CppuMWh:
    case CppuMVARh:
        switch (out)
        {
        case CpMWh:
        case CpMVARh:
            ret = val / basePower;
            break;
        default:
            break;
        }
        break;
    case CpMW2h:
    case CpMVAR2h:
        switch (out)
        {
        case CppuMW2h:
        case CppuMVAR2h:
            ret = val * (basePower * basePower);
            break;
        default:
            break;
        }
        break;
    case CppuMW2h:
    case CppuMVAR2h:
        switch (out)
        {
        case CpMW2h:
        case CpMVAR2h:
            ret = val / (basePower * basePower);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return ret;
}

double unitConversion (double val, const units_t in, const units_t out, double basePower, double localBaseVoltage)
{
    // check if no conversion is needed
    if (conversionNotNeeded (in, out))
    {
        return val;
    }
    double ret = badConversion;

    units_type_t utype = mapFind (unit2Type, in, deftype);

    // switch over possible conversions
    switch (utype)
    {
    case electrical:
        ret = unitConversionPower (val, in, out, basePower, localBaseVoltage);
        break;
    case rotation:
        if (basePower == 50.0)
        {
            ret = unitConversionFreq (val, in, out, 50.0);
        }
        else
        {
            ret = unitConversionFreq (val, in, out, 60.0);
        }
        break;
    // distance Units
    case distance:
        ret = unitConversionDistance (val, in, out);
        break;
    // angle units
    case angle:
        ret = unitConversionAngle (val, in, out);
        break;
    case time:
        ret = unitConversionTime (val, in, out);
        break;
    case price:
        ret = unitConversionCost (val, in, out, basePower);
        break;
    case temperature:
        ret = unitConversionTemperature (val, in, out);
        break;
    default:
        break;
    }
    return ret;
}
}  // namespace gridUnits
