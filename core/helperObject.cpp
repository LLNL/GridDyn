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

#include "helperObject.h"
#include "coreExceptions.h" 
#include "dataDictionary.h"
#include "stringOps.h"

//start at 100 since there are some objects that use low numbers as a check for interface number and the id as secondary
std::atomic<std::uint64_t> helperObject::s_obcnt(101);

	helperObject::helperObject():m_oid(s_obcnt++)
	{

	}

	
	helperObject::~helperObject() = default;

	helperObject::helperObject(const std::string &objectName) :m_oid(s_obcnt++),um_name(objectName)
	{

	}

	static dataDictionary<std::uint64_t, std::string> descriptionDictionary;

	void helperObject::set(const std::string &param, const std::string &val)
	{
		if ((param == "name")||(param=="id"))
		{
			setName(val);
		}
		else if (param == "description")
		{

		}
		else if ((param == "flags") || (param == "flag"))
		{
			setMultipleFlags(this, val);
		}
		else
		{
			throw(unrecognizedParameter());
		}
	}

	void helperObject::set(const std::string &param, double val)
	{
		setFlag(param, (val > 0.1));
	}

	void helperObject::setDescription(const std::string &description)
	{
		descriptionDictionary.update(m_oid, description);
	}

	std::string helperObject::getDescription() const
	{
		return descriptionDictionary.query(m_oid);
	}

	void helperObject::setFlag(const std::string & /*flag*/, bool /*val*/)
	{
		throw(unrecognizedParameter());
	}
	bool helperObject::getFlag(const std::string &/*flag*/) const
	{
		throw(unrecognizedParameter());
	}
	double helperObject::get(const std::string &param) const
	{
		return getFlag(param) ? 1.0 : 0.0;
	}
	
	void helperObject::nameUpdate()
	{
	}

	void setMultipleFlags(helperObject *obj, const std::string &flags)
	{
		auto flgs = stringOps::splitline(convertToLowerCase(flags));
		stringOps::trim(flgs);
		for (const auto &flag : flgs)
		{
			obj->setFlag(flag, true);
		}
	}


