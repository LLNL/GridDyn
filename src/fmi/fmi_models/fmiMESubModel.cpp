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



#include "fmiMESubModel.h"
#include "core/coreObjectTemplates.hpp"
#include "fmi_import/fmiLibraryManager.h"
#include "fmi_import/fmiObjects.h"
#include "utilities/vectorOps.hpp"
#include  "utilities/matrixData.hpp"
#include "outputEstimator.h"
#include "utilities/stringOps.h"
#include "core/coreExceptions.h"

#include <algorithm>


namespace griddyn
{
namespace fmi
{
fmiMESubModel::fmiMESubModel(const std::string &newName, std::shared_ptr<fmi2ModelExchangeObject> fmi):gridSubModel(newName), me(fmi)
{

}

fmiMESubModel::fmiMESubModel(std::shared_ptr<fmi2ModelExchangeObject> fmi) : me(fmi)
{

}

fmiMESubModel::~fmiMESubModel()
{
	
}

coreObject * fmiMESubModel::clone(coreObject *obj) const
{
	auto *gco = cloneBase<fmiMESubModel, gridSubModel>(this, obj);
	if (!(gco))
	{
		return obj;
	}

	return gco;
}

bool fmiMESubModel::isLoaded() const
{
	return (me)?true:false;
}

void fmiMESubModel::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
	if (CHECK_CONTROLFLAG(force_constant_pflow_initialization, flags))
	{
		opFlags.set(pflow_init_required);
	}
	prevTime = time0;
}

void fmiMESubModel::dynObjectInitializeB (const IOdata &inputs, const IOdata & /*desiredOutput*/, IOdata & /*inputSet*/)
{
	if (opFlags[pflow_init_required])
	{
		if (opFlags[pFlow_initialized])
		{

			me->getStates(m_state.data());
			me->setTime(prevTime - 0.01);

			if (opFlags[use_output_estimator])
			{
				//if we require the use of output estimators flag that to the simulation and load the information for the estimator
				alert(this, SINGLE_STEP_REQUIRED);
				double val;
				loadOutputJac();
				for (index_t pp = 0; pp < m_outputSize;++pp)
				{
					if (outputInformation[pp].refMode >= refMode_t::level4)
					{
						val = me->getOutput(pp);
						oEst[pp]->update(prevTime, val, inputs, m_state.data());
					}
				}
				
			}
			opFlags.set(dyn_initialized);
		}
		else//in pflow mode
		{
			me->setMode(fmuMode::initializationMode);
			
			me->setInputs(inputs.data());
			me->setMode(fmuMode::continuousTimeMode);
			oEst.resize(m_outputSize);
			probeFMU();
			opFlags.set(pFlow_initialized);
		}
	}
	else
	{
		me->setMode(fmuMode::initializationMode);

		me->setInputs(inputs.data());
		me->setMode(fmuMode::continuousTimeMode);

		me->getStates(m_state.data());
		oEst.resize(m_outputSize);
		probeFMU();  //probe the fmu 
		if (opFlags[use_output_estimator])
		{
			//if we require the use of output estimators flag that to the simulation and load the information for the estimator
			alert(this, SINGLE_STEP_REQUIRED);
			loadOutputJac();
		}
		me->setTime(prevTime-0.01);
	}


}



void fmiMESubModel::getParameterStrings(stringVec &pstr, paramStringType pstype) const
{
	int strpcnt = 0;
	auto info = me->fmuInformation();
	auto vcnt = info->getCounts("variables");
	switch (pstype)
	{
	case paramStringType::all:
		pstr.reserve(pstr.size() + info->getCounts("params")+info->getCounts("inputs")-m_inputSize);
		
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (info->getVariableInfo(kk).type == fmi_variable_type_t::string)
			{
				++strpcnt;
			}
			else if (checkType(info->getVariableInfo(kk),fmi_variable_type_t::numeric,fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		
		gridSubModel::getParameterStrings(pstr, paramStringType::numeric);
		pstr.reserve(pstr.size() + strpcnt + 1);
		pstr.push_back("#");
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_variable_type_t::string, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::str);
		break;
	case paramStringType::localnum:
		pstr.reserve(info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		pstr.resize(0);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_variable_type_t::numeric, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		break;
	case paramStringType::localstr:
		pstr.reserve(info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		pstr.resize(0);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_variable_type_t::string, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		break;
	case paramStringType::localflags:
		pstr.reserve(info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		pstr.resize(0);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_variable_type_t::boolean, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		break;
	case paramStringType::numeric:
		pstr.reserve(pstr.size()+info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_variable_type_t::numeric, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::numeric);
		break;
	case paramStringType::str:
		pstr.reserve(pstr.size()+info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_variable_type_t::string, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::str);
		break;
	case paramStringType::flags:
		pstr.reserve(pstr.size()+info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_variable_type_t::boolean, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::flags);
		break;
	}
}


stringVec fmiMESubModel::getOutputNames() const
{
	return me->getOutputNames();
	
}

stringVec fmiMESubModel::getInputNames() const
{
	return me->getInputNames();
}


void fmiMESubModel::set (const std::string &param, const std::string &val)
{
	if (param == "fmu")
	{
		if (!(me))
		{
			me = fmiLibraryManager::instance().createModelExchangeObject(val, getName());
		}
		else
		{
			throw(invalidParameterValue(param));
		}
		
	}
	else if (param == "outputs")
	{
		auto ssep = stringOps::splitline(val);
		stringOps::trim(ssep);
		me->setOutputVariables(ssep);
		m_outputSize = me->outputSize();
	}
	else if (param == "inputs")
	{
		auto ssep = stringOps::splitline(val);
		stringOps::trim(ssep);
		me->setInputVariables(ssep);
		m_inputSize = me->inputSize();
		//updateDependencyInfo();
	}
	else
	{
		bool isparam=me->isParameter(param, fmi_variable_type_t::string);
		if (isparam)
		{
			makeSettableState();
			me->set(param, val);
			resetState();
		}
		else
		{
			gridSubModel::set(param, val);
		}

	}

}

void fmiMESubModel::set (const std::string &param, double val, gridUnits::units_t unitType)
{

	if ((param == "timestep") || (param == "localintegrationtime"))
	{
		localIntegrationTime = val;
	}
	else
	{
		bool isparam = me->isParameter(param, fmi_variable_type_t::numeric);
		if (isparam)
		{
			makeSettableState();
			me->set(param, val);
			resetState();
		}
		else
		{
			gridSubModel::set(param, val,unitType);
		}
	}

}

double fmiMESubModel::get(const std::string &param, gridUnits::units_t unitType) const
{

	if (param == "localintegrationtime")
	{
		return static_cast<double>(localIntegrationTime);
	}
	else
	{
		if (me->isVariable(param, fmi_variable_type_t::numeric))
		{
			return me->get<double>(param);
		}
		else
		{
			return gridSubModel::get(param, unitType);
		}
	}
	
}


void fmiMESubModel::loadSizes(const solverMode &sMode, bool dynOnly)
{
	auto &soff = offsets.getOffsets(sMode);
	if (!isEnabled())
	{
		soff.reset();
		soff.rjLoaded = true;
		soff.stateLoaded = true;
		return;
	}
	if (dynOnly)
	{
		if (soff.rjLoaded)
		{
			return;
		}
		soff.rootAndJacobianCountReset();
		if (!isDynamic(sMode))
		{
			soff.total.jacSize = offsets.local().local.jacSize;
			soff.rjLoaded = true;
			return;
		}

		else
		{
			soff.total.jacSize = offsets.local().local.jacSize;
			soff.total.diffRoots = offsets.local().local.diffRoots;
		}
	}
	else
	{

		if (soff.stateLoaded)
		{
			return;
		}
		soff.reset();

		if (hasDifferential(sMode))
		{
			soff.total.diffSize = m_stateSize;
			soff.total.jacSize = m_jacElements;
		}
		else if (!isDynamic(sMode))
		{
			if (opFlags[pflow_init_required])
			{
				soff.total.algSize = m_stateSize;
				soff.total.jacSize = m_jacElements;
			}
			else
			{

			}
		}
	}
	soff.rjLoaded = true;
	soff.stateLoaded = true;
}


void fmiMESubModel::setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	if (hasDifferential(sMode))
	{
		auto loc = offsets.getDiffOffset(sMode);
		if (m_stateSize > 0)
		{
			me->setStates(state + loc);
			m_state.assign(state + loc, state + loc + m_stateSize);
			m_dstate_dt.assign(dstate_dt + loc, dstate_dt + loc + m_stateSize);
		}
		
		me->setTime(time);
		int eventMode;
		int terminate;
		me->completedIntegratorStep(true, &eventMode, &terminate);
		
		if ((opFlags[use_output_estimator]) && (!opFlags[fixed_output_interval]))
		{
			
			IOdata ip(m_inputSize);

			me->getCurrentInputs(ip.data());
			for (index_t pp = 0; pp < m_outputSize; ++pp)
			{
				if (outputInformation[pp].refMode >= refMode_t::level4)
				{
					double val;
					val = me->getOutput(pp);
					bool reload=oEst[pp]->update(time, val, ip,state + offsets.getDiffOffset(sMode));
					if (reload)
					{
						loadOutputJac(static_cast<int>(pp));
					}
				}
			}
		}

	}
	else if (!isDynamic(sMode) && (opFlags[pflow_init_required]))
	{
		auto loc = offsets.getAlgOffset(sMode);
		if (m_stateSize > 0)
		{
			me->setStates(state + loc);
			m_state.assign(state + loc, state + loc + m_stateSize);
		}
		me->setTime(time);
		int eventMode;
		int terminate;
		me->completedIntegratorStep(true, &eventMode, &terminate);
		

	}
	prevTime = time;
}
//for saving the state
void fmiMESubModel::guessState(coreTime /*time*/, double state[], double dstate_dt[], const solverMode &sMode)
{
	if (m_stateSize == 0)
	{
		return;
	}
	if (hasDifferential(sMode))
	{
		auto loc = offsets.getDiffOffset(sMode);
		me->getStates(state + loc);
		me->getDerivatives(dstate_dt + loc);
	}
	else if (!isDynamic(sMode) && (opFlags[pflow_init_required]))
	{
		auto loc = offsets.getAlgOffset(sMode);
		me->getStates(state + loc);
	}
}

void fmiMESubModel::getTols(double /*tols*/ [], const solverMode & /*sMode*/)
{

}

void fmiMESubModel::getStateName(stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
	if (hasDifferential(sMode))
	{
		auto loc = offsets.getDiffOffset(sMode);
		if (static_cast<count_t>(stNames.size()) < loc + m_stateSize)
		{
			stNames.resize(loc + m_stateSize);
		}
		auto fmistNames = me->getStateNames();
		for (index_t kk = 0; kk < m_stateSize; ++kk)
		{
			if (prefix.empty())
			{
				stNames[loc + kk] = getName() + ':' + fmistNames[kk];
			}
			else
			{
				stNames[loc + kk] = prefix + getName() + ':' +  fmistNames[kk];
			}
			
		}
		
	}
	else if (!isDynamic(sMode) && (opFlags[pflow_init_required]))
	{
		auto loc = offsets.getAlgOffset(sMode);
		if (static_cast<count_t>(stNames.size()) < loc + m_stateSize)
		{
			stNames.resize(loc + m_stateSize);
		}
		auto fmistNames = me->getStateNames();
		for (index_t kk = 0; kk < m_stateSize; ++kk)
		{
			if (prefix.empty())
			{
				stNames[loc + kk] = getName() + ':' + fmistNames[kk];
			}
			else
			{
				stNames[loc + kk] = prefix + '_' + getName() + ':' + fmistNames[kk];
			}

		}
	}

}

index_t fmiMESubModel::findIndex(const std::string &field, const solverMode & /*sMode*/) const
{
	auto fmistNames = me->getStateNames();
	auto fnd = std::find(fmistNames.begin(), fmistNames.end(), field);
	if (fnd != fmistNames.end())
	{
		return static_cast<index_t>(fnd-fmistNames.begin());
	}
	return kInvalidLocation;
}

void fmiMESubModel::residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
	if (hasDifferential(sMode))
	{
		Lp Loc=offsets.getLocations(sD, resid, sMode, this);
		derivative(inputs, sD, resid, sMode);
		for (index_t ii = 0; ii < Loc.diffSize; ++ii)
		{
			Loc.destDiffLoc[ii] -= Loc.dstateLoc[ii];
		}
	}
	else if (!isDynamic(sMode) && (opFlags[pflow_init_required]))
	{
		derivative(inputs, sD, resid, sMode);
	}

}

void fmiMESubModel::derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
	Lp Loc=offsets.getLocations(sD, deriv, sMode, this);
	updateLocalCache(inputs, sD, sMode);
	if (isDynamic(sMode))
	{
		me->getDerivatives(Loc.destDiffLoc);
		printf("tt=%f,I=%f, state=%f deriv=%e\n", static_cast<double>(sD.time),inputs[0],Loc.diffStateLoc[0],Loc.destDiffLoc[0]);
	}
	else
	{
		me->getDerivatives(Loc.destLoc);
		printf("tt=%f,I=%f, state=%f,deriv=%e\n", static_cast<double>(sD.time),inputs[0],Loc.algStateLoc[0],Loc.destLoc[0]);
	}
}

const static double gap = 1e-8;
double fmiMESubModel::getPartial(int depIndex, int refIndex, refMode_t mode)
{
	double res=0.0;
	double ich = 1.0;
	fmiVariableSet vx = me->getVariableSet(depIndex);
	fmiVariableSet vy = me->getVariableSet(refIndex);
	if (opFlags[has_derivative_function])
	{
		res = me->getPartialDerivative(depIndex, refIndex, ich);
	}
	else
	{
		double out1, out2;
		double val1, val2;
		fmi2Boolean evmd;
		fmi2Boolean term;

		me->get(vx, &out1);
		me->get(vy, &val1);
		val2 = val1 + gap;
		if (mode == refMode_t::direct)
		{
			me->set(vy, &val2);
			me->get(vx, &out2);
			me->set(vy, &val1);
			res = (out2 - out1) /gap;
		}
		else if (mode == refMode_t::level1)
		{
			me->set(vy, &val2);
			me->getDerivatives(tempdState.data());
			me->get(vx, &out2);
			me->set(vy, &val1);
			me->getDerivatives(tempdState.data());
			res = (out2 - out1) / gap;
		}
		else if (mode == refMode_t::level2)
		{
			me->getStates(tempState.data());
			tempState[refIndex] = val2;
			me->setStates(tempState.data());
			me->getDerivatives(tempdState.data());
			me->get(vx, &out2);
			tempState[refIndex] = val1;
			me->setStates(tempState.data());
			me->getDerivatives(tempdState.data());
			res = (out2 - out1) / gap;
		}
		else if (mode == refMode_t::level3)  //max useful for states dependent variables
		{
			me->getStates(tempState.data());
			tempState[refIndex] = val2;
			me->setStates(tempState.data());
			me->completedIntegratorStep(false, &evmd, &term);
			me->getDerivatives(tempdState.data());

			me->get(vx, &out2);
			tempState[refIndex] = val1;
			me->setStates(tempState.data());
			me->getDerivatives(tempdState.data());
			me->completedIntegratorStep(false, &evmd, &term);
			res = (out2 - out1) /gap;
		}
		else if (mode == refMode_t::level4)  //for input dependencies only
		{
			
			me->set(vy, &val2);
			me->completedIntegratorStep(false, &evmd, &term);
			me->get(vx, &out2);
			me->set(vy, &val1);
			me->completedIntegratorStep(false, &evmd, &term);
			res = (out2 - out1) /gap;
		}
		else if (mode == refMode_t::level5) //for input dependencies only
		{
			me->set(vy, &val2);
			me->getStates(tempState.data());
			me->setStates(tempState.data());
			me->getDerivatives(tempdState.data());
			me->get(vx, &out2);
			me->set(vy, &val1);
			me->setStates(tempState.data());
			me->getDerivatives(tempdState.data());
			res = (out2 - out1) / gap;
		}
		else if (mode == refMode_t::level7)  //use the estimators
		{
			if (opFlags[fixed_output_interval])
			{
				res = 0;
			}
			else
			{
				res = oEst[depIndex]->stateDiff[refIndex];
			}
			
		}
		else if (mode == refMode_t::level8) //use the estimators
		{
			if (opFlags[fixed_output_interval])
			{
				res = 0;
			}
			else
			{
				res = oEst[depIndex]->inputDiff[refIndex]; //TODO:: this is wrong
			}
	
		}
		
	}
	return res;
}
void fmiMESubModel::jacobianElements(const IOdata &inputs, const stateData &sD,
	matrixData<double> &md,
	const IOlocs &inputLocs, const solverMode &sMode)
{
		if  (hasDifferential(sMode))
		{
			Lp Loc=offsets.getLocations(sD, sMode, this);
			updateLocalCache(inputs, sD, sMode);
			//for all the inputs
			for (index_t kk = 0; kk < Loc.diffSize; ++kk)
			{
				int vu = stateInformation[kk].varIndex;
				for (int vk : stateInformation[kk].inputDep)
				{
					double res = getPartial(vu, inputVarIndices[vk], stateInformation[kk].refMode);
					if (res != 0.0)
					{
						md.assign(Loc.diffOffset + kk, inputLocs[vk], res);
					}

				}
				for (int vk : stateInformation[kk].stateDep)
				{
					double res = getPartial(vu, stateInformation[vk].varIndex, stateInformation[kk].refMode);
					if (res != 0.0)
					{
						md.assign(Loc.diffOffset + kk, Loc.diffOffset + vk, res);
					}
				}
				md.assign(Loc.diffOffset + kk, Loc.diffOffset+kk, -sD.cj);
				/* this is not allowed in fmus
				for (auto &sR : varInfo[vu].derivDep)
				{
					vk = sR;
					res = getPartial(vu, vk);
					if (res != 0.0)
					{
						md.assign(Loc.diffOffset + kk, Loc.diffOffset + varInfo[vk].index, res*sD.cj);
					}
				}
				*/
			}
		}
		else if (!isDynamic(sMode) && (opFlags[pflow_init_required]))
		{
			Lp Loc=offsets.getLocations(sD, sMode,  this);
			updateLocalCache(inputs, sD, sMode);
			//for all the inputs
			for (index_t kk = 0; kk < m_stateSize; ++kk)
			{
				int vu = stateInformation[kk].varIndex;
				for (int vk : stateInformation[kk].inputDep)
				{
					double res = getPartial(vu, inputVarIndices[vk], stateInformation[kk].refMode);
					if (res != 0.0)
					{
						md.assign(Loc.algOffset + kk, inputLocs[vk], res);
					}
				}
				for (int vk : stateInformation[kk].stateDep)
				{
					double res = getPartial(vu, stateInformation[vk].varIndex, stateInformation[kk].refMode);
					if (res != 0.0)
					{
						md.assign(Loc.algOffset + kk, Loc.algOffset + kk, res);
					}
				}
			}
		}

}

void fmiMESubModel::timestep(coreTime time, const IOdata &inputs, const solverMode & /*sMode*/)
{

	coreTime h = localIntegrationTime;
	//int sv = 0;
	//double aval = 0.95;
	//size_t aloc = 7;
	coreTime curTime = prevTime;
	fmi2Boolean eventMode;
	fmi2Boolean terminateSim;
	coreTime Tend = time;
	std::vector<double> der_x(m_stateSize);
	std::vector<double> der_x2(m_stateSize);
	std::vector<double> prevInput(m_inputSize);
	std::vector<double> inputSlope(m_inputSize);
	//get the previous inputs
	me->getCurrentInputs(prevInput.data());
	//get the current states
	me->getStates(m_state.data());
	//compute the slopes of the inputs
	double h2 = 1.0 / (time - prevTime);
	for (index_t kk = 0; kk < m_inputSize; ++kk)
	{
		inputSlope[kk] = (inputs[kk] - prevInput[kk]) *h2;
	}
	while (curTime < Tend)
	{

		// compute derivatives
		me->getDerivatives(der_x.data());
		// advance time

		curTime+= h;
		vectorMultAdd(prevInput, inputSlope, static_cast<double>(h), prevInput);
		me->setInputs(prevInput.data());
		me->setTime(curTime);
		// set states at t = time and perform one step
		vectorMultAdd(m_state, der_x, static_cast<double>(h), m_state);
		me->setStates(m_state.data());
		
		// get event indicators at t = time
		me->completedIntegratorStep(false, &eventMode, &terminateSim);
		
		h = (curTime + h > Tend) ? (Tend - curTime) : localIntegrationTime;

	}
	prevTime = time;
}

void fmiMESubModel::ioPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs & /*inputLocs*/, const solverMode &sMode)
{
	updateLocalCache (inputs, sD, sMode);
	double ich = 1.0;

		for (index_t kk = 0; kk < m_outputSize; ++kk)
		{
			int vu = outputInformation[kk].varIndex;
			auto kmode = outputInformation[kk].refMode;
			if (kmode >= refMode_t::level4)
			{
				if (isDynamic(sMode))
				{
					kmode = refMode_t::level8;
				}
			}
			for (auto &sR : outputInformation[kk].inputDep)
			{
				if (vu == inputVarIndices[sR])
				{
					md.assign(kk, sR, ich);
				}
				else
				{
					double res = getPartial(vu, inputVarIndices[sR], kmode);
					if (res != 0.0)
					{
						md.assign(kk, sR, res);
					}

				}
			}
		}
	
}

void fmiMESubModel::outputPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const solverMode &sMode)
{
	Lp Loc=offsets.getLocations(sD, sMode, this);
	updateLocalCache(inputs, sD, sMode);
	auto offsetLoc = isDynamic(sMode) ? Loc.diffOffset : Loc.algOffset;

	for (index_t kk = 0; kk < m_outputSize; ++kk)
	{
		int vu = outputInformation[kk].varIndex;
		auto kmode = outputInformation[kk].refMode;
		if (kmode >= refMode_t::level4)
		{
			if (isDynamic(sMode))
			{
				kmode = refMode_t::level7;
			}
		}
		if (outputInformation[kk].isState)
		{
			md.assign(kk, kk, 1.0);
		}
		else
		{
			for (auto &sR : outputInformation[kk].stateDep)
			{

				int vk = stateInformation[sR].varIndex;
				double res = getPartial(vu, vk, kmode);
				if (res != 0)
				{
					md.assign(kk, offsetLoc + sR, res);
				}

			}
		}
		
	}

}

