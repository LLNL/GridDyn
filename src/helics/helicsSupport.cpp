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

#include "helicsSupport.h"

#include <sstream>
#include <regex>
#include "stringConversion.h"
#include "utilities/mapOps.hpp"

namespace griddyn
{
namespace helicsLib
{
helics::Time gd2helicsTime(coreTime evntTime)
{
	return helics::Time(evntTime.toCount(timeUnits::ns),timeUnits::ns);
}

coreTime helics2gdTime(helics::Time ftime)
{
	return coreTime(ftime.toCount(timeUnits::ns), timeUnits::ns);
}




std::string helicsComplexString(double real, double imag)
{
	std::stringstream ss;
	ss << real;
	if (imag != 0.0)
	{
		if (imag >= 0.0)
		{
			ss << '+' << imag;
		}
		else
		{
			ss << imag;
		}
		ss << 'j';
	}
	return ss.str();
}

std::string helicsComplexString(std::complex<double> val)
{
	return helicsComplexString(val.real(), val.imag());
}

const std::regex creg("([+-]?(\\d+(\\.\\d+)?|\\.\\d+)([eE][+-]?\\d+)?)\\s*([+-]\\s*(\\d+(\\.\\d+)?|\\.\\d+)([eE][+-]?\\d+)?)[ji]*");

std::complex<double> helicsGetComplex(const std::string &val)
{
	
	if (val.empty())
	{
		return std::complex<double>(kNullVal, kNullVal);
	}
	std::smatch m;
	double re = 0.0;
	double im = 0.0;
	std::regex_search(val, m, creg);
	if (m.size() == 9)
	{
		re = numeric_conversionComplete(m[1], kNullVal);
		im = numeric_conversionComplete(m[5], kNullVal);

	}
	else
	{
		if ((val.back() == 'j') || (val.back() == 'i'))
		{
			im = numeric_conversionComplete(val.substr(0,val.size()-1), kNullVal);

		}
		else
		{
			re = numeric_conversionComplete(val, kNullVal);
		}
	}
	return std::complex<double>(re, im);
}



std::future<int> runBroker(const std::string &cmd_args)
{
	std::string broker_exe = std::string(HELICS_BROKER_EXECUTABLE)+" "+cmd_args;
	auto v = std::async(std::launch::async, [=]() { return system(broker_exe.c_str()); });
	return v;
}

std::future<int> runPlayer(const std::string &cmd_args)
{
	std::string player_exe = std::string(HELICS_PLAYER_EXECUTABLE) + " " + cmd_args;
	auto v = std::async(std::launch::async, [=]() { return system(player_exe.c_str()); });
	return v;
}

std::future<int> runRecorder(const std::string &cmd_args)
{
	std::string recorder_exe = std::string(HELICS_RECORDER_EXECUTABLE) + " " + cmd_args;
	auto v = std::async(std::launch::async, [=]() { return system(recorder_exe.c_str()); });
	return v;
}


std::string to_string(helicsValueType dtype)
{
	switch (dtype)
	{
	case helicsValueType::helicsDouble:
	default:
		return "double";
	case helicsValueType::helicsComplex:
		return "complex";
	case helicsValueType::helicsInteger:
		return "int64";
	case helicsValueType::helicsString:
		return "string";
	case helicsValueType::helicsVector:
		return "vector_double";

	}
}

const std::map<std::string, helicsValueType> valueMap{
	{"double",helicsValueType::helicsDouble },
	{"float64",helicsValueType::helicsDouble },
	{ "float",helicsValueType::helicsDouble },
	{ "int64",helicsValueType::helicsInteger },
	{ "int",helicsValueType::helicsInteger },
	{ "integer",helicsValueType::helicsInteger },
	{ "string",helicsValueType::helicsString },
	{ "vector",helicsValueType::helicsVector },
	{ "vector_double",helicsValueType::helicsVector },
	{ "complex",helicsValueType::helicsComplex },
	{ "complex_double",helicsValueType::helicsComplex }
};

helicsValueType helicsValueTypeFromString(const std::string &typeString)
{
	return mapFind(valueMap, typeString, helicsValueType::unknown);
}

bool isValidHelicsValueTypeString(const std::string &typeString)
{
	return (valueMap.find(typeString) != valueMap.end());
}

}// namespace helicsLib
} // namespace griddyn