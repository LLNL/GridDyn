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

#include "blockSource.h"
#include "Block.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include <algorithm>
namespace griddyn
{
namespace sources
{
blockSource::blockSource (const std::string &objName) : Source (objName) {}
coreObject *blockSource::clone (coreObject *obj) const
{
    auto blkSrc = cloneBase<blockSource, Source> (this, obj);
	if (blkSrc == nullptr)
	{
		return obj;
	}
    blkSrc->maxStepSize = maxStepSize;
    return blkSrc;
}

void blockSource::add (coreObject *obj)
{
    if (dynamic_cast<Block *> (obj) != nullptr)
    {
        if (blk != nullptr)
        {
            gridComponent::remove (blk);
        }
        blk = static_cast<Block *> (obj);
        addSubObject (blk);
    }
    if (dynamic_cast<Source *> (obj) != nullptr)
    {
        if (src != nullptr)
        {
            gridComponent::remove (src);
        }
        src = static_cast<Source *> (obj);
        addSubObject (src);
    }
    else
    {
        coreObject::add (obj);  // just pass it to the core to do the appropriate thing(probably throw an
        // exception)
    }
}

void blockSource::remove (coreObject *obj)
{
    if (isSameObject (src, obj))
    {
        gridComponent::remove (obj);
        src = nullptr;
        return;
    }

    if (isSameObject (blk, obj))
    {
        gridComponent::remove (obj);
        blk = nullptr;
        return;
    }
}


void blockSource::dynObjectInitializeB (const IOdata & /*inputs*/, const IOdata &desiredOutput, IOdata &fieldSet)
{
    if (desiredOutput.empty ())
    {
        if (src != nullptr)
        {
            src->dynInitializeB (noInputs, noInputs, fieldSet);
        }
        if (blk != nullptr)
        {
            blk->dynInitializeB (fieldSet, noInputs, fieldSet);
        }
        m_output = fieldSet[0];
    }
    else
    {
        m_output = desiredOutput[0];
        if (blk != nullptr)
        {
            blk->dynInitializeB (noInputs, desiredOutput, fieldSet);
        }
        if (src != nullptr)
        {
            src->dynInitializeB (noInputs, fieldSet, fieldSet);
        }
    }
    if (maxStepSize > kBigNum / 2.0)
    {
        if (blk != nullptr)
        {
            maxStepSize = blk->get ("maxstepsize");
        }
    }
}

void blockSource::setFlag (const std::string &flag, bool val)
{
    if (subObjectSet (flag, val))
    {
        return;
    }

    try
    {
        Source::setFlag (flag, val);
    }
    catch (const unrecognizedParameter &)
    {
        if (src != nullptr)
        {
            src->setFlag (flag, val);
        }
    }
}

void blockSource::set (const std::string &param, const std::string &val)
{
    if (subObjectSet (param, val))
    {
        return;
    }

    try
    {
        Source::set (param, val);
    }
    catch (const unrecognizedParameter &)
    {
        if (src != nullptr)
        {
            src->set (param, val);
        }
    }
}
void blockSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if (subObjectSet (param, val, unitType))
    {
        return;
    }
    if (param == "maxstepsize")
    {
        maxStepSize = val;
    }
    else
    {
        try
        {
            Source::set (param, val, unitType);
        }
        catch (const unrecognizedParameter &)
        {
            if (src != nullptr)
            {
                src->set (param, val, unitType);
            }
        }
    }
}
double blockSource::get (const std::string &param, gridUnits::units_t unitType) const
{
    double rval = Source::get (param, unitType);
    if (rval == kNullVal)
    {
        if (src != nullptr)
        {
            return src->get (param, unitType);
        }
    }
    return rval;
}

// void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode);

void blockSource::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
    double srcOut = m_output;
    double srcDout = 0.0;
    if (src != nullptr)
    {
        src->residual (inputs, sD, resid, sMode);
        srcOut = src->getOutput (inputs, sD, sMode, 0);
        srcDout = src->getDoutdt (inputs, sD, sMode, 0);
    }
    if (blk != nullptr)
    {
        blk->residElements (srcOut, srcDout, sD, resid, sMode);
    }
}

void blockSource::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
    double srcOut = m_output;
    double srcDout = 0.0;
    if (src != nullptr)
    {
        src->derivative (inputs, sD, deriv, sMode);
        srcOut = src->getOutput (inputs, sD, sMode, 0);
        srcDout = src->getDoutdt (inputs, sD, sMode, 0);
    }
    if (blk != nullptr)
    {
        blk->derivElements (srcOut, srcDout, sD, deriv, sMode);
    }
}

void blockSource::algebraicUpdate (const IOdata &inputs,
                                   const stateData &sD,
                                   double update[],
                                   const solverMode &sMode,
                                   double alpha)
{
    double srcOut = m_output;
    if (src != nullptr)
    {
        src->algebraicUpdate (inputs, sD, update, sMode, alpha);
        srcOut = src->getOutput (inputs, sD, sMode, 0);
    }
    if (blk != nullptr)
    {
        blk->algElements (srcOut, sD, update, sMode);
    }
}