void fmiMESubModel::rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)
{
	updateLocalCache(inputs, sD, sMode);
	auto rootOffset = offsets.getRootOffset(sMode);
	me->getEventIndicators(&(roots[rootOffset]));
}

void fmiMESubModel::rootTrigger(coreTime /*time*/, const IOdata & /*inputs*/, const std::vector<int> & /*rootMask*/, const solverMode & /*sMode*/)
{
	me->setMode(fmuMode::eventMode);
	//TODO: deal with the event
	me->setMode(fmuMode::continuousTimeMode);
}





IOdata fmiMESubModel::getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
	IOdata out(m_outputSize,0);
	if (me->getCurrentMode() >= fmuMode::initializationMode)
	{
		//updateInfo(inputs, sD, sMode);
		me->getOutputs(out.data());
		printf("time=%f, out1 =%f, out 2=%f\n", static_cast<double>((!sD.empty()) ? sD.time : prevTime), out[0], out[1]);
		if ((opFlags[use_output_estimator]) &&(!sD.empty())&& (!opFlags[fixed_output_interval])&&(isDynamic(sMode)))
		{
			for (index_t pp = 0; pp < m_outputSize; ++pp)
			{
				if (outputInformation[pp].refMode >= refMode_t::level4)
				{
					const double res=oEst[pp]->estimate(sD.time, inputs, sD.state + offsets.getDiffOffset(sMode));
					out[pp] = res;
				}
			}
		}
	}
	return out;
}


