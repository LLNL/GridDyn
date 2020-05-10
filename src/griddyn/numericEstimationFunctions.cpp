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

#include "numericEstimationFunctions.h"

#include "gridComponent.h"
#include "utilities/matrixData.hpp"

namespace griddyn {
// work in progress
void numericJacobianCalculation(gridComponent* /* comp */,
                                const IOdata& inputs,
                                const stateData& sD,
                                matrixData<double>& md,
                                const IOlocs& /*inputLocs*/,
                                const solverMode& /*sMode*/)
{
    stateData sDtest = sD;
    std::vector<double> test;
    std::vector<double> testState;
    double* residTest;
    double* stateTest;
    if (sD.hasScratch()) {
        residTest = sD.scratch1;
        stateTest = sD.scratch2;
    } else {
        auto ns = md.rowLimit();
        if (ns != kCountMax) {
            test.resize(ns);
            testState.resize(ns);
        } else {
        }
        residTest = test.data();
        stateTest = testState.data();
    }
    sDtest.scratch1 = nullptr;
    sDtest.scratch2 = nullptr;
    sDtest.state = stateTest;

    IOdata testInputs = inputs;
}

void copyObjectLocalState(gridComponent* comp,
                          const double state[],
                          double newState[],
                          const solverMode& sMode)
{
    auto sts = getObjectLocalStateIndices(comp, sMode);
    for (auto st : sts) {
        newState[st] = state[st];
    }
}

std::vector<index_t> getObjectLocalStateIndices(const gridComponent* comp, const solverMode& sMode)
{
    std::vector<index_t> states;
    const auto& offsets = comp->getOffsets(sMode);
    if (hasAlgebraic(sMode)) {
        if (offsets.local.vSize > 0) {
            for (index_t ii = 0; ii < offsets.local.vSize; ++ii) {
                states.push_back(offsets.vOffset + ii);
            }
        }
        if (offsets.local.aSize > 0) {
            for (index_t ii = 0; ii < offsets.local.aSize; ++ii) {
                states.push_back(offsets.aOffset + ii);
            }
        }
        if (offsets.local.algSize > 0) {
            for (index_t ii = 0; ii < offsets.local.algSize; ++ii) {
                states.push_back(offsets.algOffset + ii);
            }
        }
    }
    if (hasDifferential(sMode)) {
        if (offsets.local.diffSize > 0) {
            for (index_t ii = 0; ii < offsets.local.diffSize; ++ii) {
                states.push_back(offsets.diffOffset + ii);
            }
        }
    }
    return states;
}

}  // namespace griddyn
