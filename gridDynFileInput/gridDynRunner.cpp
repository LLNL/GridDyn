/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * -----------------------------------------------------------------
   * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 * -----------------------------------------------------------------
 */


#include "gridDynRunner.h"

#include "gridDyn.h"

#include "coupling/GhostSwingBusManager.h"
#include "gridDynFileInput.h"
#include "core/objectInterpreter.h"
#include "simulation/gridDynSimulationFileOps.h"
#include "measurement/collector.h"
#include "utilities/stringOps.h"
#include "utilities/workQueue.h"
#include "core/coreExceptions.h"



#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <chrono>
#include <cstdio>
#include <memory>
#include <iostream>
#include <iomanip>



namespace po = boost::program_options;

using namespace stringOps;

GriddynRunner::GriddynRunner() = default;

GriddynRunner::~GriddynRunner() = default;

int GriddynRunner::Initialize (int argc, char *argv[])
{

  m_startTime = std::chrono::high_resolution_clock::now ();

  m_gds = std::make_shared<gridDynSimulation> ();

  if (!m_gds)
    {
      return (-5);
    }

  gridDynSimulation::setInstance (m_gds.get ()); // peer to gridDynSimulation::GetInstance ();

     GhostSwingBusManager::Initialize (&argc, &argv);


  po::variables_map vm;
  int ret = argumentParser (argc, argv, vm);
  if (ret)
    {
      return ret;
    }

  //create the simulation

  readerInfo ri;
  //load any relevant issue into the readerInfo structure
  loadXMLinfo (vm, ri);
  if ((ret = processCommandArguments (m_gds, ri, vm)) != 0)
    {
      return ret;
    }
  
  m_gds->log (nullptr, print_level::summary, griddyn_version_string);
  m_stopTime = std::chrono::high_resolution_clock::now ();
  std::chrono::duration<double> elapsed_t = m_stopTime - m_startTime;
  m_gds->log (m_gds.get (), print_level::normal,"\nInitialization " + m_gds->getName () + " executed in " + std::to_string (elapsed_t.count ()) + " seconds");

  

 
  return 0;
}

void GriddynRunner::simInitialize()
{
  m_startTime = std::chrono::high_resolution_clock::now ();
	m_gds->dynInitialize();
  if (!(m_gds->hasDynamics ()))
    {
      eventMode = true;
    }
}

void GriddynRunner::Run (void)
{
  m_gds->run ();
}

coreTime GriddynRunner::Step (coreTime time)
{
  coreTime actual = time;
  if (m_gds)
    {
      if (eventMode)
        {
          int retval = m_gds->eventDrivenPowerflow (time);
          actual = time;
          if (retval < FUNCTION_EXECUTION_SUCCESS)
            {
              std::string error = "GridDyn failed to advance retval = " + std::to_string (retval);
              throw(std::runtime_error (error));
            }
        }
      else
        {
          int retval = m_gds->step (time, actual);
          if (retval < FUNCTION_EXECUTION_SUCCESS)
            {
              std::string error = "GridDyn failed to advance retval = " + std::to_string (retval);
              throw(std::runtime_error (error));
            }
        }

    }

  return actual;
}

coreTime GriddynRunner::getNextEvent () const
{
  return m_gds->getEventTime ();
}

void GriddynRunner::StopRecording ()
{
  m_gds->log (m_gds.get (),print_level::normal,"Saving recorders...");
  m_gds->saveRecorders ();
  m_stopTime = std::chrono::high_resolution_clock::now ();
  std::chrono::duration<double> elapsed_t = m_stopTime - m_startTime;
  m_gds->log (m_gds.get (), print_level::normal,"\nSimulation " + m_gds->getName () + " executed in " + std::to_string (elapsed_t.count ()) + " seconds");
}

void GriddynRunner::Finalize (void)
{
  StopRecording ();

  if (!m_isMpiCountMode)
    {
      GhostSwingBusManager::Instance ()->endSimulation ();
    }
}