double fmiMESubModel::getDoutdt(const IOdata &/*inputs*/,const stateData & /*sD*/, const solverMode & /*sMode*/, index_t /*num*/) const
{
	return 0;
}

double fmiMESubModel::getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode,index_t num) const
{
	double out = kNullVal;
	if (me->getCurrentMode() >= fmuMode::initializationMode)
	{
		//updateInfo(inputs, sD, sMode);
		
		if ((opFlags[use_output_estimator]) && (!sD.empty()) && (!opFlags[fixed_output_interval]) && (isDynamic(sMode)))
		{
			if (outputInformation[num].refMode >= refMode_t::level4)
			{
				out = oEst[num]->estimate(sD.time, inputs, sD.state + offsets.getDiffOffset(sMode));
			}
		}
		else
		{
			out=me->getOutput(num);
		}
	}
	return out;
}

double fmiMESubModel::getOutput(index_t num) const
{
	double out = kNullVal;
	if (me->getCurrentMode() >= fmuMode::initializationMode)
	{
		out = me->getOutput(num);
	}
	return out;
}

index_t fmiMESubModel::getOutputLoc(const solverMode & /*sMode*/,  index_t /*num*/) const
{
	return kNullLocation;
}

void fmiMESubModel::updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
	fmi2Boolean eventMode;
	fmi2Boolean terminateSim;
	if (!sD.empty())
	{
		if (sD.updateRequired(lastSeqID))
		{
			Lp Loc = offsets.getLocations(sD, sMode,  this);
			me->setTime(sD.time);
			if (m_stateSize > 0)
			{
				if (isDynamic(sMode))
				{
					me->setStates(Loc.diffStateLoc);
				}
				else
				{
					me->setStates(Loc.algStateLoc);
				}
			}
			me->setInputs(inputs.data());
			lastSeqID = sD.seqID;
			if (m_stateSize > 0)
			{
				me->getDerivatives(tempdState.data());
			}
			if (!isDynamic(sMode))
			{
				me->completedIntegratorStep(false, &eventMode, &terminateSim);
			}
			
		}
	}
	else if (!inputs.empty())
	{
		me->setInputs(inputs.data());
		if (m_stateSize > 0)
		{
			me->getDerivatives(tempdState.data());
		}
		if (!isDynamic(sMode))
		{
			me->completedIntegratorStep(false, &eventMode, &terminateSim);
		}
	}
}

