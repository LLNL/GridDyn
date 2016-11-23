/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
   * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
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
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "GhostSwingBusManager.h"
#include "gridDynRunner.h"

#include "libraryLoader.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <chrono>
#include <cstdio>
#include <memory>
#include <iostream>
#include "fncsSupport.h"
#include "fncsLibrary.h"
#include "test/fncsTest.h"


//using namespace boost;
namespace po = boost::program_options;


// main
int main (int argc, char *argv[])
{

	loadLibraries();
	loadFNCSLibrary();

  std::shared_ptr<gridDynSimulation> gds = std::make_shared<gridDynSimulation> ();

  GhostSwingBusManager::Initialize(&argc, &argv);
  if (!gds)
    {
      return (-5);  //TODO:: PT make this something meaningful
    }

  po::variables_map fncsOptions;

  po::options_description fncsOp("fncs options");
  fncsOp.add_options()
	  ("test", "test fncs program")
	  ("broker", po::value < std::string > (), "specify the broker address")
	  ("period",po::value<double>(),"specify the synchronization period");

  po::store(po::parse_command_line(argc, argv, fncsOp), fncsOptions);
  po::notify(fncsOptions);

  if (fncsOptions.count("test")) 
  {
	if (runFNCStests())
	{
		std::cout << "FNCS tests passed\n";
	}
	else
	{
		std::cout << "FNCS tests failed\n";
	}
	return 0;
  }

  po::variables_map vm;
  int ret = argumentParser (argc, argv, vm);
  if (ret)
    {
      return ret;
    }


  //create the simulation

  readerInfo ri;
  //load any relevant issue into the readerInfo structure
  loadXMLinfo (vm, &ri);
  if ((ret = processCommandArguments (gds, &ri, vm)) != 0)
    {
      return ret;
    }
  
  gds->log (nullptr, print_level::summary, griddyn_version_string);
  

  fncs::time time_granted = 0; /* the time step FNCS has allowed us to process */
  fncs::time time_desired = 0; /* the time step we would like to go to next */
 
  zplInfo info;
  info.name = gds->getName();
  if (fncsOptions.count("broker"))
  {
	  info.brokerAddress = fncsOptions["broker"].as<std::string>();
  }
  else
  {
	  info.brokerAddress = "tcp://localhost:5570";
  }
  info.minTimeStep = 1;
  info.minTimeStepUnits = "us";

  fncsRegister::instance()->makeZPLfile("gridDyn.zpl", info);

  fncs::initialize("gridDyn.zpl");

  /* unless FNCS detects another simulator terminates early, the
  * this simulator will run from time step 0 to time step 9 */

  gridDyn_time stop_time = gds->get("stoptime");


  gds->dynInitialize(0);
  while (gds->getCurrentTime() < stop_time)
  {
	  auto evntTime = gds->getEventTime();
	  time_desired = gd2fncsTime(evntTime);
	  gds->run();

	  time_granted = fncs::time_request(time_desired);
  }

      GhostSwingBusManager::Instance ()->endSimulation ();
  return 0;
}

