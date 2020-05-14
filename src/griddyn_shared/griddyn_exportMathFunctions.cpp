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

static constexpr char invalidSimulation[] = "the simulation object is not valid";
static constexpr char invalidSolver[] = "the given solver key was not valid";

SolverKey gridDynSimulationGetSolverKey(GridDynSimulation sim,
                                         const char* solverType,
                                         GridDynError* err)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return nullptr;
    }
    auto slv = runner->getSim()->getSolverMode(solverType);
    auto key = new solverKeyInfo(slv);
    return reinterpret_cast<void*>(key);
}

void gridDynSolverKeyFree(SolverKey key)
{
    auto *skey = static_cast<const solverKeyInfo*>(key);
    delete skey;
}

int gridDynSimulationBusCount(GridDynSimulation sim)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return 0;
    }
    return runner->getSim()->getInt("buscount");
}

int gridDynSimulationLineCount(GridDynSimulation sim)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return 0;
    }
    return runner->getSim()->getInt("linkcount");
}

void gridDynSimulationGetResults(GridDynSimulation sim,
                                 const char* dataType,
                                 double* data,
                                 int maxSize,
    int *actualSize,
                                 GridDynError* err)
{
    if (actualSize != nullptr) {
        *actualSize = 0;
    }
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return;
    }
    if (!runner->getSim()) {
        return;
    }
    std::vector<double> dataVec;
    auto fvecfunc = getObjectVectorFunction(static_cast<const Area*>(nullptr), dataType);
    if (!fvecfunc.first) {
        return;
    }
    fvecfunc.first(runner->getSim().get(), dataVec);
    for (int ii = 0; (ii < maxSize) && (ii < static_cast<int>(dataVec.size())); ++ii) {
        data[ii] = dataVec[ii];
    }
    if (actualSize != nullptr)
    {
        *actualSize = (std::min)(maxSize, static_cast<int>(dataVec.size()));
    }
}

int gridDynSimulationStateSize(GridDynSimulation sim, SolverKey key, GridDynError* err)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return 0;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        assignError(err, griddyn_error_invalid_object, invalidSolver);
        return 0;
    }
    return static_cast<int>(runner->getSim()->stateSize(sMode));
}

void gridDynSimulationGuessState(GridDynSimulation sim,
                                            double time,
                                            double* states,
                                            double* dstate_dt,
                                            SolverKey key,
                                            GridDynError* err)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        assignError(err, griddyn_error_invalid_object, invalidSolver);
        return;
    }
    runner->getSim()->guessState(time, states, dstate_dt, sMode);
}

void gridDynSimulationSetState(GridDynSimulation sim,
                                          double time,
                                          const double* states,
                                          const double* dstate_dt,
                                          SolverKey key,
                                          GridDynError* err)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        assignError(err, griddyn_error_invalid_object, invalidSolver);
        return;
    }
    runner->getSim()->setState(time, states, dstate_dt, sMode);
}

void gridDynSimulationGetStateVariableTypes(GridDynSimulation sim,
                                                       double* types,
                                                       SolverKey key,
                                                       GridDynError* err)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        assignError(err, griddyn_error_invalid_object, invalidSolver);
        return;
    }
    runner->getSim()->getVariableType(types, sMode);
    griddyn_ok;
}

void gridDynSimulationResidual(GridDynSimulation sim,
                               double time,
                               double* resid,
                               const double* states,
                               const double* dstate_dt,
                               SolverKey key,
                               GridDynError* err)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        assignError(err, griddyn_error_invalid_object, invalidSolver);
        return;
    }
    runner->getSim()->residualFunction(time, states, dstate_dt, resid, sMode);
}

void gridDynSimulationDerivative(GridDynSimulation sim,
                                            double time,
                                            double* deriv,
                                            const double* states,
                                            SolverKey key,
                                            GridDynError* err)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        assignError(err, griddyn_error_invalid_object, invalidSolver);
        return;
    }
    runner->getSim()->derivativeFunction(time, states, deriv, sMode);
}

void gridDynSimulationAlgebraicUpdate(GridDynSimulation sim,
                                                 double time,
                                                 double* update,
                                                 const double* states,
                                                 double alpha,
                                                 SolverKey key,
                                                 GridDynError* err)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        assignError(err, griddyn_error_invalid_object, invalidSolver);
        return;
    }
    runner->getSim()->algUpdateFunction(time, states, update, sMode, alpha);
}

void gridDynSimulationJacobian(GridDynSimulation sim,
                                          double time,
                                          const double* states,
                                          const double* dstate_dt,
                                          double cj,
                                          SolverKey key,
                                          void (*insert)(int, int, double),
                                          GridDynError* err)
{
    auto *runner = static_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        assignError(err, griddyn_error_invalid_object, invalidSimulation);
        return;
    }
    auto& sMode = reinterpret_cast<const solverKeyInfo*>(key)->sMode_;
    if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500)) {
        assignError(err, griddyn_error_invalid_object, invalidSolver);
        return;
    }
    matrixDataCustomWriteOnly<double> md;
    md.setFunction([insert](index_t row, index_t col, double val) {
        insert(static_cast<int>(row), static_cast<int>(col), val);
    });
    runner->getSim()->jacobianFunction(time, states, dstate_dt, md, cj, sMode);
}
