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



#include "fmiSubModel.h"
#include "gridCoreTemplates.h"
#include "fmi_importGD.h"
#include "vectorOps.hpp"
#include "arrayData.h"
#include "stringOps.h"

#include <fmilib.h>
#include <FMI2/fmi2_types.h>
#include <JM/jm_portability.h>
#include <boost/filesystem.hpp>


fmiSubModel2::fmiSubModel2(fmi_import_context_t *ctx) :fmiSubModel(ctx)
{

}

fmiSubModel2::~fmiSubModel2()
{
	if (fmiState >= fmiState_t::fmi_instantiated)
	{
		fmi2_import_free_instance(fmu);
		fmi2_import_destroy_dllfmu(fmu);
	}
	for (auto &oe : oEst)
	{
		if (oe)
		{
			delete oe;
		}
	}
	fmi2_import_free(fmu);
}

gridCoreObject * fmiSubModel2::clone(gridCoreObject *obj) const
{
	auto *gco = cloneBase<fmiSubModel2, gridSubModel>(this, obj);
	if (!(gco))
	{
		return obj;
	}

	return gco;
}

static fmi2_callback_functions_t callBackFunctions;

fmi2_fmu_kind_enu_t fmukind;


void fmiSubModel2::objectInitializeA (double time, unsigned long flags)
{
	if (CHECK_CONTROLFLAG(force_constant_pflow_initialization, flags))
	{
		opFlags.set(pflow_constant_initialization);
	}
}

void fmiSubModel2::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
	if (opFlags[pflow_constant_initialization])
	{
		if (opFlags[pFlow_initialized])
		{


			fmi2_import_get_continuous_states(fmu, m_state.data(), m_stateSize);

			fmi2_import_set_time(fmu, prevTime-0.01);
			if (opFlags[use_output_estimator])
			{
				//if we require the use of output estimators flag that to the simulation and load the information for the estimator
				alert(this, SINGLE_STEP_REQUIRED);
				double val;
				loadOutputJac();
				for (size_t pp = 0; pp < m_outputSize;++pp)
				{
					if (varInfo[outputIndexActive[pp]].refMode >= 4)
					{
						fmi2_import_get_real(fmu, &(outputRefActive[pp]), 1, &val);
						oEst[pp]->update(prevTime, val, args, m_state.data());
					}
				}
				
			}
			opFlags.set(dyn_initialized);
		}
		else//in pflow mode
		{
			fmi2_import_enter_initialization_mode(fmu);
			fmi2_import_set_real(fmu, inputRefActive.data(), m_inputSize, args.data());
			fmi2_import_exit_initialization_mode(fmu);
			fmi2_import_enter_continuous_time_mode(fmu);
			oEst.resize(m_outputSize);
			probeFMU();
			opFlags.set(pFlow_initialized);
			fmiState = fmiState_t::fmi_cont_time;
		}
	}
	else
	{
		fmi2_import_enter_initialization_mode(fmu);

		fmi2_import_set_real(fmu, inputRefActive.data(), m_inputSize, args.data());
		fmi2_import_exit_initialization_mode(fmu);
		fmi2_import_enter_continuous_time_mode(fmu);

		fmi2_import_get_continuous_states(fmu, m_state.data(), m_stateSize);
		oEst.resize(m_outputSize);
		probeFMU();  //probe the fmu 
		if (opFlags[use_output_estimator])
		{
			//if we require the use of output estimators flag that to the simulation and load the information for the estimator
			alert(this, SINGLE_STEP_REQUIRED);
			loadOutputJac();
		}
		fmi2_import_set_time(fmu, prevTime-0.01);
		fmiState = fmiState_t::fmi_cont_time;
		opFlags.set(dyn_initialized);
	}


}

