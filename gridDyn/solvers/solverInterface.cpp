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

#include "solverInterface.h"
#include "sundialsInterface.h"
#include "core/factoryTemplates.h"
#include "core/coreExceptions.h"
#include "gridDyn.h"
#include "utilities/stringConversion.h"
#include "utilities/mapOps.h"
#include <iostream>
#include <new>


static childClassFactoryArg<basicSolver, solverInterface, basicSolver::mode_t> basicFactoryG(stringVec{ "basic","gauss" },basicSolver::mode_t::gauss);
static childClassFactoryArg<basicSolver, solverInterface,  basicSolver::mode_t> basicFactoryGS(stringVec{ "gs","gauss-seidel" }, basicSolver::mode_t::gauss_seidel);
#ifdef LOAD_CVODE
static childClassFactory<basicOdeSolver,solverInterface> basicOdeFactory(stringVec{ "basicode","euler" });
#else
// if cvode is not available this becomes the default differential solver
static childClassFactory<basicOdeSolver,solverInterface> basicOdeFactory(stringVec{ "basicode","dyndiff", "differential" });

#endif

solverInterface::solverInterface(const std::string &objName) :helperObject(objName)
{
}

solverInterface::solverInterface (gridDynSimulation *gds, const solverMode& sMode) : mode (sMode),m_gds (gds)
{

}



std::shared_ptr<solverInterface> solverInterface::clone(std::shared_ptr<solverInterface> si, bool fullCopy) const
{
	if (!si)
	{
		si = std::make_shared<solverInterface>(getName());
	}
	else
	{
		si->setName(getName());
	}
	
	si->printResid = printResid;
	si->solverLogFile = solverLogFile;
	si->printLevel = printLevel;	
	si->max_iterations = 10000;
	auto ind = si->mode.offsetIndex;
	si->mode = mode;
	if (ind != kNullLocation)
	{
		si->mode.offsetIndex = ind;
	}
	si->tolerance = tolerance;
	si->flags = flags;
	if (fullCopy)
	{
		si->maskElements = maskElements;
		si->m_gds = m_gds;
		si->allocate(svsize, rootCount);
		if (flags[initialized_flag])
		{
			si->initialize(0.0);
		}
		//copy the state data
		const double *sd = state_data();
		double *statecopy = si->state_data();
		if ((sd) && (statecopy))
		{
			std::copy(sd, sd + svsize, statecopy);
		}

		//copy the derivative data
		const double *deriv = deriv_data();
		double *derivcopy = si->deriv_data();
		if ((deriv)&&(derivcopy))
		{
			std::copy(deriv, deriv + svsize, derivcopy);
		}
		//copy the type data
		const double *td = type_data();
		double *tcopy = si->type_data();
		if ((td) && (tcopy))
		{
			std::copy(td, td + svsize, tcopy);
		}
		si->jacFile = jacFile;
		si->stateFile = stateFile;

	}
	return si;
}

double * solverInterface::state_data ()
{
  return nullptr;
}



double * solverInterface::deriv_data ()
{
  return nullptr;
}

double *solverInterface::type_data ()
{
  return nullptr;
}

const double * solverInterface::state_data() const
{
	return nullptr;
}



const double * solverInterface::deriv_data() const
{
	return nullptr;
}

const double *solverInterface::type_data() const
{
	return nullptr;
}
void solverInterface::allocate (count_t /*stateSize*/, count_t numroots)
{
  rootsfound.resize (numroots);
}

void solverInterface::initialize (coreTime t0)
{
	solveTime = t0;
}
void solverInterface::sparseReInit (sparse_reinit_modes /*mode*/)
{
}
void solverInterface::setConstraints ()
{
}
int solverInterface::calcIC (coreTime /*t0*/, coreTime /*tstep0*/, ic_modes /*mode*/, bool /*constraints*/)
{
  return -101;
}
void solverInterface::getCurrentData ()
{
  
}
void solverInterface::getRoots ()
{
  
}

void solverInterface::setRootFinding (index_t /*numRoots*/)
{
  
}

void solverInterface::setSimulationData (const solverMode& sMode)
{

  mode = sMode;
}

