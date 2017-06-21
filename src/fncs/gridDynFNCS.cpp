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

// libraries


// headers
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "coupling/GhostSwingBusManager.h"
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
#include <exception>


//using namespace boost;
namespace po = boost::program_options;


// main
int main (int argc, char *argv[])
{
	loadLibraries();
	loadFNCSLibrary();

  //
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
  GriddynRunner gdr;

  gdr.Initialize(argc, argv);
  

  fncs::time time_granted = 0; /* the time step FNCS has allowed us to process */
  fncs::time time_desired = 0; /* the time step we would like to go to next */
 
  zplInfo info;
  auto gds = gdr.getSim();
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

  auto configString=fncsRegister::instance()->makeZPLConfig(info);
  printf("%s\n", configString.c_str());
  fncs::initialize(configString);

  gdr.simInitialize();
  coreTime stop_time = gds->get("stoptime");


  while (gds->getCurrentTime() < stop_time)
  {
	  auto evntTime = gdr.getNextEvent();
	  time_desired = gd2fncsTime(evntTime);
	  printf("eventTime=%f\n", static_cast<double>(evntTime));

	  time_granted = fncs::time_request(time_desired);
	  printf("grantTime=%llu\n", static_cast<unsigned long long>(time_granted));
	  //check if the granted time is too small to do anything about
	  if (time_granted < time_desired)
	  {
		  if (fncs2gdTime(time_granted) - gds->getCurrentTime() < 0.00001)
		  {
			  continue;
		  }
	  }
	  
	  try
	  {
		gdr.Step(fncs2gdTime(time_granted));
	  }
	  catch (const std::runtime_error &re)
	  {
		  printf("execution error in GridDyn: %s\n", re.what());
		  fncs::die();
		  gdr.Finalize();
		  return (-1);
	  }
  }
  fncs::finalize();
  gdr.Finalize();
  return 0;
}

