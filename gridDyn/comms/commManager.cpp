
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
#include "commManager.h"
#include "comms/gridCommunicator.h"
#include "core/propertyBuffer.h"

commManager::commManager()
{

}


commManager::commManager(const commManager &cm)
{
	commName = cm.commName;
	commId = cm.commId;
	commType = cm.commType;
	commDestName = cm.commDestName;
	commDestId = cm.commDestId;
	if (cm.commLink)
	{
		commLink = cm.commLink->clone();
	}
	if (cm.commPropBuffer)
	{
		commPropBuffer = new propertyBuffer(*cm.commPropBuffer);
	}
}

commManager::commManager(commManager &&cm): commName(std::move(cm.commName)), commType(std::move(cm.commType)),commDestName(std::move(cm.commDestName))
{
	commId = cm.commId;
	commDestId = cm.commDestId;

	commLink = cm.commLink;

	commPropBuffer = cm.commPropBuffer;
	cm.commPropBuffer = nullptr;
}


commManager &commManager::operator=(const commManager &cm)
{
	commName = cm.commName;
	commId = cm.commId;
	commType = cm.commType;
	commDestName = cm.commDestName;
	commDestId = cm.commDestId;
	if (cm.commLink)
	{
		commLink = cm.commLink->clone();
	}
	if (commPropBuffer)
	{
		delete commPropBuffer;
	}
	if (cm.commPropBuffer)
	{
		commPropBuffer = new propertyBuffer(*(cm.commPropBuffer));
	}
	return *this;
}
commManager &commManager::operator=(commManager &&cm)
{
	commName = std::move(cm.commName);
	commId = cm.commId;
	commType = std::move(cm.commType);
	commDestName = cm.commDestName;
	commDestId = std::move(cm.commDestId);
	commLink = cm.commLink;

	commPropBuffer = cm.commPropBuffer;
	cm.commPropBuffer = nullptr;
	
	return *this;
}

commManager::~commManager()
{
	if (commPropBuffer)
	{
		delete commPropBuffer;
	}

}

bool commManager::set(const std::string &param, const std::string &val)
{
	if ((param == "commname")||(param=="name"))
	{
		commName = val;
		
	}
	else if (param == "commtype")
	{
		commType = val;
		
	}
	else if ((param == "commdest") || (param == "destination"))
	{
		if (val.front() == '#')
		{
			commDestId = std::stoull(val.substr(1, std::string::npos));
		}
		else
		{
			commDestName = val;
		}
	}
	else if (param.compare(0, 6, "comm::") == 0)
	{
		if (commLink)
		{
			commLink->set(param.substr(6), val);
		}
		else
		{
			if (!commPropBuffer)
			{
				commPropBuffer = new propertyBuffer;
			}
			commPropBuffer->set(param.substr(6), val);
		}
	}
	else
	{
		return false;
	}
	return true;
}
bool commManager::set(const std::string &param, double val)
{
	if ((param == "commid")||(param=="id"))
	{
		commId = static_cast<std::uint64_t> (val);
	}
	else if ((param == "commdestid")||(param=="destid"))
	{
		commDestId = static_cast<uint64_t> (val);
	}
	else if (param.compare(0, 6, "comm::") == 0)
	{
		if (commLink)
		{
			commLink->set(param.substr(6), val);
		}
		else
		{
			if (!commPropBuffer)
			{
				commPropBuffer = new propertyBuffer;
			}
			commPropBuffer->set(param.substr(6), val);
		}
	}
	else
	{
		return false;
	}

	return true;
}


bool commManager::setFlag(const std::string &flag, bool val)
{
	if (flag.compare(0, 6, "comm::") == 0)
	{
		if (commLink)
		{
			commLink->setFlag(flag.substr(6), val);
		}
		else
		{
			if (!commPropBuffer)
			{
				commPropBuffer = new propertyBuffer;
			}
			commPropBuffer->setFlag(flag.substr(6), val);
		}
	}
	else
	{
		return false;
	}
	return true;
}


std::shared_ptr<gridCommunicator> commManager::build()
{
	commLink = makeCommunicator(commType, commName, commId);
	if (commPropBuffer)
	{
		commPropBuffer->apply(commLink.get());
		delete commPropBuffer;
		commPropBuffer = nullptr;
	}
	return commLink;
}

void commManager::send(std::shared_ptr<commMessage> m)
{
	if (!commDestName.empty())
	{
		commLink->transmit(commDestId, m);
	}
	else if (commDestId != 0)
	{
		commLink->transmit(commDestName, m);
	}
	else
	{
		commLink->transmit(0, m);
	}
}