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

#include "Load.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "gridBus.h"
#include "measurement/objectGrabbers.h"
#include "utilities/matrixData.hpp"

#include <cmath>
#include <complex>
#include <iostream>

namespace griddyn
{
using namespace gridUnits;

std::atomic<count_t> Load::loadCount (0);
Load::Load (const std::string &objName) : gridSecondary (objName) { constructionHelper (); }
Load::Load (double rP, double rQ, const std::string &objName) : gridSecondary (objName), P (rP), Q (rQ)
{
    constructionHelper ();
}

void Load::constructionHelper ()
{
    // default values
    setUserID (++loadCount);
    updateName ();
}

coreObject *Load::clone (coreObject *obj) const
{
    auto nobj = cloneBase<Load, gridSecondary> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    nobj->setLoad (
      P, Q);  // use the set load function in case we are cloning from a basic object to a higher level object
    nobj->pfq = pfq;
    return nobj;
}

void Load::setLoad (double level, units_t unitType)
{
    setP (unitConversion (level, unitType, puMW, systemBasePower));
}

void Load::setLoad (double Plevel, double Qlevel, units_t unitType)
{
    setP (unitConversion (Plevel, unitType, puMW, systemBasePower));
    setQ (unitConversion (Qlevel, unitType, puMW, systemBasePower));
}

static const stringVec locNumStrings{"p", "q", "pf"};

static const stringVec locStrStrings{

};

static const stringVec flagStrings{"usepowerfactor"};

void Load::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
    getParamString<Load, gridComponent> (this, pstr, locNumStrings, locStrStrings, flagStrings, pstype);
}

void Load::setFlag (const std::string &flag, bool val)
{
    if (flag == "usepowerfactor")
    {
        if (val)
        {
            if (!(opFlags[use_power_factor_flag]))
            {
                opFlags.set (use_power_factor_flag);
                updatepfq ();
            }
        }
        else
        {
            opFlags.reset (use_power_factor_flag);
        }
    }
    else
    {
        gridSecondary::setFlag (flag, val);
    }
}

// set properties
void Load::set (const std::string &param, const std::string &val)
{
    if (param[0] == '#')
    {
    }
    else
    {
        gridSecondary::set (param, val);
    }
}

double Load::get (const std::string &param, units_t unitType) const
{
    double val = kNullVal;
    if (param.length () == 1)
    {
        switch (param[0])
        {
        case 'p':
            val = unitConversion (P, puMW, unitType, systemBasePower, baseVoltage);
            break;
        case 'q':
            val = unitConversion (Q, puMW, unitType, systemBasePower, baseVoltage);
            break;
        default:
            break;
        }
        return val;
    }

    if (param == "pf")
    {
        val = pfq;
    }
    else if (auto fptr = getObjectFunction (this, param).first)
    {
        auto unit = getObjectFunction (this, param).second;
        coreObject *tobj = const_cast<Load *> (this);
        val = unitConversion (fptr (tobj), unit, unitType, systemBasePower, baseVoltage);
    }
    else
    {
        val = gridSecondary::get (param, unitType);
    }
    return val;
}

void Load::set (const std::string &param, double val, units_t unitType)
{
	if (param.empty())
	{
		return;
	}
    if (param.length () == 1)
    {
        switch (param.front ())
        {
        case 'p':
            setP (unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
            break;
        case 'q':
            setQ (unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
            break;
        default:
            throw (unrecognizedParameter (param));
        }
        checkFaultChange ();
        return;
    }
	if (param.empty())
	{
		return;
	}
    if (param.back () == '+')  // load increments
    {
        // load increments  allows a delta on the load through the set functions
        if (param == "p+")
        {
            P += unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
            checkpfq ();
        }
        else if (param == "q+")
        {
            Q += unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
            updatepfq ();
        }
        else
        {
            gridSecondary::set (param, val, unitType);
        }
    }
    else if (param.back () == '*')
    {
        // load increments  allows a delta on the load through the set functions
        if (param == "p*")
        {
            P *= val;
            checkpfq ();
        }
        else if (param == "q*")
        {
            Q *= val;
            updatepfq ();
        }
        else
        {
            gridSecondary::set (param, val, unitType);
        }
    }
    else if (param == "load p")
    {
        setP (unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
    }
    else if (param == "load q")
    {
        setQ (unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
    }
    else if ((param == "pf") || (param == "powerfactor"))
    {
        if (val != 0.0)
        {
            if (std::abs (val) <= 1.0)
            {
                pfq = std::sqrt (1.0 - val * val) / val;
            }
            else
            {
                pfq = 0.0;
            }
        }
        else
        {
            pfq = kBigNum;
        }
        opFlags.set (use_power_factor_flag);
    }
    else if (param == "qratio")
    {
        pfq = val;
        opFlags.set (use_power_factor_flag);
    }
    else
    {
        gridSecondary::set (param, val, unitType);
    }
}

void Load::setP (double newP)
{
    P = newP;
    checkpfq ();
    checkFaultChange ();
}

void Load::setQ (double newQ)
{
    Q = newQ;
    updatepfq ();
    checkFaultChange ();
}

void Load::updatepfq ()
{
    if (opFlags[use_power_factor_flag])
    {
        pfq = (P == 0.0) ? kBigNum : Q / P;
    }
}

void Load::checkpfq ()
{
    if (opFlags[use_power_factor_flag])
    {
        if (pfq > 1000.0)  // if the pfq is screwy, recalculate, otherwise leave it the same.
        {
            if (P != 0.0)
            {
                pfq = Q / P;
            }
        }
    }
}

void Load::checkFaultChange ()
{
    if ((opFlags[pFlow_initialized]) && (bus->getVoltage () < 0.05))
    {
        alert (this, POTENTIAL_FAULT_CHANGE);
    }
}

double Load::getRealPower () const { return P; }
double Load::getReactivePower () const { return Q; }
double Load::getRealPower (const IOdata & /*inputs*/, const stateData & /*sD*/, const solverMode & /*sMode*/) const
{
    return getRealPower ();
}

double
Load::getReactivePower (const IOdata & /*inputs*/, const stateData & /*sD*/, const solverMode & /*sMode*/) const
{
    return getReactivePower ();
}

double Load::getRealPower (const double /*V*/) const { return getRealPower (); }
double Load::getReactivePower (double /*V*/) const { return getReactivePower (); }
count_t Load::outputDependencyCount (index_t /*num*/, const solverMode & /*sMode*/) const { return 0; }
}  // namespace griddyn
