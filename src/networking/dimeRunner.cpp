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

// libraries


// headers
#include "griddyn/gridDynSimulation.h"
#include "fileInput/fileInput.h"
#include "coupling/GhostSwingBusManager.h"
#include "dimeRunner.h"
#include "dimeInterface.h"


#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <chrono>
#include <cstdio>
#include <memory>
#include <iostream>

#include <exception>


namespace griddyn
{
namespace dimeLib
{
//using namespace boost;
namespace po = boost::program_options;


dimeRunner::dimeRunner()
{

	loadDimeLibrary();
	m_gds = std::make_shared<gridDynSimulation>();


}

dimeRunner::~dimeRunner() = default;

dimeRunner::dimeRunner(std::shared_ptr<gridDynSimulation> sim) : GriddynRunner(sim)
{
	loadDimeLibrary();
}

int dimeRunner::Initialize(int argc, char * argv[])
{
	po::variables_map dimeOptions;

	po::options_description dimeOp("dime options");
	// clang-format off
	dimeOp.add_options()
		("test", "test dime program")
		("broker", po::value < std::string >(), "specify the broker address")
		("period", po::value<double>(), "specify the synchronization period");

	// clang-format on
	auto parsed = po::command_line_parser(argc, argv).options(dimeOp).allow_unregistered().run();
	po::store(parsed, dimeOptions);
	po::notify(dimeOptions);

	if (dimeOptions.count("test"))
	{
		/*if (griddyn::helicsLib::runDimetests())
		{
			std::cout << "HELICS tests passed\n";
		}
		else
		{
			std::cout << "HELICS tests failed\n";
		}
		*/
		return 1;
	}
	readerInfo ri;
	loadDimeReaderInfoDefinitions(ri);

	int ret=GriddynRunner::Initialize(argc, argv, ri,true);
	if (ret != 0)
	{
		return ret;
	}
	m_gds->pFlowInitialize();
	//register with the helics broker
	return 0;
}


coreTime dimeRunner::Run()
{
	return GriddynRunner::Run();
}


coreTime dimeRunner::Step(coreTime time)
{
	auto retTime = GriddynRunner::Step(time);
	//coord->updateOutputs(retTime);
	return retTime;
}

void dimeRunner::Finalize()
{
	GriddynRunner::Finalize();
}

void dimeRunner::setInterfaceOptions(boost::program_options::variables_map & /*dimeOptions*/)
{

}
} //namespace dimeLib
} //namespace griddyn
