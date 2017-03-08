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

#include "gridParameter.h"
#include "utilities/stringConversion.h"
#include "gridDynDefinitions.h"
#include "core/coreExceptions.h"

gridParameter::gridParameter()
{
}

gridParameter::gridParameter(const std::string &str)
{
	fromString(str);
}

gridParameter::gridParameter(std::string fld, double val) : field(fld), value(val), valid(true)
{

}
gridParameter::gridParameter(std::string fld, std::string val) : field(fld), strVal(val), valid(true), stringType(true)
{

}

void gridParameter::reset()
{
	valid = false, stringType = false, paramUnits = gridUnits::defUnit;
	applyIndex.resize(0);
}

void gridParameter::fromString(const std::string &str)
{
	valid = false;
	size_t rlc = str.find_last_of('=');
	if (rlc == std::string::npos)
	{
		throw(invalidParameterValue());
	}
	valid = true;
	field = str.substr(0, rlc);
	stringOps::trimString(field);
	//now read the value
	strVal = str.substr(rlc + 1);
	value = numeric_conversionComplete(strVal, kNullVal);
	stringType = (value == kNullVal);

	rlc = field.find_first_of('(');
	if (rlc != std::string::npos)
	{
		size_t rlc2 = field.find_last_of(')');
		paramUnits = gridUnits::getUnits(field.substr(rlc + 1, rlc2 - rlc - 1));
		field = field.substr(0, rlc);
	}
}