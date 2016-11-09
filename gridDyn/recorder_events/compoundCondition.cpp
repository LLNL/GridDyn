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
		return false;
	}

	double compoundCondition::evalCondition(const stateData *, const solverMode &)
	{
		return 0.0;
	}

	bool compoundCondition::checkCondition() const
	{
		return false;
	}

	bool compoundCondition::checkCondition(const stateData *, const solverMode &) const
	{
		return false;
	}

	int compoundCondition::add(std::shared_ptr<gridCondition> gc)
	{
		if (gc)
		{
			conditions.push_back(gc);
			return OBJECT_ADD_SUCCESS;
		}
		return OBJECT_ADD_FAILURE;
	}
