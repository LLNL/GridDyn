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

#include "solverInterface.h"
#include "sundialsInterface.h"
#include "core/factoryTemplates.h"
#include "core/gridDynExceptions.h"
#include "gridDyn.h"
#include "stringConversion.h"

#include <string>
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

solverInterface::solverInterface(const std::string &objName) :name(objName)
{
}

solverInterface::solverInterface (gridDynSimulation *gds, const solverMode& sMode) : mode (sMode),m_gds (gds)
{

}

solverInterface::~solverInterface ()
{
}


std::shared_ptr<solverInterface> solverInterface::clone(std::shared_ptr<solverInterface> si, bool fullCopy) const
{
	if (!si)
	{
		si = std::make_shared<solverInterface>(name);
	}
	else
	{
		si->name = name;
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
	si->dense = dense;
	si->constantJacobian = constantJacobian;
	si->useMask = useMask;
	si->parallel = parallel;
	si->locked = locked;
	si->use_omp = use_omp;

	if (fullCopy)
	{
		si->maskElements = maskElements;
		si->m_gds = m_gds;
		si->allocate(svsize, rootCount);
		if (initialized)
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
		si->fileCapture = fileCapture;
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

void solverInterface::initialize (gridDyn_time t0)
{
	solveTime = t0;
}
void solverInterface::sparseReInit (sparse_reinit_modes /*mode*/)
{
}
void solverInterface::setConstraints ()
{
}
int solverInterface::calcIC (gridDyn_time /*t0*/, gridDyn_time /*tstep0*/, ic_modes /*mode*/, bool /*constraints*/)
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
  double res = kNullVal;
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
  return res;
}

void solverInterface::set (const std::string &param, const std::string &val)
{

  if (param[0] == '#')
    {

    }
  else if ((param == "approx") || (param == "approximation"))
    {
      auto vstr = convertToLowerCase (val);
      if ((vstr == "normal") || (vstr == "none"))
        {
          setLinkApprox (mode, approxKeyMask::none);
        }
      else if ((vstr == "simple") || (vstr == "simplified"))
        {
          setLinkApprox (mode, approxKeyMask::simplified);
        }
      else if (vstr == "small_angle")
        {
          setLinkApprox (mode, approxKeyMask::sm_angle);
        }
      else if (vstr == "small_angle_decoupled")
        {
          setLinkApprox (mode, approxKeyMask::sm_angle_decoupled);
        }
      else if (vstr == "simplified_decoupled")
        {
          setLinkApprox (mode, approxKeyMask::simplified_decoupled);
        }
      else if ((vstr == "small_angle_simplified")||(vstr == "simplified_small_angle"))
        {
          setLinkApprox (mode, approxKeyMask::simplified_sm_angle);
        }
      else if ((vstr == "r")||(vstr == "small_r"))
        {
          setLinkApprox (mode, linear, false);
          setLinkApprox (mode, small_r);
        }
      else if (vstr == "angle")
        {
          setLinkApprox (mode, linear, false);
          setLinkApprox (mode, small_angle);
        }
      else if (vstr == "coupling")
        {
          setLinkApprox (mode, linear, false);
          setLinkApprox (mode, decoupled);
        }
      else if (vstr == "decoupled")
        {
          setLinkApprox (mode, approxKeyMask::decoupled);
        }
      else if (vstr == "linear")
        {
          setLinkApprox (mode, approxKeyMask::linear);
        }
      else if ((vstr == "fast_decoupled") || (vstr == "fdpf"))
        {
          setLinkApprox (mode, approxKeyMask::fast_decoupled);
        }
      else
        {
		  throw(invalidParameterValue());
        }
    }
  else if (param == "printlevel")
    {
      auto vstr = convertToLowerCase (val);
      if (vstr == "debug")
        {
          printLevel = solver_print_level::s_debug_print;
        }
      else if ((vstr == "none")||(vstr == "trap"))
        {
          printLevel = solver_print_level::s_error_trap;
        }
      else if (vstr == "error")
        {
          printLevel = solver_print_level::s_error_log;
        }
      else
        {
		  throw(invalidParameterValue());
        }
    }
  else if (param == "flags")
    {
      auto sep = splitlineTrim (convertToLowerCase (val));
      for (const auto &str : sep)
        {
          if (str == "dense")
            {
              dense = true;
            }
          else if (str == "sparse")
            {
              dense = false;
            }
          else if (str == "parallel")
            {
              parallel = true;
            }
          else if (str == "serial")
            {
              parallel = false;
            }
          else if (str == "constantjacobian")
            {
              constantJacobian = true;
            }
          else if (str == "mask")
            {
              useMask = true;
            }
          else if (str == "debug")
            {
              printLevel = solver_print_level::s_debug_print;
            }
          else if ((str == "none")||(str == "trap"))
            {
              printLevel = solver_print_level::s_error_trap;
            }
          else if (str == "error")
            {
              printLevel = solver_print_level::s_error_log;
            }
          else
            {
              solverInterface::set ("mode",str);
            }
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
      auto sep = splitlineTrim (convertToLowerCase (val));
      for (const auto &str : sep)
        {
          if (str == "dc")
            {
              setDC (mode);
            }
          else if (str == "ac")
            {
              mode.approx.set (dc, false);
            }
          else if (str == "dynamic")
            {
              mode.dynamic = true;
            }
          else if (str == "powerflow")
            {
              mode.dynamic = false;
              mode.differential = false;
              mode.dynamic = false;
              mode.algebraic = true;
            }
          else if (str == "differential")
            {
              mode.differential = true;
              mode.dynamic = true;
            }
          else if (str == "algebraic")
            {
              mode.algebraic = true;
            }
          else if (str == "local")
            {
              mode.local = true;
            }
          else if (str == "dae")
            {
              mode.differential = true;
              mode.dynamic = true;
              mode.algebraic = true;
            }
          else if (str == "extended")
            {
              mode.extended_state = true;
            }
          else if (str == "primary")
            {
              mode.extended_state = false;
            }
          else
            {
              solverInterface::set ("approx", str);
            }
        }
    }
  else if ((param == "file") || (param == "logfile"))
    {
      solverLogFile = val;
    }
  else if (param == "name")
    {
      setName(val);
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
	  throw(unrecognizedParameter());
    }
 
}

void solverInterface::set (const std::string &param, double val)
{
  auto pstr = convertToLowerCase (param);
  
  if ((pstr == "pair")||(pstr == "pairedmode"))
    {
      mode.pairedOffsetIndex = static_cast<index_t> (val);
    }
  else if (pstr == "printlevel")
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
  else if (param == "filecapture")
  {
	  fileCapture = (val >= 0.1);
  }
  else if (pstr == "dense")
    {
      dense = (val > 0);
    }
  else if (pstr == "sparse")
    {
      dense = (val <= 0);
    }
  else if (pstr == "dc")
    {
      mode.approx.set (dc,(val > 0));
    }
  else if (pstr == "tolerance")
    {
      tolerance = val;
    }
  else if (pstr == "parallel")
    {
      parallel = (val > 0);
    }
  else if (pstr == "constantjacobian")
    {
      constantJacobian = (val > 0);
    }
  else if (pstr == "mask")
    {
      useMask = (val > 0);
    }
  else if (pstr == "maskElement")
    {
      addMaskElement (static_cast<index_t> (val));
    }
  else if (pstr == "index")
    {
      mode.offsetIndex = static_cast<index_t> (val);
    }
  else
    {
	  throw(unrecognizedParameter());
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

int solverInterface::solve (gridDyn_time /*tStop*/, gridDyn_time & /*tReturn*/, step_mode)
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

std::shared_ptr<solverInterface> makeSolver (gridDynSimulation *gds, const solverMode &sMode)
{
  std::shared_ptr<solverInterface> sd = nullptr;
  if (isLocal (sMode))
    {
      sd = std::make_shared<solverInterface> (gds, sMode);
    }
  else if ((isAlgebraicOnly (sMode)) || (!isDynamic (sMode)))
    {
      sd = std::make_shared<kinsolInterface> (gds, sMode);
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
      sd = std::make_shared<idaInterface> (gds, sMode);
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
//TODO:: add Name option
std::shared_ptr<solverInterface> makeSolver (const std::string &type)
{
	return coreClassFactory<solverInterface>::instance()->createObject(type);
  
}
