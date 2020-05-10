/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "fncsGhostBus.h"

#include "core/coreObjectTemplates.h"
#include "fncsLibrary.h"
#include "fncsSupport.h"
#include "gridBus.h"
#include "stringOps.h"
#include "vectorOps.hpp"

fncsGhostBus::fncsGhostBus(const std::string& objName): gridBus(objName) {}

coreObject* fncsGhostBus::clone(coreObject* obj) const
{
    fncsGhostBus* nobj = cloneBase<fncsGhostBus, gridBus>(this, obj);
    if (!(nobj)) {
        return obj;
    }
    nobj->loadKey = loadKey;
    nobj->voltageKey = voltageKey;

    return nobj;
}

void fncsGhostBus::pFlowObjectInitializeA(coreTime time0, unsigned long flags)
{
    gridBus::pFlowObjectInitializeA(time0, flags);
}

void fncsGhostBus::pFlowObjectInitializeB()
{
    gridBus::pFlowInitializeB();
    updateA(prevTime);
    updateB();
}

void fncsGhostBus::updateA(coreTime time)
{
    if (!loadKey.empty()) {
        double Pact = unitConversion(S.sumP(), gridUnits::puMW, outUnits, systemBasePower);
        double Qact = unitConversion(S.sumQ(), gridUnits::puMW, outUnits, systemBasePower);
        std::complex<double> ld(Pact, Qact);

        fncsSendComplex(loadKey, ld);
    }
    lastUpdateTime = time;
}

coreTime fncsGhostBus::updateB()
{
    nextUpdateTime += updatePeriod;

    //now get the updates
    if (!voltageKey.empty()) {
        auto res = fncsGetComplex(loadKey);
        if (res.real() == kNullVal) {
            voltage = std::abs(res);
            angle = std::arg(res);
        }
    }

    return nextUpdateTime;
}

void fncsGhostBus::timestep(coreTime ttime, const IOdata& inputs, const solverMode& sMode)
{
    while (ttime > nextUpdateTime) {
        updateA(nextUpdateTime);
        updateB();
        gridBus::timestep(nextUpdateTime, inputs, sMode);
    }

    gridBus::timestep(ttime, inputs, sMode);
}

void fncsGhostBus::setFlag(const std::string& param, bool val)
{
    if (param[0] == '#') {
    } else {
        gridBus::setFlag(param, val);
    }
}

void fncsGhostBus::set(const std::string& param, const std::string& val)
{
    if (param == "voltagekey") {
        voltageKey = val;
        updateSubscription();
    } else if (param == "loadkey") {
        loadKey = val;

        fncsRegister::instance()->registerPublication(loadKey, fncsRegister::dataType::fncsComplex);

    } else if ((param == "outunits") || (param == "outputunits")) {
        outUnits = gridUnits::getUnits(val);
    } else {
        gridBus::set(param, val);
    }
}

void fncsGhostBus::set(const std::string& param, double val, gridUnits::units_t unitType)
{
    if (param[0] == '#') {
    } else {
        gridBus::set(param, val, unitType);
    }
}

void fncsGhostBus::updateSubscription()
{
    std::complex<double> cv = std::polar(voltage, angle);
    std::string def = std::to_string(cv.real()) + "+" + std::to_string(cv.imag()) + "j";
    fncsRegister::instance()->registerSubscription(voltageKey,
                                                   fncsRegister::dataType::fncsComplex,
                                                   def);
}
