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

#include "griddyn/gridDynSimulation.h"
#include "griddyn/measurement/objectGrabbers.h"
#include "griddyn_export.h"
#include "internal/griddyn_export_internal.h"
#include "runner/gridDynRunner.h"
#include "utilities/matrixDataCustomWriteOnly.hpp"

using namespace griddyn;

SolverKey gridDynSimulationGetSolverKey(GridDynSimReference sim,
                                         const char* solverType,
                                         GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return nullptr;
    }
    auto slv = runner->getSim()->getSolverMode(solverType);
    auto key = new solverKeyInfo(slv);
    return reinterpret_cast<void*>(key);
}

void gridDynSolverKeyFree(SolverKey key)
{
    auto skey = reinterpret_cast<const solverKeyInfo*>(key);
    delete skey;
}

int gridDynSimulationBusCount(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return 0;
    }
    return runner->getSim()->getInt("buscount");
}

int gridDynSimulationLineCount(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return 0;
    }
    return runner->getSim()->getInt("linkcount");
}

void gridDynSimulationGetResults(GridDynSimReference sim,
                                 const char* dataType,
                                 double* data,
                                 int maxSize,
    int *actualSize,
                                 GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    if (!runner->getSim()) {
        return 0;
    }
    std::vector<double> dataVec;
    auto fvecfunc = getObjectVectorFunction(static_cast<const Area*>(nullptr), dataType);
    if (!fvecfunc.first) {
        return 0;
    }
    fvecfunc.first(runner->getSim().get(), dataVec);
    for (int ii = 0; (ii < maxSize) && (ii < static_cast<int>(dataVec.size())); ++ii) {
        data[ii] = dataVec[ii];
    }
    return (std::min)(maxSize, static_cast<int>(dataVec.size()));
}

int gridDynSimulationStateSize(GridDynSimReference sim, SolverKey key, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        return griddyn_invalid_object;
    }
    return static_cast<int>(runner->getSim()->stateSize(sMode));
}

void gridDynSimulationGuessState(GridDynSimReference sim,
                                            double time,
                                            double* states,
                                            double* dstate_dt,
                                            SolverKey key,
                                            GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        return griddyn_invalid_object;
    }
    runner->getSim()->guessState(time, states, dstate_dt, sMode);
    return griddyn_ok;
}

void gridDynSimulationSetState(GridDynSimReference sim,
                                          double time,
                                          const double* states,
                                          const double* dstate_dt,
                                          SolverKey key,
                                          GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        return griddyn_invalid_object;
    }
    runner->getSim()->setState(time, states, dstate_dt, sMode);
    return griddyn_ok;
}

void gridDynSimulationGetStateVariableTypes(GridDynSimReference sim,
                                                       double* types,
                                                       SolverKey key,
                                                       GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        return griddyn_invalid_object;
    }
    runner->getSim()->getVariableType(types, sMode);
    return griddyn_ok;
}

int gridDynSimulationResidual(GridDynSimReference sim,
                               double time,
                               double* resid,
                               const double* states,
                               const double* dstate_dt,
                               SolverKey key,
                               GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        return griddyn_invalid_object;
    }
    return runner->getSim()->residualFunction(time, states, dstate_dt, resid, sMode);
}

void gridDynSimulationDerivative(GridDynSimReference sim,
                                            double time,
                                            double* deriv,
                                            const double* states,
                                            SolverKey key,
                                            GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        return griddyn_invalid_object;
    }
    return runner->getSim()->derivativeFunction(time, states, deriv, sMode);
}

void gridDynSimulationAlgebraicUpdate(GridDynSimReference sim,
                                                 double time,
                                                 double* update,
                                                 const double* states,
                                                 double alpha,
                                                 SolverKey key,
                                                 GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        return griddyn_invalid_object;
    }
    return runner->getSim()->algUpdateFunction(time, states, update, sMode, alpha);
}

void gridDynSimulationJacobian(GridDynSimReference sim,
                                          double time,
                                          const double* states,
                                          const double* dstate_dt,
                                          double cj,
                                          SolverKey key,
                                          void (*insert)(int, int, double),
                                          GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        return griddyn_invalid_object;
    }
    matrixDataCustomWriteOnly<double> md;
    md.setFunction([insert](index_t row, index_t col, double val) {
        insert(static_cast<int>(row), static_cast<int>(col), val);
    });
    return runner->getSim()->jacobianFunction(time, states, dstate_dt, md, cj, sMode);
}
