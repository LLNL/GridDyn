/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "CymeDistLoad.h"

#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "json/jsoncpp.h"
#include "utilities/stringOps.h"
#include <fstream>
#include <iostream>

namespace griddyn
{
namespace fmi
{
std::atomic<int> CymeDistLoadME::indexCounter{0};

CymeDistLoadME::CymeDistLoadME (const std::string &objName) : fmiMELoad3phase (objName)
{
    opFlags.set (current_output);
    configIndex = indexCounter++;
}

coreObject *CymeDistLoadME::clone (coreObject *obj) const { return nullptr; }

void CymeDistLoadME::set (const std::string &param, const std::string &val)
{
    if ((param == "config") || (param == "configfile") || (param == "configuration_file"))
    {
        printf ("loading config file %s\n", val.c_str ());
        loadConfigFile (val);
    }
    else
    {
        printf ("setting parameter %s to %s\n", param.c_str (), val.c_str ());
        fmiMELoad3phase::set (param, val);
    }
}

void CymeDistLoadME::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "configindex") || (param == "config"))
    {
        configIndex = static_cast<int> (val);
    }
    else
    {
        fmiMELoad3phase::set (param, val, unitType);
    }
}

void CymeDistLoadME::loadConfigFile (const std::string &configFileName)
{
    std::ifstream file (configFileName);
    if (!file.is_open ())
    {
        std::cerr << "unable to open the file:" << configFileName << std::endl;
        LOG_WARNING (std::string ("unable to open the configuration file ") + configFileName);
        return;
    }
    Json_gd::Value doc;

    Json_gd::CharReaderBuilder rbuilder;
    std::string errs;
    bool ok = Json_gd::parseFromStream (rbuilder, file, &doc, &errs);
    if (!ok)
    {
        fprintf (stderr, "unable to parse json file %s\n", errs.c_str ());
        return;
    }
    configFile = configFileName;
    auto mval = doc["models"];

    auto model = mval[configIndex];
    if (model.isObject ())
    {
        auto fmu_path = model["fmu_path"].asString ();
        fprintf (stderr, "setting fmu_path to %s\n", fmu_path.c_str ());
        fmiMELoad3phase::set ("fmu", fmu_path);
        LOG_DEBUG (std::string ("setting fmu to ") + model["fmu_path"].asString ());

        if (model.isMember ("fmu_config_path"))
        {
            auto config_path = model["fmu_config_path"].asString ();
            fprintf (stderr, "fmu config_path=%s\n", config_path.c_str ());
            if (config_path.size () > 5)
            {
                fmiMELoad3phase::set ("_configurationFileName", config_path);
                LOG_DEBUG (std::string ("setting config file to ") + config_path);
            }
        }
    }
}
}  // namespace fmi
}  // namespace griddyn
