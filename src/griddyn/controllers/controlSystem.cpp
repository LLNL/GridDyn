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

#include "controlSystem.h"
#include "../Block.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"

namespace griddyn
{
controlSystem::controlSystem (const std::string &objName) : gridSubModel (objName) {}

controlSystem::~controlSystem () = default;

coreObject *controlSystem::clone (coreObject *obj) const
{
    auto *cs = cloneBase<controlSystem, gridSubModel> (this, obj);
    if (cs == nullptr)
    {
        return obj;
    }
    return cs;
}

void controlSystem::add (coreObject *obj)
{
    if (dynamic_cast<Block *> (obj) != nullptr)
    {
        add (static_cast<Block *> (obj));
    }
    else
    {
        throw (unrecognizedObjectException (this));
    }
}

void controlSystem::add (Block *blk)
{
    blocks.push_back (blk);
    blk->locIndex = static_cast<index_t> (blocks.size ()) - 1;
    addSubObject (blk);
}

void controlSystem::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    for (auto &bb : blocks)
    {
        bb->dynInitializeA (time0, flags);
    }
}
void controlSystem::dynObjectInitializeB (const IOdata & /*inputs*/,
                                          const IOdata & /*desiredOutput*/,
                                          IOdata & /*inputSet*/)
{
}

void controlSystem::set (const std::string &param, const std::string &val)
{
    if (param[0] == '#')
    {
    }
    else
    {
        gridSubModel::set (param, val);
    }
}

void controlSystem::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if (param[0] == '#')
    {
    }
    else
    {
        gridSubModel::set (param, val, unitType);
    }
}

index_t controlSystem::findIndex (const std::string & /*field*/, const solverMode & /*sMode*/) const
{
    return kInvalidLocation;
}

void controlSystem::residual (const IOdata & /*inputs*/,
                              const stateData & /*sD*/,
                              double /*resid*/[],
                              const solverMode & /*sMode*/)
{
}

void controlSystem::jacobianElements (const IOdata & /*inputs*/,
                                      const stateData & /*sD*/,
                                      matrixData<double> & /*md*/,
                                      const IOlocs & /*inputLocs*/,
                                      const solverMode & /*sMode*/)
{
}

void controlSystem::timestep (coreTime /*time*/, const IOdata & /*inputs*/, const solverMode & /*sMode*/) {}

void controlSystem::rootTest (const IOdata & /*inputs*/,
                              const stateData & /*sD*/,
                              double /*roots*/[],
                              const solverMode & /*sMode*/)
{
}

void controlSystem::rootTrigger (coreTime /*time*/,
                                 const IOdata & /*inputs*/,
                                 const std::vector<int> & /*rootMask*/,
                                 const solverMode & /*sMode*/)
{
}

change_code controlSystem::rootCheck (const IOdata & /*inputs*/,
                                      const stateData & /*sD*/,
                                      const solverMode & /*sMode*/,
                                      check_level_t /*level*/)
{
    return change_code::no_change;
}
// virtual void setTime(coreTime time){prevTime=time;};
}  // namespace griddyn
