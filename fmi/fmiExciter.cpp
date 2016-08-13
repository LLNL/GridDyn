/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
  * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department 
 * of Energy by Lawrence Livermore National Laboratory in part under 
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/


#include "fmiExciter.h"
#include "gridCoreTemplates.h"
#include "gridBus.h"
#include "stringOps.h"

fmiExciter::fmiExciter(std::string fmd):gridDynExciter("fmi_exciter_#")
{
	if (!fmd.empty())
	{
		fmisub = makefmiSubModel(fmd);
		subObjectList.push_back(fmisub);
		fmisub->setParent(this);
	}

}

fmiExciter::~fmiExciter()
{
	//fmisub gets deleted from the object
}

gridCoreObject * fmiExciter::clone(gridCoreObject *obj) const
{
	fmiExciter *nobj = cloneBase<fmiExciter, gridDynExciter>(this, obj);
	if (!(nobj))
	{
		return obj;
	}
	nobj->v_in = v_in;
	nobj->E_out = E_out;

	if (fmisub)
	{
		nobj->fmisub = static_cast<fmiSubModel *>(fmisub->clone());
	}
	return nobj;

}

void fmiExciter::setupFmiIo()
{
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

	std::string inputs;
	
	inputs = v_in_string;

	fmisub->set("inputs", inputs);

	std::string outputs;
		
	fmisub->set("outputs", e_out_string);
}

void fmiExciter::objectInitializeA (double time0, unsigned long flags)
{
	if ((fmisub) && (fmisub->getFmiState() != fmiSubModel::fmiState_t::fmi_startup)) //check to make sure the fmi is loaded
	{
		setupFmiIo();
		SET_CONTROLFLAG(flags, force_constant_pflow_initialization);
		fmisub->initializeA(time0, flags);
		gridDynExciter::objectInitializeA(time0, flags);
	}
	else
	{
		disable();
	}
}

void fmiExciter::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
	if ((fmisub) && (fmisub->getFmiState() != fmiSubModel::fmiState_t::fmi_startup)) //check to make sure the fmi is loaded
	{
		IOdata outset;
		fmisub->initializeB(args, outset, outset);
		opFlags.set(pFlow_initialized);
	}
	else
	{
		IOdata inSet(3);
		fmisub->initializeB(args, outputSet, inSet);
		opFlags.set(dyn_initialized);
	}
}

int fmiExciter::set (const std::string &param, const std::string &val)
{
	int out = PARAMETER_FOUND;
	
	if ((param == "fmu") || (param == "fmu_dir") || (param == "file"))
	{
		if (fmisub)
		{
			delete fmisub;
			subObjectList.clear();
		}
		fmisub = makefmiSubModel(val);
		subObjectList.push_back(fmisub);
		fmisub->setParent(this);
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
		out = gridDynExciter::set(param, val);

		if (fmisub)
		{
			auto out2 = fmisub->set(param, val);
			if (out2 == PARAMETER_FOUND)
			{
				out = out2;
			}
		}


	}
	return out;
}

int fmiExciter::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	int out = gridDynExciter::set(param, val);

	if (fmisub)
	{
		auto out2 = fmisub->set(param, val);
		if (out2 == PARAMETER_FOUND)
		{
			out = out2;
		}
	}
	return out;
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
		gridSubModel::getParameterStrings(pstr, paramStringType::string);
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
	case paramStringType::string:
		fmisub->getParameterStrings(pstr, paramStringType::localstr);
		gridSubModel::getParameterStrings(pstr, paramStringType::string);
		break;
	case paramStringType::flags:
		fmisub->getParameterStrings(pstr, paramStringType::localflags);
		gridSubModel::getParameterStrings(pstr, paramStringType::flags);
		break;
	}
}

void fmiExciter::residual(const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode)
{
	fmisub->residual(args, sD, resid, sMode);
}

void fmiExciter::derivative(const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
	fmisub->derivative(args, sD, deriv, sMode);
}

void fmiExciter::outputPartialDerivatives(const IOdata &args, const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{
	fmisub->outputPartialDerivatives(args, sD, ad, sMode);
}
void fmiExciter::ioPartialDerivatives(const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
	fmisub->ioPartialDerivatives (args, sD, ad, argLocs, sMode);
}
void fmiExciter::jacobianElements(const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
	fmisub->jacobianElements (args, sD, ad, argLocs, sMode);
}

void fmiExciter::rootTest(const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode)
{
	fmisub->rootTest(args, sD, roots, sMode);
}
void fmiExciter::rootTrigger(double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
	fmisub->rootTrigger(ttime, args, rootMask, sMode);
}

void fmiExciter::setState(double ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	fmisub->setState(ttime, state, dstate_dt, sMode);
	auto out = fmisub->getOutputs({}, nullptr, cLocalSolverMode);
	
}

index_t fmiExciter::findIndex(const std::string &field, const solverMode &sMode) const
{
	return fmisub->findIndex(field, sMode);
}

double fmiExciter::timestep(double ttime, const IOdata &args, const solverMode &sMode)
{
	prevTime = ttime;
	return fmisub->timestep(ttime, args, sMode);
}
IOdata fmiExciter::getOutputs(const IOdata &args, const stateData *sD, const solverMode &sMode)
{
	return fmisub->getOutputs(args, sD, sMode);
}
