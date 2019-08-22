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


#include "fmiExciter.h"
#include "core/coreObjectTemplates.hpp"
#include "fmiMESubModel.h"
#include "griddyn/gridBus.h"
#include "gmlc/utilities/stringOps.h"
#include "core/coreExceptions.h"

namespace griddyn
{
namespace fmi
{
fmiExciter::fmiExciter(const std::string &objName):fmiMEWrapper<Exciter>(objName)
{
	
}



coreObject * fmiExciter::clone(coreObject *obj) const
{
	auto nobj = cloneBase<fmiExciter, fmiMEWrapper<Exciter>>(this, obj);
	if (nobj==nullptr)
	{
		return obj;
	}

	return nobj;

}


void fmiExciter::set (const std::string &param, const std::string &val)
{
	
	if (param.empty())
	{

	}
	else
	{
		fmiMEWrapper<Exciter>::set(param, val);
	}
}

void fmiExciter::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	if (param.empty())
	{

	}
	else
	{
		fmiMEWrapper<Exciter>::set(param, val,unitType);
	}

}

}//namespace fmi
}//namespace griddyn