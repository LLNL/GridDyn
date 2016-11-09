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

#include "gridCondition.h"
#include "grabberInterpreter.hpp"
#include <map>

std::shared_ptr<gridCondition> make_condition(const std::string &condString, gridCoreObject *rootObject)
{
	auto cString = xmlCharacterCodeReplace(condString);
	//size_t posA = condString.find_first_of("&|");
	//TODO:PT: deal with parenthesis and &| conditions

	
	size_t pos = cString.find_first_of("><=!");
	if (pos == std::string::npos)
	{
		return nullptr;
	}

	
	char A = cString[pos];
	char B = cString[pos + 1];
	std::string BlockA = trim(cString.substr(0, pos - 1));
	std::string BlockB = (B == '=') ? cString.substr(pos + 2) : cString.substr(pos + 1);
		
	trimString(BlockB);
	auto gc = std::make_shared<gridCondition>();

	//get the state grabbers part
	auto stGrabber = makeStateGrabbers(BlockA, rootObject);
	auto grabber = makeGrabbers(BlockA, rootObject);
	gc->setConditionLHS(grabber[0], stGrabber[0]);

	stGrabber = makeStateGrabbers(BlockB, rootObject);
	grabber = makeGrabbers(BlockB, rootObject);
	gc->setConditionRHS(grabber[0], stGrabber[0]);
	
	std::string condstr;
	condstr.push_back(A);
	if (B == '=')
	{
		condstr.push_back(B);
	}

	gc->setComparison(comparisonFromString(condstr));

	return gc;
}

static const std::map<std::string, comparison_type> compStrMap
{
	{">", comparison_type::gt},{ "gt", comparison_type::gt },
	{ ">=", comparison_type::ge },{ "ge", comparison_type::ge },
	{ "<", comparison_type::lt },{ "lt", comparison_type::lt },
	{ "<=", comparison_type::le },{ "le", comparison_type::le },
	{ "=", comparison_type::eq },{ "eq", comparison_type::eq },
	{ "==", comparison_type::eq },
	{ "!=", comparison_type::ne },{ "ne", comparison_type::ne },
	{ "~=", comparison_type::ne },{ "<>",comparison_type::ne },
	{ "===", comparison_type::eq },
	{"??",comparison_type::null},
};

comparison_type comparisonFromString(const std::string &compStr)
{
	auto cfind = compStrMap.find(compStr);
	if (cfind != compStrMap.end())
	{
		return cfind->second;
	}
	else
	{
		return comparison_type::null;
	}
	
}

std::string fromString(comparison_type comp)
{
	switch (comp)
	{
	case comparison_type::gt:
		return ">";
	case comparison_type::ge:
		return ">=";
	case comparison_type::lt:
		return "<";
	case comparison_type::le:
		return "<=";
	case comparison_type::eq:
		return "==";
	case comparison_type::ne:
		return "!=";
	case comparison_type::null:
		return "??";
	default:
		return "??";
	}
}

std::shared_ptr<gridCondition> make_condition(const std::string &field, const std::string &compare, double level, gridCoreObject *rootObject)
{
	//get the state grabbers part
	
	auto stGrabber = makeStateGrabbers(field, rootObject);
	
	if (stGrabber.empty())
	{
		rootObject->log(rootObject, GD_WARNING_PRINT, "unable to generate state grabber from " + field + '\n');
		return nullptr;
	}
	
	//get the regular grabbers parts
	auto grabber = makeGrabbers(field, rootObject);
	if (grabber.empty())
	{
		rootObject->log(rootObject, GD_WARNING_PRINT, "unable to generate grabber from " + field + '\n');
		return nullptr;
	}
	auto gc = std::make_shared<gridCondition>(grabber[0], stGrabber[0]);
	gc->setConditionRHS(level);

	gc->setComparison(comparisonFromString(compare));
	
	return gc;
}

gridCondition::gridCondition(std::shared_ptr<gridGrabber> valGrabber, std::shared_ptr<stateGrabber> valGrabberSt):conditionA(valGrabber),conditionAst(valGrabberSt)
{

}


std::shared_ptr<gridCondition> gridCondition::clone(std::shared_ptr<gridCondition> gc) const
{
	auto ngc = gc;
	if (!ngc)
	{
		ngc = std::make_shared<gridCondition>();
	}
	ngc->m_constant = m_constant;
	ngc->m_margin = m_margin;
	ngc->m_constB = m_constB;
	ngc->m_curr_margin = m_curr_margin;
	ngc->use_margin = use_margin;

	if (conditionA)
	{
		ngc->conditionA = conditionA->clone(ngc->conditionA);
	}

	if (conditionAst)
	{
		ngc->conditionAst = conditionAst->clone(ngc->conditionAst);
	}
	if (!m_constB)
	{
		if (conditionB)
		{
			ngc->conditionB = conditionB->clone(ngc->conditionB);
		}

		if (conditionBst)
		{
			ngc->conditionBst = conditionBst->clone(ngc->conditionBst);
		}
	}
	return ngc;
}