int processCommandArguments (std::shared_ptr<gridDynSimulation> &gds, readerInfo &ri, po::variables_map &vm)
{

  if (vm.count ("quiet"))
    {
      readerConfig::setPrintMode (0);
      gds->set ("printlevel", 0);
    }
  if (vm.count ("verbose"))
    {
      int temp = vm["verbose"].as<int> ();
      readerConfig::setPrintMode (temp);        //set the gridDynXML reader print mode
      //default is normal mode
    }

  if (vm.count ("warn"))
    {
      int temp = vm["warn"].as<int> ();
      readerConfig::setWarnMode (temp);        //set the gridDynXML reader warn mode
      //default is warn all
    }
  if (vm.count ("mpicount"))     //if we are in mpi mode don't prvoid anything
    {
      readerConfig::setPrintMode (READER_NO_PRINT);
      readerConfig::setWarnMode (READER_WARN_NONE);
    }

  //get the main input file
  if (vm.count ("log-file") > 0)
    {
      std::string log_file = vm["log-file"].as<std::string> ();
      ri.checkDefines (log_file);
      gds->set ("logfile", log_file);
    }

  if (vm.count ("file-flags"))
    {
      stringVec flagstrings = vm["file-flags"].as<stringVec > ();
      for (auto &str : flagstrings)
        {
          ri.flags = addflags (ri.flags, str);
        }
    }
	if (vm.count("threads"))
	{
		//initiate the work queue with the requested number of threads
		workQueue::instance(vm["threads"].as<int>());
	}
   std::string grid_file = vm["input"].as<std::string> ();

  loadFile (gds.get (), grid_file, &ri);
  if (vm.count ("import"))
    {
      auto importList = vm["import"].as<stringVec > ();
      for (auto &iF : importList)
        {
          loadFile (gds.get (), iF, &ri);
        }
    }

  if (gds->getErrorCode () != 0)
    {
      return gds->getErrorCode ();
    }

  if (vm.count ("mpicount"))
    {
      gds->countMpiObjects (true);
      return 0;
    }
  else if (vm.count("buildfmu"))
  {

  }
  else
    {

      int areas = gds->getInt ("totalareacount");
      int buses = gds->getInt ("totalbuscount");
      int links = gds->getInt ("totallinkcount");
      int gens = gds->getInt ("gencount");
      std::cout << "area count =" << areas << " buses=" << buses << " links= " << links << " gens= " << gens << '\n';
    }

  
  //set any flags used by the system
  if (vm.count ("flags"))
    {
      stringVec flagstrings = vm["flags"].as<stringVec > ();
      for (auto &str : flagstrings)
        {
          auto fstr = splitline(str);
		  trim(fstr);
          for (auto &flag : fstr)
            {
              makeLowerCase (flag);
			  try
			  {
				  gds->setFlag(flag, true);
			  }
              catch (const unrecognizedParameter &)
                {
                  std::cout << "flag " << str << " not recognized\n";
                }
            }

          //	std::cout << "set flags to " << str << '\n';
        }
    }

  //set any parameters
  if (vm.count ("param"))
    {
      stringVec paramstrings = vm["param"].as<stringVec > ();
      for (auto &str : paramstrings)
        {
          gridParameter p (str);
          if (p.valid)
            {
              objInfo oi (p.field, gds.get ());
			  try
			  {
				  if (p.stringType)
				  {
					  oi.m_obj->set(oi.m_field, p.strVal);
				  }
				  else
				  {
					  oi.m_obj->set(oi.m_field, p.value, p.paramUnits);
				  }
			  }
              catch (const unrecognizedParameter &)
			  {
                  std::cout << "param " << str << " not able to be processed\n";
               }
            }
        }
    }

  if (vm.count ("powerflow-output"))
    {

      std::string pFlowOut = vm["powerflow-output"].as<std::string> ();
      ri.checkDefines (pFlowOut);
      gds->set ("powerflowfile", pFlowOut);
    }
  if (vm.count ("jac-output"))
    {
      std::string JacOut = vm["jac-output"].as<std::string> ();
      ri.checkDefines (JacOut);
      captureJacState (gds.get (),JacOut, gds->getSolverMode ("pflow"));
    }

  if ((vm.count ("powerflow_only"))|| (vm.count ("powerflow-only")))
    {
      gds->setFlag ("powerflow_only", true);
    }

  if (vm.count ("state-output"))
    {
      gds->set ("stateFile", vm["powerflow-output"].as<std::string> ());
      if (vm.count ("save-state-period"))
        {
          gds->set ("state_record_period", vm["save-state-period"].as<int> ());
        }
    }

  if (vm.count ("auto-capture-period"))
    {
      double recperiod = vm["auto-capture-period"].as<double> ();
      std::string recfile = (vm.count ("auto-capture")) ? vm["auto-capture"].as<std::string> () : "auto_capture.bin";

      std::string capfield = (vm.count ("auto-capture-field")) ? vm["auto-capture-field"].as<std::string> () : "auto";
      auto autorec = std::make_shared<gridRecorder> ();
      autorec->set ("file", recfile);
      autorec->set ("period", recperiod);
      autorec->add (capfield, gds.get ());
    }
  else if (vm.count ("auto-capture"))
    {
      gds->log (gds.get (), print_level::warning, "auto-capture file specified without auto-capture-period");
    }
  return 0;
}


