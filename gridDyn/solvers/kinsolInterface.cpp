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
#include "simulation/gridDynSimulationFileOps.h"
#include "sundialsArrayData.h"
//#include "arrayDataBoost.h"
#include "arrayDataSparseSM.h"
#include "core/helperTemplates.h"
#include <sundials/sundials_math.h>
#include <kinsol/kinsol.h>
#include <kinsol/kinsol_dense.h>

#ifdef KLU_ENABLE
#include <kinsol/kinsol_klu.h>
#include <kinsol/kinsol_sparse.h>
#endif

#if MEASURE_TIMINGS > 0
#include <chrono>
#endif

#include <cstdio>
#include <algorithm>
#include <string>
#include <map>
#include <cassert>


int kinsolFunc (N_Vector u, N_Vector f, void *user_data);
int kinsolJacDense (long int N, N_Vector u, N_Vector f, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);
#ifdef KLU_ENABLE
int kinsolJacSparse (N_Vector u, N_Vector f, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);
#endif
//int kinsolAlgFunc (N_Vector u, N_Vector f, void *user_data);
//int kinsolAlgJacDense (long int N, N_Vector u, N_Vector f, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);

kinsolInterface::kinsolInterface ()
{
  tolerance = 1e-8;
}

kinsolInterface::kinsolInterface (gridDynSimulation *gds, const solverMode& sMode) : sundialsInterface (gds, sMode)
{
  tolerance = 1e-8;
}

kinsolInterface::~kinsolInterface ()
{
  // clear variables for IDA to use
  if (initialized)
    {
      if (m_kinsolInfoFile)
        {
          fclose (m_kinsolInfoFile);
        }

      KINFree (&solverMem);
    }
}

std::shared_ptr<solverInterface> kinsolInterface::clone(std::shared_ptr<solverInterface> si, bool fullCopy) const
{
	auto rp = cloneBaseStack<kinsolInterface, sundialsInterface,solverInterface>(this, si, fullCopy);
	if (!rp)
	{
		return si;
	}
	
	if (fullCopy)
	{
		rp->fileCapture = fileCapture;
		rp->jacFile = jacFile;
		rp->stateFile = stateFile;
	}
	return rp;
}

int kinsolInterface::allocate (count_t stateCount, count_t /*numroots*/)
{
  // load the vectors
  if (stateCount == svsize)
    {
      return FUNCTION_EXECUTION_SUCCESS;
    }
  
  if (solverMem)
    {
      KINFree (&(solverMem));
    }
  solverMem = KINCreate ();
  if (check_flag (solverMem, "KINCreate", 0))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  return sundialsInterface::allocate(stateCount, 0);
}


