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


#include "fmiLoad.h"
#include "fmi_import/fmiObjects.h"
#include "fmiMESubModel.h"
#include "gridCoreTemplates.h"
#include "gridBus.h"
#include "stringOps.h"
#include "core/gridDynExceptions.h"

fmiLoad::fmiLoad(std::string objName):gridLoad(objName)
{
	
}

fmiLoad::~fmiLoad()
{
//fmisub gets deleted from the object
}

gridCoreObject * fmiLoad::clone(gridCoreObject *obj) const
{
	fmiLoad *nobj = cloneBase<fmiLoad, gridLoad>(this, obj);
	if (!(nobj))
	{
		return obj;
	}
	nobj->a_in = a_in;
	nobj->v_in = v_in;
	nobj->f_in = f_in;
	nobj->P_out = P_out;
	nobj->Q_out = Q_out;

	if (fmisub)
	{
		nobj->fmisub = static_cast<fmiMESubModel *>(fmisub->clone());
	}
	return nobj;
	
}

void fmiLoad::setupFmiIo()
{
	auto ostrings = fmisub->getOutputNames();
	auto istrings = fmisub->getInputNames();
	std::string v_in_string;
	std::string a_in_string;
	std::string f_in_string;
	std::string p_out_string;
	std::string q_out_string;
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
		int ind = findCloseStringMatch({v_in}, istrings, string_match_exact);
		if (ind >= 0)
		{
			v_in_string = istrings[ind];
		}
	}
	//deal with a_in
	if (a_in.empty())
	{
		int ind = findCloseStringMatch({ "angle", "a_in", "a" }, istrings, string_match_close);
		if (ind >= 0)
		{
			a_in_string = istrings[ind];
		}
	}
	else
	{
		int ind = findCloseStringMatch({ a_in }, istrings, string_match_exact);
		if (ind >= 0)
		{
			a_in_string = istrings[ind];
		}
	}
	//deal with f_in
	if (f_in.empty())
	{
		int ind = findCloseStringMatch({ "freq", "f_in", "f" }, istrings, string_match_close);
		if (ind >= 0)
		{
			f_in_string = istrings[ind];
		}
	}
	else
	{
		int ind = findCloseStringMatch({ f_in }, istrings, string_match_exact);
		if (ind >= 0)
		{
			f_in_string = istrings[ind];
		}
	}
	//deal with p_out
	if (P_out.empty())
	{
		int ind = findCloseStringMatch({ "real_out", "p","power_out","real_power","real_load" }, ostrings, string_match_close);
		if (ind >= 0)
		{
			p_out_string = ostrings[ind];
		}
	}
	else
	{
		int ind = findCloseStringMatch({ P_out }, ostrings, string_match_exact);
		if (ind >= 0)
		{
			p_out_string =ostrings[ind];
		}
	}
	//deal with q_out
	if (Q_out.empty())
	{
		int ind = findCloseStringMatch({ "reactive_out", "q", "reactive_power_out", "reactive_load" }, ostrings, string_match_close);
		if (ind >= 0)
		{
			q_out_string = ostrings[ind];
		}
	}
	else
	{
		int ind = findCloseStringMatch({ Q_out }, ostrings, string_match_exact);
		if (ind >= 0)
		{
			q_out_string = ostrings[ind];
		}
	}

	std::string inputs;
	if (voltageInLocation == 0)
	{
		inputs = v_in_string;
		if (!a_in_string.empty())
		{
			inputs += ", " + a_in_string;
			if (!f_in_string.empty())
			{
				inputs += ", " + f_in_string;
			}
		}
		
	}
	else
	{
		inputs = a_in_string;
		if (!v_in_string.empty())
		{
			inputs += ", " + v_in_string;
			if (!f_in_string.empty())
			{
				inputs += ", " + f_in_string;
			}
		}
		
	}
	fmisub->set("inputs", inputs);

	std::string outputs;
	if (PoutLocation == 0)
	{
		outputs = p_out_string;
		if (!q_out_string.empty())
		{
			outputs += ", " + q_out_string;
		}

	}
	else
	{
		outputs =q_out_string;
		if (!p_out_string.empty())
		{
			outputs += ", " + q_out_string;
		}

	}
	fmisub->set("outputs", outputs);
}
void fmiLoad::pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
	if (fmisub->isLoaded()) 
	{
		setupFmiIo();
		SET_CONTROLFLAG(flags, force_constant_pflow_initialization);
		fmisub->initializeA(time0, flags);
		gridLoad::pFlowObjectInitializeA(time0, flags);
		auto args = bus->getOutputs(nullptr,cLocalSolverMode);
		IOdata outset;
		fmisub->initializeB(args, outset, outset);
		opFlags.set(pFlow_initialized);
	}
	else
	{
		disable();
	}
}
void fmiLoad::dynObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
	fmisub->initializeA(time0, flags);
	gridLoad::dynObjectInitializeA(time0, flags);
}

void fmiLoad::dynObjectInitializeB (const IOdata &args, const IOdata &outputSet)
{
	IOdata inSet(3);
	fmisub->initializeB(args, outputSet,inSet);
}

