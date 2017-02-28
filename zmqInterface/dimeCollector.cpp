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

#include "dimeCollector.h"
#include "dimeClientInterface.h"
#include "core/helperTemplates.h"

dimeCollector::dimeCollector(coreTime time0, coreTime period):collector(time0,period)
{

}

dimeCollector::dimeCollector(const std::string &collectorName):collector(collectorName)
{

}

dimeCollector::~dimeCollector()
{
	if (dime)
	{
		dime->close();
	}
}

std::shared_ptr<collector> dimeCollector::clone(std::shared_ptr<collector> gr) const
{
	auto nrec = cloneBase<dimeCollector, collector>(this, gr);
	if (!nrec)
	{
		return gr;
	}

	nrec->server = server;
	nrec->processName = processName;

	return nrec;
}

change_code dimeCollector::trigger(coreTime time)
{
	if (!dime)
	{
		dime = std::make_unique<dimeClientInterface>(processName, server);
	}
	auto out=collector::trigger(time);
	//figure out what to do with the data
	return out;
}


void dimeCollector::set(const std::string &param, double val)
{
	
	collector::set(param, val);
}

void dimeCollector::set(const std::string &param, const std::string &val)
{
	if (param == "server")
	{
		server = val;
	}
	else if (param == "processname")
	{
		processName = val;
	}
	else
	{
		collector::set(param, val);
	}
	
}

const std::string &dimeCollector::getSinkName() const
{
	return server;
}