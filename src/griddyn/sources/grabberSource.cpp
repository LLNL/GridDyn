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

#include "grabberSource.h"
#include "core/objectInterpreter.h"
#include "../measurement/grabberSet.h"
#include "core/coreObjectTemplates.hpp"

namespace griddyn
{
namespace sources
{
grabberSource::grabberSource (const std::string &objName) : rampSource (objName) {}
grabberSource::~grabberSource () = default;

coreObject *grabberSource::clone (coreObject *obj) const 
{ 
	auto src = cloneBase<grabberSource, Source>(this, obj);
	if (src == nullptr)
	{
		return obj;
	}
	src->updateTarget(target);
	src->updateField(field);
	src->set("gain", multiplier);
	if ((gset)&&(!src->gset))
	{
		src->pFlowInitializeA(prevTime, 0);
	}
	return src;
}

void grabberSource::setFlag (const std::string &flag, bool val) 
{
	if (flag.empty())
	{

	}
	else
	{
		Source::setFlag(flag, val);
	}
}
void grabberSource::set (const std::string &param, const std::string &val)
{
    if (param == "field")
    {
        if (opFlags[dyn_initialized])
        {
            updateField (val);
        }
        else
        {
            field = val;
        }
    }
    else if (param == "target")
    {
        if (opFlags[dyn_initialized])
        {
            updateTarget (target);
        }
        else
        {
            target = val;
        }
    }
    else
    {
        Source::set (param, val);
    }
}

void grabberSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	if ((param == "gain")||(param=="multipler"))
	{
		multiplier = val;
		if (gset)
		{
			gset->setGain(multiplier);
		}
	}
	else
	{
		Source::set(param, val, unitType);
	}
   
}

double grabberSource::get (const std::string &param, gridUnits::units_t unitType) const
{
	if (param == "multiplier")
	{
		return multiplier;
	}
    return Source::get (param, unitType);
}

void grabberSource::pFlowObjectInitializeA (coreTime /*time0*/, std::uint32_t /*flags*/)
{
    coreObject *obj = locateObject (target, this);
    gset = std::make_unique<grabberSet> (field, obj);
	gset->setGain(multiplier);
}

void grabberSource::dynObjectInitializeB (const IOdata & /*inputs*/, const IOdata & /*desiredOutput*/, IOdata &fieldSet) 
{
	fieldSet.resize(1);
	fieldSet[0] = gset->grabData();
}
IOdata grabberSource::getOutputs (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode) const
{
    return {gset->grabData (sD, sMode)};
}
double grabberSource::getOutput (const IOdata & /*inputs*/,
                                 const stateData &sD,
                                 const solverMode &sMode,
                                 index_t outputNum) const
{
    if (outputNum == 0)
    {
        return gset->grabData (sD, sMode);
    }
    return kNullVal;
}

double grabberSource::getOutput (index_t outputNum) const
{
    if (outputNum == 0)
    {
        return gset->grabData ();
    }
    return kNullVal;
}

double grabberSource::getDoutdt (const IOdata & /*inputs*/,
                                 const stateData & /*sD*/,
                                 const solverMode &/*sMode*/,
                                 index_t /*outputNum*/) const
{
    return 0.0;
}

void grabberSource::updateField (const std::string &newField)
{
    if (gset)
    {
        gset->updateField (newField);
    }
    field = newField;
}

void grabberSource::updateTarget (const std::string &newTarget)
{
    if (gset)
    {
        auto obj = locateObject (newTarget, this);
        gset->updateObject (obj);
    }
    target = newTarget;
}

void grabberSource::updateTarget (coreObject *obj)
{
    if (gset)
    {
        gset->updateObject (obj);
    }
    target = obj->getName ();
}
}  // namespace sources
}  // namespace griddyn