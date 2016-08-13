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

#include "txThermalModel.h"
#include "gridCoreTemplates.h"

#include "linkModels/gridLink.h"
#include "submodels/gridControlBlocks.h"
#include "recorder_events/gridGrabbers.h"
#include "recorder_events/gridCondition.h"
#include "recorder_events/gridEvent.h"

#include "stringOps.h"

#include <cmath>

txThermalModel::txThermalModel(const std::string &objName):sensor(objName)
{
	opFlags.reset(continuous_flag);  //this is a not a continuous model
	outputNames = { "ambient", "top_oil", "hot_spot" }; //preset the outputNames
}

gridCoreObject * txThermalModel::clone(gridCoreObject *obj) const
{
	txThermalModel *nobj = cloneBase<txThermalModel, sensor>(this, obj);
	if (!(nobj))
	{
		return obj;
	}

	nobj->Ttor = Ttor;
		nobj->DThs = DThs;
		nobj->DTtor = DTtor;
		nobj->Tgr = Tgr;
		nobj->mp_LR = mp_LR;
		nobj->mp_n = mp_n;
		nobj->mp_m = mp_n;
		nobj->ambientTemp = ambientTemp;
		nobj->dTempdt = dTempdt;
		nobj->rating = rating;
		nobj->Plossr = Plossr;
		nobj->m_C = m_C;
		nobj->m_k = m_k;
		nobj->alarmTemp1 = alarmTemp1;
		nobj->alarmTemp2 = alarmTemp2;
		nobj->cutoutTemp = cutoutTemp;
	return nobj;
}

int txThermalModel::setFlag(const std::string &flag, bool val)
{
	int out = PARAMETER_FOUND;
	if (flag == "auto")
	{
		opFlags.set(auto_parameter_load, val);
	}
	else if (flag == "parameter_updates")
	{
		opFlags.set(enable_parameter_updates, val);
	}
	else if (flag == "enable_alarms")
	{
		opFlags.set(enable_alarms, val);
	}
	else
	{
		out = sensor::setFlag(flag, val);
	}
	return out;
}

int txThermalModel::set (const std::string &param, const std::string &val)
{
	int out = PARAMETER_FOUND;
	if ((param == "txtype") || (param == "cooling"))
	{
		auto v2 = convertToLowerCase(val);
		if (v2 == "auto")
		{
			opFlags.set(auto_parameter_load);
		}
		else if (v2 == "oa")
		{
			DThs = 25.0;
			DTtor = 55.0;
			Ttor = 3.0*3600.0;
			Tgr = 5.0*60.0;
			mp_LR = 3.2;
			mp_n = 0.8;
			mp_m = 0.8;
		}
		else if (v2 == "fa")
		{
			DThs = 30.0;
			DTtor = 50.0;
			Ttor = 2.0*3600.0;
			Tgr = 5.0*60.0;
			mp_LR = 4.5;
			mp_n = 0.8;
			mp_m = 0.8;
		}
		else if (v2 == "fahot")
		{
			DThs = 35.0;
			DTtor = 45.0;
			Ttor = 1.25*3600.0;
			Tgr = 5.0*60.0;
			mp_LR = 6.5;
			mp_n = 0.9;
			mp_m = 0.8;
		}
		else if (v2 == "ndfoa")
		{
			DThs = 35.0;
			DTtor = 45.0;
			Ttor = 1.25*3600.0;
			Tgr = 5.0*60.0;
			mp_LR = 6.5;
			mp_n = 1.0;
			mp_m = 0.8;
		}
		else if (v2 == "dfoa")
		{
			DThs = 35.0;
			DTtor = 45.0;
			Ttor = 1.25*3600.0;
			Tgr = 5.0*60.0;
			mp_LR = 6.5;
			mp_n = 1.0;
			mp_m = 1.0;
			
		}
	}
	else
	{
		out= sensor::set(param, val);
	}
	return out;
}

using namespace gridUnits;

