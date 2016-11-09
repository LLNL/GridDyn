/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2015, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "fmiInfo.h"
#include "stringOps.h"

fmi_variability_type_t variabilityFromString(const std::string &vstring)
{
	if (vstring == "continuous")
	{
		return fmi_variability_type_t::continuous;
	}
	else if (vstring == "constant")
	{
		return fmi_variability_type_t::constant;
	}
	else if (vstring == "fixed")
	{
		return fmi_variability_type_t::fixed;
	}
	else if (vstring == "tunable")
	{
		return fmi_variability_type_t::tunable;
	}
	else if (vstring == "discrete")
	{
		return fmi_variability_type_t::discrete;
	}
	else
	{
		return fmi_variability_type_t::unknown;
	}
}

std::string toString(fmi_variability_type_t variability)
{
	switch (variability)
	{
	case fmi_variability_type_t::continuous:
		return "continuous";
		break;
	case fmi_variability_type_t::fixed:
		return "fixed";
		break;
	case fmi_variability_type_t::constant:
		return "constant";
		break;
	case fmi_variability_type_t::discrete:
		return "discrete";
		break;
	case fmi_variability_type_t::tunable:
		return "tunable";
		break;
	default:
		return "unknown";
	}
}


fmi_causality_type_t causalityFromString(const std::string &vstring)
{
	if (vstring == "local")
	{
		return fmi_causality_type_t::local;
	}
	else if (vstring == "parameter")
	{
		return fmi_causality_type_t::parameter;
	}
	else if (vstring == "calculatedParameter")
	{
		return fmi_causality_type_t::calculatedParameter;
	}
	else if (vstring == "input")
	{
		return fmi_causality_type_t::input;
	}
	else if (vstring == "output")
	{
		return fmi_causality_type_t::output;
	}
	else if (vstring == "independent")
	{
		return fmi_causality_type_t::independent;
	}
	else if (vstring == "any")
	{
		return fmi_causality_type_t::any;
	}
	else
	{
		return fmi_causality_type_t::unknown;
	}
}

std::string toString(fmi_causality_type_t causality)
{
	switch (causality)
	{
	case fmi_causality_type_t::local:
		return "local";
		break;
	case fmi_causality_type_t::parameter:
		return "parameter";
		break;
	case fmi_causality_type_t::calculatedParameter:
		return "calculatedParameter";
		break;
	case fmi_causality_type_t::input:
		return "input";
		break;
	case fmi_causality_type_t::output:
		return "output";
		break;
	case fmi_causality_type_t::independent:
		return "independent";
		break;
	case fmi_causality_type_t::any:
		return "any";
		break;
	default:
		return "unknown";
		break;
	}
}


fmi_type_t typeFromString(const std::string &vstring)
{
	if (vstring == "real")
	{
		return fmi_type_t::real;
	}
	else if (vstring == "integer")
	{
		return fmi_type_t::integer;
	}
	else if (vstring == "boolean")
	{
		return fmi_type_t::boolean;
	}
	else if (vstring == "string")
	{
		return fmi_type_t::string;
	}
	else if (vstring == "enumeration")
	{
		return fmi_type_t::enumeration;
	}
	else
	{
		return fmi_type_t::unknown;
	}
}

std::string toString(fmi_type_t type)
{
	switch (type)
	{
	case fmi_type_t::real:
		return "real";
		break;
	case fmi_type_t::integer:
		return "integer";
		break;
	case fmi_type_t::boolean:
		return "boolean";
		break;
	case fmi_type_t::string:
		return "string";
		break;
	case fmi_type_t::enumeration:
		return "enumeration";
		break;
	default:
		return "unknown";
		break;
	}
}

fmi_dependencies_t dependenciesFromString(const std::string &vstring)
{
	if (vstring == "dependent")
	{
		return fmi_dependencies_t::dependent;
	}
	else if (vstring == "constant")
	{
		return fmi_dependencies_t::constant;
	}
	else if (vstring == "fixed")
	{
		return fmi_dependencies_t::fixed;
	}
	else if (vstring == "tunable")
	{
		return fmi_dependencies_t::tunable;
	}
	else if (vstring == "discrete")
	{
		return fmi_dependencies_t::discrete;
	}
	else if (vstring == "independent")
	{
		return fmi_dependencies_t::independent;
	}
	else
	{
		return fmi_dependencies_t::unknown;
	}
}

std::string toString(fmi_dependencies_t dependencies)
{
	switch (dependencies)
	{
	case fmi_dependencies_t::dependent:
		return "dependent";
	case fmi_dependencies_t::constant:
		return "constant";
	case fmi_dependencies_t::fixed:
		return "fixed";
	case fmi_dependencies_t::tunable:
		return "tunable";
	case fmi_dependencies_t::discrete:
		return "discrete";
	case fmi_dependencies_t::independent:
		return "independent";
	default:
		return "unknown";
	}
}