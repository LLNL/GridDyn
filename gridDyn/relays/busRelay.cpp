/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include "busRelay.h"
#include "gridCondition.h"
#include "eventQueue.h"
#include "gridEvent.h"
#include "gridCoreTemplates.h"

#include <boost/format.hpp>


busRelay::busRelay (const std::string&objName) : gridRelay (objName)
{
  opFlags.set (continuous_flag);
}

gridCoreObject *busRelay::clone (gridCoreObject *obj) const
{
	busRelay *nobj = cloneBase<busRelay, gridRelay>(this, obj);
	if (nobj == nullptr)
	{
		return obj;
	}
  nobj->cutoutVoltage = cutoutVoltage;
  nobj->cutoutFrequency = cutoutFrequency;
  nobj->voltageDelay = voltageDelay;
  nobj->frequencyDelay = frequencyDelay;
  return nobj;
}

void busRelay::setFlag (const std::string &flag, bool val)
{
  if (flag[0] == '#')
    {

    }
  else
    {
      gridRelay::setFlag (flag, val);
    }

}
/*
std::string commDestName;
std::uint64_t commDestId=0;
std::string commType;
*/
void busRelay::set (const std::string &param,  const std::string &val)
{

  if (param[0] == '#')
    {

    }
  else
    {
      gridRelay::set (param, val);
    }

}

void busRelay::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if ((param == "cutoutvoltage") || (param == "voltagelimit"))
    {
      cutoutVoltage = gridUnits::unitConversion (val,unitType,gridUnits::puV,systemBasePower);
      if (opFlags[dyn_initialized])
        {
          setConditionLevel (0,cutoutVoltage);
        }
    }
  else if ((param == "cutoutfrequency") || (param == "freqlimit"))
    {
      cutoutFrequency = gridUnits::unitConversion (val, unitType, gridUnits::puHz, m_baseFreq);
      if (opFlags[dyn_initialized])
        {
          setConditionLevel (1, cutoutFrequency);
        }
    }
  else if (param == "delay")
    {
      voltageDelay = val;
      frequencyDelay = val;
      if (opFlags[dyn_initialized])
        {
          setActionTrigger (0,0,voltageDelay);
          setActionTrigger (1,0,frequencyDelay);
        }
    }
  else if (param == "voltagedelay")
    {
      voltageDelay = val;
      if (opFlags[dyn_initialized])
        {
          setActionTrigger (0, 0, voltageDelay);
        }
    }
  else if (param == "frequencydelay")
    {
      frequencyDelay = val;
      if (opFlags[dyn_initialized])
        {
          setActionTrigger (1, 0, frequencyDelay);
        }
    }
  else
    {
      gridRelay::set (param, val, unitType);
    }

}

void busRelay::dynObjectInitializeA (gridDyn_time time0, unsigned long flags)
{

  auto ge = std::make_shared<gridEvent> (0.0);

  ge->setValue(0.0);
  ge->setTarget (m_sinkObject,"status");

  add (ge);

  add (make_condition ("voltage", "<", cutoutVoltage, m_sourceObject));
  setActionTrigger (0, 0, voltageDelay);
  if ((cutoutVoltage > 2.0)||(cutoutVoltage <= 0))
    {
      setConditionState (0,condition_states::disabled);
    }
  add (make_condition ("frequency", "<", cutoutFrequency, m_sourceObject));
  setActionTrigger (1, 0, frequencyDelay);
  if ((cutoutFrequency > 2.0) || (cutoutFrequency <= 0))
    {
      setConditionState (1, condition_states::disabled);
    }


  gridRelay::dynObjectInitializeA (time0,flags);

}


void busRelay::actionTaken (index_t conditionNum, index_t /*ActionNum*/, change_code /*actionReturn*/, double /*actionTime*/)
{
  if (conditionNum == 0)
    {
      alert (m_sourceObject,BUS_UNDER_VOLTAGE);
    }
  else if (conditionNum == 1)
    {
      alert (m_sourceObject,BUS_UNDER_FREQUENCY);
    }
}




