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

#ifndef _FMI_LIBRARY_MANAGER_H_
#define _FMI_LIBRARY_MANAGER_H_

#include <map>
#include <memory>
#include <mutex>

class fmiLibrary;
class fmi2ModelExchangeObject;
class fmi2CoSimObject;

/** singleton class for managing fmi library objects*/
class fmiLibraryManager
{
private:
	std::map<std::string, std::shared_ptr<fmiLibrary>> libraries;
	std::map<std::string, std::string> quickReferenceLibraries;
	mutable std::mutex libraryLock;
public:

	~fmiLibraryManager();
	std::shared_ptr<fmiLibrary> getLibrary(const std::string &libFile);
	std::unique_ptr<fmi2ModelExchangeObject> createModelExchangeObject(const std::string &fmuIdentifier, const std::string &ObjectName);
	std::unique_ptr<fmi2CoSimObject> createCoSimulationObject(const std::string &fmuIdentifier, const std::string &ObjectName);
	void loadBookMarkFile(const std::string &bookmarksFile);
	void addShortCut(const std::string &name, const std::string &fmuLocation);
	static fmiLibraryManager& instance();

private:
	fmiLibraryManager();

};

#endif
