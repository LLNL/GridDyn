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

#include "fmiLibraryManager.h"
#include "fmiImport.h"
#include "fmiObjects.h"

std::shared_ptr<fmiLibrary> fmiLibraryManager::getLibrary(const std::string &libFile)
{
	std::unique_lock<std::mutex> lock(libraryLock);
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
		lock.unlock();
		//this can be a big operation so free the lock while it is occurring
		auto newLib = std::make_shared<fmiLibrary>(libFile);
		lock.lock();
		libraries.emplace(fmilib, newLib);
		return newLib;
	}
}

std::unique_ptr<fmi2ModelExchangeObject> fmiLibraryManager::createModelExchangeObject(const std::string &fmuIdentifier, const std::string &ObjectName)
{
	auto Lib = getLibrary(fmuIdentifier);
	return Lib->createModelExchangeObject(ObjectName);
}

std::unique_ptr<fmi2CoSimObject> fmiLibraryManager::createCoSimulationObject(const std::string &fmuIdentifier, const std::string &ObjectName)
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
	std::lock_guard<std::mutex> lock(libraryLock);
	quickReferenceLibraries.emplace(name, fmuLocation);
}

fmiLibraryManager& fmiLibraryManager::instance()
{
	static fmiLibraryManager s_instance;
	return s_instance;
}


fmiLibraryManager::fmiLibraryManager() = default;

fmiLibraryManager::~fmiLibraryManager() = default;