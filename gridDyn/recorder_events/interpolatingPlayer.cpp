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

#include "gridEvent.h"
#include "timeSeries.h"
#include "gridDyn.h"
#include "units.h"
#include "objectInterpreter.h"
#include "stringOps.h"
#include "core/helperTemplates.h"
#include "core/gridDynExceptions.h"
#include <sstream>

#include <string>

interpolatingPlayer::interpolatingPlayer(const std::string &eventName) :gridPlayer(eventName)
{

}

interpolatingPlayer::interpolatingPlayer(gridDyn_time time0, double loopPeriod) : gridPlayer(time0,loopPeriod)
{

}


interpolatingPlayer::interpolatingPlayer(gridEventInfo *gdEI, gridCoreObject *rootObject) : gridPlayer(gdEI, rootObject)
{

}

std::shared_ptr<gridEvent> interpolatingPlayer::clone(std::shared_ptr<gridEvent> gE) const
{
	auto gp = cloneBaseStack<interpolatingPlayer, gridPlayer>(this, gE);
	if (!gp)
	{
		return gE;
	}
	gp->slopeField = slopeField;
	gp->useSlopeField = useSlopeField;
	return gp;
}


void interpolatingPlayer::set(const std::string &param, double val)
{
	if (param == "sampleperiod")
	{
		samplePeriod = val;
	}
	else if (param == "samplerate")
	{
		if (val > 0)
		{
			samplePeriod = 1.0 / val;
		}
		else
		{
			throw(invalidParameterValue());
		}
	}
	else
	{
		gridPlayer::set(param, val);
	}
}

void interpolatingPlayer::set(const std::string &param, const std::string &val)
{
	if (param == "slopefield")
	{
		slopeField = val;
		useSlopeField = true;
		setNextValue();
	}
	else
	{
		gridPlayer::set(param, val);
	}
}

void interpolatingPlayer::setNextValue()
{
	if (ts.count <= currIndex)
	{
		return;
	}
	if (currIndex >= ts.count - 1)
	{
		if (period > timeZero)
		{
			slope = (ts.data[0] - ts.data.back()) / (ts.time[0] - ts.time.back()-period);
		}
		else
		{
			slope = 0;
		}
	}
	else
	{
		slope = (ts.data[currIndex+1] - ts.data[currIndex]) / (ts.time[currIndex+1] - ts.time[currIndex]);
	}
	
		if (useSlopeField)
		{
			value = ts.data[currIndex];
			triggerTime = ts.time[currIndex];
		}
		else
		{
			triggerTime = std::min(ts.time[currIndex], triggerTime + samplePeriod);
			if (currIndex == 0)
			{
				if (period > timeZero)
				{
					value = ts.data.back() + slope*(triggerTime - ts.time.back());
				}
				else
				{
					value = ts.data[currIndex];
				}
			}
			else
			{
				value = ts.data[currIndex - 1] + slope*(triggerTime - ts.time[currIndex - 1]);
			}
			
		}
}

std::string interpolatingPlayer::toString()
{
	// @time1[,time2,time3,... |+ period] >[rootobj::obj:]field(units) = val1,[val2,val3,...]
	std::stringstream ss;
	if (eFile.empty())
	{
		ss << '@' << triggerTime;
		if (period > timeZero)
		{
			ss << '+' << period << '|';
		}
		else if (ts.count > 0)
		{
			for (size_t nn = currIndex + 1; nn < ts.count; ++nn)
			{
				ss << ", " << ts.time[nn];
			}
			ss << "| ";
		}
		else
		{
			ss << " | ";
		}
		ss << fullObjectName(m_obj) << ':' << field;
		if (unitType != gridUnits::defUnit)
		{
			ss << '(' << gridUnits::to_string(unitType) << ')';
		}
		ss << " = " << value;
		if (ts.count > 0)
		{
			for (size_t nn = currIndex + 1; nn < ts.count; ++nn)
			{
				ss << ", " << ts.data[nn];
			}
		}
	}
	else
	{
		ss << fullObjectName(m_obj) << ':' << field;
		if (unitType != gridUnits::defUnit)
		{
			ss << '(' << gridUnits::to_string(unitType) << ')';
		}
		ss << " = {" << eFile;
		if (column > 0)
		{
			ss << '#' << column;
		}
		ss << '}';
		if (period > timeZero)
		{
			ss << '+' << period;
		}
	}
	return ss.str();
}

change_code interpolatingPlayer::trigger()
{
	try
	{
		m_obj->set(field, value, unitType);
		return change_code::parameter_change;
	}
	catch (const gridDynException &)
	{
		return change_code::execution_failure;
	}
}

change_code interpolatingPlayer::trigger(gridDyn_time time)
{
	change_code ret = change_code::not_triggered;
	if (time+kSmallTime >= triggerTime)
	{
		try
		{
			if (useSlopeField)
			{
				m_obj->set(field, value, unitType);
				m_obj->set(slopeField, slope, unitType);
			}
			else
			{
				m_obj->set(field, value, unitType);
			}
			
			ret = change_code::parameter_change;
		}
		catch (const gridDynException &)
		{
			ret = change_code::execution_failure;
		}
		updateTrigger(time);
	}
	return ret;
}




