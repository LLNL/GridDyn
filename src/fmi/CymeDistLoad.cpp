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


#include "CymeDistLoad.h"

#include "core/coreObjectTemplates.hpp"
#include "utilities/stringOps.h"
#include "core/coreExceptions.h"
#include "json/json.h"
#include <fstream>
namespace griddyn
{
namespace fmi
{
std::atomic<int> CymeDistLoadME::indexCounter{ 0 };

CymeDistLoadME::CymeDistLoadME(const std::string &objName): fmiMELoad3phase(objName)
{
	opFlags.set(current_output);
	configIndex = ++indexCounter;
}

coreObject * CymeDistLoadME::clone(coreObject *obj) const
{
	return nullptr;
}

void CymeDistLoadME::set(const std::string &param, const std::string &val)
{
	if ((param == "config") || (param == "configfile"))
	{
		loadConfigFile(val);
	}
	else
	{
		fmiMELoad3phase::set(param, val);
	}
}

void CymeDistLoadME::set(const std::string &param, double val, gridUnits::units_t unitType)
{
	if ((param == "configindex")||(param=="config"))
	{
		configIndex = static_cast<int>(val);
	}
	else
	{
		fmiMELoad3phase::set(param, val, unitType);
	}
}


void CymeDistLoadME::loadConfigFile(const std::string &configFileName)
{
	std::ifstream file(configFileName);
	if (!file.is_open())
	{
		return;
	}
	Json_gd::Value doc;

	Json_gd::CharReaderBuilder rbuilder;
	std::string errs;
	bool ok = Json_gd::parseFromStream(rbuilder, file, &doc, &errs);
	if (!ok)
	{
		return;
			
	}
	configFile = configFileName;
	auto mval = doc["models"];

	auto model = mval[configIndex];
	if (model.isObject())
	{
		fmiMELoad3phase::set("fmu", model["fmu_path"].asString());
		fmiMELoad3phase::set("_configuration_file", model["config_file_path"].asString());
	}
}
} //namespace fmi
} //namespace griddyn