int txThermalModel::set (const std::string &param, double val, units_t unitType)
{
	int out = PARAMETER_FOUND;
	if ((param == "ambient") || (param == "ambienttemp"))
	{
		ambientTemp = unitConversionTemperature(val, unitType, C);
	}
	else if ((param == "dtempdt") || (param == "temp_rate_of_change"))
	{
		dTempdt = unitConversionTemperature(val, unitType, C);
	}
	else if ((param == "dths") || (param == "rated_hot_spot_rise") || (param == "dthsr"))
	{
		DThs = unitConversionTemperature(val, unitType, C);
	}
	else if ((param == "dttor") || (param == "rated_top_oil_rise") || (param == "dtto"))
	{
		DTtor = unitConversionTemperature(val, unitType, C);
	}
	else if ((param == "ttor") || (param == "oil_time_constant"))
	{
		Ttor= unitConversionTime(val, unitType, sec);
	}
	else if ((param == "tgr") || (param == "winding_time_constant"))
	{
		Tgr = unitConversionTime(val, unitType, sec);
	}
	else if ((param == "alarmtemp") || (param == "alarmtemp1"))
	{
		alarmTemp1= unitConversionTemperature(val, unitType, C);
		if (opFlags[dyn_initialized])
		{
			getCondition(0)->setLevel(alarmTemp1);
			setConditionState(0, (alarmTemp1 > 0.1) ? condition_states::active : condition_states::disabled);
		}
	}
	else if (param == "alarmtemp2")
	{
		alarmTemp2 = unitConversionTemperature(val, unitType, C);
		if (opFlags[dyn_initialized])
		{
			getCondition(1)->setLevel(alarmTemp1);
			setConditionState(1, (alarmTemp1 > 0.1) ? condition_states::active : condition_states::disabled);
		}
	}
	else if (param == "cutouttemp")
	{
		cutoutTemp = unitConversionTemperature(val, unitType, C);
		if (opFlags[dyn_initialized])
		{
			getCondition(2)->setLevel(alarmTemp1);
			setConditionState(2, (alarmTemp1 > 0.1) ? condition_states::active : condition_states::disabled);
		}
	}
	else if (param == "alarmdelay")
	{
		alarmDelay = unitConversionTime(val, unitType, sec);
		if (opFlags[dyn_initialized])
		{
			setActionTrigger(0, 0, alarmDelay);
			setActionTrigger(1, 1, alarmDelay);
			setActionTrigger(2, 2, alarmDelay);
			setActionTrigger(2, 3, alarmDelay);
		}
	}
	else if ((param == "lr") || (param == "loss_ratio"))
	{
		mp_LR = val;
	}
	else if ((param == "m") || (param == "winding_exponent"))
	{
		mp_m = val;
	}
	else if ((param == "n") || (param == "oil_exponent"))
	{
		mp_m = val;
	}
	
	else
	{
		out= gridPrimary::set(param, val, unitType);
	}
	return out;
}

double txThermalModel::get(const std::string & param, gridUnits::units_t unitType) const
{
	
	return sensor::get(param, unitType);
}

int txThermalModel::add(gridCoreObject * /*obj*/)
{
	return OBJECT_ADD_FAILURE;
}

void txThermalModel::dynObjectInitializeA (double time0, unsigned long flags)
{
	if (!(m_sourceObject))
	{
		return sensor::dynObjectInitializeA(time0, flags);
	}

	if (updatePeriod > kHalfBigNum)
	{        //set the period to the period of the simulation to at least 1/5 the winding time constant
		double pstep = parent->find("root")->get("steptime");
		if (pstep < 0)
		{
			pstep = 1.0;
		}
		double mtimestep = Tgr / 5.0;
		updatePeriod = pstep*std::floor(mtimestep / pstep);
		if (updatePeriod < pstep)
		{
			updatePeriod = pstep;
		}
	}

		rating = m_sourceObject->get("rating");
		double bp = m_sourceObject->get("basepower");
		if (opFlags[auto_parameter_load])
		{
			if (rating*bp < 2.5)
			{
				set("cooling", "oa");
			}
			else if (rating*bp < 10)
			{
				set("cooling", "fa");
			}
			else if (rating*bp < 100)
			{
				set("cooling", "fahot");
			}
			else if (rating*bp < 200)
			{
				set("cooling", "ndfoa");
			}
			else
			{
				set("cooling", "dfoa");
			}
		}
		double r = m_sourceObject->get("r");
		double g = m_sourceObject->get("g");  //get conductance
		Plossr = rating*rating*r + g;  //loss is I^2*r+g in per unit;

		if (g > 0)
		{
			mp_LR = Plossr / g;
		}
		
		m_k = Plossr / DTtor;  //compute the radiation constant
		m_C = Ttor*Plossr / DTtor; //compute the thermal mass constant
		if (!opFlags[dyn_initialized])
		{
			sensor::setFlag("sampled", true);
			sensor::set("input0", "current");
			sensor::set("input1", "loss");
			sensor::set("input2", "attached");
			
			auto b1 = new delayBlock(Ttor);
			auto b2 = new delayBlock(Tgr);

			sensor::add(b1);
			sensor::add(b2);
			auto g1 = std::make_shared<customGrabber>();
			g1->setGrabberFunction("ambient", [this]()->double {return ambientTemp; });
			sensor::add(g1, nullptr);

			outputSize = (outputSize > 3) ? outputSize : 3;


			outputMode.resize(outputSize);
			outputs.resize(outputSize);
			outGrabber.resize(outputSize, nullptr);
			outGrabberSt.resize(outputSize, nullptr);
			outputMode[0] = outputMode_t::direct;
			outputMode[1] = outputMode_t::block;
			outputs[0] = 3;  //the first input was setup as the current, second as the loss, 3rd as attached
			outputs[1] = 0;
			sensor::set("output2", "block0+block1");
			auto c1 = make_condition("output1", ">", alarmTemp1, this);
			gridRelay::add(c1);
			c1 = make_condition("output1", ">", alarmTemp2, this);
			gridRelay::add(c1);
			c1 = make_condition("output1", ">", cutoutTemp, this);
			gridRelay::add(c1);
			
			gridRelay::set("action", "alarm temperature_alarm1");
			gridRelay::set("action", "alarm temperature_alarm2");
			auto ge = std::make_shared<gridEvent>();

			ge->field = "switch1";
			ge->value = 1;
			ge->setTarget(m_sinkObject);

			gridRelay::add(ge);
			ge = std::make_shared<gridEvent>();

			ge->field = "switch2";
			ge->value = 1;
			ge->setTarget(m_sinkObject);

			gridRelay::add(ge);
			//add the triggers
			setActionTrigger(0, 0, alarmDelay);
			setActionTrigger(1, 1, alarmDelay);
			setActionTrigger(2, 2, alarmDelay);
			setActionTrigger(2, 3, alarmDelay);

			if (alarmTemp1 <= 0.1)
			{
				setConditionState(0, condition_states::disabled);
			}
			if (alarmTemp1 <= 0.1)
			{
				setConditionState(1, condition_states::disabled);
			}
			if (cutoutTemp <= 0.1)
			{
				setConditionState(2, condition_states::disabled);
			}


		}
	return sensor::dynObjectInitializeA(time0, flags);
}

