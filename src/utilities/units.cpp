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

namespace gridUnits
{

std::string to_string (units_t unitType) { return units::to_string(unitType); }

units_t getUnits (const std::string &unitString, units_t /* defValue */)
{
  return units::unit_from_string(unitString);
}

double
unitConversionPower (double val, const units_t in, const units_t out, double basePower, double localBaseVoltage)
{
  (void) val;
  (void) in;
  (void) out;
  (void) basePower;
  (void) localBaseVoltage;
  return -1;
}

double unitConversionTemperature (double val, const units_t in, const units_t out)
{
  (void) val;
  (void) in;
  (void) out;
  return -1;
}

double unitConversionTime (double val, const units_t in, const units_t out)
{
  (void) val;
  (void) in;
  (void) out;
  return -1;
}

double unitConversionAngle (double val, const units_t in, const units_t out)
{
  (void) val;
  (void) in;
  (void) out;
  return -1;
}

double unitConversionDistance (double val, const units_t in, const units_t out)
{
  (void) val;
  (void) in;
  (void) out;
  return -1;
}

double unitConversionFreq (double val, const units_t in, const units_t out, double baseFreq)
{
  (void) val;
  (void) in;
  (void) out;
  (void) baseFreq;
  return -1;
}

double unitConversionCost (double val, const units_t in, const units_t out, double basePower)
{
  (void) val;
  (void) in;
  (void) out;
  (void) basePower;
  return -1;
}

double unitConversion (double val, const units_t in, const units_t out, double basePower, double localBaseVoltage)
{
  (void) val;
  (void) in;
  (void) out;
  (void) basePower;
  (void) localBaseVoltage;
  return -1;
}

}  // namespace gridUnits
