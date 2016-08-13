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

#include <cvode/cvode.h>
#include <cvode/cvode_dense.h>
#include <sundials/sundials_math.h>


#ifdef KLU_ENABLE
#include <cvode/cvode_klu.h>
#include <cvode/cvode_sparse.h>
#endif

#include <cstdio>
#include <algorithm>
#include <string>
#include <cassert>
#include <map>


int cvodeFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, void *user_data);
int cvodeJacDense (long int Neq, realtype ttime, N_Vector state, N_Vector dstate_dt, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#ifdef KLU_ENABLE
int cvodeJacSparse (realtype ttime, N_Vector state, N_Vector dstate_dt, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#endif
int cvodeRootFunc (realtype ttime, N_Vector state, realtype *gout, void *user_data);

cvodeInterface::cvodeInterface ()
{

}

cvodeInterface::cvodeInterface (gridDynSimulation *gds, const solverMode& sMode) : solverInterface (gds, sMode)
{
}


cvodeInterface::~cvodeInterface ()
{
  // clear variables for CVode to use
  if (initialized)
    {
      NVECTOR_DESTROY (use_omp,state);
      NVECTOR_DESTROY (use_omp,abstols);
      NVECTOR_DESTROY (use_omp,scale);
      NVECTOR_DESTROY (use_omp, types);
      NVECTOR_DESTROY (use_omp, consData);
      NVECTOR_DESTROY (use_omp, deriv);
      NVECTOR_DESTROY (use_omp, eweight);
      NVECTOR_DESTROY (use_omp, ele);
      CVodeFree (&solverMem);
    }
}


int cvodeInterface::allocate (count_t size, count_t numRoots)
{
  // load the vectors
  if (size == svsize)
    {
      return 0;
    }
  initialized = false;
  if (state)
    {
      NVECTOR_DESTROY (use_omp,state);
    }

  state = NVECTOR_NEW (use_omp,size);
  if (check_flag ((void *)state, "NVECTOR_NEW", 0))
    {
      return(1);
    }

  if (scale)
    {
      NVECTOR_DESTROY (use_omp,scale);
    }
  scale = NVECTOR_NEW (use_omp, size);
  if (check_flag ((void *)scale, "NVECTOR_NEW", 0))
    {
      return(1);
    }
  N_VConst (ONE, scale);

  if (types)
    {
      NVECTOR_DESTROY (use_omp,types);
    }
  types = NVECTOR_NEW (use_omp, size);
  if (check_flag ((void *)types, "NVECTOR_NEW", 0))
    {
      return(1);
    }
  N_VConst (ONE, types);



  if (abstols)
    {
      NVECTOR_DESTROY (use_omp, abstols);
    }
  abstols = NVECTOR_NEW (use_omp, size);
  if (check_flag ((void *)abstols, "NVECTOR_NEW", 0))
    {
      return(1);
    }

  if (consData)
    {
      NVECTOR_DESTROY (use_omp, consData);
    }
  consData = NVECTOR_NEW (use_omp, size);
  if (check_flag ((void *)consData, "NVECTOR_NEW", 0))
    {
      return(1);
    }

  svsize = size;
  a1.setRowLimit (size);
  a1.setColLimit (size);

  if (deriv)
    {
      NVECTOR_DESTROY (use_omp, deriv);
    }
  deriv = NVECTOR_NEW (use_omp, svsize);
  if (check_flag ((void *)deriv, "NVECTOR_NEW", 0))
    {
      return(1);
    }
  N_VConst (ZERO, deriv);

  if (eweight)
    {
      NVECTOR_DESTROY (use_omp, eweight);
    }
  eweight = NVECTOR_NEW (use_omp, svsize);
  if (check_flag ((void *)eweight, "NVECTOR_NEW", 0))
    {
      return(1);
    }

  if (ele)
    {
      NVECTOR_DESTROY (use_omp, ele);
    }
  ele = NVECTOR_NEW (use_omp, svsize);
  if (check_flag ((void *)ele, "NVECTOR_NEW", 0))
    {
      return(1);
    }
  //update the rootCount
  rootCount = numRoots;
  rootsfound.resize (numRoots);

  //allocate the solverMemory
  if (solverMem)
    {
      CVodeFree (&(solverMem));
    }
  solverMem = CVodeCreate (CV_ADAMS,CV_FUNCTIONAL);
  if (check_flag (solverMem, "CVodeCreate", 0))
    {
      return(1);
    }



  allocated = true;

  return 0;
}


void cvodeInterface::setMaxNonZeros (count_t size)
{
  a1.reserve (size);
  a1.clear ();
}


double * cvodeInterface::state_data ()
{
  return NVECTOR_DATA (use_omp, state);
}
double * cvodeInterface::deriv_data ()
{
  return NVECTOR_DATA (use_omp, deriv);
}
double * cvodeInterface::type_data ()
{
  return NVECTOR_DATA (use_omp, types);
}

int cvodeInterface::set (const std::string &param, const std::string &val)
{
  int out = PARAMETER_FOUND;
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
              out = INVALID_PARAMETER_VALUE;
            }
        }
    }
  else
    {
      out = solverInterface::set (param, val);
    }
  return out;
}


