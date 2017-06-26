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


#include "fmiLoad.h"
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
fmiLoad::fmiLoad(std::string objName):zipLoad(objName)
{
	
}

fmiLoad::~fmiLoad()
{
//fmisub gets deleted from the object
}

coreObject * fmiLoad::clone(coreObject *obj) const
{
	fmiLoad *nobj = cloneBase<fmiLoad, zipLoad>(this, obj);
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
	using namespace stringOps;
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
	if  (voltageInLocation == 0/* DISABLES CODE */)
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
	if  (PoutLocation == 0/* DISABLES CODE */)
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
			outputs += ", " + p_out_string;
		}

	}
	fmisub->set("outputs", outputs);
}
void fmiLoad::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
	if (fmisub->isLoaded()) 
	{
		setupFmiIo();
		SET_CONTROLFLAG(flags, force_constant_pflow_initialization);
		fmisub->dynInitializeA(time0, flags);
		zipLoad::pFlowObjectInitializeA(time0, flags);
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
void fmiLoad::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
	fmisub->dynInitializeA(time0, flags);
	zipLoad::dynObjectInitializeA(time0, flags);
}

void fmiLoad::dynObjectInitializeB (const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet)
{
	fmisub->dynInitializeB(inputs, desiredOutput,fieldSet);
}

void fmiLoad::set (const std::string &param, const std::string &val)
{
	auto param2 = convertToLowerCase(param);
	if ((param2 == "fmu") || (param2 == "fmu_dir")||(param2=="file"))
	{
		if (fmisub)
		{
			remove(fmisub);
		}
		fmisub = new fmiMESubModel(getName());
		fmisub->set("fmu", val);
		addSubObject(fmisub);
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
			zipLoad::set(param, val);
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
			throw(unrecognizedParameter(param));
		}
		
	}
}
void fmiLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	bool valid = false;
	try
	{
		zipLoad::set(param, val,unitType);
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
		throw(unrecognizedParameter(param));
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
		gridSecondary::getParameterStrings(pstr, paramStringType::str);
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
	case paramStringType::str:
		fmisub->getParameterStrings(pstr, paramStringType::localstr);
		gridSecondary::getParameterStrings(pstr, paramStringType::str);
		break;
	case paramStringType::flags:
		fmisub->getParameterStrings(pstr, paramStringType::localflags);
		gridSecondary::getParameterStrings(pstr, paramStringType::flags);
		break;
	}
}

void fmiLoad::residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
	fmisub->residual(inputs, sD, resid, sMode);
}

void fmiLoad::derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
	fmisub->derivative(inputs, sD,deriv, sMode);
}

void fmiLoad::outputPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const solverMode &sMode)
{
	fmisub->outputPartialDerivatives(inputs, sD, md, sMode);
}
void fmiLoad::ioPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode)
{
	fmisub->ioPartialDerivatives (inputs, sD, md, inputLocs,sMode);
}
void fmiLoad::jacobianElements(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode)
{
	fmisub->jacobianElements (inputs, sD, md,inputLocs, sMode);
}

void fmiLoad::rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)
{
	fmisub->rootTest(inputs, sD, roots, sMode);
}
void fmiLoad::rootTrigger(coreTime time, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode)
{
	fmisub->rootTrigger(time,inputs, rootMask, sMode);
}

void fmiLoad::setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	fmisub->setState(time, state, dstate_dt, sMode);
	auto out = fmisub->getOutputs(noInputs, emptyStateData, cLocalSolverMode);
	setP(out[PoutLocation]);
	setQ(out[QoutLocation]);
}

index_t fmiLoad::findIndex(const std::string &field, const solverMode &sMode) const
{
	return fmisub->findIndex(field, sMode);
}

void fmiLoad::timestep(coreTime time, const IOdata &inputs, const solverMode &sMode)
{
	prevTime = time;
	fmisub->timestep(time, inputs, sMode);
}
IOdata fmiLoad::getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
	return fmisub->getOutputs(inputs, sD, sMode);
}
double fmiLoad::getRealPower(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
	return fmisub->getOutput(inputs, sD, sMode,0);
}
double fmiLoad::getReactivePower(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
	return fmisub->getOutput(inputs, sD, sMode, 1);
}
double fmiLoad::getRealPower(const double V) const
{
	auto inputs = bus->getOutputs(noInputs,emptyStateData,cLocalSolverMode);
	inputs[voltageInLocation] = V;
	return fmisub->getOutput(inputs, emptyStateData, cLocalSolverMode,0);
}
double fmiLoad::getReactivePower(const double V) const
{
	auto inputs = bus->getOutputs(noInputs,emptyStateData, cLocalSolverMode);
	inputs[voltageInLocation] = V;
	return fmisub->getOutput(inputs, emptyStateData, cLocalSolverMode, 1);
}
double fmiLoad::getRealPower() const
{
	return getP();
}
double fmiLoad::getReactivePower() const
{
	return getQ();
}

}//namespace fmi
}//namespace griddyn