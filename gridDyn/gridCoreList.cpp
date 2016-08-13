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

#include "gridCoreList.h"

gridCoreList::gridCoreList()
{
}

bool gridCoreList::insert(gridCoreObject *obj, bool replace)
{
	auto inp = m_objects.insert(obj);
	if (inp.second)
	{
		return true;
	}
	if (replace)
	{
		m_objects.replace(inp.first, obj);
		return true;
	}
	return false;

}
gridCoreObject *gridCoreList::find(const std::string &objname) const
{
	auto fp = m_objects.get<name>().find(objname);
	if (fp != m_objects.get<name>().end())
	{
		return (*fp);
	}
	else
	{
		return nullptr;
	}
}

std::vector<gridCoreObject *> gridCoreList::find(index_t searchID) const
{
	auto fp = m_objects.get<uid>().lower_bound(searchID);
	auto fp2 = m_objects.get<uid>().upper_bound(searchID);
	std::vector<gridCoreObject *> out;
	while (fp != fp2)
	{
		if ((*fp)->getUserID() == searchID)
		{
			out.push_back(*fp);
		}
		++fp;
	}
	return out;
}

bool gridCoreList::remove(gridCoreObject *obj)
{
	auto fp = m_objects.get<id>().find(obj->getID());
	if (fp != m_objects.get<id>().end())
	{
		m_objects.erase(fp);
		return true;
	}
	return false;
}

bool gridCoreList::remove(const std::string &objname)
{
	auto fp = m_objects.get<name>().find(objname);
	if (fp != m_objects.get<name>().end())
	{
		//I don't know why I have to do this find on the id index
		//Not understanding these multindex objects well enough I guess
		auto fp2 = m_objects.get<id>().find((*fp)->getID());
		m_objects.erase(fp2);
		
		return true;
	}
	return false;
}

bool gridCoreList::isMember(gridCoreObject *obj) const
{
	auto fp = m_objects.get<id>().find(obj->getID());
	return (fp != m_objects.get<id>().end()) ? true : false;
}

void gridCoreList::deleteAll(gridCoreObject *parent)
{
	for (auto &it : m_objects)
	{
		condDelete(it, parent);
	}

}

void gridCoreList::updateObject(gridCoreObject *obj)
{
	auto fp = m_objects.get<id>().find(obj->getID());
	m_objects.replace(fp, obj);
}