void solverInterface::setSimulationData (gridDynSimulation *gds, const solverMode& sMode)
{

  mode = sMode;
  if (gds)
    {
      m_gds = gds;
    }
}

void solverInterface::setSimulationData (gridDynSimulation *gds)
{
  if (gds)
    {
      m_gds = gds;
    }
}


double solverInterface::get (const std::string & param) const
{
  double res;
  if (param == "solvercount")
  {
	  res = static_cast<double> (solverCallCount);
  }
  else if (param == "jaccallcount")
  {
	  res = static_cast<double> (jacCallCount);
  }
  else if ((param == "rootcallcount")||(param=="roottestcount"))
  {
	  res = static_cast<double> (rootCallCount);
  }
  else if (param == "funccallcount")
  {
	  res = static_cast<double> (funcCallCount);
  }
  else if (param == "approx")
    {
      res = static_cast<double> (getLinkApprox (mode));
    }
  else if (param == "printlevel")
    {
      res = static_cast<double> (printLevel);
    }
  else if (param == "tolerance")
    {
      res = tolerance;
    }
  else
  {
	  return helperObject::get(param);
  }
  return res;
}

void solverInterface::set (const std::string &param, const std::string &val)
{

  if ((param == "approx") || (param == "approximation"))
    {
	  setApproximation(convertToLowerCase(val));
      
    }
  else if (param == "printlevel")
    {
      auto plevel = convertToLowerCase (val);
      if (plevel == "debug")
        {
          printLevel = solver_print_level::s_debug_print;
        }
      else if ((plevel == "none")||(plevel == "trap"))
        {
          printLevel = solver_print_level::s_error_trap;
        }
      else if (plevel == "error")
        {
          printLevel = solver_print_level::s_error_log;
        }
      else
        {
		  throw(invalidParameterValue());
        }
    }
  
  else if ((param == "pair")||(param == "pairedmode"))
    {
      if (m_gds)
        {
          auto nsmode = m_gds->getSolverMode (val);
          mode.pairedOffsetIndex = nsmode.offsetIndex;
        }
    }
  else if (param == "mask")
    {
      auto sep = str2vector<int>(val, -1, ",;");
      maskElements.resize (sep.size ());
      for (size_t kk = 0; kk < sep.size (); ++kk)
        {
          maskElements[kk] = sep[kk];
        }
    }
  else if (param == "mode")
    {
	  setMultipleFlags(this, val);
    }
  else if ((param == "file") || (param == "logfile"))
    {
      solverLogFile = val;
    }
	else if (param == "jacfile")
	{
		jacFile = val;
	}
	else if (param == "statefile")
	{
		stateFile = val;
	}
	else if (param == "capturefile")
	{
		jacFile = val;
		stateFile = val;
	}
  else
    {
	  helperObject::set(param, val);
    }
 
}

void solverInterface::set (const std::string &param, double val)
{
  if ((param== "pair")||(param == "pairedmode"))
    {
      mode.pairedOffsetIndex = static_cast<index_t> (val);
    }
  else if (param == "tolerance")
  {
	  tolerance = val;
  }
  else if (param == "printlevel")
    {
      switch (static_cast<int> (val))
        {
        case 0:
          printLevel = solver_print_level::s_error_trap;
          break;
        case 1:
          printLevel = solver_print_level::s_error_log;
          break;
        case 2:
          printLevel = solver_print_level::s_debug_print;
          break;
        default:
			throw(invalidParameterValue());
        }
    }
  
  else if (param == "maskElement")
    {
      addMaskElement (static_cast<index_t> (val));
    }
  else if (param == "index")
    {
      mode.offsetIndex = static_cast<index_t> (val);
    }
  else
    {
	  helperObject::set(param,val);
    }
 
}

/* *INDENT-OFF* */
static const std::map<std::string, int> solverFlagMap
{
	{"filecapture",fileCapture_flag},
	{"dense",dense_flag },
	{"sparse",-dense_flag },
	{"parallel",parallel_flag },
	{"serial",-parallel_flag },
	{"mask",useMask_flag },
	{"constantjacobian",constantJacobian_flag },
	{"omp",use_omp_flag },
	{"useomp",use_omp_flag },
	{"bdf", use_bdf_flag },
	{"adams",-use_bdf_flag },
	{"functional", -use_newton_flag },
	{"newton",use_newton_flag },
};
/* *INDENT-ON* */