void gridCondition::setConditionLHS(std::shared_ptr<gridGrabber> valGrabber, std::shared_ptr<stateGrabber> valGrabberSt)
{
	if (valGrabber)
	{
		conditionA = valGrabber;
	}
	if (valGrabberSt)
	{
		conditionAst = valGrabberSt;
	}
	
}

void gridCondition::setConditionRHS(std::shared_ptr<gridGrabber> valGrabber, std::shared_ptr<stateGrabber> valGrabberSt)
{
	if (valGrabber)
	{
		conditionB = valGrabber;
		m_constB = false;
	}
	if (valGrabberSt)
	{
		conditionBst = valGrabberSt;
	}
}

void gridCondition::updateObject(gridCoreObject *obj, object_update_mode mode)
{
	//TODO:: This function could potentially leave stuff in an unstable state
	//Practically speaking it is very likely that if condition A update doesn't throw an error the others won't either and in that codition
	//everything is still good.  Not sure how to do a rollback yet  (need to think)
	if (conditionA)
	{
		conditionA->updateObject(obj, mode);
	}
	if (conditionAst)
	{
		conditionAst->updateObject(obj, mode);
	}
	if (conditionB)
	{
		conditionB->updateObject(obj,mode);
	}
	if (conditionBst)
	{
		conditionBst->updateObject(obj,mode);
	}
}
void gridCondition::setComparison(const std::string &compStr)
{
	setComparison(comparisonFromString(compStr));
}

void gridCondition::setComparison(comparison_type ct)
{
	comp = ct;
	switch (comp)
	{
	case comparison_type::gt: case comparison_type::ge:
		evalf = [](double p1, double p2,double margin){return p2 - p1-margin; };
		break;
	case comparison_type::lt: case comparison_type::le:
		evalf = [](double p1, double p2,double margin){return p1 - p2+margin; };
		break;
	case comparison_type::eq:
		evalf = [](double p1, double p2,double margin){return std::abs(p1 - p2)-margin; };
		break;
	case comparison_type::ne:
		evalf = [](double p1, double p2,double margin){return -std::abs(p1 - p2)+margin; };
		break;
	default:
		evalf = [](double, double, double) {return kNullVal; };
		break;
	}
}

double gridCondition::evalCondition()
{
	double v1 = conditionA->grabData();
	double v2 = (m_constB) ? m_constant : conditionB->grabData();
	
	return evalf(v1,v2,m_curr_margin);
}

double gridCondition::evalCondition(const stateData *sD, const solverMode &sMode)
{
	double v1 = conditionAst->grabData(sD, sMode);
	double v2 = (m_constB)?m_constant:conditionBst->grabData(sD, sMode);
	return evalf(v1, v2,m_curr_margin);

}

double gridCondition::getVal(int side) const
{
  double v = (side==2)?((m_constB) ? m_constant : conditionB->grabData()): conditionA->grabData();
return v;
}

double gridCondition::getVal(int side,const stateData *sD, const solverMode &sMode) const
{
  double v = (side == 2) ? ((m_constB) ? m_constant : conditionBst->grabData(sD, sMode)) : conditionAst->grabData(sD, sMode);
  return v;
}

bool gridCondition::checkCondition() const
{
	double v1 = conditionA->grabData();
	double v2 = (m_constB) ? m_constant : conditionB->grabData();
	double ret= evalf(v1, v2,m_curr_margin);
	if ((comp == comparison_type::ge) || (comp == comparison_type::le) || (comp == comparison_type::eq))
	{
		return (ret <= 0);
	}
	else
	{
		return (ret < 0);
	}

}

bool gridCondition::checkCondition(const stateData *sD, const solverMode &sMode) const
{
	if (!conditionAst)
	{
		return checkCondition();
	}
	double v1 = conditionAst->grabData(sD, sMode);
	double v2 = (m_constB) ? m_constant : conditionBst->grabData(sD, sMode);

	double ret= evalf(v1, v2,m_curr_margin);
	if ((comp == comparison_type::ge) || (comp == comparison_type::le) || (comp == comparison_type::eq))
	{
		return (ret <= 0);
	}
	else
	{
		return (ret < 0);

	}

}

void gridCondition::setMargin(double val)
{
  m_margin = val;
  if (use_margin)
  {
    m_curr_margin = m_margin;
  }
}

gridCoreObject * gridCondition::getObject() const
{
	if (conditionA)
	{
		return conditionA->getObject();
	}
	else if (conditionAst)
	{
		return conditionAst->getObject();
	}
	return nullptr;
}

void gridCondition::getObjects(std::vector<gridCoreObject *> &objects) const
{
	if (conditionA)
	{
		conditionA->getObjects(objects);
	}
	else if (conditionAst)
	{
		return conditionAst->getObjects(objects);
	}
	if (!m_constB)
	{
		if (conditionB)
		{
			conditionB->getObjects(objects);
		}
		else if (conditionBst)
		{
			return conditionBst->getObjects(objects);
		}
	}
	
}