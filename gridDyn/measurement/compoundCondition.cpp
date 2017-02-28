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

#include "gridCondition.h"
#include "grabberInterpreter.hpp"
	/*
	enum class compound_mode
	{
		c_and, c_or, c_any, c_xor, c_one_of, c_two_of, c_three_of
	};
	*/
	compoundCondition::compoundCondition()
	{

	}

	double compoundCondition::evalCondition()
	{
		return 0.0;
	}

	double compoundCondition::evalCondition(const stateData &, const solverMode &)
	{
		return 0.0;
	}

	bool compoundCondition::checkCondition() const
	{
		unsigned int tc = 0;
		for (auto &gc : conditions)
		{
			if (gc->checkCondition())
			{
				++tc;
				if (breakTrue)
				{
					break;
				}
			}
			else
			{
				if (breakFalse)
				{
					break;
				}
			}

		}
		return evalCombinations(tc);
	}

	bool compoundCondition::checkCondition(const stateData &sD, const solverMode &sMode) const
	{
		unsigned int tc = 0;
		for (auto &gc : conditions)
		{
			if (gc->checkCondition(sD,sMode))
			{
				++tc;
				if (breakTrue)
				{
					break;
				}
			}
			else
			{
				if (breakFalse)
				{
					break;
				}
			}

		}
		return evalCombinations(tc);
	}

	void compoundCondition::add(std::shared_ptr<gridCondition> gc)
	{
		if (!gc)
		{
			throw(addFailureException());
		}
	
		conditions.push_back(gc);
			
	}

	void compoundCondition::setMode(compound_mode newMode)
	{
		mode = newMode;
		switch (mode)
		{
		case compound_mode::c_and:
			breakTrue = false;
			breakFalse = true;
			break;
		case compound_mode::c_any:
		case compound_mode::c_or:
		case compound_mode::c_none:
			breakTrue = true;
			breakFalse = false;
			break;
		default:
			breakTrue = false;
			breakFalse = false;
			break;
		}
	}

	bool compoundCondition::evalCombinations(unsigned int trueCount) const
	{
		switch (mode)
		{
		case compound_mode::c_and:
		default:
			return (trueCount == static_cast<unsigned int>(conditions.size()));
		case compound_mode::c_any:
		case compound_mode::c_or:
			return (trueCount >0);
		case compound_mode::c_one_of:
			return (trueCount==1);
		case compound_mode::c_two_of:
			return (trueCount == 2);
		case compound_mode::c_three_of:
			return (trueCount == 3);
		case compound_mode::c_two_or_more:
			return (trueCount >=2);
		case compound_mode::c_three_or_more:
			return (trueCount >= 3);
		case compound_mode::c_xor:
			return ((trueCount&0x01)==1);
		case compound_mode::c_none:
			return (trueCount == 0);
		}
	}