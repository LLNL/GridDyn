/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "GovernorSteamTCSR.h"

#include "../Generator.h"
#include "../gridBus.h"
#include "utilities/matrixData.hpp"
namespace griddyn {
namespace governors {
    GovernorSteamTCSR::GovernorSteamTCSR(const std::string& objName): GovernorSteamNR(objName)
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
        offsets.local().local.diffSize = 2;
        offsets.local().local.jacSize = 5;
    }

    coreObject* GovernorSteamTCSR::clone(coreObject* obj) const
    {
        GovernorSteamTCSR* gov;
        if (obj == nullptr) {
            gov = new GovernorSteamTCSR();
        } else {
            gov = dynamic_cast<GovernorSteamTCSR*>(obj);
            if (gov == nullptr) {
                coreObject::clone(obj);
                return obj;
            }
        }
        coreObject::clone(gov);
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
    GovernorSteamTCSR::~GovernorSteamTCSR() = default;

    // initial conditions
    void GovernorSteamTCSR::dynObjectInitializeB(const IOdata& inputs,
                                                 const IOdata& desiredOutput,
                                                 IOdata& /*inputSet*/)
    {
        m_state[1] = 0;
        m_state[0] = desiredOutput[PoutLocation];
        Pset = inputs[govpSetInLocation];
    }

    // residual
    void GovernorSteamTCSR::residual(const IOdata& /*inputs*/,
                                     const stateData& /*sD*/,
                                     double resid[],
                                     const solverMode& sMode)
    {
        std::cout << "GovernorSteamTCSR::residual" << std::endl;

        auto offset = offsets.getAlgOffset(sMode);
        resid[offset] = 0;
        resid[offset + 1] = 0;
    }

    void GovernorSteamTCSR::jacobianElements(const IOdata& /*inputs*/,
                                             const stateData& sD,
                                             matrixData<double>& md,
                                             const IOlocs& /*inputLocs*/,
                                             const solverMode& sMode)
    {
        if (isAlgebraicOnly(sMode)) {
            return;
        }
        auto offset = offsets.getAlgOffset(sMode);
        int refI = offset;
        // use the md.assign Macro defined in basicDefs
        // md.assign(arrayIndex, RowIndex, ColIndex, value)
        int omegaLoc = -1;

        // Pm
        if (omegaLoc >= 0) {
            md.assign(refI, omegaLoc, -K * T2 / (T1 * T3));
        }
        md.assign(refI, refI, -1 / T3 - sD.cj);
        md.assign(refI, refI + 1, -K / T3);

        // X
        if (omegaLoc >= 0) {
            md.assign(refI + 1, omegaLoc, (T1 - T2) / (T1 * T1));
        }

        md.assign(refI + 1, refI + 1, -1 / T1 - sD.cj);
    }

    index_t GovernorSteamTCSR::findIndex(const std::string& field,
                                         const solverMode& /*sMode*/) const
    {
        index_t ret = kInvalidLocation;
        if (field == "pm") {
            ret = 0;
        } else if (field == "x") {
            ret = 1;
        }
        return ret;
    }

    // set parameters
    void GovernorSteamTCSR::set(const std::string& param, const std::string& val)
    {
        coreObject::set(param, val);
    }

    void GovernorSteamTCSR::set(const std::string& param, double val, units::unit unitType)
    {
        // param   = gridDynSimulation::toLower(param);
        if (param == "k") {
            K = val;
        } else if (param == "t1") {
            T1 = val;
        } else if (param == "t2") {
            T2 = val;
        } else if (param == "t3") {
            T3 = val;
        } else if (param == "pup") {
            Pup = val;
        } else if (param == "pdown") {
            Pdown = val;
        } else if (param == "pmax") {
            Pmax = val;
        } else if (param == "pmin") {
            Pmin = val;
        } else {
            coreObject::set(param, val, unitType);
        }
    }

}  // namespace governors
}  // namespace griddyn
