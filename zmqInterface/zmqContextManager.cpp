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

#include "zmqContextManager.h"

#include <cppzmq/zmq.hpp>

std::vector<std::shared_ptr<zmqContextManager>> zmqContextManager::contexts;
std::shared_ptr<zmqContextManager> zmqContextManager::defContext;

std::shared_ptr<zmqContextManager> zmqContextManager::getContextPointer(const std::string &contextName)
{
	//check if it already exists
	for (auto &v : contexts)
	{
		if (v->getName() == contextName)
		{
			return v;
		}
	}
	//if it doesn't make a new one with the appropriate name
	auto newContext = std::shared_ptr<zmqContextManager>(new zmqContextManager(contextName));
	contexts.push_back(newContext);
	return newContext;
}
std::shared_ptr<zmqContextManager> zmqContextManager::getContextPointer()
{
	if (!(defContext))
	{
		defContext = std::shared_ptr<zmqContextManager>(new zmqContextManager("default"));
		contexts.push_back(defContext);
	}
	
	return defContext;
}


zmq::context_t &zmqContextManager::getContext(const std::string &contextName)
{
	//check if it already exists
	for (auto &v : contexts)
	{
		if (v->getName() == contextName)
		{
			return v->getBaseContext();
		}
	}
	//if it doesn't make a new one with the appropriate name
	auto newContext = std::shared_ptr<zmqContextManager>(new zmqContextManager(contextName));
	contexts.push_back(newContext);
	return newContext->getBaseContext();
}

zmq::context_t &zmqContextManager::getContext()
{
	if (!(defContext))
	{
		defContext = std::shared_ptr<zmqContextManager>(new zmqContextManager("default"));
		contexts.push_back(defContext);
	}

	return defContext->getBaseContext();
}
void zmqContextManager::closeContext(const std::string &contextName)
{
	auto vbeg = contexts.begin();
	auto vend = contexts.end();
	while (vbeg != vend)
	{
		if ((*vbeg)->getName() == contextName)
		{
			contexts.erase(vbeg);
			return;
		}
	}
}

void zmqContextManager::closeContext()
{
	closeContext("default");
	defContext = nullptr;
}

zmqContextManager::~zmqContextManager()
{
	if (zcontext != nullptr)
	{
		delete zcontext;
	}
}


zmqContextManager::zmqContextManager(const std::string &contextName):name(contextName)
{
	zcontext = new zmq::context_t();
}