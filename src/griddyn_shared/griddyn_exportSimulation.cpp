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

#include "core/coreExceptions.h"
#include "fileInput/fileInput.h"
#include "griddyn/gridDynSimulation.h"
#include "griddyn_export.h"
#include "internal/griddyn_export_internal.h"
#include "runner/gridDynRunner.h"

using namespace griddyn;

GridDynSimReference gridDynSimulationCreate(const char* type, const char* name, GridDynError* err)
{
    GriddynRunner* sim;
    std::string typeStr(type);
    if (typeStr == "helics") {
        return nullptr;
    }
    if (typeStr == "buildfmu") {
        return nullptr;
    }
    if (typeStr == "dime") {
        return nullptr;
    }
    if (typeStr == "buildgdz") {
        return nullptr;
    }

    sim = new GriddynRunner();
    if (sim != nullptr) {
        sim->getSim()->setName(name);
    }

    return reinterpret_cast<GridDynSimReference>(sim);
}

void gridDynSimulationFree(GridDynSimReference sim)
{
    if (sim != nullptr) {
        delete reinterpret_cast<GriddynRunner*>(sim);
    }
}

void gridDynSimulationInitializeFromString(GridDynSimReference sim,
                                            const char* initializationString,
                                            GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    return runner->InitializeFromString(initializationString);
}

void gridDynSimulationInitializeFromArgs(GridDynSimReference sim,
                                                    int argc,
                                                    char* argv[],
                                          int ignoreUnrecognized,
                                          GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    return runner->Initialize(argc, argv, (ignoreUnrecognized != 0));
}

void gridDynSimulationLoadfile(GridDynSimReference sim,
                                const char* fileName,
                                const char* fileType,
                                GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }

    try {
        auto typestr = std::string(fileType);
        if (typestr.empty()) {
            loadFile(runner->getSim().get(), fileName);
        } else {
            loadFile(runner->getSim().get(), fileName, nullptr, typestr);
        }
        return griddyn_ok;
    }
    catch (...) {
        return griddyn_file_load_failure;
    }
}

void gridDynSimulationAddCommand(GridDynSimReference sim, const char* command, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    gridDynAction action(command);
    if (action.command != gridDynAction::gd_action_t::invalid) {
        runner->getSim()->add(action);
        return griddyn_ok;
    }
    return griddyn_add_failure;
}

void gridDynSimulationRun(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    try {
        runner->Run();
    }
    catch (...) {
        return griddyn_solve_error;
    }
    return griddyn_ok;
}

void gridDynSimulationRunTo(GridDynSimReference sim, double runToTime, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    try {
        runner->Step(runToTime);
    }
    catch (...) {
        return griddyn_solve_error;
    }
    return griddyn_ok;
}

void gridDynSimulationStep(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    auto ret = runner->getSim()->step();
    return ret;
}

void gridDynSimulationRunAsync(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    try {
        runner->RunAsync();
    }
    catch (const executionFailure&) {
        return FUNCTION_EXECUTION_FAILURE;
    }
    return 0;
}

void gridDynSimulationRunToAsync(GridDynSimReference sim, double runToTime, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    try {
        runner->StepAsync(runToTime);
    }
    catch (const executionFailure&) {
        return FUNCTION_EXECUTION_FAILURE;
    }
    return 0;
}

void gridDynSimulationStepAsync(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    return 0;
}

int gridDynSimulationGetStatus(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    coreTime tRet;
    auto res = runner->getStatus(tRet);
    return res;
}

GridDynObject getSimulationObject(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return nullptr;
    }
    runner->getSim()->addOwningReference();
    return creategridDynObject(runner->getSim().get());
}

void gridDynSimulationPowerflowInitialize(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    return runner->getSim()->pFlowInitialize();
}

void gridDynSimulationPowerflow(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    return runner->getSim()->powerflow();
}

void gridDynSimulationDynamicInitialize(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    runner->simInitialize();
    return griddyn_ok;
}

void gridDynSimulationReset(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return griddyn_invalid_object;
    }
    return runner->Reset();
}

double gridDynSimulationGetCurrentTime(GridDynSimReference sim, GridDynError* err)
{
    auto runner = reinterpret_cast<GriddynRunner*>(sim);

    if (runner == nullptr) {
        return kNullVal;
    }
    return static_cast<double>(runner->getSim()->getSimulationTime());
}
