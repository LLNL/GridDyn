/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "fileInput/gridDynRunner.h"
#include "gridDyn.h"

#include "griddyn_export.h"

#include "gridDyn_export_internal.h"
#include "utilities/matrixDataCustomWriteOnly.hpp"

using namespace griddyn;

solverKey gridDynSimulation_getSolverKey (gridDynSimReference sim, const char *solverType)
{
    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return nullptr;
    }
    auto slv = runner->getSim ()->getSolverMode(solverType);
	auto key = new solverKeyInfo(slv);
    return reinterpret_cast<void *> (key);
}

void gridDynSolverKey_free(solverKey key)
{
	auto skey = reinterpret_cast<const solverKeyInfo *>(key);
	if (skey != nullptr)
	{
		delete skey;
	}
}
int gridDynSimulation_stateSize (gridDynSimReference sim, solverKey key)
{
    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return INVALID_OBJECT;
    }
    auto &sMode = reinterpret_cast<const solverKeyInfo *>(key)->sMode_;
	if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500))
	{
		return INVALID_OBJECT;
	}
    return static_cast<int> (runner->getSim ()->stateSize (sMode));
}

int gridDynSimulation_guessState (gridDynSimReference sim,
                                  double time,
                                  double *states,
                                  double *dstate_dt,
                                  solverKey key)
{
    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return INVALID_OBJECT;
    }
	auto &sMode = reinterpret_cast<const solverKeyInfo *>(key)->sMode_;
	if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500))
	{
		return INVALID_OBJECT;
	}
    runner->getSim ()->guessState (time, states, dstate_dt, sMode);
    return EXECUTION_SUCCESS;
}

int gridDynSimulation_setState (gridDynSimReference sim,
                                double time,
                                const double *states,
                                const double *dstate_dt,
                                solverKey key)
{
    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return INVALID_OBJECT;
    }
	auto &sMode = reinterpret_cast<const solverKeyInfo *>(key)->sMode_;
	if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500))
	{
		return INVALID_OBJECT;
	}
    runner->getSim ()->setState (time, states, dstate_dt, sMode);
    return EXECUTION_SUCCESS;
}

int gridDynSimulation_getStateVariableTypes (gridDynSimReference sim, double *types, solverKey key)
{
    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return INVALID_OBJECT;
    }
	auto &sMode = reinterpret_cast<const solverKeyInfo *>(key)->sMode_;
	if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500))
	{
		return INVALID_OBJECT;
	}
    runner->getSim ()->getVariableType (types, sMode);
    return EXECUTION_SUCCESS;
}

int gridDynSimulation_residual (gridDynSimReference sim,
                                double time,
                                double *resid,
                                const double *states,
                                const double *dstate_dt,
                                solverKey key)
{
    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return INVALID_OBJECT;
    }
	auto &sMode = reinterpret_cast<const solverKeyInfo *>(key)->sMode_;
	if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500))
	{
		return INVALID_OBJECT;
	}
    return runner->getSim ()->residualFunction (time, states, dstate_dt, resid, sMode);
}

int gridDynSimulation_derivative (gridDynSimReference sim,
                                  double time,
                                  double *deriv,
                                  const double *states,
                                  solverKey key)
{
    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return INVALID_OBJECT;
    }
	auto &sMode = reinterpret_cast<const solverKeyInfo *>(key)->sMode_;
	if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500))
	{
		return INVALID_OBJECT;
	}
    return runner->getSim ()->derivativeFunction (time, states, deriv, sMode);
}

int gridDynSimulation_algebraicUpdate (gridDynSimReference sim,
                                       double time,
                                       double *update,
                                       const double *states,
                                       double alpha,
                                       solverKey key)
{
    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return INVALID_OBJECT;
    }
	auto &sMode = reinterpret_cast<const solverKeyInfo *>(key)->sMode_;
	if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500))
	{
		return INVALID_OBJECT;
	}
	return runner->getSim()->algUpdateFunction(time, states, update, sMode,alpha);
}

int gridDynSimulation_jacobian (gridDynSimReference sim,
                                double time,
                                const double *states,
                                const double *dstate_dt,
	double cj,
                                solverKey key,
                                void (*insert) (int, int, double))
{
    auto runner = reinterpret_cast<GriddynRunner *> (sim);

    if (runner == nullptr)
    {
        return INVALID_OBJECT;
    }
	auto &sMode = reinterpret_cast<const solverKeyInfo *>(key)->sMode_;
	if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500))
	{
		return INVALID_OBJECT;
	}
	matrixDataCustomWriteOnly<double> md;
	md.setFunction([insert](index_t row, index_t col, double val) {insert(static_cast<int>(row), static_cast<int>(col), val); });
	return runner->getSim()->jacobianFunction(time, states, dstate_dt, md, cj, sMode);

}