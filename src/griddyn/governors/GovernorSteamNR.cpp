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

#include "../Generator.h"
#include "../gridBus.h"
#include "GovernorSteamNR.h"
#include "utilities/matrixData.hpp"

namespace griddyn
{
namespace governors
{
GovernorSteamNR::GovernorSteamNR (const std::string &objName) : GovernorIeeeSimple (objName)
{
    // default values
    K = 0;
    T1 = 0;
    T2 = 0;
    T3 = 0;
    Pup = kBigNum;
    Pdown = kBigNum;
    Pmax = kBigNum;
    Pmin = 0;
    Pset = 0;
    offsets.local ().local.diffSize = 2;
    offsets.local ().local.jacSize = 5;
}

coreObject *GovernorSteamNR::clone (coreObject *obj) const
{
    GovernorSteamNR *gov;
    if (obj == nullptr)
    {
        gov = new GovernorSteamNR ();
    }
    else
    {
        gov = dynamic_cast<GovernorSteamNR *> (obj);
        if (gov == nullptr)
        {
            coreObject::clone (obj);
            return obj;
        }
    }
    coreObject::clone (gov);
    gov->K = K;
    gov->T1 = T1;
    gov->T2 = T2;
    gov->T3 = T3;
    gov->Pup = Pup;
    gov->Pdown = Pdown;
    gov->Pmax = Pmax;
    gov->Pmin = Pmin;
    gov->Pset = Pset;
    return gov;
}

// destructor
GovernorSteamNR::~GovernorSteamNR () {}
// initial conditions
void GovernorSteamNR::dynObjectInitializeB (const IOdata & /*inputs*/,
                                                   const IOdata &desiredOutput,
                                                   IOdata & /*inputSet*/)
{
    auto offset = offsets.getAlgOffset (cLocalSolverMode);
    m_state[offset + 1] = 0;
    m_state[offset + offset] = desiredOutput[0];

    Pset = dynamic_cast<Generator*> (getParent ())->getPset ();
}

// residual
void GovernorSteamNR::residual (const IOdata & /*inputs*/,
                                       const stateData & /*sD*/,
                                       double resid[],
                                       const solverMode &sMode)
{
    auto offset = offsets.getAlgOffset (sMode);
    resid[offset] = 0;
    resid[offset + 1] = 0;
}

void GovernorSteamNR::jacobianElements (const IOdata & /*inputs*/,
                                               const stateData &sD,
                                               matrixData<double> &md,
                                               const IOlocs & /*inputLocs*/,
                                               const solverMode &sMode)
{
    if (isAlgebraicOnly (sMode))
    {
        return;
    }
    auto offset = offsets.getAlgOffset (sMode);
    int refI = offset;
    // use the md.assign Macro defined in basicDefs
    // md.assign(arrayIndex, RowIndex, ColIndex, value)
    int omegaLoc = -1;
    int nn = 0;

    // Pm
    if (omegaLoc >= 0)
    {
        md.assign (refI, omegaLoc, -K * T2 / (T1 * T3));
        nn++;
    }
    md.assign (refI, refI, -1 / T3 - sD.cj);
    md.assign (refI, refI + 1, -K / T3);
    nn += 2;
    // X
    if (omegaLoc >= 0)
    {
        md.assign (refI + 1, omegaLoc, (T1 - T2) / (T1 * T1));
        nn++;
    }

    md.assign (refI + 1, refI + 1, -1 / T1 - sD.cj);
}

index_t GovernorSteamNR::findIndex (const std::string &field, const solverMode & /*sMode*/) const
{
    index_t ret = kInvalidLocation;
    if (field == "pm")
    {
        ret = 0;
    }
    else if (field == "x")
    {
        ret = 1;
    }
    return ret;
}

// set parameters
void GovernorSteamNR::set (const std::string &param, const std::string &val)
{
    coreObject::set (param, val);
}

void GovernorSteamNR::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    // param   = gridDynSimulation::toLower(param);

    if (param == "k")
    {
        K = val;
    }
    else if (param == "t1")
    {
        T1 = val;
    }
    else if (param == "t2")
    {
        T2 = val;
    }
    else if (param == "t3")
    {
        T3 = val;
    }
    else if (param == "pup")
    {
        Pup = val;
    }
    else if (param == "pdown")
    {
        Pdown = val;
    }
    else if (param == "pmax")
    {
        Pmax = val;
    }
    else if (param == "pmin")
    {
        Pmin = val;
    }
    else
    {
        Governor::set (param, val, unitType);
    }
}

}//namespace governors
}//namespace griddyn
