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

#include "core/coreObjectTemplates.hpp"

#include "nullBlock.h"

namespace griddyn
{
namespace blocks
{
nullBlock::nullBlock (const std::string &objName) : Block (objName)
{
    opFlags[use_direct] = true;
    opFlags[no_powerflow_operations] = true;
    opFlags[no_dynamics] = true;
}

coreObject *nullBlock::clone (coreObject *obj) const
{
    auto nobj = cloneBase<nullBlock, gridSubModel> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    return nobj;
}

void nullBlock::dynObjectInitializeA (coreTime /*time0*/, std::uint32_t /*flags*/)
{
    auto &lcinfo = offsets.local ();
    lcinfo.reset ();
    offsets.unload ();  // unload all the offsets

    lcinfo.local.jacSize = 0;
    lcinfo.local.diffSize = 0;
    lcinfo.local.algSize = 0;

    if (opFlags[differential_input])
    {
        m_inputSize = 2;
    }
}

// initial conditions
void nullBlock::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
    if (desiredOutput.empty ())
    {
        assert (!inputs.empty ());
        prevInput = inputs[0];
    }
    else
    {
        prevInput = desiredOutput[0];
    }
    fieldSet.resize (1);
    fieldSet[0] = prevInput;
}

void nullBlock::timestep (coreTime time, const IOdata &inputs, const solverMode & /*sMode*/)
{
    step (time, inputs[0]);
}

static IOdata kNullVec;

double nullBlock::step (coreTime time, double input)
{
    prevTime = time;
    return input;
}

double nullBlock::getBlockOutput (const stateData &sD, const solverMode &sMode) const
{
    auto Loc = offsets.getLocations (sD, sMode, this);
    return opFlags[differential_output] ? *Loc.diffStateLoc : *Loc.algStateLoc;
}

double nullBlock::getBlockOutput () const
{
    auto offset = opFlags[differential_output] ? (offsets.getDiffOffset (cLocalSolverMode)) : 0;
    return m_state[offset];
}

double nullBlock::getBlockDoutDt (const stateData &sD, const solverMode &sMode) const
{
    if (opFlags[differential_output])
    {
        auto Loc = offsets.getLocations (sD, sMode, this);
        return *Loc.dstateLoc;
    }
    return 0.0;
}

double nullBlock::getBlockDoutDt () const { return 0.0; }

void nullBlock::blockResidual (double /*input*/,
                               double /*didt*/,
                               const stateData & /*sD*/,
                               double /*resid*/[],
                               const solverMode & /*sMode*/)
{
}

void nullBlock::blockAlgebraicUpdate (double /*input*/,
                                      const stateData & /*sD*/,
                                      double /*update*/[],
                                      const solverMode & /*sMode*/)
{
}

void nullBlock::blockDerivative (double /*input*/,
                                 double /*didt*/,
                                 const stateData & /*sD*/,
                                 double /*deriv*/[],
                                 const solverMode & /*sMode*/)
{
}

void nullBlock::blockJacobianElements (double /*input*/,
                                       double /*didt*/,
                                       const stateData & /*sD*/,
                                       matrixData<double> & /*md*/,
                                       index_t /*argLoc*/,
                                       const solverMode & /*sMode*/)
{
}

void nullBlock::rootTest (const IOdata & /*inputs*/,
                          const stateData & /*sD*/,
                          double /*roots*/[],
                          const solverMode & /*sMode*/)
{
}

change_code nullBlock::rootCheck (const IOdata & /*inputs*/,
                                  const stateData & /*sD*/,
                                  const solverMode & /*sMode*/,
                                  check_level_t /*level*/)
{
    return change_code::no_change;
}

void nullBlock::rootTrigger (coreTime /*time*/,
                             const IOdata & /*inputs*/,
                             const std::vector<int> & /*rootMask*/,
                             const solverMode & /*sMode*/)
{
}

void nullBlock::setFlag (const std::string &flag, bool val)
{
    if (flag == "differential_input")
    {
        opFlags[differential_input] = val;
        opFlags[differential_output] = val;
    }
    else
    {
        gridSubModel::setFlag (flag, val);
    }
}

// set parameters
void nullBlock::set (const std::string &param, const std::string &val) { gridSubModel::set (param, val); }
void nullBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    gridSubModel::set (param, val, unitType);
}

double nullBlock::get (const std::string &param, gridUnits::units_t unitType) const
{
    return gridSubModel::get (param, unitType);
}

}  // namespace blocks
}  // namespace griddyn