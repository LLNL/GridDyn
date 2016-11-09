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


//using namespace boost;
namespace po = boost::program_options;


// main
int main (int argc, char *argv[])
{

	loadLibraries();
  std::shared_ptr<gridDynSimulation> gds = std::make_shared<gridDynSimulation> ();

  // Store the simulation pointer somewhere so that it can be accessed in other modules.
  gridDynSimulation::setInstance (gds.get ()); // peer to gridDynSimulation::GetInstance ();


  if (!gds)
    {
      return (-5);  //TODO:: PT make this something meaningful
    }

  bool isMpiCountMode = false;
  //check for an MPI run setup
  for (int ii = 0; ii < argc; ++ii)
    {
      if (!strcmp ("--mpicount", argv[ii]))
        {
          isMpiCountMode = true;
        }
    }

  if (!isMpiCountMode)
    {
      GhostSwingBusManager::Initialize (&argc, &argv);
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
  if (isMpiCountMode)
    {
      return 0;
    }
  gds->log (nullptr, GD_SUMMARY_PRINT, griddyn_version_string);
  auto start_t = std::chrono::high_resolution_clock::now ();

  gds->run ();

  auto stop_t = std::chrono::high_resolution_clock::now ();
  std::chrono::duration<double> elapsed_t = stop_t - start_t;
  std::cout << "\nSimulation " << gds->getName () << " executed in " << elapsed_t.count () << " seconds\n";
  int ssize, jsize;
  auto pState = gds->currentProcessState ();
  if (pState >= gridDynSimulation::gridState_t::DYNAMIC_COMPLETE)
    {
      ssize = gds->getInt ("dynstatesize");
      jsize = gds->getInt ("dynnonzeros");
      std::cout << "simulation final Dynamic statesize= " << ssize << ", " << jsize << " non zero elements in Jacobian\n";
    }
  else if (pState <= gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED)
    {
      ssize = gds->getInt ("pflowstatesize");
      jsize = gds->getInt ("pflownonzeros");
      std::cout << "simulation final Power flow statesize= " << ssize << ", " << jsize << " non zero elements in Jacobian\n";
    }



  if (!isMpiCountMode)
    {
      GhostSwingBusManager::Instance ()->endSimulation ();
    }


  return 0;
}

