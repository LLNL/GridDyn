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

#include "sourceModels/gridSource.h"
#include "loadModels/otherLoads.h"
#include "gridCoreTemplates.h"
#include "core/gridDynExceptions.h"
#include <map>
#include <cmath>


sourceLoad::sourceLoad(const std::string &objName) :gridLoad(objName)
{
	sourceLink.fill(-1);
}

gridCoreObject * sourceLoad::clone(gridCoreObject *obj) const
{
	sourceLoad *nobj = cloneBase<sourceLoad, gridLoad>(this, obj);
	if (nobj == nullptr)
	{
		return obj;
	}
	nobj->sourceLink = sourceLink;
	for (auto &src : sources)
	{
		nobj->add(src->clone());
	}
	return nobj;
}

void sourceLoad::add(gridCoreObject *obj)
{
	if (dynamic_cast<gridSource *>(obj))
	{
		add(static_cast<gridSource *>(obj));
	}
}


void sourceLoad::add(gridSource *src)
{
	src->setParent(this);
	src->setFlag("pflow_initialization", 1);
	if (src->locIndex != kNullLocation)
	{
		if (sources.size() <= src->locIndex)
		{
			sources.resize(src->locIndex + 1);
		}
		sources[src->locIndex] = src;
	}
	else
	{
		src->locIndex = static_cast<int>(sources.size());
		sources.push_back(src);
	}
	subObjectList.push_back(src);
}

void sourceLoad::set(const std::string &param, const std::string &val)
{
	if (param[0] == '#')
	{

	}
	else
	{
		gridLoad::set(param, val);
	}
}

static const std::map<std::string, int> source_lookup
{
	{"source",sourceLoad::p_source },
	{"psource",sourceLoad::p_source},
	{"p_source",sourceLoad::p_source },
	{ "qsource",sourceLoad::q_source },
	{ "q_source",sourceLoad::q_source },
	{ "rsource",sourceLoad::r_source },
	{ "r_source",sourceLoad::r_source },
	{ "xsource",sourceLoad::x_source },
	{ "x_source",sourceLoad::x_source },
	{ "ypsource",sourceLoad::yp_source },
	{ "yp_source",sourceLoad::yp_source },
	{ "yqsource",sourceLoad::yq_source },
	{ "yq_source",sourceLoad::yq_source },
	{ "ipsource",sourceLoad::ip_source },
	{ "ip_source",sourceLoad::ip_source },
	{ "iqsource",sourceLoad::iq_source },
	{ "iq_source",sourceLoad::iq_source },
};

static const std::map<std::string, int> sourcekey_lookup
{
	{ "p",sourceLoad::p_source },
	{ "q",sourceLoad::q_source },
	{ "r",sourceLoad::r_source },
	{ "x",sourceLoad::x_source },
	{ "yp",sourceLoad::yp_source },
	{ "zp",sourceLoad::yp_source },
	{ "zr",sourceLoad::yp_source },
	{ "yq",sourceLoad::yq_source },
	{ "zq",sourceLoad::yq_source },
	{ "ip",sourceLoad::ip_source },
	{ "iq",sourceLoad::iq_source },
};
	

void sourceLoad::set(const std::string &param, double val, gridUnits::units_t unitType)
{
	auto sfnd = param.find_last_of(":?");
	if (sfnd != std::string::npos)
	{
		auto ind = source_lookup.find(param.substr(0, sfnd - 1));
		if (ind != source_lookup.end())
		{
			if ((static_cast<int>(sources.size()) < ind->second) && (sources[ind->second]))
			{
				sources[ind->second]->set(param.substr(sfnd + 1, std::string::npos), val, unitType);
				return;
			}
			else
			{
				throw(unrecognizedParameter());
			}
		}
		else
		{
			throw(unrecognizedParameter());
		}
	}
	else
	{
		auto ind = source_lookup.find(param.substr(0, sfnd - 1));
		if (ind != source_lookup.end())
		{
			if ((static_cast<int>(sources.size()) < ind->second) && (sources[ind->second]))
			{
				sourceLink[ind->second] = static_cast<int>(val);
			}
			else if (!opFlags[pFlow_initialized])
			{
				sourceLink[ind->second] = static_cast<int>(val);
				
			}
			else
			{
				throw(unrecognizedParameter());
			}
			return;
		}
		else
		{
			auto keyind = sourcekey_lookup.find(param.substr(0, sfnd - 1));

			if (keyind != source_lookup.end())
			{
				if ((static_cast<int>(sources.size()) < keyind->second) && (sources[keyind->second]))
				{
					sources[keyind->second]->set("level", gridUnits::unitConversion(val, unitType, gridUnits::puMW, systemBasePower));
					return;
				}
			}
			gridLoad::set(param, val, unitType);
		}
	}
	
}

void sourceLoad::pFlowObjectInitializeA(gridDyn_time time0, unsigned long flags)
{
	//Do a check on the sources;
	int fnd = 0;
	for (auto &sL : sourceLink)
	{
		if (sL < 0)
		{
			continue;
		}
		++fnd;
		if (sL >= static_cast<int>(sources.size()))
		{
			LOG_WARNING("no source given at called index");
		}
		else if (!sources[sL])
		{
			LOG_WARNING("no source given at called index");
		}
	}
	if (fnd == 0) //auto fill them if none were specified
	{
		for (int kk = 0; kk < 8; ++kk)
		{
			if (sources[kk])
			{
				sourceLink[kk] = kk;
			}
		}
	}
	gridSecondary::pFlowObjectInitializeA(time0, flags); //to initialize the submodels
	
	getSourceLoads();
}

void sourceLoad::dynObjectInitializeA(gridDyn_time time0, unsigned long flags)
{
	gridSecondary::dynObjectInitializeA(time0, flags);
	getSourceLoads();
}

void sourceLoad::loadUpdate(gridDyn_time ttime)
{
	for (auto &src : sources)
	{
		src->sourceUpdate(ttime);
	}
	getSourceLoads();
}

void sourceLoad::loadUpdateForward(gridDyn_time ttime)
{
	for (auto &src : sources)
	{
		src->sourceUpdateForward(ttime);
	}
	getSourceLoads();
}

void sourceLoad::getSourceLoads()
{
	if (sourceLink[p_source] >= 0)
	{
		P = sources[sourceLink[p_source]]->getOutput();
	}
	if (sourceLink[q_source] >= 0)
	{
		Q = sources[sourceLink[q_source]]->getOutput();
	}
	if (sourceLink[yp_source] >= 0)
	{
		Yp = sources[sourceLink[yp_source]]->getOutput();
	}
	if (sourceLink[yq_source] >= 0)
	{
		Yq = sources[sourceLink[yq_source]]->getOutput();
	}
	if (sourceLink[ip_source] >= 0)
	{
		Ip = sources[sourceLink[ip_source]]->getOutput();
	}
	if (sourceLink[iq_source] >= 0)
	{
		Iq = sources[sourceLink[iq_source]]->getOutput();
	}
	if (sourceLink[r_source] >= 0)
	{
		r = sources[sourceLink[r_source]]->getOutput();
	}
	if (sourceLink[x_source] >= 0)
	{
		x = sources[sourceLink[x_source]]->getOutput();
	}
}