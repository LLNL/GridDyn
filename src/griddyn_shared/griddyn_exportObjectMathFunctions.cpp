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

#include "griddyn_export_advanced.h"
#include "griddyn_export_internal.h"

#include "core/coreOwningPtr.hpp"
#include "core/objectFactory.hpp"
#include "core/coreExceptions.h"
#include "gridDynSimulation.h"
#include "utilities/matrixDataCustomWriteOnly.hpp"

using namespace griddyn;

int gridDynObject_stateSize(gridDynObject obj, solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto &sMode = reinterpret_cast<const solverKeyInfo *>(key)->sMode_;
	if ((sMode.offsetIndex < 0) || (sMode.offsetIndex > 500))
	{
		return INVALID_OBJECT;
	}
	return static_cast<int>(comp->stateSize(sMode));
}


void setUpSolverKeyInfo(solverKeyInfo *key, gridComponent *comp)
{
	auto root = dynamic_cast<gridDynSimulation *>(comp->getRoot());
	auto ssize = root->stateSize(key->sMode_);
	key->stateBuffer.resize(ssize);
	key->dstateBuffer.resize(ssize);
}


void TranslateToLocal(const std::vector<double> &orig, double *newData, const gridComponent *comp, const solverMode &sMode)
{
	auto offsets = comp->getOffsets(sMode);
	double *cData = newData;
	for (index_t ii = 0; ii < offsets.total.vSize; ii++)
	{
		*cData = orig[offsets.vOffset + ii];
		++cData;
	}

	for (index_t ii = 0; ii < offsets.total.aSize; ii++)
	{
		*cData = orig[offsets.aOffset + ii];
		++cData;
	}

	for (index_t ii = 0; ii < offsets.total.algSize; ii++)
	{
		*cData = orig[offsets.algOffset + ii];
		++cData;
	}

	for (index_t ii = 0; ii < offsets.total.diffSize; ii++)
	{
		*cData = orig[offsets.algOffset + ii];
		++cData;
	}

}

void CopyFromLocal(std::vector<double> &dest, const double *localData, const gridComponent *comp, const solverMode &sMode)
{
	auto offset = comp->getOffsets(sMode);
	auto offsets = comp->getOffsets(sMode);
	const double *cData = localData;
	for (index_t ii = 0; ii < offsets.total.vSize; ii++)
	{
		dest[offsets.vOffset+ii] = *cData;
		++cData;
	}

	for (index_t ii = 0; ii < offsets.total.aSize; ii++)
	{
		dest[offsets.aOffset + ii] = *cData;
		++cData;
	}

	for (index_t ii = 0; ii < offsets.total.algSize; ii++)
	{
		dest[offsets.algOffset + ii] = *cData;
		++cData;
	}

	for (index_t ii = 0; ii < offsets.total.diffSize; ii++)
	{
		dest[offsets.diffOffset + ii] = *cData;
		++cData;
	}

}

