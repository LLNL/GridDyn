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

#ifndef _FMI_LIBRARY_MANAGER_H_
#define _FMI_LIBRARY_MANAGER_H_

#include <map>
#include <memory>

class fmiLibrary;
class fmi2ME;
class fmi2CoSim;

/** singleton class for managing fmi library objects*/
class fmiLibraryManager
{
private:
	std::map<std::string, std::shared_ptr<fmiLibrary>> libraries;
	std::map<std::string, std::string> quickReferenceLibraries;
	static std::shared_ptr<fmiLibraryManager> s_instance;
public:
	std::shared_ptr<fmiLibrary> getLibrary(const std::string &libFile);
	std::shared_ptr<fmi2ME> createModelExchangeObject(const std::string &fmuIdentifier, std::string &ObjectName);
	std::shared_ptr<fmi2CoSim> createCoSimulationObject(const std::string &fmuIdentifier, const std::string &ObjectName);
	void loadBookMarkFile(const std::string &bookmarksFile);
	void addShortCut(const std::string &name, const std::string &fmuLocation);
	static std::shared_ptr<fmiLibraryManager> instance();

private:
	fmiLibraryManager();

};

#endif
