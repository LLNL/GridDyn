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


#include "gridDynFileInput.h"
#include <boost/filesystem.hpp>
#include "utilities/zipUtilities.h"

using namespace boost::filesystem;

void loadGDZ(coreObject *parentObject, const std::string &filename, readerInfo &ri)
{
	path fpath(filename);
	if (!exists(fpath))
	{
		return;
	}

	auto extractPath = temp_directory_path() / fpath.stem();
	auto keyFile = extractPath / "simulation.xml";
	if (!exists(keyFile))
	{
		auto ret = unzip(filename, extractPath.string());
		if (ret != 0)
		{
			return;
		}
	}
	
	
	if (!exists(keyFile))
	{
		return;
	}
	ri.addDirectory(extractPath.string());
	auto resourcePath = extractPath / "resources";
	if (exists(resourcePath))
	{
		ri.addDirectory(resourcePath.string());
	}
	loadFile(parentObject,keyFile.string(), &ri,"xml");
	
}