int gridDynObject_guessState(gridDynObject obj, double time, double *states, double *dstate_dt,solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	if (states == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto keyInfo = reinterpret_cast<solverKeyInfo *>(key);
	if (keyInfo == nullptr)
	{
		return INVALID_OBJECT;
	}
	if (keyInfo->stateBuffer.empty())
	{
		setUpSolverKeyInfo(keyInfo, comp);
	}
	if (comp->checkFlag(dyn_initialized))
	{
		if (dstate_dt == nullptr)
		{
			return INVALID_OBJECT;
		}
		comp->guessState(time, keyInfo->stateBuffer.data(), keyInfo->dstateBuffer.data(), keyInfo->sMode_);
	}
	else
	{
		comp->guessState(time, keyInfo->stateBuffer.data(), keyInfo->dstateBuffer.data(), keyInfo->sMode_);
	}
	TranslateToLocal(keyInfo->stateBuffer, states, comp, keyInfo->sMode_);
	TranslateToLocal(keyInfo->dstateBuffer, dstate_dt, comp, keyInfo->sMode_);
	return EXECUTION_SUCCESS;
}

int gridDynObject_setState(gridDynObject obj, double time, const double *states, const double *dstate_dt, solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto keyInfo = reinterpret_cast<solverKeyInfo *>(key);
	if (keyInfo == nullptr)
	{
		return INVALID_OBJECT;
	}
	if (keyInfo->stateBuffer.empty())
	{
		setUpSolverKeyInfo(keyInfo, comp);
	}
	CopyFromLocal(keyInfo->stateBuffer, states, comp, keyInfo->sMode_);
	if (hasDifferential(keyInfo->sMode_))
	{
		CopyFromLocal(keyInfo->dstateBuffer, dstate_dt, comp, keyInfo->sMode_);
	}
	comp->setState(time, keyInfo->stateBuffer.data(), keyInfo->dstateBuffer.data(), keyInfo->sMode_);
	return EXECUTION_SUCCESS;
}

int gridDynObject_getStateVariableTypes(gridDynObject obj, double *types,solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto keyInfo = reinterpret_cast<solverKeyInfo *>(key);
	if (keyInfo == nullptr)
	{
		return INVALID_OBJECT;
	}
	if (keyInfo->stateBuffer.empty())
	{
		setUpSolverKeyInfo(keyInfo, comp);
	}
	comp->getVariableType(keyInfo->stateBuffer.data(), keyInfo->sMode_);
	TranslateToLocal(keyInfo->stateBuffer, types, comp, keyInfo->sMode_);
	return EXECUTION_SUCCESS;
}

int gridDynObject_residual(gridDynObject obj, const double *inputs, int inputSize, double *resid, solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto keyInfo = reinterpret_cast<solverKeyInfo *>(key);
	if (keyInfo == nullptr)
	{
		return INVALID_OBJECT;
	}
	if (keyInfo->stateBuffer.empty())
	{
		setUpSolverKeyInfo(keyInfo, comp);
	}
	comp->residual(IOdata(inputs, inputs + inputSize), emptyStateData, keyInfo->stateBuffer.data(), keyInfo->sMode_);
	TranslateToLocal(keyInfo->stateBuffer, resid, comp, keyInfo->sMode_);
	return EXECUTION_SUCCESS;
}

int gridDynObject_derivative(gridDynObject obj, const double *inputs, int inputSize, double *deriv, solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	if (!comp->checkFlag(dyn_initialized))
	{
		return OBJECT_NOT_INITIALIZED;
	}
	auto keyInfo = reinterpret_cast<solverKeyInfo *>(key);
	if (keyInfo == nullptr)
	{
		return INVALID_OBJECT;
	}
	if (keyInfo->stateBuffer.empty())
	{
		setUpSolverKeyInfo(keyInfo, comp);
	}
	comp->derivative(IOdata(inputs, inputs + inputSize), emptyStateData, keyInfo->stateBuffer.data(), keyInfo->sMode_);
	TranslateToLocal(keyInfo->stateBuffer, deriv,comp, keyInfo->sMode_);
	return EXECUTION_SUCCESS;
}

int gridDynObject_algebraicUpdate(gridDynObject obj, const double *inputs, int inputSize, double *update, double alpha, solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto keyInfo = reinterpret_cast<solverKeyInfo *>(key);
	if (keyInfo == nullptr)
	{
		return INVALID_OBJECT;
	}
	if (keyInfo->stateBuffer.empty())
	{
		setUpSolverKeyInfo(keyInfo, comp);
	}
	comp->algebraicUpdate(IOdata(inputs, inputs + inputSize), emptyStateData, keyInfo->stateBuffer.data(), keyInfo->sMode_,alpha);
	TranslateToLocal(keyInfo->stateBuffer, update, comp, keyInfo->sMode_);
	return EXECUTION_SUCCESS;
}

const IOlocs defInputlocs{kNullLocation,kNullLocation,kNullLocation,kNullLocation,kNullLocation,kNullLocation,kNullLocation,kNullLocation,kNullLocation,kNullLocation,kNullLocation };
int gridDynObject_jacobian(gridDynObject obj, const double *inputs, int inputSize, double cj, void(*insert)(int, int, double), solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto keyInfo = reinterpret_cast<solverKeyInfo *>(key);
	if (keyInfo == nullptr)
	{
		return INVALID_OBJECT;
	}
	matrixDataCustomWriteOnly<double> md;
	md.setFunction([insert](index_t row, index_t col, double val) {insert(static_cast<int>(row), static_cast<int>(col), val); });
	stateData sD;
	sD.cj = cj;
	comp->jacobianElements(IOdata(inputs, inputs + inputSize), sD, md, defInputlocs, keyInfo->sMode_);
	return EXECUTION_SUCCESS;
}

const IOlocs defInputlocs_act{0,1,2,3,4,5,6,7,8,9,10};

int gridDynObject_ioPartialDerivatives(gridDynObject obj, const double *inputs, int inputSize, void(*insert)(int, int, double), solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto keyInfo = reinterpret_cast<solverKeyInfo *>(key);
	if (keyInfo == nullptr)
	{
		return INVALID_OBJECT;
	}
	matrixDataCustomWriteOnly<double> md;
	md.setFunction([insert](index_t row, index_t col, double val) {insert(static_cast<int>(row), static_cast<int>(col), val); });
	comp->ioPartialDerivatives(IOdata(inputs, inputs + inputSize), emptyStateData, md, defInputlocs, keyInfo->sMode_);
	return EXECUTION_SUCCESS;
}

int gridDynObject_outputPartialDerivatives(gridDynObject obj, const double *inputs, int inputSize, void(*insert)(int, int, double), solverKey key)
{
	gridComponent *comp = getComponentPointer(obj);

	if (comp == nullptr)
	{
		return INVALID_OBJECT;
	}
	auto keyInfo = reinterpret_cast<solverKeyInfo *>(key);
	if (keyInfo == nullptr)
	{
		return INVALID_OBJECT;
	}

	matrixDataCustomWriteOnly<double> md;
	md.setFunction([insert](index_t row, index_t col, double val) {insert(static_cast<int>(row), static_cast<int>(col), val); });
	comp->outputPartialDerivatives(IOdata(inputs, inputs + inputSize), emptyStateData, md, keyInfo->sMode_);
	return EXECUTION_SUCCESS;
}