void fmiSubModel2::getParameterStrings(stringVec &pstr, paramStringType pstype) const
{
	int strpcnt = 0;
	switch (pstype)
	{
	case paramStringType::all:
		pstr.reserve(pstr.size() + paramStr.size());
		for (auto &ps : paramStr)
		{
			if (varInfo[ps.second].type == fmi2_base_type_str)
			{
				++strpcnt;
			}
			else
			{
				pstr.push_back(ps.first);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::numeric);
		pstr.reserve(pstr.size() + strpcnt + 1);
		pstr.push_back("#");
		for (auto &ps : paramStr)
		{
			if (varInfo[ps.second].type == fmi2_base_type_str)
			{
				pstr.push_back(ps.first);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::string);
		break;
	case paramStringType::localnum:
		pstr.reserve(paramStr.size());
		pstr.resize(0);
		for (auto &ps : paramStr)
		{
			if (varInfo[ps.second].type != fmi2_base_type_str)
			{
				pstr.push_back(ps.first);
			}
		}
		break;
	case paramStringType::localstr:
		pstr.reserve(paramStr.size());
		pstr.resize(0);
		for (auto &ps : paramStr)
		{
			if (varInfo[ps.second].type != fmi2_base_type_str)
			{
				pstr.push_back(ps.first);
			}
		}
		break;
	case paramStringType::localflags:
		pstr = {};
		break;
	case paramStringType::numeric:
		pstr.reserve(pstr.size() + paramStr.size());
		for (auto &ps : paramStr)
		{
			if (varInfo[ps.second].type != fmi2_base_type_str)
			{
				pstr.push_back(ps.first);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::numeric);
		break;
	case paramStringType::string:
		pstr.reserve(pstr.size() + paramStr.size());
		for (auto &ps : paramStr)
		{
			if (varInfo[ps.second].type == fmi2_base_type_str)
			{
				pstr.push_back(ps.first);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::string);
		break;
	case paramStringType::flags:
		gridSubModel::getParameterStrings(pstr, paramStringType::flags);
		break;
	}
}


stringVec fmiSubModel2::getOutputNames() const
{
	stringVec oVec;
	oVec.reserve(outputStr.size());
	for (auto &os : outputStr)
	{
		oVec.push_back(os.first);
	}
	return oVec;
}

stringVec fmiSubModel2::getInputNames() const
{
	stringVec iVec;
	iVec.reserve(inputStr.size());
	for (auto &is : inputStr)
	{
		iVec.push_back(is.first);
	}
	return iVec;
}


int fmiSubModel2::set (const std::string &param, const std::string &val)
{
	int out = PARAMETER_FOUND;
	if (param == "fmu_dir")
	{
		fmu_dir = val;
		loadFMU();
	}
	else if (param == "fmu")
	{
		fmu_name = val;
		fmu_dir = extractFMU();
		loadFMU();
	}
	else if (param == "outputs")
	{
		auto ssep = splitlineTrim(val);
		outputRefActive.clear();
		outputIndexActive.clear();
		for (auto &out : ssep)
		{
			auto fnd = outputStr.find(out);
			if (fnd != outputStr.end())
			{
				outputIndexActive.push_back(fnd->second);
				outputRefActive.push_back(varInfo[fnd->second].vr);
			}
		}
		m_outputSize = static_cast<count_t>(outputRefActive.size());
	}
	else if (param == "inputs")
	{
		auto ssep = splitlineTrim(val);
		for (auto &iRef : inputIndexActive)
		{
			varInfo[iRef].active = false;
		}
		inputRefActive.clear();
		inputIndexActive.clear();
		for (auto &in : ssep)
		{
			auto fnd = inputStr.find(in);
			if (fnd != inputStr.end())
			{
				inputIndexActive.push_back(fnd->second);
				inputRefActive.push_back(varInfo[fnd->second].vr);
				varInfo[fnd->second].index = static_cast<int>(inputRefActive.size() - 1);
			}
		}
		m_inputSize = static_cast<count_t>(inputRefActive.size());
		for (auto &iRef : inputRefActive)
		{
			varInfo[iRef].active = true;
		}
		updateDependencyInfo();
	}
	else
	{
		auto fnd = paramStr.find(param);
		fmi2_string_t b;
		if (fnd != paramStr.end())
		{
			auto loc = varInfo[fnd->second].vr;
			switch (varInfo[loc].type)
			{
			case fmi2_base_type_str:
				b = val.c_str();
				makeSettableState();
				fmi2_import_set_string(fmu, &loc, 1, &b);
				resetState();
				break;
			default:
				out = gridSubModel::set(param, val);
			}
		}
		else
		{
			out = gridSubModel::set(param, val);
		}

	}
	return out;
}

int fmiSubModel2::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	int out = PARAMETER_FOUND;
	int ival;
	fmi2_boolean_t bval;
	auto param2 = param;
	makeLowerCase(param2);
	if (param2 == "#")
	{

	}
	else if ((param2 == "timestep") || (param2 == "localintegrationtime"))
	{
		localIntegrationTime = val;
	}
	else
	{
		fmi2_value_reference_t loc = -1;
		auto fnd = paramStr.find(param);
		if (fnd != paramStr.end())
		{
			loc = varInfo[fnd->second].vr;

		}
		else
		{
			fnd = inputStr.find(param);
			if (fnd != inputStr.end())
			{
				loc = varInfo[fnd->second].vr;
			}
		}
		if (loc != -1)
		{
			makeSettableState();
			switch (varInfo[loc].type)
			{
			case fmi2_base_type_real:
				fmi2_import_set_real(fmu, &loc, 1, &val);
				break;
			case fmi2_base_type_int:
				ival = static_cast<int>(val);
				fmi2_import_set_integer(fmu, &loc, 1, &ival);
				break;
			case fmi2_base_type_bool:
				bval = static_cast<fmi2_boolean_t>((val > 0));
				fmi2_import_set_boolean(fmu, &loc, 1, &bval);
				break;
			case fmi2_base_type_enum:
				break;
			default:
				out = gridSubModel::set(param, val);
			}
			resetState();
		}
		else
		{
			out = gridSubModel::set(param, val);
		}
	}
	return out;
}

double fmiSubModel2::get(const std::string &param, gridUnits::units_t unitType) const
{

	double val = kNullVal;
	int ival;
	fmi2_boolean_t bval;
	fmi2_value_reference_t loc = -1;
	auto fnd = paramStr.find(param);
	if (fnd != paramStr.end())
	{
		loc = varInfo[fnd->second].vr;
	}
	else
	{
		fnd = outputStr.find(param);
		if (fnd != outputStr.end())
		{
			loc = varInfo[fnd->second].vr;
		}
		else
		{
			fnd = localStr.find(param);
			if (fnd != localStr.end())
			{
				loc = varInfo[fnd->second].vr;
			}
		}
	}
	if (loc != -1)
	{

		switch (varInfo[loc].type)
		{
		case fmi2_base_type_real:
			fmi2_import_get_real(fmu, &loc, 1, &val);
			break;
		case fmi2_base_type_int:
			fmi2_import_get_integer(fmu, &loc, 1, &ival);
			val = static_cast<double>(ival);
			break;
		case fmi2_base_type_bool:
			fmi2_import_get_boolean(fmu, &loc, 1, &bval);
			val = static_cast<double>(bval);
			break;
		case fmi2_base_type_enum:
			break;
		default:
			val = kNullVal;
		}

	}
	return val;
}


void fmiSubModel2::loadSizes(const solverMode &sMode, bool dynOnly)
{
	auto soff = offsets.getOffsets(sMode);
	if (!enabled)
	{
		soff->reset();
		soff->rjLoaded = true;
		soff->stateLoaded = true;
		return;
	}
	if (dynOnly)
	{
		if (soff->rjLoaded)
		{
			return;
		}
		soff->rootAndJacobianCountReset();
		if (!isDynamic(sMode))
		{
			soff->total.jacSize = offsets.local->local.jacSize;
			soff->rjLoaded = true;
			return;
		}

		else
		{
			soff->total.jacSize = offsets.local->local.jacSize;
			soff->total.diffRoots = offsets.local->local.diffRoots;
		}
	}
	else
	{

		if (soff->stateLoaded)
		{
			return;
		}
		soff->reset();

		if (hasDifferential(sMode))
		{
			soff->total.diffSize = m_stateSize;
			soff->total.jacSize = m_jacElements;
		}
		else if (!isDynamic(sMode))
		{
			if (opFlags[pflow_constant_initialization])
			{
				soff->total.algSize = m_stateSize;
				soff->total.jacSize = m_jacElements;
			}
			else
			{

			}
		}
	}
	soff->rjLoaded = true;
	soff->stateLoaded = true;
}


void fmiSubModel2::setState(double ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	if (hasDifferential(sMode))
	{
		auto loc = offsets.getDiffOffset(sMode);
		if (m_stateSize > 0)
		{
			fmi2_import_set_continuous_states(fmu, state + loc, m_stateSize);
			m_state.assign(state + loc, state + loc + m_stateSize);
			m_dstate_dt.assign(dstate_dt + loc, dstate_dt + loc + m_stateSize);
		}
		
		fmi2_import_set_time(fmu, ttime);
		int eventMode;
		int terminate;
		fmi2_import_completed_integrator_step(fmu, true, &eventMode, &terminate);
		
		if ((opFlags[use_output_estimator]) && (!opFlags[fixed_output_interval]))
		{
			
			IOdata ip(m_inputSize);
			fmi2_import_get_real(fmu, inputRefActive.data(), m_inputSize, ip.data());
			for (size_t pp = 0; pp < m_outputSize; ++pp)
			{
				if (varInfo[outputRefActive[pp]].refMode >= 4)
				{
					double val;
					fmi2_import_get_real(fmu, &(outputRefActive[pp]),1, &val);
					bool reload=oEst[pp]->update(ttime, val, ip,state + offsets.getDiffOffset(sMode));
					if (reload)
					{
						loadOutputJac(static_cast<int>(pp));
					}
				}
			}
		}

	}
	else if (!isDynamic(sMode) && (opFlags[pflow_constant_initialization]))
	{
		auto loc = offsets.getAlgOffset(sMode);
		if (m_stateSize > 0)
		{
			fmi2_import_set_continuous_states(fmu, state + loc, m_stateSize);
			m_state.assign(state + loc, state + loc + m_stateSize);
		}
		fmi2_import_set_time(fmu, ttime);
		int eventMode;
		int terminate;
		fmi2_import_completed_integrator_step(fmu, true, &eventMode, &terminate);
		

	}
	prevTime = ttime;
}
//for saving the state
void fmiSubModel2::guess(double ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
	if (m_stateSize == 0)
	{
		return;
	}
	if (hasDifferential(sMode))
	{
		auto loc = offsets.getDiffOffset(sMode);
		fmi2_import_get_continuous_states(fmu, state + loc, m_stateSize);
	}
	else if (!isDynamic(sMode) && (opFlags[pflow_constant_initialization]))
	{
		auto loc = offsets.getAlgOffset(sMode);
		fmi2_import_get_continuous_states(fmu, state + loc, m_stateSize);
	}
}

void fmiSubModel2::getTols(double tols[], const solverMode &sMode)
{

}

void fmiSubModel2::getStateName(stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
	if (hasDifferential(sMode))
	{
		auto loc = offsets.getDiffOffset(sMode);
		if (stNames.size() < loc + m_stateSize)
		{
			stNames.resize(loc + m_stateSize);
		}
		for (size_t kk = 0; kk < m_stateSize; ++kk)
		{
			if (prefix.empty())
			{
				stNames[loc + kk] = name + ':' + varInfo[kk].name;
			}
			else
			{
				stNames[loc + kk] = prefix +  name + ':' + varInfo[kk].name;
			}
			
		}
		
	}
	else if (!isDynamic(sMode) && (opFlags[pflow_constant_initialization]))
	{
		auto loc = offsets.getAlgOffset(sMode);
		if (stNames.size() < loc + m_stateSize)
		{
			stNames.resize(loc + m_stateSize);
		}
		for (size_t kk = 0; kk < m_stateSize; ++kk)
		{
			if (prefix.empty())
			{
				stNames[loc + kk] = name + ':' + varInfo[kk].name;
			}
			else
			{
				stNames[loc + kk] = prefix + '_' + name + ':' + varInfo[kk].name;
			}

		}
	}

}

index_t fmiSubModel2::findIndex(const std::string &field, const solverMode &sMode) const
{
	auto fnd = stateStr.find(field);
	if (fnd != stateStr.end())
	{
		return static_cast<index_t>(varInfo[fnd->second].index);
	}
	return kInvalidLocation;
}

void fmiSubModel2::residual(const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode)
{
	if (hasDifferential(sMode))
	{
		Lp Loc=offsets.getLocations(sD, resid, sMode, this);
		derivative(args, sD, resid, sMode);
		for (size_t ii = 0; ii < Loc.diffSize; ++ii)
		{
			Loc.destDiffLoc[ii] -= Loc.dstateLoc[ii];
		}
	}
	else if (!isDynamic(sMode) && (opFlags[pflow_constant_initialization]))
	{
		derivative(args, sD, resid, sMode);
	}

}

void fmiSubModel2::derivative(const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
	Lp Loc=offsets.getLocations(sD, deriv, sMode, this);
	updateInfo(args, sD, sMode);
	if (isDynamic(sMode))
	{
		fmi2_import_get_derivatives(fmu, Loc.destDiffLoc, Loc.diffSize);
		printf("tt=%f,I=%f, state=%f deriv=%e\n", sD->time,args[0],Loc.diffStateLoc[0],Loc.destDiffLoc[0]);
	}
	else
	{
		fmi2_import_get_derivatives(fmu, Loc.destLoc, Loc.algSize);
		printf("tt=%f,I=%f, state=%f,deriv=%e\n", sD->time,args[0],Loc.algStateLoc[0],Loc.destLoc[0]);
	}
}

const static double gap = 1e-8;
double fmiSubModel2::getPartial(int depIndex, int refIndex, int mode)
{
	double res=0.0;
	double ich = 1.0;
	fmi2_value_reference_t vx = varInfo[depIndex].vr;
	fmi2_value_reference_t vy = varInfo[refIndex].vr;
	if (opFlags[has_derivative_function])
	{
		fmi2_import_get_directional_derivative(fmu, &vx, 1, &vy, 1, &ich, &res);
	}
	else
	{
		double out1, out2;
		double val1, val2;
		fmi2_boolean_t evmd;
		fmi2_boolean_t term;
		fmi2_import_get_real(fmu, &vx, 1, &out1);
		fmi2_import_get_real(fmu, &vy, 1, &val1);
		val2 = val1 + gap;
		if (mode == 0)
		{
			fmi2_import_set_real(fmu, &vy, 1, &val2);
			fmi2_import_get_real(fmu, &vx, 1, &out2);
			fmi2_import_set_real(fmu, &vy, 1, &val1);
			res = (out2 - out1) /gap;
		}
		else if (mode == 1)
		{
			fmi2_import_set_real(fmu, &vy, 1,&val2);
			fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
			fmi2_import_get_real(fmu, &vx, 1, &out2);
			fmi2_import_set_real(fmu, &vy, 1, &val1);
			fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
			res = (out2 - out1) / gap;
		}
		else if (mode==2)
		{
			fmi2_import_get_continuous_states(fmu, tempState.data(), m_stateSize);
			tempState[vy] = val2;
			fmi2_import_set_continuous_states(fmu, tempState.data(), m_stateSize);
			fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
			fmi2_import_get_real(fmu, &vx, 1, &out2); 
			tempState[vy] = val1;
			fmi2_import_set_continuous_states(fmu, tempState.data(), m_stateSize);
			fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
			res = (out2 - out1) / gap;
		}
		else if (mode == 3)  //max useful for states dependent variables
		{
			fmi2_import_get_continuous_states(fmu, tempState.data(), m_stateSize);
			tempState[vy] = val2;
			fmi2_import_set_continuous_states(fmu, tempState.data(), m_stateSize);
			fmi2_import_completed_integrator_step(fmu, false, &evmd, &term);
			fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
			fmi2_import_get_real(fmu, &vx, 1, &out2);
			tempState[vy] = val1;
			fmi2_import_set_continuous_states(fmu, tempState.data(), m_stateSize);
			fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
			fmi2_import_completed_integrator_step(fmu, false, &evmd, &term);
			res = (out2 - out1) /gap;
		}
		else if (mode==4)  //for input dependencies only
		{
			
			fmi2_import_set_real(fmu, &vy, 1, &val2);
			fmi2_import_completed_integrator_step(fmu, false, &evmd, &term);
			fmi2_import_get_real(fmu, &vx, 1, &out2);
			fmi2_import_set_real(fmu, &vy, 1, &val1);
			fmi2_import_completed_integrator_step(fmu, false, &evmd, &term);
			res = (out2 - out1) /gap;
		}
		else if (mode == 5) //for input dependencies only
		{
			fmi2_import_set_real(fmu, &vy, 1, &val2);
			fmi2_import_get_continuous_states(fmu, tempState.data(), m_stateSize);
			fmi2_import_set_continuous_states(fmu, tempState.data(), m_stateSize);
			fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
			fmi2_import_get_real(fmu, &vx, 1, &out2);
			fmi2_import_set_real(fmu, &vy, 1, &val1);
			fmi2_import_set_continuous_states(fmu, tempState.data(), m_stateSize);
			fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
			res = (out2 - out1) / gap;
		}
		else if (mode == 7)  //use the estimators
		{
			if (opFlags[fixed_output_interval])
			{
				res = 0;
			}
			else
			{
				res = oEst[varInfo[depIndex].index]->stateDiff[varInfo[refIndex].index];
			}
			
		}
		else if (mode == 8) //use the estimators
		{
			if (opFlags[fixed_output_interval])
			{
				res = 0;
			}
			else
			{
				res = oEst[varInfo[depIndex].index]->inputDiff[varInfo[refIndex].index];
			}
	
		}
		
	}
	return res;
}
void fmiSubModel2::jacobianElements(const IOdata &args, const stateData *sD,
	arrayData<double> *ad,
	const IOlocs &argLocs, const solverMode &sMode)
{
		if  (hasDifferential(sMode))
		{
			Lp Loc=offsets.getLocations(sD, sMode, this);
			updateInfo(args, sD, sMode);
			double res;
			//for all the inputs
			index_t kk;
			int vu;
			for (kk = 0; kk < Loc.diffSize; ++kk)
			{
				vu = dstateIndex[kk];
				for (int vk : varInfo[vu].inputDep)
				{
					res = getPartial(vu, vk, varInfo[vu].refMode);
					if (res != 0.0)
					{
						ad->assign(Loc.diffOffset + kk, argLocs[varInfo[vk].index], res);
					}

				}
				for (int vk : varInfo[vu].stateDep)
				{
					res = getPartial(vu, vk,varInfo[vu].refMode);
					if (res != 0.0)
					{
						ad->assign(Loc.diffOffset + kk, Loc.diffOffset + varInfo[vk].index, res);
					}
				}
				ad->assign(Loc.diffOffset + kk, Loc.diffOffset+kk, -sD->cj);
				/* this is not allowed in fmus
				for (auto &sR : varInfo[vu].derivDep)
				{
					vk = sR;
					res = getPartial(vu, vk);
					if (res != 0.0)
					{
						ad->assign(Loc.diffOffset + kk, Loc.diffOffset + varInfo[vk].index, res*sD->cj);
					}
				}
				*/
			}
		}
		else if (!isDynamic(sMode) && (opFlags[pflow_constant_initialization]))
		{
			Lp Loc=offsets.getLocations(sD, sMode,  this);
			updateInfo(args, sD, sMode);
			double res;
			double ich = 1.0;
			//for all the inputs
			index_t kk;
			int vu;
			for (kk = 0; kk < m_stateSize; ++kk)
			{
				vu = dstateIndex[kk];
				for (int vk : varInfo[vu].inputDep)
				{
					res = getPartial(vu, vk, varInfo[vu].refMode);
					if (res != 0.0)
					{
						ad->assign(Loc.algOffset + kk, argLocs[varInfo[vk].index], res);
					}
				}
				for (int vk : varInfo[vu].stateDep)
				{
					res = getPartial(vu, vk, varInfo[vu].refMode);
					if (res != 0.0)
					{
						ad->assign(Loc.algOffset + kk, Loc.algOffset + varInfo[vk].index, res);
					}
				}
			}
		}

}

double fmiSubModel2::timestep(double ttime, const IOdata &args, const solverMode &sMode)
{

	double h = localIntegrationTime;
	int sv = 0;
	double aval = 0.95;
	size_t aloc = 7;
	double time = prevTime;
	fmi2_boolean_t eventMode;
	fmi2_boolean_t terminateSim;
	double Tend = ttime;
	std::vector<double> der_x(m_stateSize);
	std::vector<double> der_x2(m_stateSize);
	std::vector<double> prevInput(m_inputSize);
	std::vector<double> inputSlope(m_inputSize);
	//get the previous inputs
	fmi2_import_get_real(fmu, inputRefActive.data(), m_inputSize, prevInput.data());
	//get the current states
	fmi2_import_get_continuous_states(fmu, m_state.data(), m_stateSize);
	//compute the slopes of the inputs
	for (size_t kk = 0; kk < m_inputSize; ++kk)
	{
		inputSlope[kk] = (args[kk] - prevInput[kk]) / (ttime - prevTime);
	}
	while (time < Tend)
	{

		// compute derivatives
		fmi2_import_get_derivatives(fmu, der_x.data(), m_stateSize);
		// advance time

		time = time + h;
		vectorMultAdd(prevInput, inputSlope, h, prevInput);
		fmi2_import_set_real(fmu, inputRefActive.data(), m_inputSize, prevInput.data());
		fmi2_import_set_time(fmu, time);
		// set states at t = time and perform one step
		vectorMultAdd(m_state, der_x, h, m_state);
		fmi2_import_set_continuous_states(fmu, m_state.data(), m_stateSize);
		// get event indicators at t = time
		
		fmi2_import_completed_integrator_step(fmu, false, &eventMode, &terminateSim);
		
		h = (time + h > Tend) ? (Tend - time) : localIntegrationTime;

	}
	prevTime = time;
	double out;
	fmi2_import_get_real(fmu, outputRefActive.data(), 1, &out);
	return out;
}

void fmiSubModel2::ioPartialDerivatives(const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
	updateInfo (args, sD, sMode);
	double res;
	double ich = 1.0;
	index_t kk;
	int kmode = 0;
	fmi2_value_reference_t vk, vu;

		for (kk = 0; kk < m_outputSize; ++kk)
		{
			vu = outputRefActive[kk];
			kmode = varInfo[vu].refMode;
			if (kmode >= 4)
			{
				if (isDynamic(sMode))
				{
					kmode = 8;
				}
			}
			for (auto &sR : varInfo[vu].inputDep)
			{
				if (vu == sR)
				{
					ad->assign(kk, varInfo[sR].index, 1.0);
				}
				else
				{
					vk = sR;
					res = getPartial(vu, vk, kmode);
					if (res != 0.0)
					{
						ad->assign(kk, varInfo[sR].index, res);
					}

				}
			}
		}
	
}

void fmiSubModel2::outputPartialDerivatives(const IOdata &args, const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{
	Lp Loc=offsets.getLocations(sD, sMode, this);
	updateInfo(args, sD, sMode);
	double res;
	index_t kk;
	int kmode;
	auto offsetLoc = isDynamic(sMode) ? Loc.diffOffset : Loc.algOffset;

	fmi2_value_reference_t vk, vu;
	for (kk = 0; kk < m_outputSize; ++kk)
	{
		vu = outputRefActive[kk];
		kmode = varInfo[vu].refMode;
		if (kmode >= 4)
		{
			if (isDynamic(sMode))
			{
				kmode = 7;
			}
		}
		for (auto &sR : varInfo[vu].stateDep)
		{
			if (vu == sR)
			{
				ad->assign(kk, offsetLoc + varInfo[sR].index, 1.0);
			}
			else
			{
				vk = sR;
				res = getPartial(vu, vk, kmode);
				if (res != 0)
				{
					ad->assign(kk, offsetLoc + varInfo[sR].index, res);
				}

			}
		}
	}

}

void fmiSubModel2::rootTest(const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode)
{
	updateInfo(args, sD, sMode);
	auto rootOffset = offsets.getRootOffset(sMode);
	fmi2_import_get_event_indicators(fmu, &(roots[rootOffset]), m_eventCount);
}

void fmiSubModel2::rootTrigger(double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
	fmi2_import_enter_event_mode(fmu);
	//TODO: deal with the event
	fmi2_import_enter_continuous_time_mode(fmu);
}



static const std::vector<std::string> enuVar{ "constant", "fixed", "tunable", "discrete", "continuous", "unknown" };
static const std::vector<std::string> enuCaus{ "param", "calcParam", "input", "output", "local", "ind", "unknown" };
static const std::vector<std::string> enuInit{ "exact", "approx", "calc", "unknown" };

static const std::vector<std::string> enuType{ "real", "int", "bool", "str", "enum" };

void fmiSubModel2::loadFMU()
{

	boost::filesystem::path fmuPath(fmu_dir);
	index_t kk;
	if (!boost::filesystem::exists(fmu_dir / boost::filesystem::path("modelDescription.xml")))
	{
		return;
	}
	if (fmiState != fmiState_t::fmi_startup)
	{
		if (fmiState >= fmiState_t::fmi_instantiated)
		{
			fmi2_import_free_instance(fmu);
			fmi2_import_destroy_dllfmu(fmu);
		}
		fmi2_import_free(fmu);
	}
	fmu = fmi2_import_parse_xml(context, fmu_dir.c_str(), 0);
	if (!fmu)
	{
		LOG_ERROR("Error parsing FMU XML, exiting\n");
		return;
	}
	auto modelName = fmi2_import_get_model_name(fmu);
	description = std::string(modelName);


	m_stateSize = static_cast<count_t>(fmi2_import_get_number_of_continuous_states(fmu));
	offsets.local->local.diffRoots = static_cast<count_t>(fmi2_import_get_number_of_event_indicators(fmu));

	m_state.resize(m_stateSize);
	m_dstate_dt.resize(m_stateSize);
	tempState.resize(m_stateSize);
	tempdState.resize(m_stateSize);
	auto vl = fmi2_import_get_variable_list(fmu, 0);

	auto vsize = fmi2_import_get_variable_list_size(vl);
	varInfo.resize(vsize);

	fmi2_import_variable_t *iv;
	fmi2_value_reference_t vr;
	int index=0;
	for (kk = 0; kk < vsize; ++kk)
	{
		iv = fmi2_import_get_variable(vl, kk);
		auto name = fmi2_import_get_variable_name(iv);
		//auto desc = fmi2_import_get_variable_description(iv);
		//printf("variable %d: %s:: %s ", fmi2_import_get_variable_vr(iv), name, desc);

		vr = fmi2_import_get_variable_vr(iv);
		index = static_cast<int>(vr);
		varInfo[index].vari = fmi2_import_get_variability(iv);
		varInfo[index].caus = fmi2_import_get_causality(iv);
		varInfo[index].name = std::string(name);
		varInfo[index].type = fmi2_import_get_variable_base_type(iv);
		varInfo[index].vr = vr;
		//insert the lookup table into the refmap
		refMap.insert(std::make_pair(vr, kk));
		switch (varInfo[index].caus)
		{

		case fmi2_causality_enu_local:
			localStr.insert(std::make_pair(varInfo[index].name, vr));
			break;
		case fmi2_causality_enu_input:
			inputStr.insert(std::make_pair(varInfo[index].name, vr));
			inputRefActive.push_back(vr);
			varInfo[index].index = static_cast<index_t>(inputStr.size()) - 1;
			inputIndex.push_back(kk);
			break;
		case fmi2_causality_enu_output:
			outputStr.insert(std::make_pair(varInfo[index].name, vr));
			varInfo[index].index = static_cast<index_t>(outputStr.size()) - 1;
			outputIndex.push_back(kk);
			outputRefActive.push_back(vr);
			break;
		case fmi2_causality_enu_parameter:
			paramStr.insert(std::make_pair(varInfo[index].name, vr));
		case fmi2_causality_enu_calculated_parameter:
		case fmi2_causality_enu_independent:
		case fmi2_causality_enu_unknown:
			//don't worry about these
			break;
		}

	}

	m_outputSize = static_cast<count_t>(outputIndex.size());
	m_inputSize = static_cast<count_t>(inputIndex.size());
	
	
	auto derivS = fmi2_import_get_derivatives_list(fmu);
	auto ds = fmi2_import_get_variable_list_size(derivS);
	int sindex;
	for (kk = 0; kk < ds; ++kk)
	{
		iv = fmi2_import_get_variable(derivS, kk);
		vr = fmi2_import_get_variable_vr(iv);
		index = searchByRef(vr);
		dstateIndex.push_back(index);
		varInfo[index].deriv = true;
		auto rv = fmi2_import_get_variable_as_real(iv);
		auto dov = fmi2_import_get_real_variable_derivative_of(rv);
		auto dovi = reinterpret_cast<fmi2_import_variable_t *>(dov);
		auto vref = fmi2_import_get_variable_vr(dovi);
		
		sindex = searchByRef(vref);
		stateIndex.push_back(sindex);
		auto name = fmi2_import_get_variable_name(dovi);
		stateStr.insert(std::make_pair(std::string(name), vref));
		varInfo[index].reference = sindex;
		varInfo[sindex].index = static_cast<index_t>(stateStr.size())-1;
		varInfo[sindex].state = true;
	}
	fmi2_import_free_variable_list(derivS);

	
	size_t *stIn;
	size_t *deps;
	char *deptype;
	size_t si;
	size_t ci = 0;
	if (m_stateSize > 0)
	{
		fmi2_import_get_derivatives_dependencies(fmu, &stIn, &deps, &deptype);
		
		for (kk = 0; kk < m_stateSize; ++kk)
		{
			si = stIn[kk + 1];
			index = dstateIndex[kk];
			while (ci < si)
			{
				varInfo[index].dependencies.push_back(static_cast<int>(deps[ci] - 1));
				++ci;
			}
		}
	}
	if (m_outputSize>0)
	{
		fmi2_import_get_outputs_dependencies(fmu, &stIn, &deps, &deptype);
		ci = 0;
		for (kk = 0; kk < m_outputSize; ++kk)
		{
			si = stIn[kk + 1];
			index = outputIndex[kk];
			while (ci < si)
			{
				varInfo[index].dependencies.push_back(static_cast<int>(deps[ci] - 1));
				++ci;
			}
		}
	}
	fmi2_import_free_variable_list(vl);

	auto cap = fmi2_import_get_capability(fmu, fmi2_me_providesDirectionalDerivatives);
	if (cap)
	{
		opFlags.set(has_derivative_function);
	}
	cap = fmi2_import_get_capability(fmu, fmi2_me_completedIntegratorStepNotNeeded);
	
	updateDependencyInfo();
	fmiState = fmiState_t::fmi_loaded;
	instantiateFMU();
}

IOdata fmiSubModel2::getOutputs(const IOdata &args, const stateData *sD, const solverMode &sMode)
{
	IOdata out(m_outputSize,0);
	if (fmiState >= fmiState_t::fmi_init)
	{
		updateInfo(args, sD, sMode);
		fmi2_import_get_real(fmu, outputRefActive.data(), m_outputSize, out.data());
		printf("time=%f, out1 =%f, out 2=%f\n", (sD) ? sD->time : prevTime, out[0], out[1]);
		if ((opFlags[use_output_estimator]) &&(sD)&& (!opFlags[fixed_output_interval])&&(isDynamic(sMode)))
		{
			for (size_t pp = 0; pp < m_outputSize; ++pp)
			{
				if (varInfo[outputRefActive[pp]].refMode >= 4)
				{
					const double res=oEst[varInfo[outputRefActive[pp]].index]->estimate(sD->time, args, sD->state + offsets.getDiffOffset(sMode));
					out[pp] = res;
				}
			}
		}
	}
	return out;
}


double fmiSubModel2::getDoutdt(const stateData *sD, const solverMode &sMode, index_t num)
{
	return 0;
}
double fmiSubModel2::getOutput(const IOdata &args, const stateData *sD, const solverMode &sMode,index_t num) const
{
	double out = kNullVal;
	if ((fmiState >= fmiState_t::fmi_init) && (num<m_outputSize))
	{
		//updateInfo(args, sD, sMode);
		
		if ((opFlags[use_output_estimator]) && (sD) && (!opFlags[fixed_output_interval]) && (isDynamic(sMode)))
		{
			if (varInfo[outputRefActive[num]].refMode >= 4)
			{
				out = oEst[varInfo[outputRefActive[num]].index]->estimate(sD->time, args, sD->state + offsets.getDiffOffset(sMode));
			}
		}
		else
		{
			fmi2_import_get_real(fmu, &(outputRefActive[num]), m_outputSize, &out);
		}
	}
	return out;
}

double fmiSubModel2::getOutput(index_t num) const
{
	double out = kNullVal;
	if ((fmiState >= fmiState_t::fmi_init) && (num < m_outputSize))
	{
		fmi2_import_get_real(fmu, &(outputRefActive[num]), m_outputSize, &out);
	}
	return out;
}

double fmiSubModel2::getOutputLoc(const IOdata &args, const stateData *sD, const solverMode &sMode, index_t &currentLoc, index_t num) const
{
	currentLoc = kNullLocation;
	return getOutput(args, sD, sMode, num);
}

void fmiSubModel2::updateInfo(const IOdata &args, const stateData *sD, const solverMode &sMode)
{
	fmi2_boolean_t eventMode;
	fmi2_boolean_t terminateSim;
	if (sD)
	{
		if ((sD->seqID == 0) || (sD->seqID != lastSeqID))
		{
			Lp Loc = offsets.getLocations(sD, sMode,  this);
			fmi2_import_set_time(fmu, sD->time);
			if (m_stateSize > 0)
			{
				if (isDynamic(sMode))
				{
					fmi2_import_set_continuous_states(fmu, Loc.diffStateLoc, Loc.diffSize);
				}
				else
				{
					fmi2_import_set_continuous_states(fmu, Loc.algStateLoc, Loc.algSize);
				}
			}
			auto status = fmi2_import_set_real(fmu, inputRefActive.data(), m_inputSize, args.data());
			lastSeqID = sD->seqID;
			if (m_stateSize > 0)
			{
				fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
			}
			if (!isDynamic(sMode))
			{
				fmi2_import_completed_integrator_step(fmu, false, &eventMode, &terminateSim);
			}
			
		}
	}
	else if (!args.empty())
	{
		auto status = fmi2_import_set_real(fmu, inputRefActive.data(), m_inputSize, args.data());
		if (m_stateSize > 0)
		{
			fmi2_import_get_derivatives(fmu, tempdState.data(), m_stateSize);
		}
		if (!isDynamic(sMode))
		{
			fmi2_import_completed_integrator_step(fmu, false, &eventMode, &terminateSim);
		}
	}



}



#define BUFFER 1000

static void fmi2logger(fmi2_component_environment_t env, fmi2_string_t instanceName, fmi2_status_t status, fmi2_string_t category, fmi2_string_t message, ...)
{
	if (strcmp(category, "logFmi2Call") == 0)
	{
		return;
	}
	int len;
	char msg[BUFFER];
	va_list argp;
	va_start(argp, message);
	len = vsnprintf(msg, BUFFER, message, argp);
	printf("fmiStatus = %s;  %s (%s): %s\n", fmi2_status_to_string(status), instanceName, category, msg);
}

static void stepFinished(fmi2_component_environment_t env, fmi2_status_t status)
{
	printf("stepFinished is called with fmiStatus = %s\n", fmi2_status_to_string(status));
}


void fmiSubModel2::instantiateFMU()
{
	callBackFunctions.logger = fmi2logger;
	callBackFunctions.allocateMemory = calloc;
	callBackFunctions.freeMemory = free;
	callBackFunctions.stepFinished = stepFinished;
	callBackFunctions.componentEnvironment = 0;

	auto status = fmi2_import_create_dllfmu(fmu, fmi2_fmu_kind_me, &callBackFunctions);
	if (status == jm_status_error)
	{
		LOG_ERROR("Could not create the DLL loading mechanism(C-API).");
		opFlags.set(error_flag);
		return;
	}
	status = fmi2_import_instantiate(fmu, name.c_str(), fmi2_model_exchange, NULL, 0);
	if (status == jm_status_error)
	{
		LOG_ERROR("Could not create the instantiate the FMU.");
		opFlags.set(error_flag);
		return;
	}
	fmiState = fmiState_t::fmi_instantiated;

}

void fmiSubModel2::updateDependencyInfo()
{
	for (auto &vI : outputRefActive)
	{
		varInfo[vI].inputDep.clear();
		varInfo[vI].stateDep.clear();
		varInfo[vI].derivDep.clear();

		for (auto &dI : varInfo[vI].dependencies)
		{
			if (varInfo[dI].active == false)
			{
				continue;
			}
			if (varInfo[dI].state)
			{
				varInfo[vI].stateDep.push_back(dI);
			}
			else if (varInfo[dI].deriv)
			{
				varInfo[vI].derivDep.push_back(dI);
			}
			else if (varInfo[dI].caus == fmi2_causality_enu_input)
			{
				for (auto aI : inputIndexActive)
				{
					if (aI == dI)
					{
						varInfo[vI].inputDep.push_back(dI);
						break;
					}
					
				}
				
			}
		}
	}
	m_jacElements = 0;
	for (auto &vI : dstateIndex)
	{
		varInfo[vI].inputDep.clear();
		varInfo[vI].stateDep.clear();
		varInfo[vI].derivDep.clear();

		for (auto &dI : varInfo[vI].dependencies)
		{
			if (varInfo[dI].active == false)
			{
				continue;
			}
			if (varInfo[dI].state)
			{
				varInfo[vI].stateDep.push_back(dI);
			}
			else if (varInfo[dI].deriv)
			{
				varInfo[vI].derivDep.push_back(dI);
			}
			else if (varInfo[dI].caus == fmi2_causality_enu_input)
			{
				for (auto aI : inputIndexActive)
				{
					if (aI == dI)
					{
						varInfo[vI].inputDep.push_back(dI);
						break;
					}

				}
			}
		}
		m_jacElements += static_cast<count_t>(varInfo[vI].stateDep.size() + varInfo[vI].derivDep.size() + varInfo[vI].inputDep.size());
	}
}

void fmiSubModel2::makeSettableState()
{
	prevFmiState = fmiState;
	switch (fmiState)
	{
	case fmiState_t::fmi_cont_time:
		fmi2_import_enter_event_mode(fmu);
		fmiState = fmiState_t::fmi_event;
		break;
	default:
		return;
	}
}
void fmiSubModel2::resetState()
{
	if (prevFmiState == fmiState)
	{
		return;
	}
	switch (fmiState)
	{
	case fmiState_t::fmi_event:
		fmi2_import_enter_continuous_time_mode(fmu);
		fmiState = fmiState_t::fmi_cont_time;
		break;
	default:
		break;
	}
	prevFmiState = fmiState;
}

void fmiSubModel2::probeFMU()
{
	int mode;
	int defMode = (m_stateSize>0)?1:4;
	double res = 0;
	if (opFlags[reprobe_flag])
	{
		defMode = (m_stateSize>0) ? 0 : 4;
	}
	for (auto kk : dstateIndex)
	{
		mode = -1;

		for (auto &dep : varInfo[kk].stateDep)
		{
			res = getPartial(kk, dep, 0);
			if (res != 0)
			{
				mode = max(mode, 0);
				continue;
			}
			res = getPartial(kk, dep, 1);
			if (res != 0)
			{
				mode = max(mode, 1);
				continue;
			}
			res = getPartial(kk, dep, 2);
			if (res != 0)
			{
				mode = max(mode, 2);
				continue;
			}
			res = getPartial(kk, dep, 3);
			if (res != 0)
			{
				mode = max(mode, 3);
				continue;
			}
			mode = max(mode, defMode);
			opFlags.set(reprobe_flag);
		}
		for (auto &dep : varInfo[kk].inputDep)
		{
			res = getPartial(kk, dep, 0);
			if (res != 0)
			{
				mode = max(mode, 0);
				continue;
			}
			res = getPartial(kk, dep, 1);
			if (res != 0)
			{
				mode = max(mode, 1);
				continue;
			}
			mode = max(mode, defMode);
			opFlags.set(reprobe_flag);
		}
		varInfo[kk].refMode = mode;
	}
	for (auto kk : outputIndexActive)
	{
		mode = -1;
		for (auto &dep : varInfo[kk].stateDep)
		{
			res = getPartial(kk, dep, 0);
			if (res != 0)
			{
				mode = max(mode, 0);
				continue;
			}
			res = getPartial(kk, dep, 1);
			if (res != 0)
			{
				mode = max(mode, 1);
				continue;
			}
			res = getPartial(kk, dep, 2);
			if (res != 0)
			{
				mode = max(mode, 2);
				continue;
			}
			res = getPartial(kk, dep, 4);
			if (res != 0)
			{
				mode = max(mode, 4);
				continue;
			}
			res = getPartial(kk, dep, 5);
			if (res != 0)
			{
				mode = max(mode, 5);
				continue;
			}
			mode = max(mode, defMode);
			opFlags.set(reprobe_flag);
		}
		for (auto &dep : varInfo[kk].inputDep)
		{
			res = getPartial(kk, dep, 0);
			if (res != 0)
			{
				mode = max(mode, 0);
				continue;
			}
			if (m_stateSize > 0)
			{
				res = getPartial(kk, dep, 1);
				if (res != 0)
				{
					mode = max(mode, 1);
					continue;
				}
			}
			res = getPartial(kk, dep, 4);
			if (res != 0)
			{
				mode = max(mode, 4);
				continue;
			}
			if (m_stateSize > 0)
			{
				res = getPartial(kk, dep, 5);
				if (res != 0)
				{
					mode = max(mode, 5);
					continue;
				}
			}
			mode = max(mode, defMode);
			opFlags.set(reprobe_flag);
		}
		varInfo[kk].refMode = mode;
		if (mode >= 4)
		{
			opFlags.set(use_output_estimator);
			std::vector<int> sDep(varInfo[kk].stateDep.size());
			std::vector<int> iDep(varInfo[kk].inputDep.size());
			for (size_t dd = 0; dd < varInfo[kk].stateDep.size(); ++dd)
			{
				sDep[dd] = varInfo[varInfo[kk].stateDep[dd]].index;
			}
			for (size_t dd = 0; dd < varInfo[kk].inputDep.size(); ++dd)
			{
				iDep[dd] = varInfo[varInfo[kk].inputDep[dd]].index;
			}
			oEst[varInfo[kk].index] = new outputEstimator(sDep, iDep);
		}
	}


}

void fmiSubModel2::loadOutputJac(int index)
{
	double pd;
	int ct = 0;
	if (index == -1)
	{
		for (auto &out : outputIndexActive)
		{
			if (varInfo[out].refMode >= 4)
			{
				ct = 0;
				for (auto kk : varInfo[out].stateDep)
				{
					pd = getPartial(out, kk, varInfo[out].refMode);
					oEst[varInfo[out].index]->stateDiff[ct] = pd;
					++ct;
				}
				ct = 0;
				for (auto kk : varInfo[out].inputDep)
				{
					pd = getPartial(out, kk, varInfo[out].refMode);
					oEst[varInfo[out].index]->inputDiff[ct] = pd;
					++ct;
				}
			}
		}
	}
	else
	{
		if (varInfo[index].refMode >= 4)
		{
			ct = 0;
			for (auto kk : varInfo[index].stateDep)
			{
				pd = getPartial(index, kk, varInfo[index].refMode);
				oEst[varInfo[index].index]->stateDiff[ct] = pd;
				++ct;
			}
			ct = 0;
			for (auto kk : varInfo[index].inputDep)
			{
				pd = getPartial(index, kk, varInfo[index].refMode);
				oEst[varInfo[index].index]->inputDiff[ct] = pd;
				++ct;
			}
		}
	}
}


int fmiSubModel2::searchByRef(fmi2_value_reference_t ref)
{
	auto loc = refMap.find(ref);
	return (loc != refMap.end()) ? loc->second : -1;
	
}

