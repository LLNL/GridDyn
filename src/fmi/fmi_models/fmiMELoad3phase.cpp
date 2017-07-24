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


#include "fmiMELoad3phase.h"
#include "fmi_import/fmiObjects.h"
#include "fmiMESubModel.h"
#include "core/coreObjectTemplates.hpp"
#include "gridBus.h"
#include "utilities/stringOps.h"
#include "core/coreExceptions.h"

namespace griddyn
{
namespace fmi
{
fmiMELoad3phase::fmiMELoad3phase(const std::string &objName):fmiMEWrapper<loads::ThreePhaseLoad>(objName)
{
	
}


coreObject * fmiMELoad3phase::clone(coreObject *obj) const
{
	auto nobj = cloneBase<fmiMELoad3phase, fmiMEWrapper<loads::ThreePhaseLoad>>(this, obj);
	if (nobj==nullptr)
	{
		return obj;
	}
	return nobj;
	
}

void fmiMELoad3phase::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
	if (fmisub->isLoaded()) 
	{
		setupFmiIo();
		SET_CONTROLFLAG(flags, force_constant_pflow_initialization);
		fmisub->dynInitializeA(time0, flags);
		loads::ThreePhaseLoad::pFlowObjectInitializeA(time0, flags);
		auto inputs = bus->getOutputs(noInputs,emptyStateData,cLocalSolverMode);
		IOdata outset;
		fmisub->dynInitializeB(inputs, outset, outset);
		opFlags.set(pFlow_initialized);
	}
	else
	{
		disable();
	}
}


void fmiMELoad3phase::set (const std::string &param, const std::string &val)
{

	if (param.empty())
	{

	}
	else
	{
		fmiMEWrapper<loads::ThreePhaseLoad>::set(param, val);
	}

	
}
void fmiMELoad3phase::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	if (param.empty())
	{

	}
	else
	{
		fmiMEWrapper<loads::ThreePhaseLoad>::set(param, val,unitType);
	}
}


void fmiMELoad3phase::setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	fmisub->setState(time, state, dstate_dt, sMode);
	auto out = fmisub->getOutputs(noInputs, emptyStateData, cLocalSolverMode);
	setP(out[PoutLocation]);
	setQ(out[QoutLocation]);
}

}//namespace fmi
}//namespace griddyn