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

#include "../Source.h"
#include "core/coreObjectTemplates.hpp"
#include "core/objectFactoryTemplates.hpp"
#include "otherSources.h"
#include "sourceTypes.h"

namespace griddyn
{
// setup the load object factories
static typeFactory<Source>
  glf ("source", stringVec{"basic", "constant", "simple"}, "constant");  // set constant to the default
namespace sources
{
static childTypeFactory<pulseSource, Source> glfp ("source", "pulse");
static childTypeFactory<sineSource, Source> cfgsl("source", stringVec{ "sine","oscillatory" });
static childTypeFactory<rampSource, Source> glfr ("source", "ramp");
static childTypeFactory<randomSource, Source> glfrand("source", stringVec{ "random","rand" });
static childTypeFactory<fileSource, Source> glfld ("source", "file");
static childTypeFactory<grabberSource, Source> grbsrc("source", stringVec{ "grabber","data" });
}  // namespace sources

Source::Source (const std::string &objName, double startVal) : gridSubModel (objName), m_tempOut (startVal)
{
    m_output = startVal;
    m_inputSize = 0;
    m_outputSize = 1;
    opFlags.set (pflow_init_required);
}

coreObject *Source::clone (coreObject *obj) const
{
    auto gS = cloneBase<Source, gridSubModel> (this, obj);
    if (gS == nullptr)
    {
        return obj;
    }
    gS->m_tempOut = m_tempOut;
    gS->lastTime = lastTime;
    gS->purpose_ = purpose_;
    gS->outputUnits_ = outputUnits_;
    return gS;
}

void Source::set (const std::string &param, const std::string &val)
{
    if (param == "purpose")
    {
        purpose_ = val;
    }
    else if ((param == "units") || (param == "outputunits"))
    {
        outputUnits_ = gridUnits::getUnits(val);
    }
    else
    {
        gridSubModel::set (param, val);
    }
}

void Source::setLevel (double newLevel) { m_tempOut = m_output = newLevel; }
void Source::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "val") || (param == "setval") || (param == "level") || (param == "value") || (param == "output"))
    {
        setLevel (gridUnits::unitConversion(val,unitType,outputUnits_,systemBasePower,localBaseVoltage));
    }
    else
    {
        gridSubModel::set (param, val, unitType);
    }
}

void Source::setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
    updateOutput (time);
    gridComponent::setState (time, state, dstate_dt, sMode);
    m_tempOut = m_output;
    lastTime = time;
}

void Source::updateOutput (coreTime time)
{
    m_tempOut = computeOutput (time);
    m_output = m_tempOut;
    prevTime = time;
    lastTime = time;
}

void Source::timestep (coreTime time, const IOdata & /*inputs*/, const solverMode & /*sMode*/)
{
    if (time != prevTime)
    {
        updateOutput (time);
        m_tempOut = m_output;
        prevTime = time;
    }
}

count_t Source::outputDependencyCount (index_t /*outputNum*/, const solverMode & /*sMode*/) const { return 0; }
IOdata Source::getOutputs (const IOdata & /*inputs*/, const stateData & /*sD*/, const solverMode & /*sMode*/) const
{
    return {m_tempOut};
}

double Source::getOutput (const IOdata & /*inputs*/,
                          const stateData & /*sD*/,
                          const solverMode & /*sMode*/,
                          index_t outputNum) const
{
    return (outputNum == 0) ? m_tempOut : kNullVal;
}


double Source::getOutput (index_t outputNum) const { return (outputNum == 0) ? m_tempOut : kNullVal; }

gridUnits::units_t Source::outputUnits(index_t outputNum) const
{
    return (outputNum == 0) ? outputUnits_ : gridUnits::defUnit;
}

index_t Source::getOutputLoc (const solverMode & /*sMode*/, index_t /*outputNum*/) const { return kNullLocation; }
void Source::updateLocalCache (const IOdata & /*inputs*/, const stateData &sD, const solverMode & /*sMode*/)
{
    if ((prevTime != sD.time) && (sD.time > timeZero))
    {
        m_tempOut = computeOutput (sD.time);
        lastTime = sD.time;
    }
}

double Source::computeOutput (coreTime /*time*/) const { return m_output; }
}  // namespace griddyn
