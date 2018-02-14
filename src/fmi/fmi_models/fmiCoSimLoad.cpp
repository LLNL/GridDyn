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


#include "fmiCoSimLoad.h"
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
fmiCoSimLoad::fmiCoSimLoad(const std::string &objName):fmiCoSimWrapper<Load>(objName)
{
	
}



coreObject * fmiCoSimLoad::clone(coreObject *obj) const
{
	auto nobj = cloneBase<fmiCoSimLoad, fmiCoSimWrapper<Load>>(this, obj);
	if (nobj==nullptr)
	{
		return obj;
	}

	return nobj;
	
}

void fmiCoSimLoad::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
	if (fmisub->isLoaded()) 
	{
		setupFmiIo();
		SET_CONTROLFLAG(flags, force_constant_pflow_initialization);
		fmisub->dynInitializeA(time0, flags);
		//zipLoad::pFlowObjectInitializeA(time0, flags);
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
void fmiCoSimLoad::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
	fmisub->dynInitializeA(time0, flags);
	//zipLoad::dynObjectInitializeA(time0, flags);
}

void fmiCoSimLoad::dynObjectInitializeB (const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet)
{
	fmisub->dynInitializeB(inputs, desiredOutput,fieldSet);
}



void fmiCoSimLoad::setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	fmisub->setState(time, state, dstate_dt, sMode);
	auto out = fmisub->getOutputs(noInputs, emptyStateData, cLocalSolverMode);
	setP(out[PoutLocation]);
	setQ(out[QoutLocation]);
}

}//namespace fmi
}//namespace griddyn