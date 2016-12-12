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
#include "sourceModels/sourceTypes.h"
#include "loadModels/otherLoads.h"
#include "gridCoreTemplates.h"
#include "core/gridDynExceptions.h"
#include <map>
#include <cmath>


sourceLoad::sourceLoad(const std::string &objName) :gridLoad(objName)
{
	sourceLink.fill(-1);
}

sourceLoad::sourceLoad(sourceType type, const std::string &objName):sourceLoad(objName)
{
	sType = type;
	//add the sources for P and Q
	add(makeSource(p_source));
	add(makeSource(q_source));
		
	
}

coreObject * sourceLoad::clone(coreObject *obj) const
{
	sourceLoad *nobj = cloneBase<sourceLoad, gridLoad>(this, obj);
	if (nobj == nullptr)
	{
		return obj;
	}
	nobj->sourceLink = sourceLink;
	for (auto &src : sources)
	{
		if (src)
		{
			auto newsrc = static_cast<gridSource *>(src->clone());
			newsrc->locIndex = src->locIndex;
			nobj->add(newsrc);
		}
	}
	return nobj;
}

static const std::map<std::string, int> source_lookup
{
	{ "source",sourceLoad::p_source },
	{ "psource",sourceLoad::p_source },
	{ "p_source",sourceLoad::p_source },
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

static const std::map<std::string, int> source_match
{
	{ "source",sourceLoad::p_source },
	{ "psource",sourceLoad::p_source },
	{ "p_source",sourceLoad::p_source },
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

void sourceLoad::add(coreObject *obj)
{
	if (dynamic_cast<gridSource *>(obj))
	{
		add(static_cast<gridSource *>(obj));
	}
}


void sourceLoad::add(gridSource *src)
{
	src->setParent(this);
	src->setFlag("pflow_init_required", true);
	if (src->locIndex != kNullLocation)
	{
		
	}
	else if (!src->m_purpose.empty())
	{
		auto ind = source_match.find(src->m_purpose);
		if (ind != source_match.end())
		{
			src->locIndex = ind->second;
		}
		else
		{
			src->locIndex = static_cast<int>(sources.size());
		}
	}
	else
	{
		src->locIndex = static_cast<int>(sources.size());
	}

	if (sources.size() <= src->locIndex)
	{
		sources.resize(src->locIndex + 1,nullptr);
	}
	if (sources[src->locIndex])
	{
		remove(sources[src->locIndex]);

	}
	sources[src->locIndex] = src;
	if (src->locIndex < 8)
	{
		if (sourceLink[src->locIndex] < 0)
		{
			sourceLink[src->locIndex] = static_cast<int>(src->locIndex);
		}
	}
	//now add to the subObjectList
	addSubObject(src);
}


void sourceLoad::remove(coreObject *obj)
{
	if (dynamic_cast<gridSource *>(obj))
	{
		remove(static_cast<gridSource *>(obj));
	}
}

void sourceLoad::remove(gridSource *obj)
{
	if ((obj->locIndex != kNullLocation) && (obj->locIndex < sources.size()))
	{
		if ((sources[obj->locIndex]) && (sources[obj->locIndex]->getID() == obj->getID()))
		{
			sources[obj->locIndex] = nullptr;
			obj->setParent(nullptr);
			for (auto &lnk : sourceLink)
			{
				if (lnk == static_cast<int>(obj->locIndex))
				{
					lnk = -1;
				}
			}
			gridSecondary::remove(obj);
		}
	}

}

gridSource *sourceLoad::findSource(const std::string &srcname)
{
	auto ind = source_match.find(srcname);
	if (ind != source_match.end())
	{
		int index = sourceLink[ind->second];
		if (index < 0)
		{
			add(makeSource(static_cast<sourceLoad::sourceLoc>(ind->second)));
		}
		else if ((static_cast<int>(sources.size())<=index)||(!sources[index]))
		{
			//this may not actually do anything is the sType is set to other
			add(makeSource(static_cast<sourceLoad::sourceLoc>(ind->second)));
		}
		return sources[index];
	}
	return nullptr;
}

gridSource *sourceLoad::findSource(const std::string &srcname) const
{
	auto ind = source_match.find(srcname);
	if (ind != source_match.end())
	{
		int index = sourceLink[ind->second];
		if (index < 0)
		{
			return nullptr;
		}
		else if ((static_cast<int>(sources.size()) <= index) || (!sources[index]))
		{
			return nullptr;
		}
		return sources[index];
	}
	return nullptr;
}

void sourceLoad::setFlag(const std::string &flag, bool val)
{
	auto sfnd = flag.find_last_of(":?");
	if (sfnd != std::string::npos)
	{
		auto src = findSource(flag.substr(0, sfnd));
		if (src)
		{
			src->setFlag(flag.substr(sfnd + 1, std::string::npos), val);
		}
		else
		{
			throw(unrecognizedParameter());
		}
	}
	else
	{
		gridLoad::setFlag(flag, val);
	}
}

void sourceLoad::set(const std::string &param, const std::string &val)
{
	auto sfnd = param.find_last_of(":?");
	if (sfnd != std::string::npos)
	{
		auto src = findSource(param.substr(0, sfnd));
		if (src)
		{
			src->set(param.substr(sfnd + 1, std::string::npos), val);
		}
		else
		{
			throw(unrecognizedParameter());
		}
	}
	else
	{
		gridLoad::set(param, val);
	}
}

void sourceLoad::timestep(gridDyn_time ttime, const IOdata &args, const solverMode &sMode)
{
	for (auto &src : subObjectList)
	{
		static_cast<gridSource *>(src)->timestep(ttime, emptyArguments, sMode);
	}
	getSourceLoads();
	prevTime = ttime;
	gridLoad::timestep(ttime, args, sMode);
}

void sourceLoad::setState(gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	
	for (auto &src : subObjectList)
	{
		src->setState(ttime,state,dstate_dt,sMode);
	}
	getSourceLoads();
	prevTime = ttime;
}

void sourceLoad::set(const std::string &param, double val, gridUnits::units_t unitType)
{
	auto sfnd = param.find_last_of(":?");
	if (sfnd != std::string::npos)
	{
		if (sfnd != std::string::npos)
		{
			auto src = findSource(param.substr(0, sfnd));
			if (src)
			{
				src->set(param.substr(sfnd + 1, std::string::npos), val,unitType);
			}
			else
			{
				throw(unrecognizedParameter());
			}
		}
	}
	else
	{
		auto ind = source_lookup.find(param.substr(0, sfnd));
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
			auto keyind = sourcekey_lookup.find(param.substr(0, sfnd));

			if (keyind != sourcekey_lookup.end())
			{
				if ((static_cast<int>(sources.size()) > keyind->second) && (sources[keyind->second]))
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
	gridSecondary::pFlowObjectInitializeA(time0, flags); //to initialize the submodels
	
	getSourceLoads();
}

void sourceLoad::dynObjectInitializeA(gridDyn_time time0, unsigned long flags)
{
	gridSecondary::dynObjectInitializeA(time0, flags);
	getSourceLoads();
}

void sourceLoad::updateLocalCache(const IOdata & /*args*/, const stateData &sD, const solverMode &sMode)
{
	for (auto &src : sources)
	{
		src->updateLocalCache(emptyArguments,sD,sMode);
	}
	getSourceLoads();
}

void sourceLoad::getSourceLoads()
{
	if (sourceLink[p_source] >= 0)
	{
		setP(sources[sourceLink[p_source]]->getOutput());
	}
	if (sourceLink[q_source] >= 0)
	{
		setQ(sources[sourceLink[q_source]]->getOutput());
	}
	if (sourceLink[yp_source] >= 0)
	{
		setYp(sources[sourceLink[yp_source]]->getOutput());
	}
	if (sourceLink[yq_source] >= 0)
	{
		setYq(sources[sourceLink[yq_source]]->getOutput());
	}
	if (sourceLink[ip_source] >= 0)
	{
		setIp(sources[sourceLink[ip_source]]->getOutput());
	}
	if (sourceLink[iq_source] >= 0)
	{
		setIq(sources[sourceLink[iq_source]]->getOutput());
	}
	if (sourceLink[r_source] >= 0)
	{
		setr(sources[sourceLink[r_source]]->getOutput());
	}
	if (sourceLink[x_source] >= 0)
	{
		setx(sources[sourceLink[x_source]]->getOutput());
	}
}

gridSource *sourceLoad::makeSource(sourceLoc loc)
{
	gridSource *src = nullptr;
	switch (sType)
	{
	case sourceType::pulse:
		src = new pulseSource();
		break;
	case sourceType::random:
		src = new randomSource();
		break;
	case sourceType::sine:
		src = new sineSource();
		break;
	case sourceType::other:
	default:
		return nullptr;
	}
	src->locIndex = static_cast<index_t>(loc);
	return src;
}

coreObject *sourceLoad::find(const std::string &obj) const
{
	auto src = findSource(obj);
	if (src == nullptr)
	{
		return gridObject::find(obj);
	}
	return src;
}