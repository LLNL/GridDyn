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

#include "isocController.h"
#include "Generator.h"

#include "core/coreExceptions.h"
#include "core/objectFactoryTemplates.hpp"
#include "utilities/vectorOps.hpp"

#include <algorithm>

namespace griddyn
{
isocController::isocController (const std::string &objName) : gridSubModel (objName) {}
coreObject *isocController::clone (coreObject *obj) const
{
    isocController *nobj;
    if (obj == nullptr)
    {
        nobj = new isocController ();
    }
    else
    {
        nobj = dynamic_cast<isocController *> (obj);
        if (nobj == nullptr)
        {
            // if we can't cast the pointer clone at the next lower m_output
            coreObject::clone (obj);
            return obj;
        }
    }
    coreObject::clone (nobj);
    nobj->db = db;
    nobj->upStep = upStep;
    nobj->downStep = downStep;
    nobj->upPeriod = upPeriod;
    nobj->downPeriod = downPeriod;
    nobj->maxLevel = maxLevel;
    nobj->minLevel = minLevel;
    nobj->integralTrigger = integralTrigger;
    return nobj;
}

void isocController::dynObjectInitializeA (coreTime /*time0*/, std::uint32_t /*flags*/)
{
    gen = dynamic_cast<Generator *> (getParent ());
    updatePeriod = upPeriod;
    integratorLevel = 0;
}

void isocController::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
    if (!inputs.empty ())
    {
        lastFreq = inputs[0];
        if (lastFreq < -db)
        {
            updatePeriod = downPeriod;
        }
    }
    if (!desiredOutput.empty ())
    {
        m_output = desiredOutput[0];
        fieldSet[0] = 0;
    }
    else
    {
        fieldSet[0] = m_output;
    }
}

void isocController::setLimits (double maxV, double minV)
{
    minLevel = std::min (maxV, minV);
    maxLevel = std::max (maxV, minV);
    m_output = valLimit (m_output, minLevel, maxLevel);
}

void isocController::updateA (coreTime time)
{
    if (time < nextUpdateTime)
    {
        assert (false);
        return;
    }
    integratorLevel += lastFreq * updatePeriod;
    if (lastFreq > db)
    {
        m_output += upStep;
        updatePeriod = upPeriod;
    }
    else if (lastFreq < -db)
    {
        m_output += downStep;
        updatePeriod = downPeriod;
    }
    else
    {
        updatePeriod = upPeriod;
        if (integratorLevel > integralTrigger)
        {
            m_output += upStep;
        }
        else if (integratorLevel < -integralTrigger)
        {
            m_output += downStep;
        }
    }
    m_output = valLimit (m_output, minLevel, maxLevel);
    lastUpdateTime = time;
    // printf("t=%f,output=%f\n", time, m_output);
}

void isocController::timestep (coreTime time, const IOdata &inputs, const solverMode & /*sMode*/)
{
    prevTime = time;
    lastFreq = inputs[0];
    while (nextUpdateTime <= time)
    {
        updateA (time);
        updateB ();
    }
}

void isocController::set (const std::string &param, const std::string &val) { gridSubModel::set (param, val); }
void isocController::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "deadband") || (param == "db"))
    {
        db = val;
    }
    else if (param == "upstep")
    {
        upStep = val;
    }
    else if (param == "downstep")
    {
        downStep = val;
    }
    else if (param == "upperiod")
    {
        upPeriod = val;
    }
    else if (param == "downperiod")
    {
        downPeriod = val;
    }
    else if ((param == "max") || (param == "maxlevel"))
    {
        maxLevel = val;
    }
    else if ((param == "min") || (param == "minLevel"))
    {
        minLevel = val;
    }
    else if (param == "m_output")
    {
        m_output = val;
    }

    else
    {
        gridSubModel::set (param, val, unitType);
    }
}

void isocController::setLevel (double newLevel) { m_output = valLimit (newLevel, minLevel, maxLevel); }
void isocController::setFreq (double freq) { lastFreq = freq; }
void isocController::deactivate ()
{
    m_output = 0;
    nextUpdateTime = maxTime;
}

void isocController::activate (coreTime time) { nextUpdateTime = time + upPeriod; }
}  // namespace griddyn
