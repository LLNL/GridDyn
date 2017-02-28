/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include "gridEvent.h"
#include "timeSeries.h"
#include "gridDyn.h"
#include "units.h"
#include "core/objectInterpreter.h"
#include "stringOps.h"
#include "core/helperTemplates.h"
#include "core/coreExceptions.h"
#include <sstream>

#include <string>

gridPlayer::gridPlayer(const std::string &eventName):gridEvent(eventName)
{
	
}

gridPlayer::gridPlayer(coreTime time0,double loopPeriod) : gridEvent(time0), period(loopPeriod)
{

}


gridPlayer::gridPlayer(gridEventInfo &gdEI, coreObject *rootObject) : gridEvent(gdEI,rootObject), period(gdEI.period)
{
	if (gdEI.file.empty())
	{
		setTimeValue(gdEI.time, gdEI.value);
	}
	else
	{
		if (!gdEI.columns.empty())
		{
			column = gdEI.columns[0];
		}
		loadEventFile(gdEI.file);
	}
	
}

void gridPlayer::updateEvent(gridEventInfo &gdEI, coreObject *rootObject)
{
	if (gdEI.file.empty())
	{
		setTimeValue(gdEI.time, gdEI.value);
	}
	else
	{
		if (!gdEI.columns.empty())
		{
			column = gdEI.columns[0];
		}
		loadEventFile(gdEI.file);
	}
	gridEvent::updateEvent(gdEI, rootObject);
}

std::shared_ptr<gridEvent> gridPlayer::clone(std::shared_ptr<gridEvent> gE) const
{
	auto gp = cloneBase<gridPlayer, gridEvent>(this, gE);
	if (!gp)
	{
		return gE;
	}
	gp->period = period;
	gp->eFile = eFile;
	gp->ts = ts;
	gp->column = column;
	gp->currIndex = currIndex;
	gp->timeOffset = timeOffset;
	return gp;
}


void gridPlayer::set(const std::string &param, double val)
{
	if (param == "period")
	{
		value = val;
	}
	else if (param == "column")
	{
		column = static_cast<index_t> (val);
	}
	else if (param == "timeoffset")
	{
		timeOffset = val;
	}
	else 
	{
		gridEvent::set(param, val);
	}
}

void gridPlayer::set(const std::string &param, const std::string &val)
{
	if (param == "file")
	{
		loadEventFile(val);
	}
	else
	{
		gridEvent::set(param,val);
	}
}

void gridPlayer::setTime(coreTime time)
{
	triggerTime = time;
}

void gridPlayer::setTimeValue(coreTime time, double val)
{
	triggerTime = time;
	value = val;
}

void gridPlayer::setTimeValue(const std::vector<coreTime> &time, const std::vector<double> &val)
{
	
	ts.reserve(static_cast<fsize_t> (time.size()));

	ts.addData(time, val);

	currIndex = 0;
	setNextValue();
}

void gridPlayer::setNextValue()
{
	if (static_cast<size_t> (currIndex) < ts.size())
	{
		triggerTime = ts.time(currIndex)+timeOffset;
		value = ts.data(currIndex);
	}
}

void gridPlayer::updateTrigger(coreTime time)
{
	if (time>= triggerTime)
	{
		if (time >= ts.time(currIndex)+timeOffset)
		{
			++currIndex;
		}
	}
		if (static_cast<size_t> (currIndex) >= ts.size())
		{
			if (period > timeZero)                     //if we have a period loop the time series
			{
				if (time - ts.lastTime() > period)
				{
					timeOffset = period + triggerTime;
					
				}
				else
				{
					timeOffset += period;
				}

				currIndex = 0;
				setNextValue();
			}
			else
			{
				armed = false;
			}
		}
		else                   //just proceed to the next trigger and Value
		{
			setNextValue();
		}
}

std::string gridPlayer::toString()
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
		else if (ts.size() > 0)
		{
			for (auto nn = currIndex + 1; nn < ts.size(); ++nn)
			{
				ss << ", " << ts.time(nn);
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
		if (ts.size() > 0)
		{
			for (index_t nn = currIndex + 1; nn < ts.size(); ++nn)
			{
				ss << ", " << ts.data(nn);
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
			ss << '+' << period ;
		}
	}
	return ss.str();
}

change_code gridPlayer::trigger()
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

change_code gridPlayer::trigger(coreTime time)
{
	change_code ret = change_code::not_triggered;
	if (time+kSmallTime >= triggerTime)
	{
		try
		{
			m_obj->set(field, value, unitType);
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

void gridPlayer::initialize()
{

}

bool gridPlayer::setTarget(coreObject *gdo, const std::string &var)
{
	if (!var.empty())
	{
		field = var;
	}
	m_obj = gdo;

	if (m_obj)
	{
		setName(m_obj->getName());
		armed = true;
	}
	return armed;
}

void gridPlayer::loadEventFile(const std::string &fname)
{
	if (!fname.empty())
	{
		eFile = fname;
	}
	
	ts.loadFile(eFile, column);

	currIndex = 0;
	setNextValue();
}