void solverInterface::setFlag(const std::string &flagName, bool val)
{
	auto flgInd = mapFind(solverFlagMap, flagName, -60);
	if (flgInd > -32)
	{
		if (flgInd > 0)
		{
			flags.set(flgInd, val);
		}
		else
		{
			flags.set(-flgInd, !val);
		}
		return;
	}
	
	if (flagName == "dc")
	{
		mode.approx.set(dc, val);
	}
	else if (flagName == "ac")
	{
		mode.approx.set(dc, !val);
	}
	else if (flagName == "dynamic")
	{
		mode.dynamic = val;
	}
	else if (flagName == "powerflow")
	{
		mode.dynamic = false;
		mode.differential = false;
		mode.dynamic = false;
		mode.algebraic = true;
	}
	else if (flagName == "differential")
	{
		if (val)
		{
			mode.differential = true;
			mode.dynamic = true;
		}
		else
		{
			mode.differential = false;
		}
	}
	else if (flagName == "algebraic")
	{
		mode.algebraic = val;
	}
	else if (flagName == "local")
	{
		mode.local = val;
	}
	else if (flagName == "dae")
	{
		if (val)
		{
			mode.differential = true;
			mode.dynamic = true;
			mode.algebraic = true;
		}
		else
		{
			//PT:: what does dae false mean?  probably not do anything
		}
		
	}
	else if (flagName == "extended")
	{
		mode.extended_state = val;
	}
	else if (flagName == "primary")
	{
		mode.extended_state = !val;
	}
	else if (flagName == "debug")
	{
		printLevel = solver_print_level::s_debug_print;
	}
	else if (flagName == "trap")
	{
		printLevel = solver_print_level::s_error_trap;
	}
	else if (flagName == "error")
	{
		printLevel = solver_print_level::s_error_log;
	}
	else
	{
		if (val)
		{
			setApproximation(flagName);
		}
		else
		{
			throw(unrecognizedParameter());
		}
	}
}


void solverInterface::setApproximation(const std::string &approx)
{
	if ((approx == "normal") || (approx == "none"))
	{
		setLinkApprox(mode, approxKeyMask::none);
	}
	else if ((approx == "simple") || (approx == "simplified"))
	{
		setLinkApprox(mode, approxKeyMask::simplified);
	}
	else if (approx == "small_angle")
	{
		setLinkApprox(mode, approxKeyMask::sm_angle);
	}
	else if (approx == "small_angle_decoupled")
	{
		setLinkApprox(mode, approxKeyMask::sm_angle_decoupled);
	}
	else if (approx == "simplified_decoupled")
	{
		setLinkApprox(mode, approxKeyMask::simplified_decoupled);
	}
	else if ((approx == "small_angle_simplified") || (approx == "simplified_small_angle"))
	{
		setLinkApprox(mode, approxKeyMask::simplified_sm_angle);
	}
	else if ((approx == "r") || (approx == "small_r"))
	{
		setLinkApprox(mode, linear, false);
		setLinkApprox(mode, small_r);
	}
	else if (approx == "angle")
	{
		setLinkApprox(mode, linear, false);
		setLinkApprox(mode, small_angle);
	}
	else if (approx == "coupling")
	{
		setLinkApprox(mode, linear, false);
		setLinkApprox(mode, decoupled);
	}
	else if (approx == "decoupled")
	{
		setLinkApprox(mode, approxKeyMask::decoupled);
	}
	else if (approx == "linear")
	{
		setLinkApprox(mode, approxKeyMask::linear);
	}
	else if ((approx == "fast_decoupled") || (approx == "fdpf"))
	{
		setLinkApprox(mode, approxKeyMask::fast_decoupled);
	}
	else
	{
		throw(invalidParameterValue());
	}
}

void solverInterface::setMaskElements (std::vector<index_t> msk)
{
  maskElements = msk;
}

void solverInterface::addMaskElement (index_t newMaskElement)
{
  maskElements.push_back (newMaskElement);
}