void blockSource::jacobianElements (const IOdata &inputs,
                                    const stateData &sD,
                                    matrixData<double> &md,
                                    const IOlocs &inputLocs,
                                    const solverMode &sMode)
{
    double srcOut = m_output;
    double srcDout = 0.0;
    index_t srcLoc = kNullLocation;
    if (src != nullptr)
    {
        src->jacobianElements (inputs, sD, md, inputLocs, sMode);
        srcOut = src->getOutput (inputs, sD, sMode, 0);
        srcDout = src->getDoutdt (inputs, sD, sMode, 0);
        srcLoc = src->getOutputLoc (sMode, 0);
    }
    if (blk != nullptr)
    {
        blk->jacElements (srcOut, srcDout, sD, md, srcLoc, sMode);
    }
}

void blockSource::timestep (coreTime time, const IOdata &inputs, const solverMode &sMode)
{
    while (prevTime < time)
    {
        auto ntime = std::min (prevTime + maxStepSize, time);
        double srcOut = m_output;
        if (src != nullptr)
        {
            src->timestep (ntime, inputs, sMode);
            srcOut = getOutput (0);
        }
        if (blk != nullptr)
        {
            blk->step (ntime, srcOut);
        }
    }
}

void blockSource::rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)
{
    double srcOut = m_output;
    double srcDout = 0.0;
    if (src != nullptr)
    {
        src->rootTest (inputs, sD, roots, sMode);
        srcOut = src->getOutput (inputs, sD, sMode, 0);
        srcDout = src->getDoutdt (inputs, sD, sMode, 0);
    }
    if (blk != nullptr)
    {
        blk->rootTest ({srcOut, srcDout}, sD, roots, sMode);
    }
}
void blockSource::rootTrigger (coreTime time,
                               const IOdata &inputs,
                               const std::vector<int> &rootMask,
                               const solverMode &sMode)
{
    double srcOut = m_output;
    double srcDout = 0.0;
    if (src != nullptr)
    {
        src->rootTrigger (time, inputs, rootMask, sMode);
        srcOut = src->getOutput (0);
        srcDout = src->getDoutdt (noInputs, emptyStateData, sMode, 0);
    }
    if (blk != nullptr)
    {
        blk->rootTrigger (time, {srcOut, srcDout}, rootMask, sMode);
    }
}

change_code
blockSource::rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level)
{
    double srcOut = m_output;
    double srcDout = 0.0;
    change_code ret = change_code::no_change;
    if (src != nullptr)
    {
        auto iret = src->rootCheck (inputs, sD, sMode, level);
        srcOut = src->getOutput (inputs, sD, sMode, 0);
        srcDout = src->getDoutdt (inputs, sD, sMode, 0);
        ret = (std::max) (iret, ret);
    }
    if (blk != nullptr)
    {
        auto iret = blk->rootCheck ({srcOut, srcDout}, sD, sMode, level);
        ret = (std::max) (iret, ret);
    }
    return ret;
}

void blockSource::updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
    double srcOut = m_output;
    double srcDout = 0.0;
    if (src != nullptr)
    {
        src->updateLocalCache (inputs, sD, sMode);
        srcOut = src->getOutput (inputs, sD, sMode, 0);
        srcDout = src->getDoutdt (inputs, sD, sMode, 0);
    }
    if (blk != nullptr)
    {
        blk->updateLocalCache ({srcOut, srcDout}, sD, sMode);
    }
}

/** set the output level
@param[in] newLevel the level to set the output at
*/
void blockSource::setLevel (double newLevel)
{
    if (src != nullptr)
    {
        src->setLevel (newLevel);
    }
}

IOdata blockSource::getOutputs (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode) const
{
    if (blk != nullptr)
    {
        return blk->getOutputs (noInputs, sD, sMode);
    }
    if (src != nullptr)
    {
        return src->getOutputs (noInputs, sD, sMode);
    }
    return Source::getOutputs (noInputs, sD, sMode);
}

double
blockSource::getOutput (const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num) const
{
    if (blk != nullptr)
    {
        return blk->getOutput (noInputs, sD, sMode, num);
    }
    if (src != nullptr)
    {
        return src->getOutput (inputs, sD, sMode, num);
    }
    return Source::getOutput (inputs, sD, sMode, num);
}

double blockSource::getOutput (index_t num) const
{
    if (blk != nullptr)
    {
        return blk->getOutput (num);
    }
    if (src != nullptr)
    {
        return src->getOutput (num);
    }

    return Source::getOutput (num);
}

double
blockSource::getDoutdt (const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num) const
{
    if (blk != nullptr)
    {
        return blk->getDoutdt (noInputs, sD, sMode, num);
    }
    if (src != nullptr)
    {
        return src->getDoutdt (inputs, sD, sMode, num);
    }

    return Source::getDoutdt (inputs, sD, sMode, num);
}

coreObject *blockSource::find (const std::string &object) const
{
    if (object == "source")
    {
        return src;
    }
    if (object == "block")
    {
        return blk;
    }
    return gridComponent::find (object);
}

coreObject *blockSource::getSubObject (const std::string &typeName, index_t num) const
{
    if (typeName == "source")
    {
        return (num == 0) ? src : nullptr;
    }
    if (typeName == "block")
    {
        return (num == 0) ? blk : nullptr;
    }

    return gridComponent::getSubObject (typeName, num);
}
}  // namespace sources
}  // namespace griddyn