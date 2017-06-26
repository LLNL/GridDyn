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


#include "fmiExciter.h"
#include "core/coreObjectTemplates.hpp"
#include "fmiMESubModel.h"
#include "gridBus.h"
#include "utilities/stringOps.h"
#include "core/coreExceptions.h"

namespace griddyn
{
namespace fmi
{
fmiExciter::fmiExciter(std::string objName):Exciter(objName)
{
	
}

fmiExciter::~fmiExciter()
{
	//fmisub gets deleted from the object
}

coreObject * fmiExciter::clone(coreObject *obj) const
{
	fmiExciter *nobj = cloneBase<fmiExciter, Exciter>(this, obj);
	if (!(nobj))
	{
		return obj;
	}
	nobj->v_in = v_in;
	nobj->E_out = E_out;

	if (fmisub)
	{
		nobj->fmisub = static_cast<fmiMESubModel *>(fmisub->clone());
	}
	return nobj;

}

void fmiExciter::setupFmiIo()
{
	using namespace stringOps;
	auto ostrings = fmisub->getOutputNames();
	auto istrings = fmisub->getInputNames();
	std::string v_in_string;
	std::string e_out_string;
	//deal with V_in
	if (v_in.empty())
	{
		int ind = findCloseStringMatch({ "voltage", "v_in", "v" }, istrings, string_match_close);
		if (ind >= 0)
		{
			v_in_string = istrings[ind];
		}
	}
	else
	{
		int ind = findCloseStringMatch({ v_in }, istrings, string_match_exact);
		if (ind >= 0)
		{
			v_in_string = istrings[ind];
		}
	}
	
	//deal with E_out
	if (E_out.empty())
	{
		int ind = findCloseStringMatch({ "reactive_out", "q", "reactive_power_out", "reactive_load" }, ostrings, string_match_close);
		if (ind >= 0)
		{
			e_out_string = ostrings[ind];
		}
	}
	else
	{
		int ind = findCloseStringMatch({ E_out }, ostrings, string_match_exact);
		if (ind >= 0)
		{
			e_out_string = ostrings[ind];
		}
	}

	
	std::string inputs = v_in_string;

	fmisub->set("inputs", inputs);

	std::string outputs;
		
	fmisub->set("outputs", e_out_string);
}

void fmiExciter::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
	if ((fmisub) && (fmisub->isLoaded())) //check to make sure the fmi is loaded
	{
		setupFmiIo();
		SET_CONTROLFLAG(flags, force_constant_pflow_initialization);
		fmisub->dynInitializeA(time0, flags);
		Exciter::dynObjectInitializeA(time0, flags);
	}
	else
	{
		disable();
	}
}

void fmiExciter::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata & /*inputSet*/)
{
	if ((fmisub) && (fmisub->isLoaded())) //check to make sure the fmi is loaded
	{
		IOdata outset;
		fmisub->dynInitializeB(inputs, outset, outset);
		opFlags.set(pFlow_initialized);
	}
	else
	{
		IOdata inSet(3);
		fmisub->dynInitializeB(inputs, desiredOutput, inSet);
		opFlags.set(dyn_initialized);
	}
}

void fmiExciter::set (const std::string &param, const std::string &val)
{
	
	if ((param == "fmu") || (param == "fmu_dir") || (param == "file"))
	{
		if (fmisub)
		{
			remove(fmisub);
			
		}
		fmisub = new fmiMESubModel(getName());
		fmisub->set("fmu", val);
		addSubObject(fmisub);
	}
	else if (param == "v_in")
	{
		v_in = val;
	}
	else if (param == "e_out")
	{
		E_out = val;
	}
	else
	{
		bool valid = true;
		

		if (fmisub)
		{
			try
			{
				fmisub->set(param, val);
			}
			catch (unrecognizedParameter &)
			{
				valid = false;
			}
		}

		try
		{
			Exciter::set(param, val);
		}
		catch (const unrecognizedParameter &)
		{
			if (!valid)
			{
				throw;
			}
		}


	}
}

void fmiExciter::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	bool valid = true;


	if (fmisub)
	{
		try
		{
			fmisub->set(param, val,unitType);
		}
		catch (unrecognizedParameter &)
		{
			valid = false;
		}
	}

	try
	{
		Exciter::set(param, val,unitType);
	}
	catch (const unrecognizedParameter &)
	{
		if (!valid)
		{
			throw;
		}
	}

}

void fmiExciter::getParameterStrings(stringVec &pstr, paramStringType pstype) const
{
	switch (pstype)
	{
	case paramStringType::all:
		fmisub->getParameterStrings(pstr, paramStringType::localnum);
		gridSubModel::getParameterStrings(pstr, paramStringType::numeric);
		pstr.push_back("#");
		fmisub->getParameterStrings(pstr, paramStringType::localstr);
		gridSubModel::getParameterStrings(pstr, paramStringType::str);
		break;
	case paramStringType::localnum:
		fmisub->getParameterStrings(pstr, paramStringType::localnum);
		break;
	case paramStringType::localstr:
		fmisub->getParameterStrings(pstr, paramStringType::localstr);
		break;
	case paramStringType::localflags:
		fmisub->getParameterStrings(pstr, paramStringType::localflags);
		break;
	case paramStringType::numeric:
		fmisub->getParameterStrings(pstr, paramStringType::localnum);
		gridSubModel::getParameterStrings(pstr, paramStringType::numeric);
		break;
	case paramStringType::str:
		fmisub->getParameterStrings(pstr, paramStringType::localstr);
		gridSubModel::getParameterStrings(pstr, paramStringType::str);
		break;
	case paramStringType::flags:
		fmisub->getParameterStrings(pstr, paramStringType::localflags);
		gridSubModel::getParameterStrings(pstr, paramStringType::flags);
		break;
	}
}

void fmiExciter::residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
	fmisub->residual(inputs, sD, resid, sMode);
}

void fmiExciter::derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
	fmisub->derivative(inputs, sD, deriv, sMode);
}

void fmiExciter::outputPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const solverMode &sMode)
{
	fmisub->outputPartialDerivatives(inputs, sD, md, sMode);
}
void fmiExciter::ioPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode)
{
	fmisub->ioPartialDerivatives (inputs, sD, md, inputLocs, sMode);
}
void fmiExciter::jacobianElements(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode)
{
	fmisub->jacobianElements (inputs, sD, md, inputLocs, sMode);
}

void fmiExciter::rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)
{
	fmisub->rootTest(inputs, sD, roots, sMode);
}
void fmiExciter::rootTrigger(coreTime time, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode)
{
	fmisub->rootTrigger(time, inputs, rootMask, sMode);
}

void fmiExciter::setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	fmisub->setState(time, state, dstate_dt, sMode);
	auto out = fmisub->getOutputs({}, emptyStateData, cLocalSolverMode);
	
}

index_t fmiExciter::findIndex(const std::string &field, const solverMode &sMode) const
{
	return fmisub->findIndex(field, sMode);
}

void fmiExciter::timestep(coreTime time, const IOdata &inputs, const solverMode &sMode)
{
	prevTime = time;
	fmisub->timestep(time, inputs, sMode);
}
IOdata fmiExciter::getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
	return fmisub->getOutputs(inputs, sD, sMode);
}

}//namespace fmi
}//namespace griddyn