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

#include "../Stabilizer.h"
#include "../Generator.h"
#include "../gridBus.h"
#include "core/coreObjectTemplates.hpp"
#include "core/objectFactoryTemplates.hpp"
#include <cmath>

namespace griddyn
{
static const typeFactory<Stabilizer> gf ("pss", stringVec{"basic"});

Stabilizer::Stabilizer (const std::string &objName) : gridSubModel (objName) {}
coreObject *Stabilizer::clone (coreObject *obj) const
{
    auto pss = cloneBase<Stabilizer, gridSubModel> (this, obj);
    if (pss == nullptr)
    {
        return obj;
    }

    return pss;
}

// destructor
Stabilizer::~Stabilizer () = default;
// initial conditions
void Stabilizer::dynObjectInitializeB (const IOdata & /*inputs*/,
                                       const IOdata & /*desiredOutput*/,
                                       IOdata & /*fieldSet*/)
{
}

// residual
void Stabilizer::residual (const IOdata & /*inputs*/,
                           const stateData & /*sD*/,
                           double /*resid*/[],
                           const solverMode & /*sMode*/)
{
}

index_t Stabilizer::findIndex (const std::string & /*field*/, const solverMode & /*sMode*/) const
{
    return kInvalidLocation;
}

void Stabilizer::set (const std::string &param, const std::string &val) { return coreObject::set (param, val); }
// set parameters
void Stabilizer::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    {
        coreObject::set (param, val, unitType);
    }
}

void Stabilizer::jacobianElements (const IOdata & /*inputs*/,
                                   const stateData & /*sD*/,
                                   matrixData<double> & /*md*/,
                                   const IOlocs & /*inputLocs*/,
                                   const solverMode &sMode)
{
    if (isAlgebraicOnly (sMode))
    {
        return;
    }
}

void Stabilizer::derivative (const IOdata & /*inputs*/,
                             const stateData & /*sD*/,
                             double /*deriv*/[],
                             const solverMode & /*sMode*/)
{
}

}  // namespace griddyn
