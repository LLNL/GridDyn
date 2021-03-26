/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gridComponentHelperClasses.h"

#include "gridComponent.h"
#include <cstring>

namespace griddyn {

solverMode::solverMode(index_t index): offsetIndex(index)
{
    if (index == local_mode) {  // predefined local
        local = true;
        dynamic = true;
        differential = true;
        algebraic = true;
    } else if (index == power_flow) {  // predefined pflow
        algebraic = true;
        differential = false;
        dynamic = false;
    } else if (index == dae) {  // predefined dae
        dynamic = true;
        differential = true;
        algebraic = true;
    } else if (index == dynamic_algebraic) {  // predefined dynAlg
        algebraic = true;
        differential = false;
        dynamic = true;
    } else if (index == dynamic_differential) {  // predefined dynDiff
        algebraic = false;
        differential = true;
        dynamic = true;
    }
}

void stateSizes::reset()
{
    std::cout << "stateSizes::rest()" << std::endl;
    std::memset(this, 0, sizeof(stateSizes));
}
void stateSizes::stateReset()
{
    std::cout << "stateSizes::stateReset()" << std::endl;
    vSize = aSize = algSize = diffSize = 0;
}
void stateSizes::add(const stateSizes& arg)
{
    vSize += arg.vSize;
    aSize += arg.aSize;
    algSize += arg.algSize;
    diffSize += arg.diffSize;
    algRoots += arg.algRoots;
    diffRoots += arg.diffRoots;
    jacSize += arg.jacSize;
    std::cout << "stateSizes::add algRoots = " << algRoots << std::endl;
}

void stateSizes::addStateSizes(const stateSizes& arg)
{
    vSize += arg.vSize;
    aSize += arg.aSize;
    algSize += arg.algSize;
    diffSize += arg.diffSize;
}

void stateSizes::addRootSizes(const stateSizes& arg)
{
    algRoots += arg.algRoots;
    diffRoots += arg.diffRoots;
    std::cout << "stateSizes::addRootSizes algRoots = " << algRoots << std::endl;
}

void stateSizes::addJacobianSizes(const stateSizes& arg)
{
    jacSize += arg.jacSize;
}

count_t stateSizes::totalSize() const
{
    return vSize + aSize + algSize + diffSize;
}

void solverOffsets::reset()
{
    std::cout << "sovlerOffsets::reset()" << std::endl;
    diffOffset = aOffset = vOffset = algOffset = rootOffset = kNullLocation;
    local.reset();
    total.reset();

    rootsLoaded = jacobianLoaded = stateLoaded = offetLoaded = false;
}

void solverOffsets::stateReset()
{
    local.stateReset();
    total.stateReset();
    diffOffset = aOffset = vOffset = algOffset = kNullLocation;
    stateLoaded = false;
}

void solverOffsets::rootCountReset()
{
    std::cout << "sovlerOffsets::rootCountReset()" << std::endl;
    rootOffset = kNullLocation;
    local.rootReset();
    total.rootReset();

    rootsLoaded = false;
}

void solverOffsets::JacobianCountReset()
{
    local.JacobianReset();
    total.JacobianReset();

    jacobianLoaded = false;
}

void solverOffsets::increment()
{
    count_t algExtra = 0;
    if (aOffset != kNullLocation) {
        aOffset += total.aSize;
    } else {
        algExtra = total.aSize;
    }
    if (vOffset != kNullLocation) {
        vOffset += total.vSize;
    } else {
        algExtra += total.vSize;
    }

    algOffset += total.algSize + algExtra;

    if (diffOffset != kNullLocation) {
        diffOffset += total.diffSize;
    } else {
        algOffset += total.diffSize;
    }
    if (rootOffset != kNullLocation) {
        std::cout << "solverOffsets::increment 1 total.algRoots = " << total.algRoots << std::endl;
        rootOffset += total.algRoots + total.diffRoots;
    }
}

void solverOffsets::increment(const solverOffsets& offsets)
{
    count_t algExtra = 0;
    if (aOffset != kNullLocation) {
        aOffset += offsets.total.aSize;
    } else {
        algExtra = offsets.total.aSize;
    }
    if (vOffset != kNullLocation) {
        vOffset += offsets.total.vSize;
    } else {
        algExtra += offsets.total.vSize;
    }

    algOffset += offsets.total.algSize + algExtra;

    if (diffOffset != kNullLocation) {
        diffOffset += offsets.total.diffSize;
    } else {
        algOffset += offsets.total.diffSize;
    }
    if (rootOffset != kNullLocation) {
        std::cout << "solverOffsets::increment 2 total.algRoots = " << total.algRoots << std::endl;
        rootOffset += offsets.total.algRoots + offsets.total.diffRoots;
    }
}

void solverOffsets::localIncrement(const solverOffsets& offsets)
{
    count_t algExtra = 0;
    if (aOffset != kNullLocation) {
        aOffset += offsets.local.aSize;
    } else {
        algExtra = offsets.local.aSize;
    }
    if (vOffset != kNullLocation) {
        vOffset += offsets.local.vSize;
    } else {
        algExtra += offsets.local.vSize;
    }

    algOffset += offsets.local.algSize + algExtra;

    if (diffOffset != kNullLocation) {
        diffOffset += offsets.local.diffSize;
    } else {
        algOffset += offsets.local.diffSize;
    }
    if (rootOffset != kNullLocation) {
        std::cout << "solverOffsets::localincrement total.algRoots = " << total.algRoots << std::endl;
        rootOffset += offsets.local.algRoots + local.diffRoots;
    }
}

void solverOffsets::addSizes(const solverOffsets& offsets)
{
    total.add(offsets.total);
}
void solverOffsets::addStateSizes(const solverOffsets& offsets)
{
    total.addStateSizes(offsets.total);
}
void solverOffsets::addJacobianSizes(const solverOffsets& offsets)
{
    total.addJacobianSizes(offsets.total);
}

void solverOffsets::addRootSizes(const solverOffsets& offsets)
{
    std::cout << "sovlerOffsets::addRootSizes()" << std::endl;
    total.addRootSizes(offsets.total);
}

void solverOffsets::localStateLoad(bool finishedLoading)
{
    total.algSize = local.algSize;
    total.diffSize = local.diffSize;
    total.aSize = local.aSize;
    total.vSize = local.vSize;
    stateLoaded = finishedLoading;
}

void solverOffsets::localLoadAll(bool finishedLoading)
{
    total = local;
    stateLoaded = finishedLoading;
    jacobianLoaded = finishedLoading;
    rootsLoaded = finishedLoading;
    std::cout << "sovlerOffsets::localLoadAll()" << std::endl;
}

void solverOffsets::setOffsets(const solverOffsets& newOffsets)
{
    algOffset = newOffsets.algOffset;
    diffOffset = newOffsets.diffOffset;

    if (total.aSize > 0) {
        if (newOffsets.aOffset != kNullLocation) {
            aOffset = newOffsets.aOffset;
        } else {
            aOffset = algOffset;
            algOffset += total.aSize;
        }
    } else {
        aOffset = kNullLocation;
    }

    if (total.vSize > 0) {
        if (newOffsets.vOffset != kNullLocation) {
            vOffset = newOffsets.vOffset;
        } else {
            vOffset = algOffset;
            algOffset += total.vSize;
        }
    } else {
        vOffset = kNullLocation;
    }

    if (diffOffset == kNullLocation) {
        diffOffset = algOffset + total.algSize;
    }
}

void solverOffsets::setOffset(index_t newOffset)
{
    aOffset = newOffset;
    vOffset = aOffset + total.aSize;
    algOffset = vOffset + total.vSize;
    diffOffset = algOffset + total.algSize;
    if (total.aSize == 0) {
        aOffset = kNullLocation;
    }
    if (total.vSize == 0) {
        vOffset = kNullLocation;
    }
}

}  // namespace griddyn
