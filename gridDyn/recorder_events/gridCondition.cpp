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
	std::string BlockA = cString.substr(0, pos - 1);
	std::string BlockB = (B == '=') ? cString.substr(pos + 2) : cString.substr(pos + 1);
		
	trimString(BlockA);
	trimString(BlockB);
	auto gc = std::make_shared<gridCondition>();

	//get the state grabbers part
	auto v = makeStateGrabbers(BlockA, rootObject);
	gc->conditionAst = v[0];
	v = makeStateGrabbers(BlockB, rootObject);
	gc->conditionBst = v[0];
	//get the regular grabbers parts
	auto v2 = makeGrabbers(BlockA, rootObject);
	gc->conditionA = v2[0];
	v2 = makeGrabbers(BlockB, rootObject);
	gc->conditionB = v2[0];

	if (A == '>')
	{
		if (B == '=')
		{
			gc->setComparison(gridCondition::comparison_type::ge);
		}
		else
		{
			gc->setComparison(gridCondition::comparison_type::gt);
		}
		
	}
	else if (A == '<')
	{
		if (B == '=')
		{
			gc->setComparison(gridCondition::comparison_type::le);
		}
		else
		{
			gc->setComparison(gridCondition::comparison_type::lt);
		}
	}
	else if (A == '=')
	{
		gc->setComparison(gridCondition::comparison_type::eq);
	}
	else
	{
		gc->setComparison(gridCondition::comparison_type::ne);
	}

	return gc;
}


std::shared_ptr<gridCondition> make_condition(const std::string &field, const std::string &compare, double level, gridCoreObject *rootObject)
{
	
	
	

	//get the state grabbers part
	auto gc = std::make_shared<gridCondition>();
	auto v = makeStateGrabbers(field, rootObject);
	if (!v.empty())
	{
		gc->conditionAst = v[0];
	}
	else
	{
		rootObject->log(rootObject, GD_WARNING_PRINT, "unable to generate state grabber from " + field + '\n');
		return nullptr;
	}
	
	//get the regular grabbers parts
	auto v2 = makeGrabbers(field, rootObject);
	if (!v2.empty())
	{
		gc->conditionA = v2[0];
	}
	else
	{
		rootObject->log(rootObject, GD_WARNING_PRINT, "unable to generate grabber from " + field + '\n');
		return nullptr;
	}
	gc->setLevel(level);

	if (compare == ">")
	{
		gc->setComparison(gridCondition::comparison_type::gt);
	}
	else if (compare == ">=")
	{
		gc->setComparison(gridCondition::comparison_type::ge);
	}
	else if (compare == "<")
	{
		gc->setComparison(gridCondition::comparison_type::lt);
	}
	else if (compare == "<=")
	{
		gc->setComparison(gridCondition::comparison_type::le);
	}
	else if (compare == "==")
	{
		gc->setComparison(gridCondition::comparison_type::eq);
	}
	else if (compare == "!=")
	{
		gc->setComparison(gridCondition::comparison_type::ne);
	}

	return gc;
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
double v;
if (side==2)
{
  v = (m_constB) ? m_constant : conditionB->grabData();
}
else
{
  v = conditionA->grabData();
}
return v;
}

double gridCondition::getVal(int side,const stateData *sD, const solverMode &sMode) const
{
  double v;
  if (side == 2)
  {
    v = (m_constB) ? m_constant : conditionBst->grabData(sD, sMode);
  }
  else
  {
    v = conditionAst->grabData(sD, sMode);
  }
  return v;
}

bool gridCondition::checkCondition()
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

bool gridCondition::checkCondition(const stateData *sD, const solverMode &sMode)
{
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