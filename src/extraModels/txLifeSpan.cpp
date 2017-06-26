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


#include "txLifeSpan.h"
#include "core/coreObjectTemplates.hpp"

#include "blocks/integralBlock.h"
#include "Link.h"
#include "measurement/gridGrabbers.h"
#include "measurement/grabberSet.h"
#include "events/Event.h"
#include "measurement/Condition.h"
#include "core/coreExceptions.h"
#include <cmath>

namespace griddyn {
namespace extra {
txLifeSpan::txLifeSpan(const std::string &objName):sensor(objName)
{
	opFlags.reset(continuous_flag);  //this is a not a continuous model everything is slow so no need to make it continuous
	outputNames = { "remaininglife", "lossoflife" ,"rate"};
}

coreObject * txLifeSpan::clone(coreObject *obj) const
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

void txLifeSpan::setFlag(const std::string &flag, bool val)
{
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
		sensor::setFlag(flag, val);
	}
}

void txLifeSpan::set (const std::string &param, const std::string &val)
{

	if (param[0] == '#')
	{
	}
	else if ((param == "input")||(param == "input0"))
	{
		sensor::set(param, val);
	}
	else
	{
		Relay::set(param, val);
	}
}

using namespace gridUnits;

void txLifeSpan::set (const std::string &param, double val, units_t unitType)
{
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
		gridPrimary::set(param, val, unitType);
	}
}

double txLifeSpan::get(const std::string & param, gridUnits::units_t unitType) const
{
	
	return sensor::get(param, unitType);
}

void txLifeSpan::add(coreObject * /*obj*/)
{
	throw(unrecognizedObjectException(this));
}

void txLifeSpan::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
	if (!(m_sourceObject))
	{
		return sensor::dynObjectInitializeA(time0, flags);
	}

	if (updatePeriod > negTime)
	{        //set the period to the period of the simulation to at least 1/5 the winding time constant
		coreTime pstep = getRoot()->get("steptime");
		if (pstep < timeZero)
		{
			pstep = 1.0;
		}
		coreTime mtimestep = 120.0;  //update once per minute
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
		auto b1 = new blocks::integralBlock(1.0/3600);  //add a gain so the output is in hours
		sensor::add(b1);
		b1->parentSetFlag(separate_processing, true, this);

		sensor::set("output0", std::to_string(initialLife) + "-block0");
		sensor::set("output1", "block0");

		auto g1 = std::make_shared<customGrabber>();
		g1->setGrabberFunction("rate", [this](coreObject *)->double {return Faa; });
		sensor::add(g1);
		
		sensor::set("output2", "input1");
		if (m_sinkObject)
		{
			auto ge = std::make_unique<Event>();
			ge->setTarget(m_sinkObject, "g");
			ge->setValue(100.0);
			Relay::add(std::shared_ptr<Event>(std::move(ge)));

			ge = std::make_unique<Event>();
			ge->setTarget(m_sinkObject, "switch1");
			ge->setValue(1.0);
			Relay::add(std::shared_ptr<Event>(std::move(ge)));

			ge = std::make_unique<Event>();
			ge->setTarget(m_sinkObject, "switch2");
			ge->setValue(1.0);
			Relay::add(std::shared_ptr<Event>(std::move(ge)));

			auto cond = make_condition("output0", "<", 0, this);
			Relay::add(std::shared_ptr<Condition>(std::move(cond)));

			setActionTrigger(0, 0);
			if (!opFlags[no_disconnect])
			{
				setActionTrigger(0, 1);
				setActionTrigger(0, 2);
			}
		}
		
	}
	return sensor::dynObjectInitializeA(time0, flags);
}
void txLifeSpan::dynObjectInitializeB(const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet)
{
	IOdata iset{0.0};
	filterBlocks[0]->dynInitializeB(iset, iset, iset);
	Relay::dynObjectInitializeB(inputs, desiredOutput,fieldSet);//skip over sensor::dynInitializeB since we are initializing the blocks here
}


void txLifeSpan::updateA(coreTime time)
{
	if (time == prevTime)
	{
		return;
	}
	double Temperature = dataSources[0]->grabData();
	if (!opFlags[useIECmethod])
	{
		Faa = agingFactor*exp(agingConstant / (baseTemp + 273.0) - (agingConstant / (Temperature + 273.0)));
	}
	else
	{
		Faa = agingFactor*exp2((Temperature - baseTemp + 12) / 6.0);
	}


	filterBlocks[0]->step(time,  Faa);
	Relay::updateA(time);
	prevTime = time;
}

void txLifeSpan::timestep(coreTime time, const IOdata & /*inputs*/, const solverMode & /*sMode*/)
{
	updateA(time);
	
}

void txLifeSpan::actionTaken(index_t ActionNum, index_t /*conditionNum*/,  change_code /*actionReturn*/, coreTime /*actionTime*/)
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

}//namespace extra
}//namespace griddyn