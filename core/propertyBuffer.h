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

#ifndef PROPERTY_BUFFER_H_
#define PROPERTY_BUFFER_H_

#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include "units.h"
class coreObject;
/** class for temporarily holding object properties if the object has delayed initialization or something to that effect
@details includes storage for string, double, integer, and binary properties,  targeted at coreObjects and helper objects
It should be able to handle setting via regular pointer or shared pointer only coreObjects are meant to make use of gridUnits*/
class propertyBuffer
{
private:
	std::vector<std::pair<std::string, std::string>> stringProperties; //!< storage for string properties
	std::vector<std::tuple<std::string, double,gridUnits::units_t>> doubleProperties; //!< storage for double properties
	std::vector<std::pair<std::string, int>> intProperties; //!< storage for integer properties
	std::vector<std::pair<std::string, bool>> flagProperties; //!< storage for binary properties
public:
	propertyBuffer();
	propertyBuffer(const propertyBuffer &buf);
	propertyBuffer(propertyBuffer &&buf);
	propertyBuffer &operator=(const propertyBuffer &buf);
	propertyBuffer &operator=(propertyBuffer &&buf);

	void set(const std::string &param, const std::string &val);
	void set(const std::string &param, double val);
	void set(const std::string &param, double val, gridUnits::units_t unitType);
	void set(const std::string &param, int val);
	void setFlag(const std::string &param, bool val=true);
	/** remove a property from the buffers
	@param[in] param the parameter to remove
	*/
	void remove(const std::string &param);
	/** apply the properties to a coreObject
	@details the properties are applied sequentially and the apply may 
	throw an exception from the underlying set function if the property is not valid
	those exceptions are not caught here and left up to the callers */
	void apply(coreObject *obj) const;
	/** the template is supposed to work for all different types of pointer objects
	regular pointers, shared_ptrs, or unique ptrs, it takes a reference to the pointer
	*/
	template <class X>
	void apply(X obj) const
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
			obj->set(std::get<std::string>(sprop), std::get<double>(sprop));
		}
		
		
	}

	void clear();

};


#endif