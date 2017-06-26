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

#include "exponentialLoad.h"
#include "core/coreObjectTemplates.hpp"
#include "gridBus.h"
#include "utilities/matrixData.hpp"
#include "utilities/stringOps.h"
#include <cmath>
namespace griddyn
{
namespace loads
{
exponentialLoad::exponentialLoad (const std::string &objName) : Load (objName) {}
exponentialLoad::exponentialLoad (double rP, double qP, const std::string &objName) : Load (rP, qP, objName) {}
coreObject *exponentialLoad::clone (coreObject *obj) const
{
    auto ld = cloneBase<exponentialLoad, Load> (this, obj);
    if (ld == nullptr)
    {
        return obj;
    }

    ld->alphaP = alphaP;
    ld->alphaQ = alphaQ;
    return ld;
}

// set properties
void exponentialLoad::set (const std::string &param, const std::string &val) { Load::set (param, val); }
void exponentialLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "alphap") || (param == "ap"))
    {
        alphaP = val;
    }
    else if ((param == "alphaq") || (param == "aq"))
    {
        alphaQ = val;
    }
    else if ((param == "alpha") || (param == "a"))
    {
        alphaP = alphaQ = val;
    }
    else
    {
        Load::set (param, val, unitType);
    }
}

void exponentialLoad::ioPartialDerivatives (const IOdata &inputs,
                                            const stateData & /*sD*/,
                                            matrixData<double> &md,
                                            const IOlocs &inputLocs,
                                            const solverMode & /*sMode*/)
{
    const double V = inputs[voltageInLocation];
    // power vs voltage
    if (inputLocs[voltageInLocation] != kNullLocation)
    {
        md.assign (PoutLocation, inputLocs[voltageInLocation], getP () * alphaP * pow (V, alphaP - 1.0));

        // reactive power vs voltage
        md.assign (QoutLocation, inputLocs[voltageInLocation], getQ () * alphaQ * pow (V, alphaQ - 1.0));
    }
}

double exponentialLoad::getRealPower () const { return getRealPower (bus->getVoltage ()); }
double exponentialLoad::getReactivePower () const { return getReactivePower (bus->getVoltage ()); }
double
exponentialLoad::getRealPower (const IOdata &inputs, const stateData & /*sD*/, const solverMode & /*sMode*/) const
{
    return getRealPower (inputs[voltageInLocation]);
}

double exponentialLoad::getReactivePower (const IOdata &inputs,
                                          const stateData & /*sD*/,
                                          const solverMode & /*sMode*/) const
{
    return getReactivePower (inputs[voltageInLocation]);
}

double exponentialLoad::getRealPower (const double V) const
{
    if (isConnected ())
    {
        double val = getP ();
        val *= pow (V, alphaP);
        return val;
    }
    return 0.0;
}

double exponentialLoad::getReactivePower (double V) const
{
    if (isConnected ())
    {
        double val = getQ ();
        val *= pow (V, alphaQ);
        return val;
    }
    return 0.0;
}
}  // namespace loads
}  // namespace griddyn
