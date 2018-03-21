/*
 * -----------------------------------------------------------------
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
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

#include "griddyn/gridDynSimulation.h"

#include "core/coreExceptions.h"
#include "core/objectInterpreter.h"
#include "coupling/GhostSwingBusManager.h"
#include "fileInput.h"
#include "griddyn/measurement/Recorder.h"
#include "griddyn/simulation/gridDynSimulationFileOps.h"
#include "utilities/stringOps.h"
#include "utilities/workQueue.h"
#include "utilities/stringToCmdLine.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <memory>

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

namespace griddyn
{
using namespace stringOps;

GriddynRunner::GriddynRunner () = default;

GriddynRunner::GriddynRunner (std::shared_ptr<gridDynSimulation> sim) : m_gds (std::move (sim)) {}

GriddynRunner::~GriddynRunner () = default;


int GriddynRunner::InitializeFromString(const std::string &cmdargs)
{
	utilities::stringToCmdLine args(cmdargs);
	return Initialize(args.getArgCount(), args.getArgV());

}

int GriddynRunner::Initialize (int argc, char *argv[], readerInfo &ri, bool allowUnrecognized)
{
    m_startTime = std::chrono::high_resolution_clock::now ();

    if (!m_gds)
    {
        m_gds = std::make_shared<gridDynSimulation> ();
        // gridDynSimulation::setInstance(m_gds.get());  // peer to gridDynSimulation::GetInstance ();
    }
    // TODO:: do something different with this
    GhostSwingBusManager::Initialize (&argc, &argv);

    vm = std::make_unique<po::variables_map> ();

    auto ret = argumentParser (argc, argv, *vm, allowUnrecognized);
    if (ret != FUNCTION_EXECUTION_SUCCESS)
    {
        return ret;
    }

    // create the simulation

    // load any relevant issue into the readerInfo structure
    loadXMLinfo (*vm, ri);
    ret = processCommandArguments (m_gds, ri, *vm);
    if (ret != FUNCTION_EXECUTION_SUCCESS)
    {
        return (ret);
    }
    m_stopTime = std::chrono::high_resolution_clock::now ();
    m_gds->log (nullptr, print_level::summary, GRIDDYN_VERSION_STRING);

    std::chrono::duration<double> elapsed_t = m_stopTime - m_startTime;
    m_gds->log (m_gds.get (), print_level::normal,
                std::string ("\nInitialization ") + m_gds->getName () + " executed in " +
                  std::to_string (elapsed_t.count ()) + " seconds");
    return FUNCTION_EXECUTION_SUCCESS;
}

int GriddynRunner::Initialize (int argc, char *argv[], bool allowUnrecognized)
{
    readerInfo ri;
    return Initialize (argc, argv, ri, allowUnrecognized);
}

int GriddynRunner::Initialize(int argc, char *argv[])
{
	readerInfo ri;
	return Initialize(argc, argv, ri, false);
}

void GriddynRunner::simInitialize ()
{
    m_startTime = std::chrono::high_resolution_clock::now ();
    m_gds->dynInitialize ();
    if (!(m_gds->hasDynamics ()))
    {
        eventMode = true;
    }
}

int GriddynRunner::Reset()
{
	if (!isReady())
	{
		throw(executionFailure(m_gds.get(), "asynchronous operation ongoing"));
	}
	readerInfo ri;
	return Reset(ri);
}


int GriddynRunner::Reset(readerInfo &ri)
{
	if (!isReady())
	{
		throw(executionFailure(m_gds.get(), "asynchronous operation ongoing"));
	}
	//make a new simulation object
    m_gds = std::make_shared<gridDynSimulation> ();
	//reload it from the existing vm
    loadXMLinfo (*vm, ri);
    auto ret = processCommandArguments (m_gds, ri, *vm);
    if (ret != FUNCTION_EXECUTION_SUCCESS)
    {
        return (ret);
    }
    m_gds->log (m_gds.get (), print_level::normal, std::string ("\nsystem reset of ") + m_gds->getName ());
	return FUNCTION_EXECUTION_SUCCESS;

}

coreTime GriddynRunner::Run ()
{
	if (!isReady())
	{
		throw(executionFailure(m_gds.get(), "asynchronous operation ongoing"));
	}
	
		m_startTime = std::chrono::high_resolution_clock::now();
		m_gds->run();
		m_stopTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_t = m_stopTime - m_startTime;
		m_gds->log(m_gds.get(), print_level::summary,
			m_gds->getName() + " executed in " + std::to_string(elapsed_t.count()) + " seconds");
		return m_gds->getSimulationTime();
	
}


void GriddynRunner::RunAsync()
{
	if (!isReady())
	{
		throw(executionFailure(m_gds.get(), "asynchronous operation ongoing"));
	}
	async_ret = std::async(std::launch::async, [this] {return Run(); });
	
}

coreTime GriddynRunner::Step (coreTime time)
{
	if (!isReady())
	{
		throw(executionFailure(m_gds.get(), "asynchronous operation ongoing"));
	}
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
                throw (std::runtime_error (error));
            }
        }
        else
        {
            int retval = m_gds->step (time, actual);
            if (retval < FUNCTION_EXECUTION_SUCCESS)
            {
                std::string error = "GridDyn failed to advance retval = " + std::to_string (retval);
                throw (std::runtime_error (error));
            }
        }
    }

    return actual;
}

void GriddynRunner::StepAsync(coreTime time)
{
	if (!isReady())
	{
		throw(executionFailure(m_gds.get(), "asynchronous operation ongoing"));
	}
	async_ret = std::async(std::launch::async, [this, time] {return Step(time); });
}

bool GriddynRunner::isReady() const
{
	return (async_ret.valid()) ? (async_ret.wait_for(std::chrono::seconds(0)) == std::future_status::ready) : true;
}

int GriddynRunner::getStatus(coreTime &timeReturn)
{
	
	timeReturn = m_gds->getSimulationTime();
	return (isReady())?static_cast<int>(m_gds->currentProcessState()):GRIDDYN_PENDING;
}

coreTime GriddynRunner::getNextEvent () const { return m_gds->getEventTime (); }
void GriddynRunner::StopRecording ()
{
    m_gds->log (m_gds.get (), print_level::normal, "Saving recorders...");
    m_gds->saveRecorders ();
    m_stopTime = std::chrono::high_resolution_clock::now ();
    std::chrono::duration<double> elapsed_t = m_stopTime - m_startTime;
    m_gds->log (m_gds.get (), print_level::normal,
                std::string ("\nSimulation ") + m_gds->getName () + " executed in " +
                  std::to_string (elapsed_t.count ()) + " seconds");
}

void GriddynRunner::Finalize ()
{
    StopRecording ();
    GhostSwingBusManager::Instance ()->endSimulation ();
}

int processCommandArguments (std::shared_ptr<gridDynSimulation> &gds, readerInfo &ri, po::variables_map &vm)
{
    if (vm.count ("quiet") > 0)
    {
        readerConfig::setPrintMode (0);
        gds->set ("printlevel", 0);
    }
    if (vm.count ("verbose") > 0)
    {
        int temp = vm["verbose"].as<int> ();
        readerConfig::setPrintMode (temp);  // set the gridDynXML reader print mode
        // default is normal mode
    }

    if (vm.count ("warn") > 0)
    {
        int temp = vm["warn"].as<int> ();
        readerConfig::setWarnMode (temp);  // set the gridDynXML reader warn mode
        // default is warn all
    }
    if (vm.count ("mpicount") > 0)  // if we are in mpi mode don't print anything
    {
        readerConfig::setPrintMode (READER_NO_PRINT);
        readerConfig::setWarnMode (READER_WARN_NONE);
    }

    // get the main input file
    if (vm.count ("log-file") > 0)
    {
        std::string log_file = vm["log-file"].as<std::string> ();
        ri.checkDefines (log_file);
        gds->set ("logfile", log_file);
    }

    if (vm.count ("file-flags") > 0)
    {
        stringVec flagstrings = vm["file-flags"].as<stringVec> ();
        for (auto &str : flagstrings)
        {
            addflags (ri, str);
        }
    }
    if (vm.count ("threads") > 0)
    {
        // initiate the work queue with the requested number of threads
        workQueue::instance (vm["threads"].as<int> ());
    }

    // set a powerflow only flag as it could change which objects get loaded
    if ((vm.count ("powerflow_only") > 0) || (vm.count ("powerflow-only") > 0))
    {
        gds->setFlag ("powerflow_only", true);
    }

    std::string grid_file = vm["input"].as<std::string> ();

    loadFile (gds.get (), grid_file, &ri);
    if (vm.count ("import") > 0)
    {
        auto importList = vm["import"].as<stringVec> ();
        for (auto &iF : importList)
        {
            loadFile (gds.get (), iF, &ri);
        }
    }

    if (gds->getErrorCode () != 0)
    {
        return gds->getErrorCode ();
    }

    int areas = gds->getInt ("totalareacount");
    int buses = gds->getInt ("totalbuscount");
    int links = gds->getInt ("totallinkcount");
    int gens = gds->getInt ("gencount");
    std::cout << "area count =" << areas << " buses=" << buses << " links= " << links << " gens= " << gens << '\n';

    // set any flags used by the system
    if (vm.count ("flags") > 0)
    {
        stringVec flagstrings = vm["flags"].as<stringVec> ();
        for (auto &str : flagstrings)
        {
            try
            {
                setMultipleFlags (gds.get (), str);
            }
            catch (const unrecognizedParameter &)
            {
                std::cout << "flag " << str << " not recognized\n";
            }
        }
    }

    // set any parameters
    if (vm.count ("param") > 0)
    {
        stringVec paramstrings = vm["param"].as<stringVec> ();
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
                        oi.m_obj->set (oi.m_field, p.strVal);
                    }
                    else
                    {
                        oi.m_obj->set (oi.m_field, p.value, p.paramUnits);
                    }
                }
                catch (const unrecognizedParameter &)
                {
                    std::cout << "param " << str << " not able to be processed\n";
                }
            }
        }
    }

    if (vm.count ("powerflow-output") > 0)
    {
        std::string pFlowOut = vm["powerflow-output"].as<std::string> ();
        ri.checkDefines (pFlowOut);
        gds->set ("powerflowfile", pFlowOut);
    }
    if (vm.count ("jac-output") > 0)
    {
        std::string JacOut = vm["jac-output"].as<std::string> ();
        ri.checkDefines (JacOut);
        captureJacState (gds.get (), JacOut, gds->getSolverMode ("pflow"));
    }

    if (vm.count ("state-output") > 0)
    {
        gds->set ("stateFile", vm["powerflow-output"].as<std::string> ());
        if (vm.count ("save-state-period") > 0)
        {
            gds->set ("state_record_period", vm["save-state-period"].as<int> ());
        }
    }

    if (vm.count ("auto-capture-period") > 0)
    {
        auto recperiod = vm["auto-capture-period"].as<double> ();
        std::string recfile =
          (vm.count ("auto-capture") > 0) ? vm["auto-capture"].as<std::string> () : "auto_capture.bin";

        std::string capfield =
          (vm.count ("auto-capture-field") > 0) ? vm["auto-capture-field"].as<std::string> () : "auto";
        auto autorec = std::make_shared<Recorder> ();
        autorec->set ("file", recfile);
        autorec->set ("period", recperiod);
        autorec->add (capfield, gds.get ());
    }
    else if (vm.count ("auto-capture") > 0)
    {
        gds->log (gds.get (), print_level::warning, "auto-capture file specified without auto-capture-period");
    }
    return FUNCTION_EXECUTION_SUCCESS;
}

int argumentParser (int argc, char *argv[], po::variables_map &vm_map, bool allowUnrecognized)
{
    po::options_description cmd_only ("command line only");
    po::options_description config ("configuration");
    po::options_description hidden ("hidden");

    // clang-format off
    // input boost controls
    cmd_only.add_options () 
		("help,h", "produce help message")
		("config-file", po::value<std::string> (),"specify a configuration file to use") 
		("config-file-output", po::value<std::string> (), "file to store current configuration options")
		("mpicount", "setup for an MPI run")
		("version", "print version string")
		("test", "run a test program[ignored in many cases]");

    config.add_options () ("powerflow-output,o", po::value<std::string> (),"file output for the powerflow solution")
		("param,P", po::value<std::vector<std::string>> (),"override simulation file parameters --param ParamName=<val>")
		("dir",po::value<std::vector<std::string>> (),"add search directory for input files")
		("import,i", po::value<std::vector<std::string>> (), "add import files loaded after the main input file")
		("powerflow_only", "set the solver to stop after the power flow solution and use some powerflow specific models")
		("powerflow-only", "set the solver to stop after the power flow solution and use some powerflow specific models")
		("state-output", po::value<std::string> (),"file for final output state")
		("auto-capture", po::value<std::string> (), "file for automatic recording")
		("auto-capture-period", po::value<double> (),"period to capture the automatic recording")
		("save-state-period", po::value<int> (),"save state every N ms, -1 for saving only at the end")
		( "log-file", po::value<std::string> (), "log file output")
		("quiet,q","set verbosity to 0 (ie only error output)")
		("jac-output", po::value<std::string> (),"powerflow Jacobian file output")
		("verbose,v", po::value<int> (),"specify verbosity output 0=verbose,1=normal, 2=summary,3=none")
        ("flags,f", po::value<std::vector<std::string>> (),"specify flags to feed to GridDyn")
		("file-flags", po::value<std::vector<std::string>> (),"specify flags to feed to the file reader")
		("define,D", po::value<std::vector<std::string>> (),"definition strings for the element file readers")
		("translate,T", po::value<std::vector<std::string>> (),"translation strings for the element file readers")
		("warn,w", po::value<int> (), "specify warning level output 0=all, 1=important,2=none")
		("threads", po::value<int> (), "specify the number of worker threads to use if multithreading is enabled")
		("xml", po::value<std::string> (), "the xml reader to use: 1 for tinyxml, 2 for tinyxml2")
		( "match-type", po::value<std::string> (),"the default parameter name matching algorithm to use for xml[exact|capital*|any] ");

    hidden.add_options () ("input", po::value<std::string> (), "input file");

    // clang-format on
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
        if (!allowUnrecognized)
        {
            po::store (po::command_line_parser (argc, argv).options (cmd_line).positional (p).run (), cmd_vm);
        }
        else
        {
            auto parsed =
              po::command_line_parser (argc, argv).options (cmd_line).positional (p).allow_unregistered ().run ();
            po::store (parsed, cmd_vm);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what () << std::endl;
        throw (e);
    }

    po::notify (cmd_vm);

    // objects/pointers/variables/constants

    // program options control
    if (cmd_vm.count ("help") > 0)
    {
        std::cout << visible << '\n';
        return 1;
    }

    if (cmd_vm.count ("version") > 0)
    {
        std::cout << GRIDDYN_VERSION_STRING << '\n';
        return 1;
    }

    if (!allowUnrecognized)
    {
        po::store (po::command_line_parser (argc, argv).options (cmd_line).positional (p).run (), vm_map);
    }
    else
    {
        auto parsed =
          po::command_line_parser (argc, argv).options (cmd_line).positional (p).allow_unregistered ().run ();
        po::store (parsed, vm_map);
    }

    if (cmd_vm.count ("config-file") > 0)
    {
        std::string config_file_name = cmd_vm["config-file"].as<std::string> ();
        if (!filesystem::exists (config_file_name))
        {
            std::cerr << "config file " << config_file_name << " does not exist\n";
            throw (fileNotFoundError ());
        }

        std::ifstream fstr (config_file_name.c_str ());

        po::store (po::parse_config_file (fstr, config_file, allowUnrecognized), vm_map);

        fstr.close ();
    }
    if (filesystem::exists ("gridDynConfig.ini"))
    {
        std::ifstream fstr ("gridDynConfig.ini");
        po::store (po::parse_config_file (fstr, config_file, true), vm_map);
        fstr.close ();
    }
    po::notify (vm_map);
    // check to make sure we have some input file
    if (vm_map.count ("input") == 0)
    {
        std::cout << " no input file specified\n";
        std::cout << visible << '\n';
        return -1;
    }
    return FUNCTION_EXECUTION_SUCCESS;
}

void loadXMLinfo (po::variables_map &vm_map, readerInfo &ri)
{
    if (vm_map.count ("dir") > 0)
    {
        auto dirList = vm_map["dir"].as<stringVec> ();
        for (const auto &dirname : dirList)
        {
            ri.addDirectory (dirname);
        }
    }

    if (vm_map.count ("define") > 0)
    {
        auto deflist = vm_map["define"].as<stringVec> ();
        for (const auto &defstr : deflist)

        {
            auto N = defstr.find_first_of ('=');
            auto def = trim (defstr.substr (0, N));
            auto rep = trim (defstr.substr (N + 1));
            ri.addLockedDefinition (def, rep);
        }
    }

    if (vm_map.count ("translate") > 0)
    {
        auto translist = vm_map["translate"].as<stringVec> ();
        for (const auto &transstr : translist)
        {
            auto N = transstr.find_first_of ('=');
            auto tran = trim (transstr.substr (0, N));
            auto rep = trim (transstr.substr (N + 1));
            ri.addTranslate (tran, rep);
        }
    }
    // set the default XML reader to use
    if (vm_map.count ("xml") > 0)
    {
        readerConfig::setDefaultXMLReader (convertToLowerCase (vm_map["xml"].as<std::string> ()));
    }
    // set the default match type to use
    if (vm_map.count ("match-type") > 0)
    {
        readerConfig::setDefaultMatchType (convertToLowerCase (vm_map["match-type"].as<std::string> ()));
    }
}

}  // namespace griddyn