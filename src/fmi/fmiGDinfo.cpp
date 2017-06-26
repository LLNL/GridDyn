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

/*
Copyright (C) 2012 Modelon AB

This program is free software: you can redistribute it and/or modify
it under the terms of the BSD style license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
FMILIB_License.txt file for more details.

You should have received a copy of the FMILIB_License.txt file
along with this program. If not, contact Modelon AB <http://www.modelon.com>.
*/


#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS   
#include "plugins/gridDynPluginApi.h"

#include "fmi_importGD.h"

#include "core/objectFactoryTemplates.hpp"
#include "fmi_models/fmiLoad.h"
#include <memory>


//create the component factories



namespace griddyn
{
static typeFactory<fmi::fmiLoad> fmild("load", stringVec{ "fmiload", "fmi" });

void loadFmiLibrary()
{
	static int loaded = 0;

	if (loaded == 0)
	{
		loaded = 1;
	}
}

}//namespace griddyn

//Someday I will get plugins to work 
namespace fmi_plugin_namespace 
{
using namespace griddyn;
	class fmiPlugin : public gridDynPlugInApi 
	{
		static std::vector<std::shared_ptr<objectFactory>> fmiFactories;
		fmiPlugin() {};

	public:
		std::string name() const override {
			return "fmi";
		}

		void load() override {
			auto b = std::make_shared<childTypeFactory<fmi::fmiLoad, zipLoad>>("load", stringVec{ "fmiload", "fmi" });
			fmiFactories.push_back(b);

		}

		void load(const std::string & /*section*/) override
		{
			load();
		}
		// Factory method
		static std::shared_ptr<fmiPlugin> create() 
		{
			return std::shared_ptr<fmiPlugin>(new fmiPlugin());
		}
	};


	/*BOOST_DLL_ALIAS(
		fmi_plugin_namespace::fmiPlugin::create, // <-- this function is exported with...
		load_plugin                               // <-- ...this alias name
	)*/

}//namespace fmi_plugin_namespace 