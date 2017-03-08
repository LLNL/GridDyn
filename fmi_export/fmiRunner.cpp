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

#include "fmiRunner.h"
#include "fmi/FMI2/fmi2Functions.h"
#include "fmiCoordinator.h"
#include "gridDyn.h"
#include "gridDynFileInput/gridDynFileInput.h"
#include "boost/filesystem.hpp"
#include "loadFMIExportObjects.h"

fmiRunner::fmiRunner(const std::string &name, const std::string &resourceLocations, const fmi2CallbackFunctions* functions)
{
	m_gds = std::make_shared<gridDynSimulation>(name);

	coord = std::make_shared<fmiCoordinator>();
	// store the coordinator as a support object so everything can find it
	m_gds->addsp(coord);

	readerInfo ri;
	loadFmiExportReaderInfoDefinitions(ri);

	ri.addDirectory(resourceLocations);

	boost::filesystem::path mainFilePath = resourceLocations; 
	mainFilePath /= "simulation.xml";

	if (boost::filesystem::exists(mainFilePath))
	{
		loadFile(m_gds.get(), mainFilePath.string(), &ri, "xml");
	}
	else
	{
		throw(std::invalid_argument("unable to locate main file"));
	}
}

int fmiRunner::Initialize(int /*argc*/, char * /*argv*/[])
{
	return 0;  
}

void fmiRunner::UpdateOutputs()
{
	coord->updateOutputs(m_gds->getCurrentTime());
}

void fmiRunner::Run(void)
{
	GriddynRunner::Run();
}


coreTime fmiRunner::Step(coreTime time)
{
	auto retTime = GriddynRunner::Step(time);
	coord->updateOutputs(retTime);
	return retTime;
}

void fmiRunner::Finalize(void)
{
	GriddynRunner::Finalize();
}


std::uint64_t fmiRunner::GetID() const
{
	return m_gds->getID();
}


bool fmiRunner::Set(index_t vr, double val)
{
	return coord->sendInput(vr, val);
}


bool fmiRunner::SetString(index_t vr, const char *s)
{
	return coord->sendInput(vr, s);
}

double fmiRunner::Get(index_t vr)
{
	return coord->getOutput(vr);
}
