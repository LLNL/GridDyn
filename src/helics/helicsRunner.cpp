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

// libraries

// headers
#include "helicsRunner.h"
#include "coupling/GhostSwingBusManager.h"
#include "fileInput/fileInput.h"
#include "griddyn/gridDynSimulation.h"

#include "gridDynCombined/libraryLoader.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "helics/helics.hpp"
#include "helicsCoordinator.h"
#include "helicsLibrary.h"
#include "helicsSupport.h"
#include "test/helicsTest.h"
#include <chrono>
#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>

namespace griddyn
{
namespace helicsLib
{
helicsRunner::helicsRunner ()
{
    griddyn::loadHELICSLibrary ();
    m_gds = std::make_shared<gridDynSimulation> ();

    coord_ = make_owningPtr<helicsCoordinator> ();
    // store the coordinator as a support object so everything can find it
    m_gds->add (coord_.get ());
}

helicsRunner::helicsRunner (std::shared_ptr<gridDynSimulation> sim) : GriddynRunner (sim)
{
    griddyn::loadHELICSLibrary ();
    coord_ = make_owningPtr<helicsCoordinator> ();
    // store the coordinator as a support object so everything can find it
    m_gds->add (coord_.get ());
}

helicsRunner::~helicsRunner() = default;

int helicsRunner::Initialize (int argc, char *argv[])
{
    // using namespace boost;
    namespace po = boost::program_options;
    po::variables_map helicsOptions;
    po::options_description helicsOp ("helics options");
    // clang-format off
	helicsOp.add_options()
		("test", "test helics program")
		("core_type", po::value < std::string >(), "specify the type of core to use (test,local, zmq, mpi)")
		("core_init", po::value < std::string >(), "specify an init string for the core")
		("core_name", po::value < std::string >(), "specify the type of core to use")
		("name", po::value < std::string >(), "specify the name of the federate")
		("broker", po::value < std::string >(), "specify the broker address")
		("help","display the help messages")
		("lookahead", po::value<double>(), "specify the lookahead time")
	    ("impactwindow", po::value<double>(), "specify the impact window time")
		("period", po::value<double>(), "specify the synchronization period");

	//clang-format on
    auto parsed =  po::command_line_parser(argc, argv).options(helicsOp).allow_unregistered().run();
	po::store(parsed, helicsOptions);
	po::notify(helicsOptions);

	if (helicsOptions.count("test"))
	{
		/*if (griddyn::helicsLib::runHELICStests())
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

	if (helicsOptions.count("help") > 0)
	{
		po::options_description visible("allowed options");
		visible.add(helicsOp);
		std::cout << visible << '\n';
	}

	readerInfo ri;
	loadHelicsReaderInfoDefinitions(ri);

	int ret=GriddynRunner::Initialize(argc, argv, ri,true);
	if (ret != 0)
	{
		return ret;
	}
	//command line options for the coordinator override any file input
	setCoordinatorOptions(helicsOptions);
	return 0;
}


void helicsRunner::simInitialize()
{
	//add an initialization function after the first part of the power flow init
    m_gds->addInitOperation([this]() {fed_ = coord_->RegisterAsFederate(); return ((fed_) ? 0 : -45); });
    int ret = 0;
    ret = m_gds->pFlowInitialize();
    if (ret!=0)
    {
        throw(executionFailure(m_gds.get(), "power flow initialize failure"));
    }

	GriddynRunner::simInitialize();  //TODO this will need to be unpacked for co-iteration on the power flow solution
    if (!fed_)
    {
        throw(executionFailure(m_gds.get(),"unable to initialize helics federate"));
    }
    fed_->enterExecutionState();
}

void helicsRunner::setCoordinatorOptions(boost::program_options::variables_map &helicsOptions)
{
	if (helicsOptions.count("core_type") > 0)
	{
		coord_->set("core_type", helicsOptions["core_type"].as<std::string>());
	}
	if (helicsOptions.count("core_init") > 0)
	{
		coord_->set("core_init", helicsOptions["core_init"].as<std::string>());
	}
	if (helicsOptions.count("core_name") > 0)
	{
		coord_->set("core_name", helicsOptions["core_name"].as<std::string>());
	}
	if (helicsOptions.count("name") > 0)
	{
		coord_->set("name", helicsOptions["name"].as<std::string>());
	}
	if (helicsOptions.count("broker") > 0)
	{
		coord_->set("broker", helicsOptions["broker"].as<std::string>());
	}
	if (helicsOptions.count("lookahead") > 0)
	{
		coord_->set("lookahead", helicsOptions["lookahead"].as<double>());
	}
	if (helicsOptions.count("impactwindow") > 0)
	{
		coord_->set("impactwindow", helicsOptions["impactwindow"].as<double>());
	}
	if (helicsOptions.count("period") > 0)
	{
		coord_->set("period", helicsOptions["period"].as<double>());
	}
	
}

coreTime helicsRunner::Run()
{
	coreTime stop_time = m_gds->getStopTime();
	auto retTime=Step(stop_time);
	fed_->finalize();
	return retTime;
}


coreTime helicsRunner::Step(coreTime time)
{
	helics::Time time_granted = 0.0; /* the time step HELICS has allowed us to process */
	helics::Time time_desired = 0.0; /* the time step we would like to go to next */

	while (m_gds->getSimulationTime() < time)
	{
		auto evntTime = m_gds->getEventTime();
		auto nextTime = std::min(evntTime, time);
		time_desired = gd2helicsTime(nextTime);
		//printf("nextTime=%f\n", static_cast<double>(nextTime));

		try
		{
			time_granted = fed_->requestTime(time_desired);
		}
		catch (...)
		{
			break;
		}
		//printf("grantTime=%llu\n", static_cast<unsigned long long>(time_granted));
		//check if the granted time is too small to do anything about
		if (time_granted < time_desired)
		{
			if (helics2gdTime(time_granted) - m_gds->getSimulationTime() < 0.00001)
			{
				continue;
			}
		}

		try
		{
			m_gds->run(helics2gdTime(time_granted));
		}
		catch (const std::runtime_error &re)
		{
			std::cerr << "execution error in GridDyn" << re.what() << '\n';
			fed_->error(m_gds->getErrorCode(), re.what());

			throw(re);
		}
	}
	return m_gds->getSimulationTime();
}

void helicsRunner::Finalize()
{
	fed_->finalize();
}
} //namespace helicsLib
} //namespace griddyn