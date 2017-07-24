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

#include "gridSecondary.h"
#include "core/coreObjectTemplates.hpp"
#include "core/objectInterpreter.h"
#include "gridBus.h"
#include "gridSubModel.h"
#include "utilities/stringOps.h"
#include <cstdio>
#include <iostream>

namespace griddyn
{
static gridBus defBus (1.0, 0);

gridSecondary::gridSecondary (const std::string &objName) : gridComponent (objName), bus (&defBus)
{
    m_outputSize = 2;
    m_inputSize = 3;
}

coreObject *gridSecondary::clone (coreObject *obj) const
{
    auto nobj = cloneBase<gridSecondary, gridComponent> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    nobj->baseVoltage = baseVoltage;
    nobj->bus = bus;
    return nobj;
}

void gridSecondary::updateObjectLinkages (coreObject *newRoot)
{
    if (opFlags[pFlow_initialized])
    {
        auto nobj = findMatchingObject (bus, newRoot);
        if (dynamic_cast<gridBus *> (nobj) != nullptr)
        {
            bus = static_cast<gridBus *> (nobj);
        }
    }
    gridComponent::updateObjectLinkages (newRoot);
}

void gridSecondary::pFlowInitializeA (coreTime time0, std::uint32_t flags)
{
    bus = static_cast<gridBus *> (getParent ()->find ("bus"));
    if (bus == nullptr)
    {
        bus = &defBus;
    }
    gridComponent::pFlowInitializeA (time0, flags);
}

void gridSecondary::pFlowInitializeB () { gridComponent::pFlowInitializeB (); }
void gridSecondary::dynInitializeA (coreTime time0, std::uint32_t flags)
{
    gridComponent::dynInitializeA (time0, flags);
}

void gridSecondary::dynInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
    if (isEnabled ())
    {
        auto ns = stateSize (cLocalSolverMode);
        m_state.resize (ns, 0);
        m_dstate_dt.clear ();
        m_dstate_dt.resize (ns, 0);
        dynObjectInitializeB (inputs, desiredOutput, fieldSet);
        if (updatePeriod < maxTime)
        {
            setUpdateTime (prevTime + updatePeriod);
            enable_updates ();
            alert (this, UPDATE_REQUIRED);
        }
        opFlags.set (dyn_initialized);
    }
}

void gridSecondary::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    if (!getSubObjects ().empty ())
    {
        for (auto &subobj : getSubObjects ())
        {
            if (dynamic_cast<gridSubModel *> (subobj) != nullptr)
            {
                if (subobj->checkFlag (pflow_init_required))
                {
                    subobj->pFlowInitializeA (time0, flags);
                }
            }
            else
            {
                subobj->pFlowInitializeA (time0, flags);
            }
        }
    }
    prevTime = time0;
}

void gridSecondary::set (const std::string &param, const std::string &val) { gridComponent::set (param, val); }
void gridSecondary::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "basevoltage") || (param=="vbase")||(param=="voltagebase")||(param == "basev") || (param == "bv") || (param == "base voltage"))
    {
        baseVoltage = gridUnits::unitConversion (val, unitType, gridUnits::kV);
    }
    else
    {
        gridComponent::set (param, val, unitType);
    }
}

double gridSecondary::getRealPower (const IOdata & /*inputs*/,
                                    const stateData & /*sD*/,
                                    const solverMode & /*sMode*/) const
{
    return 0.0;
}

double gridSecondary::getReactivePower (const IOdata & /*inputs*/,
                                        const stateData & /*sD*/,
                                        const solverMode & /*sMode*/) const
{
    return 0.0;
}

double gridSecondary::getRealPower () const { return 0.0; }
double gridSecondary::getReactivePower () const { return 0.0; }
double gridSecondary::getAdjustableCapacityUp (coreTime /*time*/) const { return 0.0; }
double gridSecondary::getAdjustableCapacityDown (coreTime /*time*/) const { return 0.0; }
double gridSecondary::getDoutdt (const IOdata & /*inputs*/,
                                 const stateData & /*sD*/,
                                 const solverMode & /*sMode*/,
                                 index_t /*outputNum*/) const
{
    return 0.0;
}

double gridSecondary::getOutput (const IOdata &inputs,
                                 const stateData &sD,
                                 const solverMode &sMode,
                                 index_t outputNum) const
{
    if (outputNum == PoutLocation)
    {
        return getRealPower (inputs, sD, sMode);
    }
    if (outputNum == QoutLocation)
    {
        return getReactivePower (inputs, sD, sMode);
    }
    return kNullVal;
}

double gridSecondary::getOutput (index_t outputNum) const
{
    if (outputNum == PoutLocation)
    {
        return getRealPower ();
    }
    if (outputNum == QoutLocation)
    {
        return getReactivePower ();
    }
    return kNullVal;
}

IOdata gridSecondary::getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
    IOdata out (2);
    out[PoutLocation] = getRealPower (inputs, sD, sMode);
    out[QoutLocation] = getReactivePower (inputs, sD, sMode);
    return out;
}

IOdata gridSecondary::predictOutputs (coreTime /*predictionTime*/,
                                      const IOdata &inputs,
                                      const stateData &sD,
                                      const solverMode &sMode) const
{
    IOdata out (2);
    out[PoutLocation] = getRealPower (inputs, sD, sMode);
    out[QoutLocation] = getReactivePower (inputs, sD, sMode);
    return out;
}


static const std::vector<stringVec> inputNamesStr
{
	{ "voltage","v","volt" },
	{ "angle","theta","ang","a" },
	{ "frequency","freq","f","omega" },
};

const std::vector<stringVec> &gridSecondary::inputNames() const
{
	return inputNamesStr;
}

static const std::vector<stringVec> outputNamesStr
{
	{ "p","power","realpower","real"},
	{ "q","reactive","reactivepower" },
};

const std::vector<stringVec> &gridSecondary::outputNames() const
{
	return outputNamesStr;
}

gridUnits::units_t gridSecondary::inputUnits(index_t inputNum) const
{ 
	switch (inputNum)
	{
	case voltageInLocation:
		return gridUnits::puV;
	case angleInLocation:
		return gridUnits::rad;
	case frequencyInLocation:
		return gridUnits::puHz;
	default:
		return gridUnits::defUnit;
	}
	
}


gridUnits::units_t gridSecondary::outputUnits(index_t outputNum) const
{ 
	switch (outputNum)
	{
	case PoutLocation:
		return gridUnits::puMW;
	case QoutLocation:
		return gridUnits::puMW;
	
	default:
		return gridUnits::defUnit;
	}
}

}  // namespace griddyn
