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

// headers
#include "infiniteBus.h"

#include "../Link.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"

namespace griddyn {
using namespace units;

infiniteBus::infiniteBus(const std::string& objName): gridBus(objName)
{
    // default values
    type = busType::SLK;
    dynType = dynBusType::dynSLK;
}

infiniteBus::infiniteBus(double startVoltage, double startAngle, const std::string& objName):
    gridBus(startVoltage, startAngle, objName)
{
    type = busType::SLK;
    dynType = dynBusType::dynSLK;
}

coreObject* infiniteBus::clone(coreObject* obj) const
{
    auto nobj = cloneBase<infiniteBus, gridBus>(this, obj);
    if (nobj == nullptr) {
        return obj;
    }
    nobj->dvdt = dvdt;
    nobj->dfdt = dfdt;
    return nobj;
}

void infiniteBus::updateVoltageAngle(coreTime time)
{
    auto dt = static_cast<double>(time - prevTime);

    angle += 2.0 * kPI * dt * (dfdt / 2.0 + (freq - 1.0));
    freq += dfdt * dt;
    voltage += dvdt * dt;
}

void infiniteBus::timestep(coreTime time, const IOdata& inputs, const solverMode& sMode)
{
    updateVoltageAngle(time);
    gridBus::timestep(time, inputs, sMode);
}

void infiniteBus::setState(coreTime time,
                           const double state[],
                           const double dstate_dt[],
                           const solverMode& sMode)
{
    updateVoltageAngle(time);
    gridBus::setState(time, state, dstate_dt, sMode);
}

// set properties
void infiniteBus::set(const std::string& param, const std::string& val)
{
    if (param == "type") {
        if (val != "infinite") {
            throw(invalidParameterValue(param + ':' + val));
        }
    } else {
        gridBus::set(param, val);
    }
}

void infiniteBus::set(const std::string& param, double val, unit unitType)
{
    if (param == "dvdt") {
        dvdt = convert(val, unitType, puV, systemBasePower, localBaseVoltage);
    } else if (param == "dfdt") {
        dfdt = convert(val, unitType, puHz, systemBaseFrequency);
    } else {
        gridBus::set(param, val, unitType);
    }
}

double infiniteBus::getVoltage(const double /*state*/[], const solverMode& /*sMode*/) const
{
    return voltage;
}

double infiniteBus::getAngle(const double /*state*/[], const solverMode& /*sMode*/) const
{
    return angle;
}

double infiniteBus::getVoltage(const stateData& sD, const solverMode& /*sMode*/) const
{
    const double dt = (!sD.empty()) ? static_cast<double>(sD.time - prevTime) : 0.0;
    return voltage + dt * dvdt;
}

double infiniteBus::getAngle(const stateData& sD, const solverMode& /*sMode*/) const
{
    const double dt = (!sD.empty()) ? static_cast<double>(sD.time - prevTime) : 0.0;
    return angle + 2 * kPI * (dt * ((freq - 1.0) + dfdt / 2));
}

double infiniteBus::getFreq(const stateData& sD, const solverMode& /*sMode*/) const
{
    double dt = (!sD.empty()) ? static_cast<double>(sD.time - prevTime) : 0.0;
    return freq + dt * dfdt;
}

bool infiniteBus::checkCapable()
{
    return true;
}

}  // namespace griddyn
