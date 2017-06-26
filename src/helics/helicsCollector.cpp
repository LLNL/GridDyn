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

#include "helicsCollector.h"
#include "helicsLibrary.h"
#include "helicsSupport.h"
#include "core/helperTemplates.hpp"
#include "stringOps.h"

namespace griddyn
{
namespace helicsLib
{
helicsCollector::helicsCollector(coreTime time0, coreTime period):collector(time0,period)
{

}

helicsCollector::helicsCollector(const std::string &collectorName):collector(collectorName)
{

}

helicsCollector::~helicsCollector() = default;

std::shared_ptr<collector> helicsCollector::clone(std::shared_ptr<collector> gr) const
{
	auto nrec = cloneBase<helicsCollector, collector>(this, gr);
	if (!nrec)
	{
		return gr;
	}

	return nrec;
}

void helicsCollector::dataPointAdded(const collectorPoint& cp)
{
	if (!cp.colname.empty())
	{
		
	}
}

change_code helicsCollector::trigger(coreTime time)
{
	
	auto out=collector::trigger(time);
	
	auto colNames = getColumnDescriptions();
	std::vector<bool> subscribe(colNames.size(), true);

	
	for (size_t ii = 0; ii < complexPairs.size(); ++ii)
	{
		auto &n1 = complexPairs[ii].first;
		auto &n2 = complexPairs[ii].second;
		int index1 = -1;
		int index2 = -1;
		for (int pp = 0; pp < static_cast<int>(colNames.size()); ++pp)
		{
			if (n1 == colNames[pp])
			{
				index1 = pp;
			}
			if (n2 == colNames[pp])
			{
				index2 = pp;
			}
		}
		if ((index1 >= 0) && (index2 >= 0))
		{
			subscribe[index1] = false;
			subscribe[index2] = false;
		}
		//helicsSendComplex(cnames[ii], data[index1], data[index2]);
	}

	for (size_t ii = 0; ii < data.size(); ++ii)
	{
		if (subscribe[ii])
		{
			//helicsSendVal(colNames[ii], data[ii]);
		}
		
	}
	return out;
}


void helicsCollector::set(const std::string &param, double val)
{
	
	collector::set(param, val);
}

void helicsCollector::set(const std::string &param, const std::string &val)
{
	using namespace stringOps;
	if (param == "complex")
	{
		auto asLoc = val.find("as");
		cnames.push_back(trim(val.substr(asLoc + 2)));
		auto commaLoc = val.find_first_of(',');
		complexPairs.emplace_back(trim(val.substr(0, commaLoc)), trim(val.substr(commaLoc + 1, asLoc - 1 - commaLoc)));
		//helicsRegister::instance()->registerPublication(cnames.back(), helicsRegister::dataType::helicsComplex);
	}
	else
	{
		collector::set(param, val);
	}
	
}

const std::string helicsName("helics");

const std::string &helicsCollector::getSinkName() const
{
	return helicsName;
}

}// namespace helicsLib
} // namespace griddyn