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

#include "rampLoad.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "gridBus.h"
#include "utilities/vectorOps.hpp"

//#include <ctime>
namespace griddyn
{
namespace loads
{
using namespace gridUnits;
rampLoad::rampLoad (const std::string &objName) : zipLoad (objName) {}
rampLoad::rampLoad (double rP, double qP, const std::string &objName) : zipLoad (rP, qP, objName) {}
coreObject *rampLoad::clone (coreObject *obj) const
{
    auto ld = cloneBase<rampLoad, zipLoad> (this, obj);
    if (ld == nullptr)
    {
        return obj;
    }

    ld->dPdt = dPdt;
    ld->dQdt = dQdt;
    ld->drdt = drdt;
    ld->dxdt = dxdt;
    ld->dIpdt = dIpdt;
    ld->dIqdt = dIqdt;
    ld->dYqdt = dYqdt;
    ld->dYpdt = dYpdt;
    return ld;
}

// set properties
void rampLoad::set (const std::string &param, const std::string &val) { zipLoad::set (param, val); }
void rampLoad::set (const std::string &param, double val, units_t unitType)
{
    if (param.length () == 4)
    {
        if ((param[0] == 'd') && (param[2] == 'd') && (param[3] == 't'))
        {
            switch (param[1])
            {
            case 'p':  // dpdt
                dPdt = unitConversion (val, unitType, puMW, systemBasePower, localBaseVoltage);
                break;
            case 'r':  // drdt
                drdt = unitConversion (val, unitType, puA, systemBasePower, localBaseVoltage);
                break;
            case 'x':  // dxdt
                dxdt = unitConversion (val, unitType, puOhm, systemBasePower, localBaseVoltage);
                break;
            case 'q':  // dqdt
                dQdt = unitConversion (val, unitType, puMW, systemBasePower, localBaseVoltage);
                break;
            case 'i':  // didt
                dIpdt = unitConversion (val, unitType, puOhm, systemBasePower, localBaseVoltage);
                break;
            case 'z':
            case 'y':  // dzdt dydt
                dYpdt = unitConversion (val, unitType, puMW, systemBasePower, localBaseVoltage);
                break;
            default:
                throw (unrecognizedParameter (param));
            }
        }
        else
        {
            zipLoad::set (param, val, unitType);
        }
    }
    else if (param.length () == 5)
    {
        if ((param[0] == 'd') && (param[3] == 'd') && (param[4] == 't'))
        {
            switch (param[2])
            {
            case 'r':
            case 'p':
                switch (param[1])
                {
                case 'i':  // dirdt dipdt
                    dIpdt = unitConversion (val, unitType, puOhm, systemBasePower, localBaseVoltage);
                    break;
                case 'z':
                case 'y':  // dzrdt dyrdt dzpdt dzrdt
                    dYpdt = unitConversion (val, unitType, puMW, systemBasePower, localBaseVoltage);
                    break;
                default:
                    throw (unrecognizedParameter (param));
                }
                break;
            case 'q':
                switch (param[1])
                {
                case 'i':  // diqdt
                    dIqdt = unitConversion (val, unitType, puOhm, systemBasePower, localBaseVoltage);
                    break;
                case 'z':
                case 'y':  // dzqdt dyqdt
                    dYqdt = unitConversion (val, unitType, puMW, systemBasePower, localBaseVoltage);
                    break;
                default:
                    throw (unrecognizedParameter (param));
                }
                break;
            default:
                throw (unrecognizedParameter (param));
            }
        }
        else
        {
            zipLoad::set (param, val, unitType);
        }
    }

    else
    {
        zipLoad::set (param, val, unitType);
    }
}

void rampLoad::updateLocalCache (const IOdata & /*inputs*/, const stateData &sD, const solverMode & /*sMode*/)
{
    auto tdiff = sD.time - lastTime;
    if (tdiff == timeZero)
    {
        return;
    }
    if (dPdt != 0.0)
    {
        setP (getP () + dPdt * tdiff);
    }
    if (dQdt != 0.0)
    {
        setQ (getQ () + dQdt * tdiff);
    }

    if ((drdt != 0) || (dxdt != 0))
    {
        setr (getr () + drdt * tdiff);
        setx (getx () + dxdt * tdiff);
    }
    else if ((dYpdt != 0.0) || (dYqdt != 0.0))
    {
        setYp (getYp () + dYpdt * tdiff);
        setYq (getYq () + dYqdt * tdiff);
    }
    if ((dIpdt != 0) || (dIqdt != 0))
    {
        setIp (getIp () + dIpdt * tdiff);
        setIq (getIq () + dIqdt * tdiff);
    }

    lastTime = sD.time;
}

void rampLoad::clearRamp ()
{
    dPdt = 0.0;
    dQdt = 0.0;
    drdt = 0.0;
    dxdt = 0.0;
    dIpdt = 0.0;
    dIqdt = 0.0;
    dYqdt = 0.0;
    dYpdt = 0.0;
}
}  // namespace loads
}  // namespace griddyn
