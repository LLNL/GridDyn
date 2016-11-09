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

#include "fmiLibraryManager.h"
#include "fmiImport.h"

std::shared_ptr<fmiLibraryManager> fmiLibraryManager::s_instance;

std::shared_ptr<fmiLibrary> fmiLibraryManager::getLibrary(const std::string &libFile)
{
	auto fnd = quickReferenceLibraries.find(libFile);
	std::string fmilib;
	if (fnd != quickReferenceLibraries.end())
	{
		fmilib = fnd->second;
	}
	else
	{
		fmilib = libFile;
	}
	auto fndLib = libraries.find(fmilib);
	if (fndLib != libraries.end())
	{
		return fndLib->second;
	}
	else
	{
		auto newLib = std::make_shared<fmiLibrary>(libFile);
		libraries.emplace(fmilib, newLib);
		return newLib;
	}
}

std::shared_ptr<fmi2ME> fmiLibraryManager::createModelExchangeObject(const std::string &fmuIdentifier, std::string &ObjectName)
{
	auto Lib = getLibrary(fmuIdentifier);
	return Lib->createModelExchangeObject(ObjectName);
}

std::shared_ptr<fmi2CoSim> fmiLibraryManager::createCoSimulationObject(const std::string &fmuIdentifier, const std::string &ObjectName)
{
	auto Lib = getLibrary(fmuIdentifier);
	return Lib->createCoSimulationObject(ObjectName);
}

void fmiLibraryManager::loadBookMarkFile(const std::string & /*bookmarksFile*/)
{
	//TODO:: loading a bookmarks file
}

void fmiLibraryManager::addShortCut(const std::string &name, const std::string &fmuLocation)
{
	quickReferenceLibraries.emplace(name, fmuLocation);
}

std::shared_ptr<fmiLibraryManager> fmiLibraryManager::instance()
{
	if (!(s_instance))
	{
		s_instance = std::shared_ptr<fmiLibraryManager>(new fmiLibraryManager());
	}
	return s_instance;
}


fmiLibraryManager::fmiLibraryManager()
{

}