int cvodeInterface::set (const std::string &param, double val)
{
  int out = PARAMETER_FOUND;
  if (param[0] == '#')
    {

    }
  else
    {
      out = solverInterface::set (param, val);
    }
  return out;
}

double cvodeInterface::get (const std::string &param) const
{
  long int val = -1;
  if ((param == "resevals") || (param == "iterationcount"))
    {
      //	CVodeGetNumResEvals(solverMem, &val);
    }
  else if (param == "solvercount")
    {
      val = solverCallCount;
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
      //CVodeDlsGetNumJacEvals(solverMem, &val);
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
  int retval, klast, kcur;
  long int nst, nre, netf, ncfn, nge;
  realtype tolsfac, hlast, hcur;

  std::string logstr = "";

  retval = CVodeGetNumRhsEvals (solverMem, &nre);
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
  CVodeGetCurrentStep (solverMem, &hcur);
  CVodeGetLastOrder (solverMem, &klast);
  CVodeGetLastStep (solverMem, &hlast);
  CVodeGetTolScaleFactor (solverMem, &tolsfac);

  logstr = "CVode Run Statistics: \n";

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
  realtype *eldata, *ewdata;

  eldata = NVECTOR_DATA (use_omp,ele);
  ewdata = NVECTOR_DATA (use_omp,eweight);
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
}


static const std::map<int, std::string> cvodeRetCodes {
  {
    CV_MEM_NULL, "The solver memory argument was NULL"
  },
  {
    CV_ILL_INPUT, "One of the function inputs is illegal"
  },
  {
    CV_NO_MALLOC, "The solver memory was not allocated by a call to CVodeMalloc"
  },
  {
    CV_TOO_MUCH_WORK, "The solver took mxstep internal steps but could not reach tout"
  },
  {
    CV_TOO_MUCH_ACC, "The solver could not satisfy the accuracy demanded by the user for some internal step"
  },
  {
    CV_TOO_CLOSE, "t0 and tout are too close and user didn't specify a step size"
  },
  {
    CV_LINIT_FAIL, "The linear solver's initialization function failed"
  },
  {
    CV_LSETUP_FAIL, "The linear solver's setup function failed in an unrecoverable manner"
  },
  {
    CV_LSOLVE_FAIL, "The linear solver's solve function failed in an unrecoverable manner"
  },
  {
    CV_ERR_FAILURE, "The error test occured too many times"
  },
  {
    CV_MEM_FAIL, "A memory allocation failed"
  },
  {
    CV_CONV_FAILURE, "convergence test failed too many times"
  },
  {
    CV_BAD_T, "The time t is outside the last step taken"
  },
  {
    CV_FIRST_RHSFUNC_ERR, "The user - provided rhs function failed recoverably on the first call"
  },
  {
    CV_REPTD_RHSFUNC_ERR, "convergence test failed with repeated recoverable erros in the rhs function"
  },

  {
    CV_RTFUNC_FAIL, "The rootfinding function failed in an unrecoverable manner"
  },
  {
    CV_UNREC_RHSFUNC_ERR, "The user-provided right hand side function repeatedly returned a recoverable error flag, but the solver was unable to recover"
  },
  {
    CV_BAD_K, "Bad K"
  },
  {
    CV_BAD_DKY, "Bad DKY"
  },
  {
    CV_BAD_DKY, "Bad DKY"
  },
};

int cvodeInterface::initialize (double t0)
{
  if (!allocated)
    {
      printf ("ERROR,  cvode data not allocated\n");
      return -2;
    }
  int retval;
  auto jsize = m_gds->jacSize (mode);

  // initializeB CVode - Sundials

  retval = CVodeSetUserData (solverMem, (void *)this);
  if (check_flag (&retval, "CVodeSetUserData", 1))
    {
      return(1);
    }

  //guess an initial condition
  m_gds->guess (t0, NVECTOR_DATA (use_omp,state), NVECTOR_DATA (use_omp,deriv), mode);

  retval = CVodeInit (solverMem, cvodeFunc, t0, state);
  if (check_flag (&retval, "CVodeInit", 1))
    {
      return(1);
    }

  if (rootCount > 0)
    {
      rootsfound.resize (rootCount);
      retval = CVodeRootInit (solverMem, rootCount, cvodeRootFunc);
      if (check_flag (&retval, "CVodeRootInit", 1))
        {
          return(1);
        }
    }

  N_VConst (tolerance, abstols);

  retval = CVodeSVtolerances (solverMem, tolerance / 100, abstols);
  if (check_flag (&retval, "CVodeSVtolerances", 1))
    {
      return(1);
    }

  retval = CVodeSetMaxNumSteps (solverMem, 1500);
  if (check_flag (&retval, "CVodeSetMaxNumSteps", 1))
    {
      return(1);
    }
#ifdef KLU_ENABLE
  if (dense)
    {
      retval = CVDense (solverMem, svsize);
      if (check_flag (&retval, "CVDense", 1))
        {
          return(1);
        }

      retval = CVDlsSetDenseJacFn (solverMem, cvodeJacDense);
      if (check_flag (&retval, "CVDlsSetDenseJacFn", 1))
        {
          return(1);
        }
    }
  else
    {

      retval = CVKLU (solverMem, svsize, jsize);
      if (check_flag (&retval, "CVodeKLU", 1))
        {
          return(1);
        }

      retval = CVSlsSetSparseJacFn (solverMem, cvodeJacSparse);
      if (check_flag (&retval, "CVSlsSetSparseJacFn", 1))
        {
          return(1);
        }
    }
#else
  retval = CVDense (solverMem, svsize);
  if (check_flag (&retval, "CVDense", 1))
    {
      return(1);
    }

  retval = CVDlsSetDenseJacFn (solverMem, cvodeJacDense);
  if (check_flag (&retval, "CVDlsSetDenseJacFn", 1))
    {
      return(1);
    }
#endif





  retval = CVodeSetMaxNonlinIters (solverMem, 20);
  if (check_flag (&retval, "CVodeSetMaxNonlinIters", 1))
    {
      return(1);
    }


  retval = CVodeSetErrHandlerFn (solverMem, sundialsErrorHandlerFunc, (void *)this);
  if (check_flag (&retval, "CVodeSetErrHandlerFn", 1))
    {
      return(1);
    }

  setConstraints ();

  initialized = true;
  return FUNCTION_EXECUTION_SUCCESS;

}

int cvodeInterface::sparseReInit (sparse_reinit_modes reInitMode)
{
#ifdef KLU_ENABLE
  int kinmode = (reInitMode == sparse_reinit_modes::refactor) ? 1 : 2;
  int retval = CVKLUReInit (solverMem, static_cast<int> (svsize), static_cast<int> (a1.capacity ()), kinmode);
  if (check_flag (&retval, "KINKLUReInit", 1))
    {
      return(FUNCTION_EXECUTION_FAILURE);
    }
#endif
  return FUNCTION_EXECUTION_SUCCESS;
}


int cvodeInterface::setRootFinding (count_t numRoots)
{
  if (numRoots != rootsfound.size ())
    {
      rootsfound.resize (numRoots);
    }
  rootCount = numRoots;
  int retval = CVodeRootInit (solverMem, numRoots, cvodeRootFunc);
  if (check_flag (&retval, "CVodeRootInit", 1))
    {
      return(retval);
    }
  return FUNCTION_EXECUTION_SUCCESS;
}

int cvodeInterface::getCurrentData ()
{
  /*
  int retval = CVodeGetConsistentIC(solverMem, state, deriv);
  if (check_flag(&retval, "CVodeGetConsistentIC", 1))
  {
          return(retval);
  }
  */
  return FUNCTION_EXECUTION_SUCCESS;
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
  return retval;
}

int cvodeInterface::getRoots ()
{
  int ret = CVodeGetRootInfo (solverMem, rootsfound.data ());
  if (!check_flag (&ret, "CVodeGetRootInfo", 1))
    {
      return ret;
    }
  return ret;
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

//#define CAPTURE_STATE_FILE

#ifdef CAPTURE_STATE_FILE
void saveStateFile (double time, count_t size, double *state, double *dstate, double *resid, std::string fname, bool append)
{
  std::ofstream  bFile;

  if (append)
    {
      bFile.open (fname.c_str (), std::ios::out | std::ios::binary | std::ios::app);
    }
  else
    {
      bFile.open (fname.c_str (), std::ios::out | std::ios::binary);
    }
  bFile.write ((char *)(&time), sizeof(double));
  bFile.write ((char *)(&size), sizeof(count_t));

  bFile.write ((char *)(state), sizeof(double) * size);
  bFile.write ((char *)(dstate), sizeof(double) * size);
  bFile.write ((char *)(resid), sizeof(double) * size);

  bFile.close ();
}
#endif

// CVode C Functions
int cvodeFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, void *user_data)
{
  cvodeInterface *sd = reinterpret_cast<cvodeInterface *> (user_data);
  //printf("time=%f\n", ttime);
  int ret = sd->m_gds->derivativeFunction (ttime, NVECTOR_DATA (sd->use_omp,state), NVECTOR_DATA (sd->use_omp, dstate_dt), sd->mode);

#ifdef CAPTURE_STATE_FILE
  saveStateFile (ttime, sd->svsize, NVECTOR_DATA (state), NVECTOR_DATA (dstate_dt), NVECTOR_DATA (resid), "jac_new_state.dat", true);
#endif
  return ret;
}

int cvodeRootFunc (realtype ttime, N_Vector state, realtype *gout, void *user_data)
{
  cvodeInterface *sd = reinterpret_cast<cvodeInterface *> (user_data);
  stateData sD;
  sD.time = ttime;
  sD.state = NVECTOR_DATA (sd->use_omp, state);
  sD.dstate_dt = NVECTOR_DATA (sd->use_omp, sd->deriv);
  sd->m_gds->rootTest (&sD, gout, sd->mode);

  return FUNCTION_EXECUTION_SUCCESS;
}

#define CHECK_JACOBIAN 0
int cvodeJacDense (long int Neq, realtype ttime, N_Vector state, N_Vector dstate_dt, DlsMat J, void *user_data, N_Vector /*tmp1*/, N_Vector /*tmp2*/, N_Vector /*tmp3*/)
{
  index_t kk;
  cvodeInterface *sd = reinterpret_cast<cvodeInterface *> (user_data);

  assert (Neq == static_cast<int> (sd->svsize));
  arrayDataSparse *a1 = &(sd->a1);
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

  arrayDataSparse *a1 = &(sd->a1);

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

