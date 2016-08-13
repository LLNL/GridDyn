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
#include "gridCoreTemplates.h"
#include "objectFactoryTemplates.h"


//setup the load object factories
static typeFactory<gridSource> glf ("source", stringVec { "basic", "constant", "simple" }, "constant"); //set constant to the default
static childTypeFactory<pulseSource, gridSource> glfp ("source", "pulse");
static childTypeFactory<sineSource, gridSource> cfgsl ("source", "sine");
static childTypeFactory<rampSource, gridSource> glfr ("source", "ramp");
static childTypeFactory<randomSource, gridSource> glfrand ("source", "random");
static childTypeFactory<fileSource, gridSource> glfld ("source", "file");


gridSource::gridSource (const std::string &objName, double startVal) : gridSubModel (objName), m_tempOut (startVal), m_output (startVal)
{
  m_inputSize = 0;
  m_outputSize = 1;
}

gridCoreObject *gridSource::clone (gridCoreObject *obj) const
{
  gridSource *gS = cloneBase<gridSource, gridSubModel> (this, obj);
  if (gS == nullptr)
    {
      return obj;
    }
  gS->m_tempOut = m_tempOut;
  gS->m_output = m_output;
  gS->lasttime = lasttime;
  return gS;
}


int gridSource::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;
  if ((param == "type")||(param == "sourcetype"))
    {
      m_type = val;
    }
  else
    {
      out = gridCoreObject::set (param, val);
    }
  return out;
}

int gridSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;

  if ((param == "val") || (param == "setval")||(param == "level")||(param == "value"))
    {
      m_tempOut = m_output = val;
    }
  else
    {
      out = gridCoreObject::set (param, val, unitType);
    }

  return out;
}

double gridSource::timestep (double ttime, const IOdata & /*args*/, const solverMode &)
{
  if (ttime != prevTime)
    {
      sourceUpdateForward (ttime);
    }

  prevTime = ttime;
  return m_output;

}

IOdata gridSource::getOutputs (const IOdata & /*args*/, const stateData *sD, const solverMode &)
{
  if ((sD) && (sD->time != lasttime))
    {
      sourceUpdate (sD->time);
    }
  return {
           m_tempOut
  };
}

double gridSource::getOutput (const IOdata & /*args*/, const stateData *sD, const solverMode &, index_t /*num*/) const
{
  if ((sD) && (sD->time != lasttime))
    {
      //  sourceUpdate (sD->time);
    }
  return m_tempOut;
}

double gridSource::getOutput (index_t /*num*/) const
{
  return m_output;
}
double gridSource::getOutputLoc (const IOdata & /*args*/, const stateData *sD, const solverMode &, index_t &currentLoc, index_t /*num*/) const
{
  currentLoc = kNullLocation;
  if ((sD) && (sD->time != lasttime))
    {
      //  sourceUpdate (sD->time);
    }
  return m_tempOut;
}


void gridSource::sourceUpdate (double /*ttime*/)
{
}

void gridSource::sourceUpdateForward (double ttime)
{
  sourceUpdate (ttime);
  m_output = m_tempOut;
}
