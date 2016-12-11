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



#include "fmiCoSimSubModel.h"
#include "gridCoreTemplates.h"
#include "fmi_import/fmiLibraryManager.h"
#include "fmi_import/fmiObjects.h"
#include "vectorOps.hpp"
#include "matrixData.h"
#include "outputEstimator.h"
#include "stringOps.h"
#include "core/gridDynExceptions.h"

#include <algorithm>


fmiCoSimSubModel::fmiCoSimSubModel(const std::string &newName, std::shared_ptr<fmi2CoSim> fmi) :gridSubModel(newName), cs(fmi)
{

}

fmiCoSimSubModel::fmiCoSimSubModel(std::shared_ptr<fmi2CoSim> fmi) : cs(fmi)
{

}

fmiCoSimSubModel::~fmiCoSimSubModel()
{

}

coreObject * fmiCoSimSubModel::clone(coreObject *obj) const
{
	auto *gco = cloneBase<fmiCoSimSubModel, gridSubModel>(this, obj);
	if (!(gco))
	{
		return obj;
	}

	return gco;
}

bool fmiCoSimSubModel::isLoaded() const
{
	return (cs) ? true : false;
}

void fmiCoSimSubModel::objectInitializeA(gridDyn_time time, unsigned long flags)
{
	if (CHECK_CONTROLFLAG(force_constant_pflow_initialization, flags))
	{
		opFlags.set(pflow_init_required);
	}
}

void fmiCoSimSubModel::objectInitializeB(const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
	if (opFlags[pflow_init_required])
	{
		if (opFlags[pFlow_initialized])
		{
			/*
			cs->getStates(m_state.data());
			cs->setTime(prevTime - 0.01);

			if (opFlags[use_output_estimator])
			{
				//if we require the use of output estimators flag that to the simulation and load the information for the estimator
				alert(this, SINGLE_STEP_REQUIRED);
				double val;
				loadOutputJac();
				for (size_t pp = 0; pp < m_outputSize; ++pp)
				{
					if (outputInformation[pp].refMode >= refMode_t::level4)
					{
						val = cs->getOutput(pp);
						oEst[pp]->update(prevTime, val, args, m_state.data());
					}
				}

			}
			*/
			opFlags.set(dyn_initialized);
		}
		else//in pflow mode
		{
			cs->setMode(fmuMode::initializationMode);

			cs->setInputs(args.data());
			cs->setMode(fmuMode::continuousTimeMode);
			oEst.resize(m_outputSize);
			probeFMU();
			opFlags.set(pFlow_initialized);
		}
	}
	else
	{
	/*	cs->setMode(fmuMode::initializationMode);

		cs->setInputs(args.data());
		cs->setMode(fmuMode::continuousTimeMode);

		cs->getStates(m_state.data());
		oEst.resize(m_outputSize);
		probeFMU();  //probe the fmu 
		if (opFlags[use_output_estimator])
		{
			//if we require the use of output estimators flag that to the simulation and load the information for the estimator
			alert(this, SINGLE_STEP_REQUIRED);
			loadOutputJac();
		}
		cs->setTime(prevTime - 0.01);
		*/
	}


}