void txThermalModel::dynObjectInitializeB(IOdata &outputSet)
{
	dataSources[0]->gain = 1.0 / rating;
	dataSources[1]->gain = 1.0 / Plossr;

	double I = dataSources[0]->grabData();

	double K2 = dataSources[1]->grabData();
	double at = dataSources[2]->grabData();
	IOdata iset(1);
	if (at > 0.1)
	{
		double DTtou = DTtor*pow((I*I*mp_LR + 1) / (mp_LR + 1), mp_n);
		double DTgu = DThs*pow(K2, mp_m);

		iset[0] = DTtou + ambientTemp;
		filterBlocks[0]->initializeB(iset, iset, iset);//I don't care what the result is so I use the same vector for all inputs
		iset[0] = DTgu;
		filterBlocks[1]->initializeB(iset, iset, iset);
	}
	else
	{
		iset[0] = ambientTemp;
		filterBlocks[0]->initializeB(iset, iset, iset);
		iset[0] = 0;
		filterBlocks[1]->initializeB(iset, iset, iset);
	}
	return gridRelay::dynObjectInitializeB(outputSet);//skip over sensor::initializeB since the filter blocks are initialized here.
}


void txThermalModel::updateA(double time)
{
	double dt = time - prevTime;
	
	double at = dataSources[2]->grabData();
	ambientTemp = ambientTemp + dt*dTempdt;
	if (at > 0.1)
	{
		double I = dataSources[0]->grabData();
		double K2 = dataSources[1]->grabData();


		double DTtou = DTtor*pow((I*I*mp_LR + 1) / (mp_LR + 1), mp_n);
		double DTgu = DThs*pow(K2, mp_m);

		//update the time constants if required
		if (mp_n != 1.0)
		{
			double Toc = filterBlocks[0]->getOutput();
			double r1 = (Toc - ambientTemp) / DTtor;
			double r2 = DTtou / DTtor;
			double Tto = Ttor*((r1 - r2) / (pow(r1, 1.0 / mp_n) - pow(r2, 1.0 / mp_n)));
			filterBlocks[0]->set("t1", Tto);
		}
		if (mp_m != 1.0)
		{
			double Thsc = filterBlocks[1]->getOutput();
			double r1 = (Thsc) / DThs;
			double r2 = DTgu / DThs;
			double Tg = Ttor*((r1 - r2) / (pow(r1, 1.0 / mp_m) - pow(r2, 1.0 / mp_m)));
			filterBlocks[1]->set("t1", Tg);
		}
		
		filterBlocks[0]->step(time, ambientTemp + DTtou);
		filterBlocks[1]->step(time, DTgu);
	}
	else
	{
		filterBlocks[0]->step(time, ambientTemp);
		filterBlocks[1]->step(time, 0);
	}
	//printf("%f:%s A=%f to(%f)=%f hs(%f)=%f\n",time, name.c_str(), ambientTemp, DTtou+ambientTemp, o1, DTgu, o2);
	gridRelay::updateA(time);
}

double txThermalModel::timestep(double ttime, const solverMode &sMode)
{
	updateA(ttime);
	return getOutput(nullptr, sMode, 2);
}