int argumentParser (int argc, char *argv[], po::variables_map &vm_map)
{
  po::options_description cmd_only ("command line only");
  po::options_description config ("configuration");
  po::options_description hidden ("hidden");

  //input boost controls
  cmd_only.add_options()
	  ("help,h", "produce help message")
	  ("config-file", po::value<std::string>(), "specify a configuration file to use")
	  ("config-file-output", po::value<std::string>(), "file to store current configuration options")
	  ("mpicount", "setup for an MPI run")
	  ("buildfmu","build an fmu to run the given configuration")
	  ("version", "print version string")
	  ("test", "run a test program[ignored in many cases]");


  config.add_options()
	  ("powerflow-output,o", po::value<std::string>(), "file output for the powerflow solution")
	  ("param,P", po::value < std::vector < std::string >>(), "override simulation file parameters --param ParamName=<val>")
	  ("dir", po::value < std::vector < std::string >>(), "add search directory for input files")
	  ("import,i", po::value < std::vector < std::string >>(), "add import files loaded after the main input file")
	  ("powerflow_only", "set the solver to stop after the power flow solution")
	  ("powerflow-only", "set the solver to stop after the power flow solution")
	  ("state-output", po::value<std::string>(), "file for final output state")
	  ("auto-capture", po::value<std::string>(), "file for automatic recording")
	  ("auto-capture-period", po::value<double>(), "period to capture the automatic recording")
	  ("save-state-period", po::value<int>(), "save state every N ms, -1 for saving only at the end")
	  ("log-file", po::value<std::string>(), "log file output")
	  ("quiet,q", "set verbosity to 0 (ie only error output)")
	  ("jac-output", po::value<std::string>(), "powerflow Jacobian file output")
	  ("verbose,v", po::value<int>(), "specify verbosity output 0=verbose,1=normal, 2=summary,3=none")
	  ("flags,f", po::value < std::vector < std::string >>(), "specify flags to feed to GridDyn")
	  ("file-flags", po::value < std::vector < std::string >>(), "specify flags to feed to the file reader")
	  ("define,D", po::value < std::vector < std::string >>(), "definition strings for the element file readers")
	  ("translate,T", po::value < std::vector < std::string >>(), "translation strings for the element file readers")
	  ("warn,w", po::value<int>(), "specify warning level output 0=all, 1=important,2=none")
	  ("threads", po::value<int>(), "specify the number of worker threads to use if multithreading is enabled")
	  ("xml", po::value<std::string>(), "the xml reader to use: 1 for tinyxml, 2 for tinyxml2")
	  ("match-type", po::value<std::string>(), "the default parameter name matching algorithm to use for xml[exact|capital*|any] ");

  hidden.add_options ()
    ("input", po::value<std::string> (), "input file");

  po::options_description cmd_line ("command line options");
  po::options_description config_file ("configuration file options");
  po::options_description visible ("allowed options");

  cmd_line.add (cmd_only).add (config).add (hidden);
  config_file.add (config).add (hidden);
  visible.add (cmd_only).add (config);

  po::positional_options_description p;
  p.add ("input", -1);

  po::variables_map cmd_vm;
  try
    {
      po::store (po::command_line_parser (argc, argv).
                 options (cmd_line).positional (p).run (), cmd_vm);
    }
  catch (std::exception &e)
    {
      std::cerr << e.what () << std::endl;
      return FUNCTION_EXECUTION_FAILURE;
    }

  po::notify (cmd_vm);

  // objects/pointers/variables/constants


  //program options control
  if (cmd_vm.count ("help"))
    {
      std::cout << visible << '\n';
      return 1;
    }

  if (cmd_vm.count ("version"))
    {
      std::cout << griddyn_version_string << '\n';
      return 1;
    }


  po::store (po::command_line_parser (argc, argv).
             options (cmd_line).positional (p).run (), vm_map);

  if (cmd_vm.count ("config-file"))
    {
      std::string config_file_name = cmd_vm["config-file"].as<std::string> ();
      if (!boost::filesystem::exists (config_file_name))
        {
          std::cerr << "config file " << config_file_name << " does not exist\n";
          return -3;
        }
      else
        {
          std::ifstream fstr (config_file_name.c_str ());
          po::store (po::parse_config_file (fstr, config_file), vm_map);
          fstr.close ();
        }
    }
  if (boost::filesystem::exists ("gridDynConfig.ini"))
    {
      std::ifstream fstr ("gridDynConfig.ini");
      po::store (po::parse_config_file (fstr, config_file), vm_map);
      fstr.close ();
    }
  po::notify (vm_map);
  //check to make sure we have some input file
  if (vm_map.count ("input") == 0)
    {
      std::cout << " no input file specified\n";
      std::cout << visible << '\n';
      return 1;
    }
  return FUNCTION_EXECUTION_SUCCESS;
}