void fmiCoSimSubModel::getParameterStrings(stringVec &pstr, paramStringType pstype) const
{
	int strpcnt = 0;
	auto info = cs->fmuInformation();
	auto vcnt = info->getCounts("variables");
	switch (pstype)
	{
	case paramStringType::all:
		pstr.reserve(pstr.size() + info->getCounts("params") + info->getCounts("inputs") - m_inputSize);

		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (info->getVariableInfo(kk).type == fmi_type_t::string)
			{
				++strpcnt;
			}
			else if (checkType(info->getVariableInfo(kk), fmi_type_t::numeric, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}

		gridSubModel::getParameterStrings(pstr, paramStringType::numeric);
		pstr.reserve(pstr.size() + strpcnt + 1);
		pstr.push_back("#");
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_type_t::string, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::string);
		break;
	case paramStringType::localnum:
		pstr.reserve(info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		pstr.resize(0);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_type_t::numeric, fmi_causality_type_t::parameter))
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
			if (checkType(info->getVariableInfo(kk), fmi_type_t::string, fmi_causality_type_t::parameter))
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
			if (checkType(info->getVariableInfo(kk), fmi_type_t::boolean, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		break;
	case paramStringType::numeric:
		pstr.reserve(pstr.size() + info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_type_t::numeric, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::numeric);
		break;
	case paramStringType::string:
		pstr.reserve(pstr.size() + info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_type_t::string, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::string);
		break;
	case paramStringType::flags:
		pstr.reserve(pstr.size() + info->getCounts("params") + info->getCounts("inputs") - m_inputSize);
		for (int kk = 0; kk < vcnt; ++kk)
		{
			if (checkType(info->getVariableInfo(kk), fmi_type_t::boolean, fmi_causality_type_t::parameter))
			{
				pstr.push_back(info->getVariableInfo(kk).name);
			}
		}
		gridSubModel::getParameterStrings(pstr, paramStringType::flags);
		break;
	}
}


stringVec fmiCoSimSubModel::getOutputNames() const
{
	return cs->getOutputNames();

}

stringVec fmiCoSimSubModel::getInputNames() const
{
	return cs->getInputNames();
}


void fmiCoSimSubModel::set(const std::string &param, const std::string &val)
{
	if (param == "fmu")
	{
		if (!(cs))
		{
			cs = fmiLibraryManager::instance()->createCoSimulationObject(val, name);
		}
		else
		{
			//return INVALID_PARAMETER_VALUE;
			return;
		}

	}
	else if (param == "outputs")
	{
		auto ssep = splitlineTrim(val);
		cs->setOutputVariables(ssep);
		m_outputSize = cs->outputSize();
	}
	else if (param == "inputs")
	{
		auto ssep = splitlineTrim(val);
		cs->setOutputVariables(ssep);
		m_outputSize = cs->inputSize();
		updateDependencyInfo();
	}
	else
	{
		bool isparam = cs->isParameter(param, fmi_type_t::string);
		if (isparam)
		{
			makeSettableState();
			cs->set(param, val);
			resetState();
		}
		else
		{
			gridSubModel::set(param, val);
		}

	}
}

void fmiCoSimSubModel::set(const std::string &param, double val, gridUnits::units_t unitType)
{

	if ((param == "timestep") || (param == "localintegrationtime"))
	{
		localIntegrationTime = val;
	}
	else
	{
		bool isparam = cs->isParameter(param, fmi_type_t::numeric);
		if (isparam)
		{
			makeSettableState();
			cs->set(param, val);
			resetState();
		}
		else
		{
			gridSubModel::set(param, val);
		}
	}
}

double fmiCoSimSubModel::get(const std::string &param, gridUnits::units_t unitType) const
{

	if (param == "localintegrationtime")
	{
		return localIntegrationTime;
	}
	else
	{
		if (cs->isVariable(param, fmi_type_t::numeric))
		{
			return cs->get<double>(param);
		}
		else
		{
			return gridSubModel::get(param, unitType);
		}
	}

}


void fmiCoSimSubModel::loadSizes(const solverMode &sMode, bool dynOnly)
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
			if (opFlags[pflow_init_required])
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


void fmiCoSimSubModel::setState(gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	/*if (hasDifferential(sMode))
	{
		auto loc = offsets.getDiffOffset(sMode);
		if (m_stateSize > 0)
		{
			cs->setStates(state + loc);
			m_state.assign(state + loc, state + loc + m_stateSize);
			m_dstate_dt.assign(dstate_dt + loc, dstate_dt + loc + m_stateSize);
		}

		cs->setTime(ttime);
		int eventMode;
		int terminate;
		cs->completedIntegratorStep(true, &eventMode, &terminate);

		if ((opFlags[use_output_estimator]) && (!opFlags[fixed_output_interval]))
		{

			IOdata ip(m_inputSize);

			cs->getCurrentInputs(ip.data());
			for (size_t pp = 0; pp < m_outputSize; ++pp)
			{
				if (outputInformation[pp].refMode >= refMode_t::level4)
				{
					double val;
					val = cs->getOutput(pp);
					bool reload = oEst[pp]->update(ttime, val, ip, state + offsets.getDiffOffset(sMode));
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
			cs->setStates(state + loc);
			m_state.assign(state + loc, state + loc + m_stateSize);
		}
		cs->setTime(ttime);
		int eventMode;
		int terminate;
		cs->completedIntegratorStep(true, &eventMode, &terminate);


	}
	prevTime = ttime;
	*/
}
//for saving the state
void fmiCoSimSubModel::guess(gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
	/*
	if (m_stateSize == 0)
	{
		return;
	}
	if (hasDifferential(sMode))
	{
		auto loc = offsets.getDiffOffset(sMode);
		cs->getStates(state + loc);
	}
	else if (!isDynamic(sMode) && (opFlags[pflow_init_required]))
	{
		auto loc = offsets.getAlgOffset(sMode);
		cs->getStates(state + loc);
	}
	*/
}

void fmiCoSimSubModel::getTols(double tols[], const solverMode &sMode)
{

}

void fmiCoSimSubModel::getStateName(stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
	

}

index_t fmiCoSimSubModel::findIndex(const std::string &field, const solverMode &sMode) const
{
	/*auto fmistNames = cs->getStateNames();
	auto fnd = std::find(fmistNames.begin(), fmistNames.end(), field);
	if (fnd != fmistNames.end())
	{
		return static_cast<index_t>(fnd - fmistNames.begin());
	}
	*/
	return kInvalidLocation;

}

void fmiCoSimSubModel::residual(const IOdata &args, const stateData &sD, double resid[], const solverMode &sMode)
{
	/*if (hasDifferential(sMode))
	{
		Lp Loc = offsets.getLocations(sD, resid, sMode, this);
		derivative(args, sD, resid, sMode);
		for (size_t ii = 0; ii < Loc.diffSize; ++ii)
		{
			Loc.destDiffLoc[ii] -= Loc.dstateLoc[ii];
		}
	}
	else if (!isDynamic(sMode) && (opFlags[pflow_init_required]))
	{
		derivative(args, sD, resid, sMode);
	}
	*/
}

void fmiCoSimSubModel::derivative(const IOdata &args, const stateData &sD, double deriv[], const solverMode &sMode)
{
	/*Lp Loc = offsets.getLocations(sD, deriv, sMode, this);
	updateInfo(args, sD, sMode);
	if (isDynamic(sMode))
	{
		cs->getDerivatives(Loc.destDiffLoc);
		printf("tt=%f,I=%f, state=%f deriv=%e\n", sD.time, args[0], Loc.diffStateLoc[0], Loc.destDiffLoc[0]);
	}
	else
	{
		cs->getDerivatives(Loc.destLoc);
		printf("tt=%f,I=%f, state=%f,deriv=%e\n", sD.time, args[0], Loc.algStateLoc[0], Loc.destLoc[0]);
	}
	*/
}

const static double gap = 1e-8;
double fmiCoSimSubModel::getPartial(int depIndex, int refIndex, refMode_t mode)
{
	double res = 0.0;
	double ich = 1.0;
	fmiVariableSet vx = cs->getVariableSet(depIndex);
	fmiVariableSet vy = cs->getVariableSet(refIndex);
	if (opFlags[has_derivative_function])
	{
		res = cs->getPartialDerivative(depIndex, refIndex, ich);
	}
	else
	{
/*		double out1, out2;
		double val1, val2;
		fmi2Boolean evmd;
		fmi2Boolean term;

		cs->get(vx, &out1);
		cs->get(vy, &out1);
		val2 = val1 + gap;
		if (mode == refMode_t::direct)
		{
			cs->set(vy, &val2);
			cs->get(vx, &out2);
			cs->set(vy, &val1);
			res = (out2 - out1) / gap;
		}
		else if (mode == refMode_t::level1)
		{
			cs->set(vy, &val2);
			cs->getDerivatives(tempdState.data());
			cs->get(vx, &out2);
			cs->set(vy, &val1);
			cs->getDerivatives(tempdState.data());
			res = (out2 - out1) / gap;
		}
		else if (mode == refMode_t::level2)
		{
			cs->getStates(tempState.data());
			tempState[refIndex] = val2;
			cs->setStates(tempState.data());
			cs->getDerivatives(tempdState.data());
			cs->get(vx, &out2);
			tempState[refIndex] = val1;
			cs->setStates(tempState.data());
			cs->getDerivatives(tempdState.data());
			res = (out2 - out1) / gap;
		}
		else if (mode == refMode_t::level3)  //max useful for states dependent variables
		{
			cs->getStates(tempState.data());
			tempState[refIndex] = val2;
			cs->setStates(tempState.data());
			cs->completedIntegratorStep(false, &evmd, &term);
			cs->getDerivatives(tempdState.data());

			cs->get(vx, &out2);
			tempState[refIndex] = val1;
			cs->setStates(tempState.data());
			cs->getDerivatives(tempdState.data());
			cs->completedIntegratorStep(false, &evmd, &term);
			res = (out2 - out1) / gap;
		}
		else if (mode == refMode_t::level4)  //for input dependencies only
		{

			cs->set(vy, &val2);
			cs->completedIntegratorStep(false, &evmd, &term);
			cs->get(vx, &out2);
			cs->set(vy, &val1);
			cs->completedIntegratorStep(false, &evmd, &term);
			res = (out2 - out1) / gap;
		}
		else if (mode == refMode_t::level5) //for input dependencies only
		{
			cs->set(vy, &val2);
			cs->getStates(tempState.data());
			cs->setStates(tempState.data());
			cs->getDerivatives(tempdState.data());
			cs->get(vx, &out2);
			cs->set(vy, &val1);
			cs->setStates(tempState.data());
			cs->getDerivatives(tempdState.data());
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
		*/
	}
	return res;
}
void fmiCoSimSubModel::jacobianElements(const IOdata &args, const stateData &sD,
	matrixData<double> &ad,
	const IOlocs &argLocs, const solverMode &sMode)
{
	

}

void fmiCoSimSubModel::timestep(gridDyn_time ttime, const IOdata &args, const solverMode &sMode)
{

/*	double h = localIntegrationTime;
	int sv = 0;
	double aval = 0.95;
	size_t aloc = 7;
	double time = prevTime;
	fmi2Boolean eventMode;
	fmi2Boolean terminateSim;
	double Tend = ttime;
	std::vector<double> der_x(m_stateSize);
	std::vector<double> der_x2(m_stateSize);
	std::vector<double> prevInput(m_inputSize);
	std::vector<double> inputSlope(m_inputSize);
	//get the previous inputs
	cs->getCurrentInputs(prevInput.data());
	//get the current states
	cs->getStates(m_state.data());
	//compute the slopes of the inputs
	for (size_t kk = 0; kk < m_inputSize; ++kk)
	{
		inputSlope[kk] = (args[kk] - prevInput[kk]) / (ttime - prevTime);
	}
	while (time < Tend)
	{

		// compute derivatives
		cs->getDerivatives(der_x.data());
		// advance time

		time = time + h;
		vectorMultAdd(prevInput, inputSlope, h, prevInput);
		cs->setInputs(prevInput.data());
		cs->setTime(time);
		// set states at t = time and perform one step
		vectorMultAdd(m_state, der_x, h, m_state);
		cs->setStates(m_state.data());

		// get event indicators at t = time
		cs->completedIntegratorStep(false, &eventMode, &terminateSim);

		h = (time + h > Tend) ? (Tend - time) : localIntegrationTime;

	}
	prevTime = time;
	double out = cs->getOutput(0);

	return out;
	*/
	
}

void fmiCoSimSubModel::ioPartialDerivatives(const IOdata &args, const stateData &sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode)
{
/*	updateInfo(args, sD, sMode);
	double res;
	double ich = 1.0;
	index_t kk;
	int vk, vu;

	for (kk = 0; kk < m_outputSize; ++kk)
	{
		vu = outputInformation[kk].varIndex;
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
				ad.assign(kk, sR, 1.0);
			}
			else
			{
				vk = sR;
				res = getPartial(vu, inputVarIndices[sR], kmode);
				if (res != 0.0)
				{
					ad.assign(kk, sR, res);
				}

			}
		}
	}
	*/

}

void fmiCoSimSubModel::outputPartialDerivatives(const IOdata &args, const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
{



}

void fmiCoSimSubModel::rootTest(const IOdata &args, const stateData &sD, double roots[], const solverMode &sMode)
{
	updateLocalCache(args, sD, sMode);
	auto rootOffset = offsets.getRootOffset(sMode);
	//cs->getEventIndicators(&(roots[rootOffset]));
}

void fmiCoSimSubModel::rootTrigger(gridDyn_time ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
	cs->setMode(fmuMode::eventMode);
	//TODO: deal with the event
	cs->setMode(fmuMode::continuousTimeMode);
}





IOdata fmiCoSimSubModel::getOutputs(const IOdata &args, const stateData &sD, const solverMode &sMode) const
{
	IOdata out(m_outputSize, 0);
	if (cs->getCurrentMode() >= fmuMode::initializationMode)
	{
		//updateInfo(args, sD, sMode);
		cs->getOutputs(out.data());
		printf("time=%f, out1 =%f, out 2=%f\n", static_cast<double>((!sD.empty()) ? sD.time : prevTime), out[0], out[1]);
		if ((opFlags[use_output_estimator]) && (!sD.empty()) && (!opFlags[fixed_output_interval]) && (isDynamic(sMode)))
		{
			for (size_t pp = 0; pp < m_outputSize; ++pp)
			{
			/*	if (outputInformation[pp].refMode >= refMode_t::level4)
				{
					const double res = oEst[pp]->estimate(sD.time, args, sD.state + offsets.getDiffOffset(sMode));
					out[pp] = res;
				}
				*/
			}
		}
	}
	return out;
}


double fmiCoSimSubModel::getDoutdt(const stateData &sD, const solverMode &sMode, index_t num) const
{
	return 0;
}

double fmiCoSimSubModel::getOutput(const IOdata &args, const stateData &sD, const solverMode &sMode, index_t num) const
{
	double out = kNullVal;
	if (cs->getCurrentMode() >= fmuMode::initializationMode)
	{
		//updateInfo(args, sD, sMode);

		if ((opFlags[use_output_estimator]) && (!sD.empty()) && (!opFlags[fixed_output_interval]) && (isDynamic(sMode)))
		{
		/*	if (outputInformation[num].refMode >= refMode_t::level4)
			{
				out = oEst[num]->estimate(sD.time, args, sD.state + offsets.getDiffOffset(sMode));
			}
			*/
		}
		else
		{
			out = cs->getOutput(num);
		}
	}
	return out;
}

double fmiCoSimSubModel::getOutput(index_t num) const
{
	double out = kNullVal;
	if (cs->getCurrentMode() >= fmuMode::initializationMode)
	{
		out = cs->getOutput(num);
	}
	return out;
}

index_t fmiCoSimSubModel::getOutputLoc(const solverMode &sMode, index_t num) const
{
	return kNullLocation;
}

void fmiCoSimSubModel::updateLocalCache(const IOdata &args, const stateData &sD, const solverMode &sMode)
{
	/*	fmi2Boolean eventMode;
	fmi2Boolean terminateSim;
	if (!sD.empty())
	{
		if ((sD.seqID == 0) || (sD.seqID != lastSeqID))
		{
			Lp Loc = offsets.getLocations(sD, sMode, this);
			cs->setTime(sD.time);
			if (m_stateSize > 0)
			{
				if (isDynamic(sMode))
				{
					cs->setStates(Loc.diffStateLoc);
				}
				else
				{
					cs->setStates(Loc.algStateLoc);
				}
			}
			cs->setInputs(args.data());
			lastSeqID = sD.seqID;
			if (m_stateSize > 0)
			{
				cs->getDerivatives(tempdState.data());
			}
			if (!isDynamic(sMode))
			{
				cs->completedIntegratorStep(false, &eventMode, &terminateSim);
			}

		}
	}
	else if (!args.empty())
	{
		cs->setInputs(args.data());
		if (m_stateSize > 0)
		{
			cs->getDerivatives(tempdState.data());
		}
		if (!isDynamic(sMode))
		{
			cs->completedIntegratorStep(false, &eventMode, &terminateSim);
		}
	}

	*/

}

void fmiCoSimSubModel::makeSettableState()
{
	if (opFlags[dyn_initialized])
	{
		//prevFmiState = cs->getCurrentMode();
		cs->setMode(fmuMode::eventMode);
	}

}
void fmiCoSimSubModel::resetState()
{
	//if (prevFmiState == cs->getCurrentMode())
	{
		return;
	}
//	cs->setMode(prevFmiState);
}

void fmiCoSimSubModel::probeFMU()
{
	refMode_t defMode = (m_stateSize>0) ? refMode_t::level1 : refMode_t::level4;

	//double res;
	if (opFlags[reprobe_flag])
	{
		defMode = (m_stateSize>0) ? refMode_t::direct : refMode_t::level4;
	}
	/*for (auto &stateInfo : stateInformation)
	{
		auto mode = refMode_t::direct;
		for (auto &dep : stateInfo.stateDep)
		{
			auto depIndex = stateInformation[dep].varIndex;
			res = getPartial(stateInfo.varIndex, depIndex, refMode_t::direct);
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
			res = getPartial(stateInfo.varIndex, depIndex, refMode_t::direct);
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
			res = getPartial(outputInfo.varIndex, depIndex, refMode_t::direct);
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
			res = getPartial(outputInfo.varIndex, depIndex, refMode_t::direct);
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
	*/

}

void fmiCoSimSubModel::loadOutputJac(int index)
{
	//double pd;
	//int ct = 0;
	if (index == -1)
	{
	/*	for (auto &out : outputInformation)
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
		*/
	}
	else
	{
		/*if (outputInformation[index].refMode >= refMode_t::level4)
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
		*/
	}
}


