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

#include "fncsCollector.h"
#include "fncsLibrary.h"
#include "fncsSupport.h"
#include "core/helperTemplates.h"

fncsCollector::fncsCollector(double time0, double period):collector(time0,period)
{

}

fncsCollector::fncsCollector(const std::string &collectorName):collector(collectorName)
{

}

fncsCollector::~fncsCollector()
{

}

std::shared_ptr<collector> fncsCollector::clone(std::shared_ptr<collector> gr) const
{
	auto nrec = cloneBase<fncsCollector, collector>(this, gr);
	if (!nrec)
	{
		return gr;
	}

	return nrec;
}

void fncsCollector::dataPointAdded(const collectorPoint& cp)
{
	if (!cp.colname.empty())
	{
		fncsRegister::instance()->registerPublication(cp.colname);
	}
}

change_code fncsCollector::trigger(double time)
{
	
	auto out=collector::trigger(time);
	
	auto colNames = getColumnDescriptions();
	for (size_t ii = 0; ii < data.size(); ++ii)
	{

		fncsSendVal(colNames[ii], data[ii]);
	}
	return out;
}


void fncsCollector::set(const std::string &param, double val)
{
	
	collector::set(param, val);
}

void fncsCollector::set(const std::string &param, const std::string &val)
{
	if (param[0] == '#')
	{
		
	}
	
	else
	{
		collector::set(param, val);
	}
	
}

const std::string fncsName("fncs");

const std::string &fncsCollector::getSinkName() const
{
	return fncsName;
}