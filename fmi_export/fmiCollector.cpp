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

#include "fmiCollector.h"
#include "core/helperTemplates.h"
#include "fmiCoordinator.h"
#include "measurement/gridGrabbers.h"

fmiCollector::fmiCollector():collector(maxTime,maxTime)
{

}
fmiCollector::fmiCollector(const std::string &name):collector(name)
{
	triggerTime = maxTime;
	timePeriod = maxTime;
}


std::shared_ptr<collector> fmiCollector::clone(std::shared_ptr<collector> gr) const
{
	auto nrec = cloneBase<fmiCollector, collector>(this, gr);
	if (!nrec)
	{
		return gr;
	}

	

	return nrec;
}

change_code fmiCollector::trigger(coreTime time)
{
	collector::trigger(time);
	return change_code::no_change;
}


void fmiCollector::set(const std::string &param, double val)
{
	if (param.front() == '#')
	{

	}
	else
	{
		collector::set(param, val);
	}
}
void fmiCollector::set(const std::string &param, const std::string &val)
{
	if (param.front() == '#')
	{

	}
	else
	{
		collector::set(param, val);
	}
}

static const std::string defFMIName("fmi");
const std::string &fmiCollector::getSinkName() const
{
	if (coord)
	{
		return coord->getFMIName();
	}
	else
	{
		return defFMIName;
	}
}


coreObject *fmiCollector::getOwner() const
{
	return coord;
}

void fmiCollector::dataPointAdded(const collectorPoint& cp)
{
	if (!coord)
	{
		//find the coordinator first
		auto gobj = cp.dataGrabber->getObject();
		if (gobj)
		{
			auto rto = gobj->getRoot();
			if (rto)
			{
				auto fmiCont = rto->find("fmiCoordinator");
				if (dynamic_cast<fmiCoordinator *>(fmiCont))
				{
					coord = static_cast<fmiCoordinator *>(fmiCont);
				}
			}
		}
	}
	if (coord)
	{
		if (cp.columnCount == 1)
		{
			coord->registerOutput(cp.colname, cp.column, this);
		}
		else
		{
			//TODO:: deal with output vectors later
		}
		
	}
	
}