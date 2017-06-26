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

#include "helicsGhostBus.h"
#include "core/coreObjectTemplates.hpp"
#include "gridBus.h"
#include "helicsCoordinator.h"
#include "helicsLibrary.h"
#include "helicsSupport.h"
#include "stringOps.h"
#include "vectorOps.hpp"

namespace griddyn
{
namespace helicsLib
{
helicsGhostBus::helicsGhostBus (const std::string &objName) : gridBus (objName) {}

coreObject *helicsGhostBus::clone (coreObject *obj) const
{
    auto nobj = cloneBase<helicsGhostBus, gridBus> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    nobj->loadKey = loadKey;
    nobj->voltageKey = voltageKey;

    return nobj;
}

void helicsGhostBus::pFlowObjectInitializeA (coreTime time0, uint32_t flags)
{
    gridBus::pFlowObjectInitializeA (time0, flags);
}

void helicsGhostBus::pFlowObjectInitializeB ()
{
    gridBus::pFlowInitializeB ();
    updateA (prevTime);
    updateB ();
}

void helicsGhostBus::updateA (coreTime time)
{
    if (!loadKey.empty ())
    {
        double Pact = unitConversion (S.sumP (), gridUnits::puMW, outUnits, systemBasePower);
        double Qact = unitConversion (S.sumQ (), gridUnits::puMW, outUnits, systemBasePower);
        std::complex<double> ld (Pact, Qact);

        coord_->setValue (loadIndex, ld);
    }
    lastUpdateTime = time;
}

coreTime helicsGhostBus::updateB ()
{
    nextUpdateTime += updatePeriod;

    // now get the updates
    if (!voltageKey.empty ())
    {
        auto res = helicsGetComplex (voltageKey);
        if (res.real () == kNullVal)
        {
            voltage = std::abs (res);
            angle = std::arg (res);
        }
    }

    return nextUpdateTime;
}

void helicsGhostBus::timestep (coreTime ttime, const IOdata &inputs, const solverMode &sMode)
{
    while (ttime > nextUpdateTime)
    {
        updateA (nextUpdateTime);
        updateB ();
        gridBus::timestep (nextUpdateTime, inputs, sMode);
    }

    gridBus::timestep (ttime, inputs, sMode);
}

void helicsGhostBus::setFlag (const std::string &param, bool val)
{
    if (param[0] == '#')
    {
    }
    else
    {
        gridBus::setFlag (param, val);
    }
}

void helicsGhostBus::set (const std::string &param, const std::string &val)
{
    if (param == "voltagekey")
    {
        voltageKey = val;
        updateSubscription ();
    }
    else if (param == "loadkey")
    {
        loadKey = val;

        // helicsRegister::instance()->registerPublication(loadKey, helicsRegister::dataType::helicsComplex);
    }
    else if ((param == "outunits") || (param == "outputunits"))
    {
        outUnits = gridUnits::getUnits (val);
    }
    else
    {
        gridBus::set (param, val);
    }
}

void helicsGhostBus::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if (param[0] == '#')
    {
    }
    else
    {
        gridBus::set (param, val, unitType);
    }
}

void helicsGhostBus::updateSubscription ()
{
    std::complex<double> cv = std::polar (voltage, angle);
    std::string def = std::to_string (cv.real ()) + "+" + std::to_string (cv.imag ()) + "j";
    // helicsRegister::instance()->registerSubscription(voltageKey, helicsRegister::dataType::helicsComplex, def);
}

}  // namespace helicsLib
}  // namespace griddyn