void solverInterface::addMaskElements (std::vector<index_t> newMsk)
{
  for (auto &nme : newMsk)
    {
      maskElements.push_back (nme);
    }
}

void solverInterface::printStates (bool stateNames)
{

  double *state = state_data ();
  double *dstate = deriv_data ();
  double *type = type_data ();
  stringVec stName;
  if (stateNames)
    {
      m_gds->getStateName (stName,mode);
    }
  for (size_t ii = 0; ii < svsize; ++ii)
    {
      if (type)
        {
          std::cout << ((type[ii] == 1) ? 'D' : 'A') << '-';
        }
      if (stateNames)
        {
          std::cout << '[' << ii << "]:" << stName[ii] << '=';
        }
      else
        {
          std::cout << "state[" << ii << "]=";
        }
      std::cout << state[ii];
      if (dstate)
        {
          std::cout << "               ds/dt=" << dstate[ii];
        }
      std::cout << '\n';
    }

}

void solverInterface::check_flag (void *flagvalue, const std::string &funcname, int opt, bool printError) const
{
  int *errflag;
  // Check if SUNDIALS function returned nullptr pointer - no memory allocated
  if (opt == 0 && flagvalue == nullptr)
    {
      if (printError)
        {
          m_gds->log (m_gds,print_level::error, funcname + " failed - returned nullptr pointer");
        }
	  throw(std::bad_alloc());
    }
  else if (opt == 1)
    {
      // Check if flag < 0
      errflag = (int *)flagvalue;
      if (*errflag < 0)
        {
          if (printError)
            {
              m_gds->log (m_gds, print_level::error, funcname + " failed with flag = " + std::to_string (*errflag));
            }
		  throw(solverException(*errflag));
        }
    }

}

int solverInterface::solve (coreTime /*tStop*/, coreTime & /*tReturn*/, step_mode)
{
  return -101;
}


void solverInterface::logSolverStats (print_level /*logLevel*/, bool /*iconly*/) const
{
}

void solverInterface::logErrorWeights (print_level /*logLevel*/) const
{

}

void solverInterface::logMessage (int errorCode, std::string message)
{
  if ((errorCode > 0)&&(printLevel == solver_print_level::s_debug_print))
    {
      m_gds->log (m_gds, print_level::debug, message);
    }
  if (errorCode != 0)
    {
      lastErrorCode = errorCode;
      lastErrorString = message;
      if (printLevel == solver_print_level::s_error_log)
        {
          m_gds->log (m_gds, print_level::warning, message);
        }
    }
}

void solverInterface::setMaxNonZeros (count_t nonZeroCount)
{
  nnz = nonZeroCount;
}

//TODO:: change this function so the defaults can be something other than sundials solvers
std::unique_ptr<solverInterface> makeSolver (gridDynSimulation *gds, const solverMode &sMode)
{
  std::unique_ptr<solverInterface> sd = nullptr;
  if (isLocal (sMode))
    {
      sd = std::make_unique<solverInterface> (gds, sMode);
    }
  else if ((isAlgebraicOnly (sMode)) || (!isDynamic (sMode)))
    {
      sd = std::make_unique<kinsolInterface> (gds, sMode);
      if (sMode.offsetIndex == 2)
        {
          sd->setName("powerflow");
        }
      else if (sMode.offsetIndex == 4)
        {
          sd->setName("algebraic");
        }
    }
  else if (isDAE (sMode))
    {
      sd = std::make_unique<idaInterface> (gds, sMode);
      if (sMode.offsetIndex == 3)
        {
          sd->setName("dynamic");
        }
    }
  else if (isDifferentialOnly (sMode))
    {
	  sd = coreClassFactory<solverInterface>::instance()->createObject("differential");
	  sd->setSimulationData(gds, sMode);
      if (sMode.offsetIndex == 5)
        {
          sd->setName("differential");
        }

    }

  return sd;
}

std::unique_ptr<solverInterface> makeSolver (const std::string &type, const std::string &name)
{
	if (name.empty())
	{
		return coreClassFactory<solverInterface>::instance()->createObject(type);
	}
	else
	{
		return coreClassFactory<solverInterface>::instance()->createObject(type,name);
	}
  
}