void loadXMLinfo (po::variables_map &vm_map, readerInfo &ri)
{
  if (vm_map.count ("dir"))
    {
      auto dirList = vm_map["dir"].as<stringVec > ();
      for (const auto &dirname : dirList)
        {
          ri.addDirectory (dirname);
        }
    }

  if (vm_map.count ("define"))
    {
      auto deflist = vm_map["define"].as<stringVec > ();
      for (const auto &defstr : deflist)

        {
          auto N = defstr.find_first_of ('=');
          auto def = trim (defstr.substr (0, N));
          auto rep = trim (defstr.substr (N + 1));
          ri.addLockedDefinition (def, rep);
        }
    }

  if (vm_map.count ("translate"))
    {
      auto translist = vm_map["translate"].as<stringVec > ();
      for (const auto &transstr : translist)
        {
          auto N = transstr.find_first_of ('=');
          auto tran = trim (transstr.substr (0, N));
          auto rep = trim (transstr.substr (N + 1));
          ri.addTranslate (tran, rep);
        }
    }
  //set the default XML reader to use
  if (vm_map.count("xml"))
  {
	  readerConfig::setDefaultXMLReader(convertToLowerCase(vm_map["xml"].as<std::string>()));
  }
  //set the default match type to use
  if (vm_map.count("match-type"))
  {
	  readerConfig::setDefaultMatchType(convertToLowerCase(vm_map["match-type"].as<std::string>()));
  }
}