void fmiLoad::set (const std::string &param, const std::string &val)
{
	auto param2 = param;
	makeLowerCase(param2);
	if ((param2 == "fmu") || (param2 == "fmu_dir")||(param2=="file"))
	{
		if (fmisub)
		{
			delete fmisub;
			subObjectList.clear();
		}
		fmisub = new fmiMESubModel(name);
		fmisub->set("fmu", val);
		subObjectList.push_back(fmisub);
		fmisub->setParent(this);
	}
	else if (param2 == "v_in")
	{
		v_in = val;
	}
	else if (param2 == "a_in")
	{
		a_in = val;
	}
	else if (param2 == "p_out")
	{
		P_out = val;
	}
	else if (param2 == "q_out")
	{
		Q_out = val;
	}
	else if (param2 == "f_in")
	{
		f_in = val;
	}
	else
	{
		bool valid = false;
		try
		{
			gridLoad::set(param, val);
			valid = true;
		}
		catch (const unrecognizedParameter &)
		{
	
		}
		
		if (fmisub)
		{
			try
			{
				fmisub->set(param, val);
				valid = true;
			}
			catch (const unrecognizedParameter &)
			{

			}
		}

		if (!valid)
		{
			throw(unrecognizedParameter());
		}
		
	}
}
void fmiLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	bool valid = false;
	try
	{
		gridLoad::set(param, val,unitType);
		valid = true;
	}
	catch (const unrecognizedParameter &)
	{
	
	}

	if (fmisub)
	{
		try
		{
			fmisub->set(param, val,unitType);
			valid = true;
		}
		catch (const unrecognizedParameter &)
		{

		}
	}

	if (!valid)
	{
		throw(unrecognizedParameter());
	}
}

void fmiLoad::getParameterStrings(stringVec &pstr, paramStringType pstype) const
{
	switch (pstype)
	{
	case paramStringType::all:
		fmisub->getParameterStrings(pstr, paramStringType::localnum);
		gridSecondary::getParameterStrings(pstr, paramStringType::numeric);
		pstr.push_back("#");
		fmisub->getParameterStrings(pstr, paramStringType::localstr);
		gridSecondary::getParameterStrings(pstr, paramStringType::string);
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
		gridSecondary::getParameterStrings(pstr, paramStringType::numeric);
		break;
	case paramStringType::string:
		fmisub->getParameterStrings(pstr, paramStringType::localstr);
		gridSecondary::getParameterStrings(pstr, paramStringType::string);
		break;
	case paramStringType::flags:
		fmisub->getParameterStrings(pstr, paramStringType::localflags);
		gridSecondary::getParameterStrings(pstr, paramStringType::flags);
		break;
	}
}

void fmiLoad::residual(const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode)
{
	fmisub->residual(args, sD, resid, sMode);
}

void fmiLoad::derivative(const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
	fmisub->derivative(args, sD,deriv, sMode);
}

void fmiLoad::outputPartialDerivatives(const IOdata &args, const stateData *sD, matrixData<double> &ad, const solverMode &sMode)
{
	fmisub->outputPartialDerivatives(args, sD, ad, sMode);
}
void fmiLoad::ioPartialDerivatives(const IOdata &args, const stateData *sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode)
{
	fmisub->ioPartialDerivatives (args, sD, ad, argLocs,sMode);
}
void fmiLoad::jacobianElements(const IOdata &args, const stateData *sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode)
{
	fmisub->jacobianElements (args, sD, ad,argLocs, sMode);
}

void fmiLoad::rootTest(const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode)
{
	fmisub->rootTest(args, sD, roots, sMode);
}
void fmiLoad::rootTrigger(gridDyn_time ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
	fmisub->rootTrigger(ttime,args, rootMask, sMode);
}

void fmiLoad::setState(gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	fmisub->setState(ttime, state, dstate_dt, sMode);
	auto out = fmisub->getOutputs({}, nullptr, cLocalSolverMode);
	P = out[PoutLocation];
	Q = out[QoutLocation];
}

index_t fmiLoad::findIndex(const std::string &field, const solverMode &sMode) const
{
	return fmisub->findIndex(field, sMode);
}

void fmiLoad::timestep(gridDyn_time ttime, const IOdata &args, const solverMode &sMode)
{
	prevTime = ttime;
	fmisub->timestep(ttime, args, sMode);
}
IOdata fmiLoad::getOutputs(const IOdata &args, const stateData *sD, const solverMode &sMode)
{
	return fmisub->getOutputs(args, sD, sMode);
}
double fmiLoad::getRealPower(const IOdata &args, const stateData *sD, const solverMode &sMode)
{
	return fmisub->getOutput(args, sD, sMode,0);
}
double fmiLoad::getReactivePower(const IOdata &args, const stateData *sD, const solverMode &sMode)
{
	return fmisub->getOutput(args, sD, sMode, 1);
}
double fmiLoad::getRealPower(const double V) const
{

	auto args = bus->getOutputs(nullptr,cLocalSolverMode);
	args[voltageInLocation] = V;
	return fmisub->getOutput(args, nullptr, cLocalSolverMode,0);
}
double fmiLoad::getReactivePower(const double V) const
{
	auto args = bus->getOutputs(nullptr, cLocalSolverMode);
	args[voltageInLocation] = V;
	return fmisub->getOutput(args, nullptr, cLocalSolverMode, 1);
}
double fmiLoad::getRealPower() const
{
	return P;
}
double fmiLoad::getReactivePower() const
{
	return Q;
}