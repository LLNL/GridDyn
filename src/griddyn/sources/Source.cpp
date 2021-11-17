/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../Source.h"

#include "core/coreObjectTemplates.hpp"
#include "core/objectFactoryTemplates.hpp"
#include "otherSources.h"
#include "sourceTypes.h"

namespace griddyn {
// setup the load object factories
static typeFactory<Source> glf("source",
                               stringVec{"basic", "constant", "simple"},
                               "constant");  // set constant to the default
namespace sources {
    static childTypeFactory<pulseSource, Source> glfp("source", "pulse");
    static childTypeFactory<sineSource, Source> cfgsl("source", stringVec{"sine", "oscillatory"});
    static childTypeFactory<rampSource, Source> glfr("source", "ramp");
    static childTypeFactory<randomSource, Source> glfrand("source", stringVec{"random", "rand"});
    static childTypeFactory<fileSource, Source> glfld("source", "file");
    static childTypeFactory<grabberSource, Source> grbsrc("source", stringVec{"grabber", "data"});
}  // namespace sources

Source::Source(const std::string& objName, double startVal):
    gridSubModel(objName), m_tempOut(startVal)
{
    m_output = startVal;
    m_inputSize = 0;
    m_outputSize = 1;
    opFlags.set(pflow_init_required);
}

coreObject* Source::clone(coreObject* obj) const
{
    auto gS = cloneBase<Source, gridSubModel>(this, obj);
    if (gS == nullptr) {
        return obj;
    }
    gS->m_tempOut = m_tempOut;
    gS->lastTime = lastTime;
    gS->purpose_ = purpose_;
    gS->outputUnits_ = outputUnits_;
    return gS;
}

void Source::set(const std::string& param, const std::string& val)
{
    if (param == "purpose") {
        purpose_ = val;
    } else if ((param == "units") || (param == "outputunits")) {
        outputUnits_ = units::unit_cast_from_string(val);
    } else {
        gridSubModel::set(param, val);
    }
}

void Source::setLevel(double newLevel)
{
    std::cout << "Source::setLevel" << std::endl;
    m_tempOut = m_output = newLevel;
    std::cout << "  m_tempOut = " << m_tempOut << std::endl;
}
void Source::set(const std::string& param, double val, units::unit unitType)
{
    if ((param == "val") || (param == "setval") || (param == "level") || (param == "value") ||
        (param == "output")) {
        setLevel(units::convert(val, unitType, outputUnits_, systemBasePower, localBaseVoltage));
    } else {
        gridSubModel::set(param, val, unitType);
    }
}

void Source::setState(coreTime time,
                      const double state[],
                      const double dstate_dt[],
                      const solverMode& sMode)
{
    std::cout << "----------------------" << std::endl;
    std::cout << "Source::setState start" << std::endl;
    std::cout << "  time      = " << time      << std::endl;
    std::cout << "  lastTime  = " << lastTime  << std::endl;
    std::cout << "  m_tempOut = " << m_tempOut << std::endl;
    std::cout << "  m_output  = " << m_output  << std::endl;
    std::cout << "  updateOutput(time) before" << std::endl;
    updateOutput(time);
    std::cout << "  updateOutput(time) after" << std::endl;
    std::cout << "  time      = " << time      << std::endl;
    std::cout << "  lastTime  = " << lastTime  << std::endl;
    std::cout << "  m_tempOut = " << m_tempOut << std::endl;
    std::cout << "  m_output  = " << m_output  << std::endl;
    std::cout << "  setState(time, state, dstate_dt, sMode) before" << std::endl;
    gridComponent::setState(time, state, dstate_dt, sMode);
    std::cout << "  setState(time, state, dstate_dt, sMode) after" << std::endl;
    std::cout << "  time      = " << time      << std::endl;
    std::cout << "  lastTime  = " << lastTime  << std::endl;
    std::cout << "  m_tempOut = " << m_tempOut << std::endl;
    std::cout << "  m_output  = " << m_output  << std::endl;
    std::cout << "  update values before" << std::endl;
    m_tempOut = m_output;
    lastTime = time;
    std::cout << "  update values after" << std::endl;
    std::cout << "  time      = " << time      << std::endl;
    std::cout << "  lastTime  = " << lastTime  << std::endl;
    std::cout << "  m_tempOut = " << m_tempOut << std::endl;
    std::cout << "  m_output  = " << m_output  << std::endl;
    std::cout << "Source::setState end" << std::endl;
    std::cout << "--------------------" << std::endl;
}

void Source::updateOutput(coreTime time)
{
    std::cout << "Source::updateOutput" << std::endl;
    m_tempOut = computeOutput(time);
    m_output = m_tempOut;
    prevTime = time;
    lastTime = time;
    std::cout << "  m_tempOut = " << m_tempOut << std::endl;
}

void Source::timestep(coreTime time, const IOdata& /*inputs*/, const solverMode& /*sMode*/)
{
    if (time != prevTime) {
        updateOutput(time);
        m_tempOut = m_output;
        prevTime = time;
    }
}

count_t Source::outputDependencyCount(index_t /*outputNum*/, const solverMode& /*sMode*/) const
{
    return 0;
}
IOdata Source::getOutputs(const IOdata& /*inputs*/,
                          const stateData& /*sD*/,
                          const solverMode& /*sMode*/) const
{
    return {m_tempOut};
}

double Source::getOutput(const IOdata& /*inputs*/,
                         const stateData& /*sD*/,
                         const solverMode& /*sMode*/,
                         index_t outputNum) const
{
    std::cout << "Source::getOutput 1" << std::endl;
    return (outputNum == 0) ? m_tempOut : kNullVal;
}

double Source::getOutput(index_t outputNum) const
{
    std::cout << "Source::getOutput 2" << std::endl;
    return (outputNum == 0) ? m_tempOut : kNullVal;
}

units::unit Source::outputUnits(index_t outputNum) const
{
    return (outputNum == 0) ? outputUnits_ : units::defunit;
}

index_t Source::getOutputLoc(const solverMode& /*sMode*/, index_t /*outputNum*/) const
{
    return kNullLocation;
}
void Source::updateLocalCache(const IOdata& /*inputs*/,
                              const stateData& sD,
                              const solverMode& /*sMode*/)
{
    std::cout << "Source::updateLocalCache" << std::endl;
    if ((prevTime != sD.time) && (sD.time > timeZero)) {
        m_tempOut = computeOutput(sD.time);
        lastTime = sD.time;
        std::cout << "  m_tempOut = " << m_tempOut << std::endl;
        std::cout << "  lastTime  = " << sD.time << std::endl;
    }
}

double Source::computeOutput(coreTime /*time*/) const
{
    return m_output;
}
}  // namespace griddyn