void fmiMESubModel::makeSettableState()
{
	if (opFlags[dyn_initialized])
	{
		prevFmiState = me->getCurrentMode();
		me->setMode(fmuMode::eventMode);
	}

}
void fmiMESubModel::resetState()
{
	if (prevFmiState == me->getCurrentMode())
	{
		return;
	}
	me->setMode(prevFmiState);
}

void fmiMESubModel::probeFMU()
{
	refMode_t defMode = (m_stateSize>0)?refMode_t::level1:refMode_t::level4;
	
	if (opFlags[reprobe_flag])
	{
		defMode = (m_stateSize>0) ? refMode_t::direct : refMode_t::level4;
	}
	for (auto &stateInfo : stateInformation)
	{
		auto mode = refMode_t::direct;
		for (auto &dep : stateInfo.stateDep)
		{
			auto depIndex = stateInformation[dep].varIndex;
			double res = getPartial(stateInfo.varIndex,depIndex , refMode_t::direct);
			if (res != 0.0)
			{
				continue;
			}
			res = getPartial(stateInfo.varIndex, depIndex, refMode_t::level1);
			if (res != 0)
			{
				mode = (std::max)(mode, refMode_t::level1);
				continue;
			}
			res = getPartial(stateInfo.varIndex, depIndex, refMode_t::level2);
			if (res != 0)
			{
				mode = (std::max)(mode, refMode_t::level2);
				continue;
			}
			res = getPartial(stateInfo.varIndex, depIndex, refMode_t::level3);
			if (res != 0)
			{
				mode = (std::max)(mode, refMode_t::level3);
				continue;
			}
			mode = (std::max)(mode, defMode);
			opFlags.set(reprobe_flag);
		}
		for (auto &dep : stateInfo.stateDep)
		{
			auto depIndex = stateInformation[dep].varIndex;
			double res = getPartial(stateInfo.varIndex, depIndex, refMode_t::direct);
			if (res != 0)
			{
				continue;
			}
			res = getPartial(stateInfo.varIndex, depIndex, refMode_t::level1);
			if (res != 0)
			{
				mode = (std::max)(mode, refMode_t::level1);
				continue;
			}
			mode = (std::max)(mode, defMode);
			opFlags.set(reprobe_flag);
		}
		stateInfo.refMode = mode;
	}
	int outputCounter = -1;
	for (auto &outputInfo : outputInformation)
	{
		++outputCounter;
		auto mode = refMode_t::direct;
		for (auto dep : outputInfo.stateDep)
		{
			auto depIndex = stateInformation[dep].varIndex;
			double res = getPartial(outputInfo.varIndex, depIndex, refMode_t::direct);
			if (res != 0)
			{
				continue;
			}
			res = getPartial(outputInfo.varIndex, depIndex, refMode_t::level1);
			if (res != 0)
			{
				mode = (std::max)(mode, refMode_t::level1);
				continue;
			}
			res = getPartial(outputInfo.varIndex, depIndex, refMode_t::level2);
			if (res != 0)
			{
				mode = (std::max)(mode, refMode_t::level2);
				continue;
			}
			res = getPartial(outputInfo.varIndex, depIndex, refMode_t::level4);
			if (res != 0)
			{
				mode = (std::max)(mode, refMode_t::level4);
				continue;
			}
			res = getPartial(outputInfo.varIndex, depIndex, refMode_t::level5);
			if (res != 0)
			{
				mode = (std::max)(mode, refMode_t::level5);
				continue;
			}
			mode = (std::max)(mode, defMode);
			opFlags.set(reprobe_flag);
		}
		for (auto &dep : outputInfo.inputDep)
		{
			auto depIndex = stateInformation[dep].varIndex;
			double res = getPartial(outputInfo.varIndex, depIndex, refMode_t::direct);
			if (res != 0)
			{
				continue;
			}
			if (m_stateSize > 0)
			{
				res = getPartial(outputInfo.varIndex, depIndex, refMode_t::level1);
				if (res != 0)
				{
					mode = (std::max)(mode, refMode_t::level1);
					continue;
				}
			}
			res = getPartial(outputInfo.varIndex, depIndex, refMode_t::level4);
			if (res != 0)
			{
				mode = (std::max)(mode, refMode_t::level4);
				continue;
			}
			if (m_stateSize > 0)
			{
				res = getPartial(outputInfo.varIndex, depIndex, refMode_t::level5);
				if (res != 0)
				{
					mode = (std::max)(mode, refMode_t::level5);
					continue;
				}
			}
			mode = (std::max)(mode, defMode);
			opFlags.set(reprobe_flag);
		}
		outputInfo.refMode = mode;
		if (mode >= refMode_t::level4)
		{
			opFlags.set(use_output_estimator);
			std::vector<int> sDep(outputInfo.stateDep.size());
			std::vector<int> iDep(outputInfo.inputDep.size());
			for (size_t dd = 0; dd < outputInfo.stateDep.size(); ++dd)
			{
				sDep[dd] = outputInfo.stateDep[dd];
			}
			for (size_t dd = 0; dd < outputInfo.inputDep.size(); ++dd)
			{
				iDep[dd] = outputInfo.inputDep[dd];
			}
			oEst[outputInfo.index] = new outputEstimator(sDep, iDep);
		}
	}


}

