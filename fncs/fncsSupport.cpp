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

#include "fncsSupport.h"

#include <sstream>


#ifdef FULLCPP14

fncs::time gd2fncsTime(gridDyn_time evntTime)
{
	return static_cast<fncs::time>(evntTime*fncsTickPerSecond<double>);
}

gridDyn_time fncs2gdTime(fncs::time ftime)
{
	double val = static_cast<double>(ftime / fncsTickPerSecond<int>); //this gets the decimal should be integer division
	val += (static_cast<double>(ftime % fncsTickPerSecond<int>)/ fncsTickPerSecond<double>);
	return val;
}
#else
fncs::time gd2fncsTime(gridDyn_time evntTime)
{
	return static_cast<fncs::time>(static_cast<double>(evntTime)*fncsTickPerSecond_f);
}

gridDyn_time fncs2gdTime(fncs::time ftime)
{
	double val = static_cast<double>(ftime / fncsTickPerSecond_i); //this gets the decimal should be integer division
	val += (static_cast<double>(ftime % fncsTickPerSecond_i) / fncsTickPerSecond_i);
	return gridDyn_time(val);
}
#endif


void fncsRegister::registerSubscription(const std::string &sub, dataType dtype, const std::string &defVal, bool requestList)
{
	subscriptions.emplace_back(sub, dtype, defVal, requestList);
}

void fncsRegister::registerPublication(const std::string &pub, dataType dtype)
{
	publications.emplace_back(pub,dtype);
}

static const std::string indent("    ");  //4 spaces
static const std::string indent2("        "); //8 spaces
std::string fncsRegister::makeZPLConfig( const zplInfo &info)
{
	std::stringstream zpl;
	zpl << "name = " << info.name << '\n';
	zpl << "time_delta = " << info.minTimeStep << info.minTimeStepUnits << '\n';
	zpl << "broker = " << info.brokerAddress << '\n';
	zpl << "values\n";
	for (auto &sub : subscriptions)
	{
		zpl << indent << sub.topic << '\n';
		zpl << indent2 << "topic = " << sub.topic << '\n';
		zpl << indent2 << "default = " << sub.defValue << '\n';
		zpl << indent2 << "type = " << type2string(sub.type) << '\n';
		zpl << indent2 << "list = " << sub.list ? "true\n" : "false\n";
	}
	return zpl.str();
}

std::shared_ptr<fncsRegister> fncsRegister::p_instance;

std::shared_ptr<fncsRegister> fncsRegister::instance()
{
	if (!p_instance)
	{
		p_instance = std::shared_ptr<fncsRegister>(new fncsRegister());
	}
	return p_instance;
}

std::string fncsRegister::type2string(dataType dtype)
{
	switch (dtype)
	{
	case dataType::fncsDouble:
	default:
			return "double";
	case dataType::fncsComplex:
		return "complex";
	case dataType::fncsInteger:
		return "integer";
	case dataType::fncsString:
		return "string";
	case dataType::fncsJSON:
		return "json";
	case dataType::fncsArray:
		return "array";
		
	}
	return "";
}