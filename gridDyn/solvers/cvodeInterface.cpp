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

#include "sundialsInterface.h"

#include "gridDyn.h"
#include "vectorOps.hpp"
#include "stringOps.h"
#include "core/helperTemplates.h"
#include "simulation/gridDynSimulationFileOps.h"
#include "core/gridDynExceptions.h"

#include <cvode/cvode.h>
#include <cvode/cvode_dense.h>


#ifdef KLU_ENABLE
#include <cvode/cvode_klu.h>
#include <cvode/cvode_sparse.h>
#endif

#include <cstdio>
#include <string>
#include <cassert>
#include <map>


int cvodeFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, void *user_data);
int cvodeJacDense (long int Neq, realtype ttime, N_Vector state, N_Vector dstate_dt, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#ifdef KLU_ENABLE
int cvodeJacSparse (realtype ttime, N_Vector state, N_Vector dstate_dt, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#endif
int cvodeRootFunc (realtype ttime, N_Vector state, realtype *gout, void *user_data);

cvodeInterface::cvodeInterface(const std::string &objName) : sundialsInterface(objName)
{
    mode.dynamic = true;
    mode.differential = true;
    mode.algebraic = false;
}

cvodeInterface::cvodeInterface (gridDynSimulation *gds, const solverMode& sMode) : sundialsInterface (gds, sMode)
{
    mode.dynamic = true;
    mode.differential = true;
    mode.algebraic = false;
}


cvodeInterface::~cvodeInterface ()
{
  // clear variables for CVode to use
  if (initialized)
    {
      CVodeFree (&solverMem);
    }
}

std::shared_ptr<solverInterface> cvodeInterface::clone(std::shared_ptr<solverInterface> si, bool fullCopy) const
{
    auto rp = cloneBaseStack<cvodeInterface, sundialsInterface, solverInterface>(this, si, fullCopy);
    if (!rp)
    {
        return si;
    }

    rp->use_bdf = use_bdf;
    rp->use_newton = use_newton;
    return rp;
}

void cvodeInterface::allocate (count_t stateCount, count_t numRoots)
{
  // load the vectors
  if (stateCount == svsize)
    {
      return;
    }
  initialized = false;
  a1.setRowLimit (stateCount);
  a1.setColLimit (stateCount);

  //update the rootCount
  rootCount = numRoots;
  rootsfound.resize (numRoots);

  //allocate the solverMemory
  if (solverMem)
    {
      CVodeFree (&(solverMem));
    }
  solverMem = CVodeCreate (CV_ADAMS,CV_FUNCTIONAL);
  check_flag(solverMem, "CVodeCreate", 0);

  sundialsInterface::allocate(stateCount, numRoots);

}


void cvodeInterface::setMaxNonZeros (count_t nonZeroCount)
{
    maxNNZ = nonZeroCount;
  a1.reserve (nonZeroCount);
  a1.clear ();
}


void cvodeInterface::set (const std::string &param, const std::string &val)
{

  if (param == "mode")
    {
      auto v2 = splitlineTrim (convertToLowerCase (val));
      for (const auto &str : v2)
        {
          if (str == "bdf")
            {
              use_bdf = true;
            }
          else if (str == "adams")
            {
              use_bdf = false;
            }
          else if (str == "functional")
            {
              use_newton = false;
            }
          else if (str == "newton")
            {
              use_newton = true;
            }
          else
            {
			  throw(invalidParameterValue());
            }
        }
    }
  else
    {
      solverInterface::set (param, val);
    }

}


void cvodeInterface::set (const std::string &param, double val)
{

  bool checkStepUpdate = false;
  if (param == "step")
    {
     
        if ((maxStep<0)||(maxStep==step))
        {
            maxStep=val;
        }
        if ((minStep<0) || (minStep == step))
        {
            minStep = val;
        }
        step = val;
        checkStepUpdate = true;
    }
  else if (param=="maxstep")
  {
      maxStep = val;
      checkStepUpdate = true;
  }
  else if (param=="minstep")
  {
      minStep = val;
      checkStepUpdate = true;
  }
  else
    {
      solverInterface::set (param, val);
    }
    if (checkStepUpdate)
    {
        if (initialized)
        {
            CVodeSetMaxStep(solverMem, maxStep);
            CVodeSetMinStep(solverMem, minStep);
            CVodeSetInitStep(solverMem, step);
        }
    }

}

double cvodeInterface::get (const std::string &param) const
{
  long int val = -1;
  if ((param == "resevals") || (param == "iterationcount"))
    {
      //	CVodeGetNumResEvals(solverMem, &val);
    }
  else if (param == "iccount")
    {
      val = icCount;
    }
  else if (param == "jac calls")
    {
#ifdef KLU_ENABLE
      //	CVodeCVodeSlsGetNumJacEvals(solverMem, &val);
#else
      CVodeDlsGetNumJacEvals (solverMem, &val);
#endif
    }
  else
    {
      return sundialsInterface::get(param);
    }

  return static_cast<double> (val);
}


// output solver stats
void cvodeInterface::logSolverStats (int logLevel, bool /*iconly*/) const
{
  if (!initialized)
    {
      return;
    }
  long int nni = 0;
  int klast, kcur;
  long int nst, nre, netf, ncfn, nge;
  realtype tolsfac, hlast, hcur;


  int retval = CVodeGetNumRhsEvals (solverMem, &nre);
  check_flag (&retval, "CVodeGetNumRhsEvals", 1);
  retval = CVodeGetNumNonlinSolvIters (solverMem, &nni);
  check_flag (&retval, "CVodeGetNumNonlinSolvIters", 1);
  retval = CVodeGetNumNonlinSolvConvFails (solverMem, &ncfn);
  check_flag (&retval, "CVodeGetNumNonlinSolvConvFails", 1);

  retval = CVodeGetNumSteps (solverMem, &nst);
  check_flag (&retval, "CVodeGetNumSteps", 1);
  retval = CVodeGetNumErrTestFails (solverMem, &netf);
  check_flag (&retval, "CVodeGetNumErrTestFails", 1);

  retval = CVodeGetNumGEvals (solverMem, &nge);
  check_flag (&retval, "CVodeGetNumGEvals", 1);
  CVodeGetCurrentOrder (solverMem, &kcur);
  check_flag(&retval, "VodeGetCurrentOrder", 1);
  CVodeGetCurrentStep (solverMem, &hcur);
  check_flag(&retval, "CVodeGetCurrentStep", 1);
  CVodeGetLastOrder (solverMem, &klast);
  check_flag(&retval, " CVodeGetLastOrder", 1);
  CVodeGetLastStep (solverMem, &hlast);
  check_flag(&retval, "CVodeGetLastStep", 1);
  CVodeGetTolScaleFactor (solverMem, &tolsfac);
  check_flag(&retval, "CVodeGetTolScaleFactor", 1);

  std::string logstr = "CVode Run Statistics: \n";

  logstr += "Number of steps                    = " + std::to_string (nst) + '\n';
  logstr += "Number of residual evaluations     = " + std::to_string (nre) + '\n';
  logstr += "Number of Jacobian evaluations     = " + std::to_string (jacCallCount) + '\n';
  logstr += "Number of nonlinear iterations     = " + std::to_string (nni) + '\n';
  logstr += "Number of error test failures      = " + std::to_string (netf) + '\n';
  logstr += "Number of nonlinear conv. failures = " + std::to_string (ncfn) + '\n';
  logstr += "Number of root fn. evaluations     = " + std::to_string (nge) + '\n';
  logstr += "Current order used                 = " + std::to_string (kcur) + '\n';
  logstr += "Current step                       = " + std::to_string (hcur) + '\n';
  logstr += "Last order used                    = " + std::to_string (klast) + '\n';
  logstr += "Last step                          = " + std::to_string (hlast) + '\n';
  logstr += "Tolerance scale factor             = " + std::to_string (tolsfac) + '\n';


  if (m_gds)
    {
      m_gds->log (m_gds, logLevel, logstr);
    }
  else
    {
      printf ("\n%s", logstr.c_str ());
    }
}


void cvodeInterface::logErrorWeights (int logLevel) const
{

    N_Vector eweight = NVECTOR_NEW(use_omp, svsize);
    N_Vector ele = NVECTOR_NEW(use_omp, svsize);
  realtype *eldata = NVECTOR_DATA (use_omp,ele);
  realtype *ewdata = NVECTOR_DATA (use_omp,eweight);
  int retval = CVodeGetErrWeights (solverMem, eweight);
  check_flag (&retval, "CVodeGetErrWeights", 1);
  retval = CVodeGetEstLocalErrors (solverMem, ele);
  check_flag (&retval, "CVodeGetEstLocalErrors", 1);
  std::string logstr = "Error Weight\tEstimated Local Errors\n";
  for (size_t kk = 0; kk < svsize; ++kk)
    {
      logstr += std::to_string (kk) + ':' + std::to_string (ewdata[kk]) + '\t' + std::to_string (eldata[kk]) + '\n';
    }

  if (m_gds)
    {
      m_gds->log (m_gds, logLevel, logstr);
    }
  else
    {
      printf ("\n%s", logstr.c_str ());
    }
  NVECTOR_DESTROY(use_omp, eweight);
  NVECTOR_DESTROY(use_omp, ele);
}

/* *INDENT-OFF* */
static const std::map<int, std::string> cvodeRetCodes {
  {CV_MEM_NULL, "The solver memory argument was NULL"},
  {CV_ILL_INPUT, "One of the function inputs is illegal"},
  {CV_NO_MALLOC, "The solver memory was not allocated by a call to CVodeMalloc"},
  {CV_TOO_MUCH_WORK, "The solver took mxstep internal steps but could not reach tout"},
  {CV_TOO_MUCH_ACC, "The solver could not satisfy the accuracy demanded by the user for some internal step"},
  {CV_TOO_CLOSE, "t0 and tout are too close and user didn't specify a step size" },
  {CV_LINIT_FAIL, "The linear solver's initialization function failed" },
  {CV_LSETUP_FAIL, "The linear solver's setup function failed in an unrecoverable manner" },
  {CV_LSOLVE_FAIL, "The linear solver's solve function failed in an unrecoverable manner" },
  {CV_ERR_FAILURE, "The error test occurred too many times" },
  {CV_MEM_FAIL, "A memory allocation failed" },
  {CV_CONV_FAILURE, "convergence test failed too many times" },
  {CV_BAD_T, "The time t is outside the last step taken" },
  {CV_FIRST_RHSFUNC_ERR, "The user - provided rhs function failed recoverably on the first call" },
  {CV_REPTD_RHSFUNC_ERR, "convergence test failed with repeated recoverable errors in the rhs function" },
  {CV_RTFUNC_FAIL, "The rootfinding function failed in an unrecoverable manner" },
  {CV_UNREC_RHSFUNC_ERR, "The user-provided right hand side function repeatedly returned a recoverable error flag, but the solver was unable to recover" },
  {CV_BAD_K, "Bad K" },
  {CV_BAD_DKY, "Bad DKY" },
};
/* *INDENT-ON* */

void cvodeInterface::initialize (double t0)
{
  if (!allocated)
    {
	  throw(InvalidSolverOperation());
    }

  auto jsize = m_gds->jacSize (mode);

  // initializeB CVode - Sundials

  int retval = CVodeSetUserData (solverMem, (void *)this);
  check_flag(&retval, "CVodeSetUserData", 1);

  //guess an initial condition
  m_gds->guess (t0, state_data(), deriv_data(), mode);

  retval = CVodeInit (solverMem, cvodeFunc, t0, state);
  check_flag(&retval, "CVodeInit", 1);

  if (rootCount > 0)
    {
      rootsfound.resize (rootCount);
      retval = CVodeRootInit (solverMem, rootCount, cvodeRootFunc);
	  check_flag(&retval, "CVodeRootInit", 1);
    }

  N_VConst (tolerance, abstols);

  retval = CVodeSVtolerances (solverMem, tolerance / 100, abstols);
  check_flag(&retval, "CVodeSVtolerances", 1);

  retval = CVodeSetMaxNumSteps (solverMem, 1500);
  check_flag(&retval, "CVodeSetMaxNumSteps", 1);

#ifdef KLU_ENABLE
  if (dense)
    {
      retval = CVDense (solverMem, svsize);
	  check_flag(&retval, "CVDense", 1);


      retval = CVDlsSetDenseJacFn (solverMem, cvodeJacDense);
	  check_flag(&retval, "CVDlsSetDenseJacFn", 1);

    }
  else
    {

      retval = CVKLU (solverMem, svsize, jsize);
	  check_flag(&retval, "CVodeKLU", 1);


      retval = CVSlsSetSparseJacFn (solverMem, cvodeJacSparse);
	  check_flag(&retval, "CVSlsSetSparseJacFn", 1);

    }
#else
  retval = CVDense (solverMem, svsize);
  check_flag(&retval, "CVDense", 1);

  retval = CVDlsSetDenseJacFn (solverMem, cvodeJacDense);
  check_flag(&retval, "CVDlsSetDenseJacFn", 1);

#endif





  retval = CVodeSetMaxNonlinIters (solverMem, 20);
  check_flag(&retval, "CVodeSetMaxNonlinIters", 1);



  retval = CVodeSetErrHandlerFn (solverMem, sundialsErrorHandlerFunc, (void *)this);
  check_flag(&retval, "CVodeSetErrHandlerFn", 1);


  if (maxStep > 0.0)
  {
      retval = CVodeSetMaxStep(solverMem, maxStep);
	  check_flag(&retval, "CVodeSetMaxStep", 1);

  }
  if (minStep > 0.0)
  {
      retval = CVodeSetMinStep(solverMem, minStep);
	  check_flag(&retval, "CVodeSetMinStep", 1);

  }
  if (step > 0.0)
  {
      retval = CVodeSetInitStep(solverMem, step);
	  check_flag(&retval, "CVodeSetInitStep", 1);

  }
  setConstraints ();

  initialized = true;


}

void cvodeInterface::sparseReInit (sparse_reinit_modes reInitMode)
{
#ifdef KLU_ENABLE
  int kinmode = (reInitMode == sparse_reinit_modes::refactor) ? 1 : 2;
  int retval = CVKLUReInit (solverMem, static_cast<int> (svsize), static_cast<int> (a1.capacity ()), kinmode);
  check_flag(&retval, "KINKLUReInit", 1);

#endif
}


void cvodeInterface::setRootFinding (count_t numRoots)
{
  if (numRoots != rootsfound.size ())
    {
      rootsfound.resize (numRoots);
    }
  rootCount = numRoots;
  int retval = CVodeRootInit (solverMem, numRoots, cvodeRootFunc);
  check_flag(&retval, "CVodeRootInit", 1);


}

void cvodeInterface::getCurrentData ()
{
  /*
  int retval = CVodeGetConsistentIC(solverMem, state, deriv);
  if (check_flag(&retval, "CVodeGetConsistentIC", 1))
  {
          return(retval);
  }
  */
}

int cvodeInterface::solve (double tStop, double &tReturn, step_mode stepMode)
{
  assert (rootCount == m_gds->rootSize (mode));
  ++solverCallCount;
  icCount = 0;
  
  int retval = CVode (solverMem, tStop, state, &tReturn, (stepMode == step_mode::normal) ? CV_NORMAL : CV_ONE_STEP);
  check_flag (&retval, "CVodeSolve", 1, false);
  if (retval == CV_ROOT_RETURN)
    {
      retval = SOLVER_ROOT_FOUND;
    }
    if (retval>=0)
    {
        //get the derivative information
        CVodeGetDky(solverMem, tStop, 1, dstate_dt);
    }
  return retval;
}

void cvodeInterface::getRoots ()
{
  int ret = CVodeGetRootInfo (solverMem, rootsfound.data ());
  check_flag(&ret, "CVodeGetRootInfo", 1);
}


void cvodeInterface::loadMaskElements ()
{
  std::vector<double> mStates (svsize, 0.0);
  m_gds->getVoltageStates (mStates.data (), mode);
  m_gds->getAngleStates (mStates.data (), mode);
  maskElements = vecFindgt<double, index_t> (mStates, 0.5);
  tempState.resize (svsize);
  double *lstate = NV_DATA_S (state);
  for (auto &v : maskElements)
    {
      tempState[v] = lstate[v];
    }
}



// CVode C Functions
int cvodeFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, void *user_data)
{

  cvodeInterface *sd = reinterpret_cast<cvodeInterface *> (user_data);
  sd->funcCallCount++;
    if (sd->mode.pairedOffsetIndex!=kNullLocation)
    {
		int ret = sd->m_gds->dynAlgebraicSolve(ttime, NVECTOR_DATA(sd->use_omp, state), NVECTOR_DATA(sd->use_omp, dstate_dt), sd->mode);
		if (ret < FUNCTION_EXECUTION_SUCCESS)
		{
			return ret;
		}
        
    }
  int ret = sd->m_gds->derivativeFunction (ttime, NVECTOR_DATA (sd->use_omp,state), NVECTOR_DATA (sd->use_omp, dstate_dt), sd->mode);

  if (sd->fileCapture)
  {
      if (!sd->stateFile.empty())
      {
          writeVector(ttime, STATE_INFORMATION, sd->funcCallCount, sd->mode.offsetIndex, sd->svsize, NVECTOR_DATA(sd->use_omp, state), sd->stateFile, (sd->funcCallCount != 1));
          writeVector(ttime, DERIVATIVE_INFORMATION, sd->funcCallCount, sd->mode.offsetIndex, sd->svsize, NVECTOR_DATA(sd->use_omp, dstate_dt), sd->stateFile);
      }
}

  return ret;
}

int cvodeRootFunc (realtype ttime, N_Vector state, realtype *gout, void *user_data)
{
    cvodeInterface *sd = reinterpret_cast<cvodeInterface *> (user_data);
    sd->m_gds->rootFindingFunction(ttime, NVECTOR_DATA(sd->use_omp, state), sd->deriv_data(), gout, sd->mode);

  return FUNCTION_EXECUTION_SUCCESS;
}

#define CHECK_JACOBIAN 0
int cvodeJacDense (long int /*Neq*/, realtype ttime, N_Vector state, N_Vector dstate_dt, DlsMat J, void *user_data, N_Vector /*tmp1*/, N_Vector /*tmp2*/, N_Vector /*tmp3*/)
{
  index_t kk;
  cvodeInterface *sd = reinterpret_cast<cvodeInterface *> (user_data);

  matrixDataSparse<double> *a1 = &(sd->a1);
  sd->m_gds->jacobianFunction (ttime, NVECTOR_DATA (sd->use_omp, state), NVECTOR_DATA (sd->use_omp, dstate_dt),a1, 0, sd->mode);

  
  if (sd->useMask)
    {
      for (auto &v : sd->maskElements)
        {
          a1->translateRow (v, kNullLocation);
          a1->assign (v, v, 100);
        }
      a1->filter ();
    }

  //assign the elements
  for (kk = 0; kk < a1->size (); ++kk)
    {
      DENSE_ELEM (J, a1->rowIndex (kk), a1->colIndex (kk)) = DENSE_ELEM (J, a1->rowIndex (kk), a1->colIndex (kk)) + a1->val (kk);
    }

#if (CHECK_JACOBIAN > 0)
  auto mv = findMissing (a1);
  for (auto &me : mv)
    {
      printf ("no entries for element %d\n", me);
    }
#endif
  return FUNCTION_EXECUTION_SUCCESS;
}

//#define CAPTURE_JAC_FILE

#ifdef KLU_ENABLE
int cvodeJacSparse (realtype ttime, N_Vector state, N_Vector dstate_dt, SlsMat J, void *user_data, N_Vector, N_Vector, N_Vector)
{
  count_t kk;
  count_t colval;

  cvodeInterface *sd = reinterpret_cast<cvodeInterface *> (user_data);

  matrixDataSparse<double> *a1 = &(sd->a1);

  sd->m_gds->jacobianFunction (ttime, NVECTOR_DATA (sd->use_omp, state), NVECTOR_DATA (sd->use_omp, dstate_dt), a1,0, sd->mode);
  a1->sortIndexCol ();
  if (sd->useMask)
    {
      for (auto &v : sd->maskElements)
        {
          a1->translateRow (v, kNullLocation);
          a1->assign (v, v, 1);
        }
      a1->filter ();
      a1->sortIndexCol ();
    }
  a1->compact ();

  SlsSetToZero (J);

  colval = 0;
  J->colptrs[0] = colval;
  for (kk = 0; kk < a1->size (); ++kk)
    {
      //	  printf("kk: %d  dataval: %f  rowind: %d   colind: %d \n ", kk, a1->val(kk), a1->rowIndex(kk), a1->colIndex(kk));
      if (a1->colIndex (kk) > colval)
        {
          colval++;
          J->colptrs[colval] = static_cast<int> (kk);
        }
      J->data[kk] = a1->val (kk);
      J->rowvals[kk] = a1->rowIndex (kk);
    }
  J->colptrs[colval + 1] = static_cast<int> (a1->size ());

#ifdef CAPTURE_JAC_FILE
  a1->saveFile (ttime, "jac_new.dat", true);
#endif
#if (CHECK_JACOBIAN > 0)
  auto mv = findMissing (a1);
  for (auto &me : mv)
    {
      printf ("no entries for element %d\n", me);
    }
#endif
  return 0;
}
#endif

