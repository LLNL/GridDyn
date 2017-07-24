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


#include "fmiMELoad.h"
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
fmiMELoad::fmiMELoad(const std::string &objName):fmiMEWrapper<Load>(objName)
{
	
}


coreObject * fmiMELoad::clone(coreObject *obj) const
{
	auto nobj = cloneBase<fmiMELoad, fmiMEWrapper<Load>>(this, obj);
	if (nobj==nullptr)
	{
		return obj;
	}

	return nobj;
	
}


void fmiMELoad::set (const std::string &param, const std::string &val)
{
	if (param.empty())
	{

	}
	else
	{
		fmiMEWrapper<Load>::set(param, val);
	}
}
void fmiMELoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	if (param.empty())
	{

	}
	else
	{
		fmiMEWrapper<Load>::set(param, val,unitType);
	}
}


void fmiMELoad::setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	fmiMEWrapper<Load>::setState(time, state, dstate_dt, sMode);
	auto out = fmisub->getOutputs(noInputs, emptyStateData, cLocalSolverMode);
	setP(out[PoutLocation]);
	setQ(out[QoutLocation]);
}

}//namespace fmi
}//namespace griddyn