
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

#include "propertyBuffer.h"
#include "coreObject.h"
#include "helperObject.h"
#include <algorithm>

propertyBuffer::propertyBuffer()
{

}


propertyBuffer::propertyBuffer(const propertyBuffer &buf) = default;

propertyBuffer::propertyBuffer(propertyBuffer &&buf) = default;

propertyBuffer &propertyBuffer::operator=(const propertyBuffer &buf) = default;

propertyBuffer &propertyBuffer::operator=(propertyBuffer &&buf) = default;

void propertyBuffer::set(const std::string &param, const std::string &val)
{
	stringProperties.emplace_back(param, val);
}
void propertyBuffer::set(const std::string &param, double val)
{
	doubleProperties.emplace_back(param, val,gridUnits::defUnit);
}
void propertyBuffer::set(const std::string &param, double val,gridUnits::units_t unitType)
{
	doubleProperties.emplace_back(param, val,unitType);
}
void propertyBuffer::set(const std::string &param, int val)
{
	intProperties.emplace_back(param, val);
}
void propertyBuffer::setFlag(const std::string &param, bool val)
{
	flagProperties.emplace_back(param, val);
}


void propertyBuffer::remove(const std::string &param)
{
	//Using auto lambda here still seems like magic that this works
	auto checkMatch = [param](auto input) {return (std::get<0>(input) == param); };

	auto strend=std::remove_if(stringProperties.begin(), stringProperties.end(), checkMatch);
	stringProperties.erase(strend, stringProperties.end());
	auto doubend = std::remove_if(doubleProperties.begin(), doubleProperties.end(), checkMatch);
	doubleProperties.erase(doubend, doubleProperties.end());
	auto intend = std::remove_if(intProperties.begin(), intProperties.end(), checkMatch);
	intProperties.erase(intend, intProperties.end());
	auto flagend = std::remove_if(flagProperties.begin(), flagProperties.end(), checkMatch);
	flagProperties.erase(flagend, flagProperties.end());
}

void propertyBuffer::clear()
{
	stringProperties.clear();
	doubleProperties.clear();
	intProperties.clear();
	flagProperties.clear();
}

void propertyBuffer::apply(coreObject *obj) const
{
	for (auto &sprop : stringProperties)
	{
		obj->set(sprop.first, sprop.second);
	}
	for (auto &sprop : flagProperties)
	{
		obj->setFlag(sprop.first, sprop.second);
	}
	for (auto &sprop : intProperties)
	{
		obj->set(sprop.first, sprop.second);
	}
	for (auto &sprop : doubleProperties)
	{
		obj->set(std::get<std::string>(sprop), std::get<double>(sprop),std::get<gridUnits::units_t>(sprop));
	}
	
	
}
