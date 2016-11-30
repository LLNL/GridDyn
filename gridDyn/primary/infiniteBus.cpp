/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
/*
  * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

// headers
#include "infiniteBus.h"
#include "linkModels/gridLink.h"
#include "gridCoreTemplates.h"
#include "core/gridDynExceptions.h"

using namespace gridUnits;

infiniteBus::infiniteBus (const std::string &objName) : gridBus (objName)
{
  // default values
  type = busType::SLK;
  dynType = dynBusType::dynSLK;
}

infiniteBus::infiniteBus (double startVoltage, double startAngle, const std::string &objName) : gridBus (startVoltage, startAngle, objName)
{
  type = busType::SLK;
  dynType = dynBusType::dynSLK;
}

gridCoreObject *infiniteBus::clone (gridCoreObject *obj) const
{
  infiniteBus *nobj = cloneBase<infiniteBus, gridBus> (this, obj);
  if (!(nobj ))
    {
      return obj;
    }
  nobj->dvdt = dvdt;
  nobj->dfdt = dfdt;
  return nobj;
}

void infiniteBus::updateVoltageAngle (gridDyn_time ttime)
{
  double dt = static_cast<double>(ttime - prevTime);

  angle += 2.0 * kPI * dt * (dfdt / 2.0 + (freq - 1.0));
  freq += dfdt * dt;
  voltage += dvdt * dt;
}

void infiniteBus::timestep (gridDyn_time ttime, const solverMode &sMode)
{
  updateVoltageAngle (ttime);
  gridBus::timestep (ttime, sMode);
}

void infiniteBus::setState (gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  updateVoltageAngle (ttime);
  gridBus::setState (ttime, state, dstate_dt, sMode);
}


// set properties
void infiniteBus::set (const std::string &param,  const std::string &val)
{

  if (param == "type")
    {
	  if (val != "infinite")
	  {
		  throw(invalidParameterValue());
	  }

    }
  else
    {
      gridBus::set (param, val);
    }
}

void infiniteBus::set (const std::string &param, double val, units_t unitType)
{

  if (param == "dvdt")
    {
      dvdt = unitConversion (val,unitType,puV,systemBasePower,baseVoltage);
    }
  else if (param == "dfdt")
    {
      dfdt = unitConversionFreq (val, unitType, puHz,m_baseFreq);
    }
  else
    {
      gridBus::set (param, val, unitType);
    }

}

double infiniteBus::getVoltage (const double /*state*/[], const solverMode &) const
{
  return voltage;
}

double infiniteBus::getAngle (const double /*state*/[], const solverMode &) const
{

  return angle;
}

double infiniteBus::getVoltage (const stateData *sD, const solverMode &) const
{
  const double dt = (sD) ? static_cast<double>(sD->time - prevTime) : 0.0;
  return voltage + dt * dvdt;
}

double infiniteBus::getAngle (const stateData *sD, const solverMode &) const
{
  const double dt = (sD) ? static_cast<double>(sD->time - prevTime) : 0.0;
  return angle + 2 * kPI * (dt * ((freq - 1.0) + dfdt / 2));
}

double infiniteBus::getFreq (const stateData *sD, const solverMode &) const
{
  double dt = (sD) ? static_cast<double>(sD->time - prevTime) : 0.0;
  return freq + dt * dfdt;
}

bool infiniteBus::checkCapable ()
{
  return true;
}