void fmiMESubModel::loadOutputJac(int index)
{
	double pd;
	int ct = 0;
	if (index == -1)
	{
		for (auto &out : outputInformation)
		{
			if (out.refMode >= refMode_t::level4)
			{
				ct = 0;
				for (auto kk : out.stateDep)
				{
					pd = getPartial(out.varIndex, stateInformation[kk].varIndex, out.refMode);
					oEst[out.index]->stateDiff[ct] = pd;
					++ct;
				}
				ct = 0;
				for (auto kk : out.inputDep)
				{
					pd = getPartial(out.varIndex, inputVarIndices[kk], out.refMode);
					oEst[out.index]->inputDiff[ct] = pd;
					++ct;
				}
			}
		}
	}
	else
	{
		if (outputInformation[index].refMode >= refMode_t::level4)
		{
			ct = 0;
			for (auto kk : outputInformation[index].stateDep)
			{
				pd = getPartial(outputInformation[index].varIndex, stateInformation[kk].varIndex, outputInformation[index].refMode);
				oEst[outputInformation[index].index]->stateDiff[ct] = pd;
				++ct;
			}
			ct = 0;
			for (auto kk : outputInformation[index].inputDep)
			{
				pd = getPartial(outputInformation[index].varIndex, inputVarIndices[kk], outputInformation[index].refMode);
				oEst[outputInformation[index].index]->inputDiff[ct] = pd;
				++ct;
			}
		}
	}
}

}//namespace fmi
}//namespace griddyn
