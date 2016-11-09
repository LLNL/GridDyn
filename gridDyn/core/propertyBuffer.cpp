
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

#include "propertyBuffer.h"

propertyBuffer::propertyBuffer()
{

}


propertyBuffer::propertyBuffer(const propertyBuffer &buf):stringProperties(buf.stringProperties),
doubleProperties(buf.doubleProperties),intProperties(buf.intProperties),flagProperties(buf.flagProperties)
{

}

propertyBuffer::propertyBuffer(propertyBuffer &&buf) : stringProperties(std::move(buf.stringProperties)),
doubleProperties(std::move(buf.doubleProperties)), intProperties(std::move(buf.intProperties)), flagProperties(std::move(buf.flagProperties))
{

}

propertyBuffer &propertyBuffer::operator=(const propertyBuffer &buf)
{
	stringProperties = buf.stringProperties;
	doubleProperties = buf.doubleProperties;
	intProperties = buf.intProperties;
	flagProperties = buf.flagProperties;
	return *this;
}

propertyBuffer &propertyBuffer::operator=(propertyBuffer &&buf)
{
	stringProperties=std::move(buf.stringProperties);
	doubleProperties = std::move(buf.doubleProperties);
	intProperties = std::move(buf.intProperties);
	flagProperties = std::move(buf.flagProperties);
	return *this;
}

void propertyBuffer::set(const std::string &param, const std::string &val)
{
	stringProperties.emplace_back(param, val);
}
void propertyBuffer::set(const std::string &param, double val)
{
	doubleProperties.emplace_back(param, val);
}
void propertyBuffer::set(const std::string &param, int val)
{
	intProperties.emplace_back(param, val);
}
void propertyBuffer::setFlag(const std::string &param, bool val)
{
	flagProperties.emplace_back(param, val);
}

void propertyBuffer::clear()
{
	stringProperties.clear();
	doubleProperties.clear();
	intProperties.clear();
	flagProperties.clear();
}