// output solver stats
void kinsolInterface::logSolverStats (int logLevel, bool /*iconly*/) const
{
  if (!initialized)
    {
      return;
    }
  long int nni = 0, nfe = 0, nje = 0, nfeD = 0;

  int flag = KINGetNumNonlinSolvIters (solverMem, &nni);
  check_flag (&flag, "KINGetNumNonlinSolvIters", 1);
  flag = KINGetNumFuncEvals (solverMem, &nfe);
  check_flag (&flag, "KINGetNumFuncEvals", 1);

  if (dense)
    {
      flag = KINDlsGetNumJacEvals (solverMem, &nje);
      check_flag (&flag, "KINDlsGetNumJacEvals", 1);
      flag = KINDlsGetNumFuncEvals (solverMem, &nfeD);
      check_flag (&flag, "KINDlsGetNumFuncEvals", 1);
    }
#ifdef KLU_ENABLE
  else
    {
      flag = KINSlsGetNumJacEvals (solverMem, &nje);
      check_flag (&flag, "KINSlsGetNumJacEvals", 1);
      nfeD = -1;
    }
#else
  else
    {
      flag = KINDlsGetNumJacEvals (solverMem, &nje);
      check_flag (&flag, "KINDlsGetNumJacEvals", 1);
      flag = KINDlsGetNumFuncEvals (solverMem, &nfeD);
      check_flag (&flag, "KINDlsGetNumFuncEvals", 1);
    }
#endif
  flag = KINDlsGetNumJacEvals (solverMem, &nje);
  check_flag (&flag, "KINDlsGetNumJacEvals", 1);
  flag = KINDlsGetNumFuncEvals (solverMem, &nfeD);
  check_flag (&flag, "KINDlsGetNumFuncEvals", 1);

  std::string logstr = "Kinsoln Statistics: \n";
  logstr += "Number of nonlinear iterations    = " + std::to_string (nni) + '\n';
  logstr += "Number of function evaluations    = " + std::to_string (nfe) + '\n';
  logstr += "Number of Jacobian evaluations    = " + std::to_string (nje) + '\n';
  if (nfeD > 0)
    {
      logstr += "Number of Jacobian function calls = " + std::to_string (nfeD) + '\n';
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

static const std::map<int, std::string> kinRetCodes {
  {
    KIN_MEM_NULL, "Null solver Memory"
  },
  {
    KIN_ILL_INPUT, "Illegal Input"
  },
  {
    KIN_NO_MALLOC, " No memory allocation"
  },
  {
    KIN_MEM_FAIL, "Memory Allocation failed"
  },
  {
    KIN_LINESEARCH_NONCONV, "linesearch failed to converge"
  },
  {
    KIN_MAXITER_REACHED, " Max iteration reached"
  },
  {
    KIN_MXNEWT_5X_EXCEEDED, "Five consecutive steps have been taken that satisfy a scaled step length test"
  },
  {
    KIN_LINESEARCH_BCFAIL, "The linesearch algorithm was unable to satisfy the beta -condition for nbcfails iterations"
  },
  {
    KIN_LINSOLV_NO_RECOVERY, "The user - supplied routine preconditioner slve function failed recoverably, but the preconditioner is already current"
  },
  {
    KIN_LINIT_FAIL, "The linear solver's initialization function failed"
  },
  {
    KIN_LSETUP_FAIL, "The linear solver's setup function failed in an unrecoverable manner"
  },
  {
    KIN_LSOLVE_FAIL, "The linear solver's solve function failed in an unrecoverable manner"
  },
  {
    KIN_SYSFUNC_FAIL, "The system function failed in an unrecoverable manner"
  },
  {
    KIN_FIRST_SYSFUNC_ERR, "The system function failed recoverably at the first call"
  },
  {
    KIN_REPTD_SYSFUNC_ERR, "The system function had repeated recoverable errors"
  },
};


int kinsolInterface::initialize (double /*t0*/)
{
  if (!allocated)
    {
      printf ("ERROR,  kinsol data not allocated\n");
      return -2;
    }

  int retval;

  if (!(solverLogFile.empty ()))
    {
      m_kinsolInfoFile = fopen (solverLogFile.c_str (), "w");
    }
  else
    {
      solverLogFile = "kinsol.out";
      m_kinsolInfoFile = fopen ("kinsol.out", "w");
    }


  retval = KINSetInfoFile (solverMem, m_kinsolInfoFile);
  if (check_flag (&retval, "KINSetInfoFile", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  retval = KINSetPrintLevel (solverMem, 3);
  if (check_flag (&retval, "KINSetPrintLevel", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  retval = KINSetUserData (solverMem, (void *)(this));
  if (check_flag (&retval, "KINSetUserData", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  //retval = KINSetFuncNormTol (solverMem, 1.e-9);
  retval = KINSetFuncNormTol (solverMem, tolerance);
  if (check_flag (&retval, "KINSetFuncNormTol", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  //retval = KINSetScaledStepTol (solverMem, 1.e-9);
  retval = KINSetScaledStepTol (solverMem, tolerance / 100);
  if (check_flag (&retval, "KINSetScaledStepTol", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }


  retval = KINSetNoInitSetup (solverMem, true);

  retval = KINInit (solverMem, kinsolFunc, state);

  if (check_flag (&retval, "KINInit", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

#ifdef KLU_ENABLE
  jacCallCount = 0;
  if (dense)
    {
      retval = KINDense (solverMem, svsize);
      if (check_flag (&retval, "KINDense", 1))
        {
          return FUNCTION_EXECUTION_FAILURE;
        }

      retval = KINDlsSetDenseJacFn (solverMem, kinsolJacDense);
      if (check_flag (&retval, "KINDlsSetDenseJacFn", 1))
        {
          return FUNCTION_EXECUTION_FAILURE;
        }
    }
  else
    {
      auto jsize = m_gds->jacSize (mode);
      retval = KINKLU (solverMem, static_cast<int> (svsize), static_cast<int> (jsize));
      if (check_flag (&retval, "KINKLU", 1))
        {
          return FUNCTION_EXECUTION_FAILURE;
        }

      retval = KINSlsSetSparseJacFn (solverMem, kinsolJacSparse);
      if (check_flag (&retval, "KINSlsSetSpasreJacFn", 1))
        {
          return FUNCTION_EXECUTION_FAILURE;
        }
      retval = KINKLUSetOrdering (solverMem, 0); //SET to AMD istead of COLAMD
      if (check_flag (&retval, "KINKLUSetOrdering", 1))
        {
          return FUNCTION_EXECUTION_FAILURE;
        }
    }
#else
  retval = KINDense (solverMem, svsize);
  if (check_flag (&retval, "KINDense", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  retval = KINDlsSetDenseJacFn (solverMem, kinsolJacDense);
  if (check_flag (&retval, "KINDlsSetDenseJacFn", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }
#endif

  retval = KINSetMaxSetupCalls (solverMem, 1);         // exact Newton
  if (check_flag (&retval, "KINSetMaxSetupCalls", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  retval = KINSetMaxSubSetupCalls (solverMem, 2);      // residual calls
  if (check_flag (&retval, "KINSetMaxSubSetupCalls", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  retval = KINSetNumMaxIters (solverMem, 50);               // residual calls
  if (check_flag (&retval, "KINSetNumMaxIters", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  retval = KINSetErrHandlerFn (solverMem, sundialsErrorHandlerFunc, (void *)this);
  if (check_flag (&retval, "KINSetErrHandlerFn", 1))
    {
      return(FUNCTION_EXECUTION_FAILURE);
    }
  initialized = true;


  return FUNCTION_EXECUTION_SUCCESS;
}


int kinsolInterface::sparseReInit (sparse_reinit_modes sparseReinitMode)
{
#ifdef KLU_ENABLE
  int retval;
  jacCallCount = 0;
  int kinmode = (sparseReinitMode == sparse_reinit_modes::refactor) ? 1 : 2;
  retval = KINKLUReInit (solverMem, static_cast<int> (svsize), maxNNZ, kinmode);
  if (check_flag (&retval, "KINKLUReInit", 1))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }
#endif
  return FUNCTION_EXECUTION_SUCCESS;

}
int kinsolInterface::set(const std::string &param, const std::string &val)
{
	int out = PARAMETER_FOUND;
	if (param == "jacfile")
	{
		jacFile = val;
	}
	else if (param == "statefile")
	{
		stateFile = val;
	}
	if (param == "capturefile")
	{
		jacFile = val;
		stateFile = val;
	}
	else
	{
		out = solverInterface::set(param, val);
	}
	return out;
}


int kinsolInterface::set(const std::string &param, double val)
{
	int out = PARAMETER_FOUND;
	if (param == "filecapture")
	{
		fileCapture = (val >= 0.1);
	}
	else
	{
		out = solverInterface::set(param, val);
	}
	return out;
}


double kinsolInterface::get (const std::string &param) const
{
  long int val = -1;
 if (param == "jac calls")
    {
      if (dense)
        {
          KINDlsGetNumJacEvals (solverMem, &val);
        }
      else
        {
#ifdef KLU_ENABLE
          KINSlsGetNumJacEvals (solverMem, &val);
#else
          KINDlsGetNumJacEvals (solverMem, &val);
#endif
        }

    }
  else
  {
	  return sundialsInterface::get(param);
  }
  return static_cast<double> (val);
}


#define SHOW_MISSING_ELEMENTS 0

//#define KIN_NONE       0
//#define KIN_LINESEARCH 1
//#define KIN_PICARD     2
//#define KIN_FP         3
int kinsolInterface::solve (double tStop, double &tReturn, step_mode /*mode*/)
{
  solveTime = tStop;
#if MEASURE_TIMINGS > 0
  auto start_t = std::chrono::high_resolution_clock::now ();

  int retval = KINSol (solverMem, state, KIN_NONE, scale, scale);
  auto stop_t = std::chrono::high_resolution_clock::now ();
  std::chrono::duration<double> elapsed_t = stop_t - start_t;
  kinTime += elapsed_t.count ();
  printf ("total solve time %f, %5.3f%% in resid %5.3f%% in Jacobian\n", kinTime, residTime / kinTime * 100.0, jacTime / kinTime * 100);
#else
  int retval = KINSol (solverMem, state, KIN_NONE, scale, scale);
#endif

#if SHOW_MISSING_ELEMENTS > 0
  if (retval == -11)
    {

      auto mvec = findMissing (&a1);
      if (mvec.size () > 0)
        {
          stringVec sL;
          m_gds->getStateName (sL, mode);
          for (auto mv : mvec)
            {
              printf ("state[%d]%s following state %s is singular\n", mv, sL[mv].c_str (), sL[mv - 1].c_str ());
            }
        }
      else
        {
          //stringVec sL;
          //m_gds->getStateName(sL, mode);
          //auto mrvec=findRank(&a1);
        }

    }
  else if (retval < 0)
    {
      auto mvec = findMissing (&a1);
    }
#endif
  tReturn = (retval >= 0) ? solveTime : m_gds->getCurrentTime ();
  ++solverCallCount;
  if (retval == KIN_REPTD_SYSFUNC_ERR)
    {
      retval = SOLVER_INVALID_STATE_ERROR;
    }
  return retval;
}

void kinsolInterface::setConstraints ()
{
  if (m_gds->hasConstraints ())
    {
      N_VConst (ZERO, consData);
      m_gds->getConstraints (NVECTOR_DATA (use_omp, consData), mode);
      KINSetConstraints (solverMem, consData);
    }
}


// function not in the class
// KINSOL C functions

int kinsolFunc (N_Vector u, N_Vector f, void *user_data)
{
  kinsolInterface *sd = reinterpret_cast<kinsolInterface *> (user_data);
  sd->funcCallCount++;
#if MEASURE_TIMINGS > 0
  auto start_t = std::chrono::high_resolution_clock::now ();

  int ret = sd->m_gds->residualFunction (sd->m_gds->getCurrentTime (), NVECTOR_DATA (sd->use_omp, u), nullptr, NVECTOR_DATA (sd->use_omp, f), sd->mode);
  auto stop_t = std::chrono::high_resolution_clock::now ();
  std::chrono::duration<double> elapsed_t = stop_t - start_t;
  sd->residTime += elapsed_t.count ();
#else
 
  int ret = sd->m_gds->residualFunction (sd->solveTime, NVECTOR_DATA (sd->use_omp, u), nullptr, NVECTOR_DATA (sd->use_omp, f), sd->mode);
#endif
  if (sd->printResid)
    {
      long int val = 0;
      KINGetNumNonlinSolvIters (sd->solverMem, &val);
      double *resid = NVECTOR_DATA (sd->use_omp, f);
      printf ("Residual for %s at time =%f iteration %ld\n", sd->name.c_str (), sd->solveTime,val);
      for (int kk = 0; kk < static_cast<int> (sd->svsize); ++kk)
        {
          printf ("resid[%u]=%f\n", kk, resid[kk]);
        }
      printf ("---------------------------------\n");

    }
  if (sd->fileCapture)
  {
	  if (!sd->stateFile.empty())
	  {
		  writeVector(sd->solveTime, 0, sd->funcCallCount, sd->mode.offsetIndex, sd->svsize, NVECTOR_DATA(sd->use_omp, u), sd->stateFile, (sd->funcCallCount != 1));
		  writeVector(sd->solveTime, 2, sd->funcCallCount, sd->mode.offsetIndex, sd->svsize, NVECTOR_DATA(sd->use_omp, f), sd->stateFile);
	  }
  }
  return ret;
}

int kinsolJacDense (long int Neq, N_Vector u, N_Vector /*f*/, DlsMat J, void *user_data, N_Vector /*tmp1*/, N_Vector /*tmp2*/)
{
  kinsolInterface *sd = reinterpret_cast<kinsolInterface *> (user_data);
  assert(Neq == static_cast<int> (sd->svsize));
  _unused(Neq);

  arrayDataSundialsDense a1 (J);
  sd->m_gds->jacobianFunction (sd->solveTime, NVECTOR_DATA (sd->use_omp, u), nullptr, &a1, 0, sd->mode);
  sd->jacCallCount++;
  return 0;
}

#ifdef KLU_ENABLE

int kinsolJacSparse (N_Vector u, N_Vector /*f*/, SlsMat J, void *user_data, N_Vector /*tmp1*/, N_Vector /*tmp2*/)
{

  kinsolInterface *sd = reinterpret_cast<kinsolInterface *> (user_data);
#if MEASURE_TIMINGS > 0
  auto start_t = std::chrono::high_resolution_clock::now ();
#endif
  if ((sd->jacCallCount == 0)||(!isSlsMatSetup (J)))
    {
      std::unique_ptr<arrayData<double>> a1;
      if (sd->svsize < 65535)
        {
          //arrayDataSparse *a1 = &(sd->a1);
          if (sd->svsize < 100)
            {
              a1.reset (new arrayDataSparseSMB<0, std::uint32_t> (sd->maxNNZ));
            }
          else if (sd->svsize < 1000)
            {
              a1.reset (new arrayDataSparseSMB<1, std::uint32_t> (sd->maxNNZ));
            }
          else if (sd->svsize < 20000)
            {
              a1.reset (new arrayDataSparseSMB<2, std::uint32_t> (sd->maxNNZ));
            }
          else
            {
              a1.reset (new arrayDataSparseSMB<2, std::uint32_t> (sd->maxNNZ));
            }

        }
      else
        {
          a1.reset ( new arrayDataSparseSMB<2, std::uint64_t> (sd->maxNNZ));
        }
      a1->setRowLimit (sd->svsize);
      a1->setColLimit (sd->svsize);

      sd->m_gds->jacobianFunction (sd->solveTime, NVECTOR_DATA (sd->use_omp, u), nullptr, a1.get (), 0, sd->mode);


      sd->jacCallCount++;
      arrayDataToSlsMat (a1.get (), J,sd->svsize);
      sd->nnz = a1->size ();
	  if (sd->fileCapture)
	  {
		  if (!sd->jacFile.empty())
		  {
			  long int val = 0;
			  KINGetNumNonlinSolvIters(sd->solverMem, &val);
			  writeArray(sd->solveTime,1, val, sd->mode.offsetIndex, a1.get(), sd->jacFile);
		  }
	  }
    }
  else
    {
      //if it isn't the first we can use the sundials arraySparse object
      arrayDataSundialsSparse a1 (J);
      sd->m_gds->jacobianFunction (sd->solveTime, NVECTOR_DATA (sd->use_omp, u), nullptr, &a1, 0, sd->mode);
      sd->jacCallCount++;
	  if (sd->fileCapture)
	  {
		  if (!sd->jacFile.empty())
		  {
			  long int val = 0;
			  KINGetNumNonlinSolvIters(sd->solverMem, &val);
			  writeArray(sd->solveTime,1, val, sd->mode.offsetIndex, &a1, sd->jacFile);
		  }
	  }
    }
  // for (kk = 0; kk<a1->points(); ++kk) {
  //   printf("kk: %d  J->data[kk]: %f  J->rowvals[kk]: %d \n ", kk, J->data[kk], J->rowvals[kk]);
  // }
  // for (kk = 0; kk<(colval+2); ++kk) {
  //   printf("kk: %d  : J->colptrs[kk]: %d \n ", kk, J->colptrs[kk]);
  // }
#if MEASURE_TIMINGS > 0
  auto stop_t = std::chrono::high_resolution_clock::now ();
  std::chrono::duration<double> elapsed_t = stop_t - start_t;
  sd->jacTime += elapsed_t.count ();
#endif
  return 0;
}

#endif
