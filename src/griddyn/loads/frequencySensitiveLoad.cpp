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

#include "frequencySensitiveLoad.h"

#include "../gridBus.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "core/objectFactoryTemplates.hpp"

#include "utilities/matrixData.hpp"

#include <cmath>
#include <complex>
#include <iostream>

namespace griddyn
{
namespace loads
{
using namespace gridUnits;

frequencySensitiveLoad::frequencySensitiveLoad (const std::string &objName) : Load (objName) {}

coreObject *frequencySensitiveLoad::clone (coreObject *obj) const
{
    auto nobj = cloneBase<frequencySensitiveLoad, Load> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    // nobj->Psched = Psched;

    nobj->dPdf = dPdf;
    nobj->M = M;
    nobj->H = H;

    return nobj;
}

void frequencySensitiveLoad::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    Load::pFlowObjectInitializeA (time0, flags);
    auto Psched = subLoad->getRealPower ();
    dPdf = -H / 30.0 * Psched;
}

void frequencySensitiveLoad::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    Load::dynObjectInitializeA (time0, flags);
}

void frequencySensitiveLoad::timestep (coreTime time, const IOdata &inputs, const solverMode &sMode)
{
    subLoad->timestep (time, inputs, sMode);
    double freq = (inputs.size () > 2) ? inputs[frequencyInLocation] : 1.0;

    updateOutputs (freq);
}

void frequencySensitiveLoad::updateOutputs (double frequency)
{
    Pout = subLoad->getRealPower ();
    Pout += Pout * (frequency - 1.0) * M;
    Qout = subLoad->getReactivePower ();
    Qout += Qout * (frequency - 1.0) * M;
}

static const stringVec locNumStrings{"h", "m"};

static const stringVec locStrStrings{};

static const stringVec flagStrings{};

void frequencySensitiveLoad::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
    getParamString<frequencySensitiveLoad, Load> (this, pstr, locNumStrings, locStrStrings, flagStrings, pstype);
}

void frequencySensitiveLoad::setFlag (const std::string &flag, bool val) { subLoad->setFlag (flag, val); }

// set properties
void frequencySensitiveLoad::set (const std::string &param, const std::string &val)
{
    if (param.empty ())
    {
    }
    else
    {
        subLoad->set (param, val);
    }
}

double frequencySensitiveLoad::get (const std::string &param, units_t unitType) const
{
    if (param == "dpdf")
    {
        return dPdf;
    }
    if (param == "h")
    {
        return H;
    }
    if (param == "m")
    {
        return M;
    }
    return subLoad->get (param, unitType);
}

void frequencySensitiveLoad::set (const std::string &param, double val, units_t unitType)
{
    if (param == "dpdf")
    {
        dPdf = val;
    }
    else if (param == "h")
    {
        H = val;
    }
    else if (param == "m")
    {
        M = val;
    }
    else
    {
        subLoad->set (param, val, unitType);
    }
}

void frequencySensitiveLoad::updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
    subLoad->updateLocalCache (inputs, sD, sMode);
    double freq = (inputs.size () >= frequencyInLocation) ? inputs[frequencyInLocation] : bus->getFreq (sD, sMode);
    updateOutputs (freq);
}

void frequencySensitiveLoad::setState (coreTime time,
                                       const double state[],
                                       const double dstate_dt[],
                                       const solverMode &sMode)
{
    subLoad->setState (time, state, dstate_dt, sMode);
    updateOutputs (bus->getFreq ());
    prevTime = time;
}

double frequencySensitiveLoad::getRealPower () const { return Pout; }

double frequencySensitiveLoad::getReactivePower () const { return Qout; }

double
frequencySensitiveLoad::getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
    double Pr = subLoad->getRealPower (inputs, sD, sMode);
    double freq = (inputs.size () >= frequencyInLocation) ? inputs[frequencyInLocation] : bus->getFreq (sD, sMode);
    return Pr + Pr * (freq - 1.0) * M;
}

double
frequencySensitiveLoad::getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
    double Qr = subLoad->getReactivePower (inputs, sD, sMode);
    double freq = (inputs.size () >= frequencyInLocation) ? inputs[frequencyInLocation] : bus->getFreq (sD, sMode);
    return Qr + Qr * (freq - 1.0) * M;
}

double frequencySensitiveLoad::getRealPower (const double V) const
{
    double Pr = subLoad->getRealPower (V);
    double freq = bus->getFreq ();
    return Pr + Pr * (freq - 1.0) * M;
}

double frequencySensitiveLoad::getReactivePower (double V) const
{
    double Qr = subLoad->getReactivePower (V);
    double freq = bus->getFreq ();
    return Qr + Qr * (freq - 1.0) * M;
}

void frequencySensitiveLoad::outputPartialDerivatives (const IOdata &inputs,
                                                       const stateData &sD,
                                                       matrixData<double> &md,
                                                       const solverMode &sMode)
{
    if (inputs.empty ())  // we only have output derivatives if the input arguments are not counted
    {
        auto argsBus = bus->getOutputs (noInputs, sD, sMode);
        auto inputLocs = bus->getOutputLocs (sMode);
        ioPartialDerivatives (argsBus, sD, md, inputLocs, sMode);
    }
}

count_t frequencySensitiveLoad::outputDependencyCount (index_t num, const solverMode &sMode) const
{
    return subLoad->outputDependencyCount (num, sMode);
}

void frequencySensitiveLoad::ioPartialDerivatives (const IOdata & /*inputs*/,
                                                   const stateData & /*sD*/,
                                                   matrixData<double> & /*md*/,
                                                   const IOlocs & /*inputLocs*/,
                                                   const solverMode & /*sMode*/)
{
}

}  // namespace loads
}  // namespace griddyn
