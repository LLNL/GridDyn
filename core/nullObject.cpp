
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
#include "nullObject.h"
#include <cassert>
/** @brief default constructor
takes a code used as the oid for special id codes below 100
*/
nullObject::nullObject(std::uint64_t nullCode):coreObject(nullCode)
{
	assert(nullCode <= 100);
	parent = this;
}

nullObject::nullObject(const std::string &objName):coreObject(objName)
{
	parent = this;
}


coreObject * nullObject::clone(coreObject *obj)
{
	if (obj)
	{
		return coreObject::clone(obj);
	}
	else if (id<100)
	{
		return coreObject::clone(new nullObject(id));
	}
	else
	{
		return coreObject::clone(new nullObject(getName()));
	}
}

void nullObject::alert(coreObject *, int /*code*/)
{

 }

void nullObject::log(coreObject *, print_level /*level*/, const std::string & /*message*/)
{

}

coreObject* nullObject::find(const std::string & /*object*/) const
{
	return nullptr;
}

coreObject * nullObject::findByUserID(const std::string & /*typeName*/, index_t /*searchID*/) const
{
	return nullptr;
}


void nullObject::setParent(coreObject * /*parentObj*/)
{
	//ignore it (null objects can't have parents)
}