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


#include "txLifeSpan.h"
#include "gridCoreTemplates.h"

#include "submodels/gridControlBlocks.h"
#include "linkModels/gridLink.h"
#include "recorder_events/gridGrabbers.h"
#include "recorder_events/gridEvent.h"
#include "recorder_events/gridCondition.h"
#include <cmath>

txLifeSpan::txLifeSpan(const std::string &objName):sensor(objName)
{
	opFlags.reset(continuous_flag);  //this is a not a continuous model everything is slow so no need to make it continuous
	outputNames = { "remaininglife", "lossoflife" ,"rate"};
}

gridCoreObject * txLifeSpan::clone(gridCoreObject *obj) const
{
	txLifeSpan *nobj = cloneBase<txLifeSpan, sensor>(this, obj);
	if (!(nobj))
	{
		return obj;
	}

	nobj->initialLife = initialLife;
	nobj->agingConstant = agingConstant;
	nobj->baseTemp = baseTemp;
	nobj->agingFactor = agingFactor;
	return nobj;
}

int txLifeSpan::setFlag(const std::string &flag, bool val)
{
	int out = PARAMETER_FOUND;
	if ((flag == "useiec") || (flag == "iec"))
	{
		opFlags.set(useIECmethod,val);
	}
	else if ((flag == "useieee") || (flag == "ieee"))
	{
		opFlags.set(useIECmethod,!val);
	}
	else if (flag == "no_discconect")
	{
		opFlags.set(no_disconnect, val);
	}
	else
	{
		out = sensor::setFlag(flag, val);
	}
	return out;
}

int txLifeSpan::set (const std::string &param, const std::string &val)
{
	int out = PARAMETER_FOUND;
	if (param[0] == '#')
	{
	}
	else if ((param == "input")||(param == "input0"))
	{
		sensor::set(param, val);
	}
	else
	{
		out= gridRelay::set(param, val);
	}
	return out;
}

using namespace gridUnits;

int txLifeSpan::set (const std::string &param, double val, units_t unitType)
{
	int out = PARAMETER_FOUND;
	if ((param == "initial") || (param == "initiallife"))
	{
		initialLife = unitConversionTime(val, unitType, hour);
	}
	else if (param == "basetemp")
	{
		baseTemp = unitConversionTemperature(val, unitType, C);
	}
	else if ((param == "agingrate") || (param == "agingconstant"))
	{
		agingConstant = val;
	}
	else
	{
		out= gridPrimary::set(param, val, unitType);
	}
	return out;
}

double txLifeSpan::get(const std::string & param, gridUnits::units_t unitType) const
{
	
	return sensor::get(param, unitType);
}

int txLifeSpan::add(gridCoreObject * /*obj*/)
{
	return OBJECT_ADD_FAILURE;
}

void txLifeSpan::dynObjectInitializeA (double time0, unsigned long flags)
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
		double mtimestep = 120.0;  //update once per minute
		updatePeriod = pstep*std::floor(mtimestep / pstep);
		if (updatePeriod < pstep)
		{
			updatePeriod = pstep;
		}
	}
	if (!opFlags[dyn_initialized])
	{
		sensor::setFlag("sampled", true);
		if (inputStrings.size() == 0)
		{
			//assume we are connected to a temperature sensor
			sensor::set("input0", "hot_spot");
		}
		auto b1 = new integralBlock(1.0/3600);  //add a gain so the output is in hours
		sensor::add(b1);

		sensor::set("output0", std::to_string(initialLife) + "-block0");
		sensor::set("output1", "block0");

		auto g1 = std::make_shared<customGrabber>();
		g1->setGrabberFunction("rate", [this]()->double {return Faa; });
		sensor::add(g1, nullptr);
		
		sensor::set("output2", "input1");
		if (m_sinkObject)
		{
			auto ge = std::make_shared<gridEvent>();
			ge->setTarget(m_sinkObject, "g");
			ge->value = 100;
			gridRelay::add(ge);

			ge = std::make_shared<gridEvent>();
			ge->setTarget(m_sinkObject, "switch1");
			ge->value = 1;
			gridRelay::add(ge);

			ge = std::make_shared<gridEvent>();
			ge->setTarget(m_sinkObject, "switch2");
			ge->value = 1;
			gridRelay::add(ge);

			auto cond = make_condition("output0", "<", 0, this);
			gridRelay::add(cond);

			setActionTrigger(0, 0, 0);
			if (!opFlags[no_disconnect])
			{
				setActionTrigger(0, 1, 0);
				setActionTrigger(0, 2, 0);
			}
		}
		
	}
	return sensor::dynObjectInitializeA(time0, flags);
}
void txLifeSpan::dynObjectInitializeB(IOdata &outputSet)
{
	IOdata iset{0.0};
	filterBlocks[0]->initializeB(iset, iset, iset);
	return gridRelay::dynObjectInitializeB(outputSet);//skip over sensor::dynInitializeB since we we are initializing the blocks here
}


void txLifeSpan::updateA(double time)
{

	double Temp = dataSources[0]->grabData();
	if (opFlags[useIECmethod])
	{
		Faa = agingFactor*exp2((Temp - baseTemp + 12) / 6);
	}
	else
	{
		Faa = agingFactor*exp(agingConstant / (baseTemp + 273.0) - (agingConstant / (Temp + 273.0)));
	}


	filterBlocks[0]->timestep(time, { Faa }, cLocalSolverMode);
	gridRelay::updateA(time);
}

double txLifeSpan::timestep(double ttime, const solverMode &sMode)
{
	updateA(ttime);
	return getOutput(nullptr, sMode, 1);
}

void txLifeSpan::actionTaken(index_t ActionNum, index_t /*conditionNum*/,  change_code /*actionReturn*/, double /*actionTime*/)
{
	if (m_sinkObject)
	{
		if (ActionNum == 0)
		{
			LOG_NORMAL(m_sinkObject->getName() + " lifespan exceeded fault produced");
		}
		else if (ActionNum == 1)
		{
			LOG_NORMAL(m_sinkObject->getName() + " lifespan exceeded breakers tripped");
		}
	}
}
