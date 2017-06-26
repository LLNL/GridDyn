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

#include "parameterOperator.h"
#include "gridComponent.h"
#include "core/objectInterpreter.h"
#include "utilities/string_viewOps.h"
#include "core/objectInterpreter.h"

namespace griddyn
{
parameterOperator::parameterOperator() = default;
parameterOperator::parameterOperator(gridComponent *target, const std::string &field) :m_field(field), comp(target)
{
	checkField();
}

void parameterOperator::setTarget(gridComponent *target, const std::string &field)
{
	if (!field.empty())
	{
		m_field = field;
	}
	if (target != nullptr)
	{
		comp = target;
	}
	checkField();
}

void parameterOperator::updateObject(coreObject *gco, object_update_mode mode)
{
	if (mode == object_update_mode::direct)
	{
		auto nobj = dynamic_cast<gridComponent *>(gco);
		if (nobj!=nullptr)
		{
			comp = nobj;
		}
	}
	else
	{
		auto newTarget = dynamic_cast<gridComponent*>(findMatchingObject(comp, gco));
		if (newTarget != nullptr)
		{
			setTarget(newTarget);
		}
		else
		{
			throw(objectUpdateFailException());
		}
	}
}

void parameterOperator::setParameter(double val)
{
	if (parameterIndex == kNullLocation)
	{
		
		return comp->set(m_field, val);
	}
	comp->setParameter(parameterIndex, val);
}

double parameterOperator::getParameter() const
{
	if (parameterIndex == kNullLocation)
	{
		return comp->get(m_field);
	}
	return comp->getParameter(parameterIndex);
	
}
coreObject * parameterOperator::getObject() const
{
	return comp;
}

void parameterOperator::getObjects(std::vector<coreObject *> &objects) const
{
	objects.push_back(comp);
}

void parameterOperator::checkField()
{

}

std::unique_ptr<parameterOperator> make_parameterOperator(const std::string &param, gridComponent *rootObject)
{
	using namespace utilities::string_viewOps;

	utilities::string_view paramS(param);
	auto renameloc = paramS.find(" as ");//spaces are important
									  //extract out a rename
	utilities::string_view rname;
	if (renameloc != std::string::npos)
	{
		rname = trim(paramS.substr(renameloc + 4));
		paramS = paramS.substr(0, renameloc);
	}
	objInfo objI(paramS.to_string(), rootObject);

		auto pop = std::make_unique<parameterOperator>(dynamic_cast<gridComponent *>(objI.m_obj), objI.m_field);
		if (!rname.empty())
		{
			pop->setName(rname.to_string());
		}
		return pop;
	
}

	index_t parameterSet::add(const std::string &paramString, gridComponent *rootObject)
	{
		params.push_back(make_parameterOperator(paramString,rootObject));
		return static_cast<index_t>(params.size()) - 1;
	}
	parameterOperator *parameterSet::operator[](index_t index)
	{
		return params[index].get();
	}

	}//namespace griddyn
