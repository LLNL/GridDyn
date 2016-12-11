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

#include "gridSource.h"
#include "sourceTypes.h"
#include "otherSources.h"
#include "gridCoreTemplates.h"
#include "objectFactoryTemplates.h"


//setup the load object factories
static typeFactory<gridSource> glf ("source", stringVec { "basic", "constant", "simple" }, "constant"); //set constant to the default
static childTypeFactory<pulseSource, gridSource> glfp ("source", "pulse");
static childTypeFactory<sineSource, gridSource> cfgsl ("source", "sine");
static childTypeFactory<rampSource, gridSource> glfr ("source", "ramp");
static childTypeFactory<randomSource, gridSource> glfrand ("source", "random");
static childTypeFactory<fileSource, gridSource> glfld ("source", "file");


gridSource::gridSource (const std::string &objName, double startVal) : gridSubModel (objName), m_tempOut (startVal)
{
	m_output = startVal;
  m_inputSize = 0;
  m_outputSize = 1;
}

coreObject *gridSource::clone (coreObject *obj) const
{
  gridSource *gS = cloneBase<gridSource, gridSubModel> (this, obj);
  if (gS == nullptr)
    {
      return obj;
    }
  gS->m_tempOut = m_tempOut;
  gS->lastTime = lastTime;
  return gS;
}


void gridSource::set (const std::string &param,  const std::string &val)
{

  if (param == "purpose")
    {
      m_purpose = val;
    }
  else
    {
      gridSubModel::set (param, val);
    }

}

void gridSource::setLevel(double newLevel)
{
	m_tempOut = m_output = newLevel;
}

void gridSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if ((param == "val") || (param == "setval")||(param == "level")||(param == "value")||(param=="output"))
    {
	  setLevel(val);
    }
  else
    {
      gridSubModel::set (param, val, unitType);
    }

}

void gridSource::setState(gridDyn_time ttime, const double /*state*/[], const double /*dstate_dt*/[], const solverMode &/*sMode*/)
{
	updateOutput(ttime);
}

void gridSource::updateOutput(gridDyn_time ttime)
{
	m_tempOut = computeOutput(ttime);
	m_output = m_tempOut;
	prevTime = ttime;
	lastTime = ttime;
}

void gridSource::timestep (gridDyn_time ttime, const IOdata & /*args*/, const solverMode &)
{
  if (ttime != prevTime)
    {
      updateOutput (ttime);
	  m_tempOut = m_output;
	  prevTime = ttime;
    }

  

}


IOdata gridSource::getOutputs (const IOdata & /*args*/, const stateData &, const solverMode &) const
{
  return {m_tempOut};
}

double gridSource::getOutput (const IOdata & /*args*/, const stateData &, const solverMode &, index_t num) const
{
	return (num == 0) ? m_tempOut : kNullVal;
}

double gridSource::getOutput (index_t num) const
{
	return (num == 0) ? m_tempOut : kNullVal;
}

index_t gridSource::getOutputLoc (const solverMode & ,index_t /*num*/) const
{
	return kNullLocation;
}

void gridSource::updateLocalCache(const IOdata &/*args*/, const stateData &sD, const solverMode &/*sMode*/)
{
	if (prevTime != sD.time)
	{
		m_tempOut = computeOutput(sD.time);
		lastTime = sD.time;
	}

}


double gridSource::computeOutput(gridDyn_time /*ttime*/) const
{
	return m_output;
}