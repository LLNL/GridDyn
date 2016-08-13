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

#include <ida/ida.h>
#include <ida/ida_dense.h>
#include <sundials/sundials_math.h>


#ifdef KLU_ENABLE
#include <ida/ida_klu.h>
#include <ida/ida_sparse.h>
#endif

#include <cstdio>
#include <algorithm>
#include <string>
#include <map>
#include <cassert>


int idaFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, N_Vector resid, void *user_data);
int idaJacDense (long int Neq, realtype ttime, realtype cj, N_Vector state, N_Vector dstate_dt, N_Vector resid, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#ifdef KLU_ENABLE
int idaJacSparse (realtype ttime, realtype sD, N_Vector state, N_Vector dstate_dt, N_Vector resid, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#endif
int idaRootFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, realtype *gout, void *user_data);


idaInterface::idaInterface ()
{

}

idaInterface::idaInterface (gridDynSimulation *gds, const solverMode& sMode) : solverInterface (gds, sMode)
{
}
idaInterface::~idaInterface ()
{
  // clear variables for IDA to use
  if (initialized)
    {
      NVECTOR_DESTROY (use_omp, state);
      NVECTOR_DESTROY (use_omp, abstols);
      NVECTOR_DESTROY (use_omp, scale);
      NVECTOR_DESTROY (use_omp, types);
      NVECTOR_DESTROY (use_omp, consData);
      NVECTOR_DESTROY (use_omp, deriv);
      NVECTOR_DESTROY (use_omp, eweight);
      NVECTOR_DESTROY (use_omp, ele);
      IDAFree (&solverMem);
    }
}


int idaInterface::allocate (count_t stateCount, count_t numRoots)
{
  // load the vectors
  if (stateCount == svsize)
    {
      return 0;
    }
  initialized = false;
  if (state)
    {
      NVECTOR_DESTROY (use_omp, state);
    }

  state = NVECTOR_NEW (use_omp, stateCount);
  if (check_flag ((void *)state, "NVECTOR_NEW", 0))
    {
      return(1);
    }

  if (scale)
    {
      NVECTOR_DESTROY (use_omp, scale);
    }
  scale = NVECTOR_NEW (use_omp, stateCount);
  if (check_flag ((void *)scale, "NVECTOR_NEW", 0))
    {
      return(1);
    }
  N_VConst (ONE, scale);

  if (types)
    {
      NVECTOR_DESTROY (use_omp, types);
    }
  types = NVECTOR_NEW (use_omp, stateCount);
  if (check_flag ((void *)types, "NVECTOR_NEW", 0))
    {
      return(1);
    }
  N_VConst (ONE, types);



  if (abstols)
    {
      NVECTOR_DESTROY (use_omp, abstols);
    }
  abstols = NVECTOR_NEW (use_omp, stateCount);
  if (check_flag ((void *)abstols, "NVECTOR_NEW", 0))
    {
      return(1);
    }

  if (consData)
    {
      NVECTOR_DESTROY (use_omp, consData);
    }
  consData = NVECTOR_NEW (use_omp, stateCount);
  if (check_flag ((void *)consData, "NVECTOR_NEW", 0))
    {
      return(1);
    }

  svsize = stateCount;
  a1.setRowLimit (stateCount);
  a1.setColLimit (stateCount);

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
      IDAFree (&(solverMem));
    }
  solverMem = IDACreate ();
  if (check_flag (solverMem, "IDACreate", 0))
    {
      return(1);
    }



  allocated = true;

  return 0;
}

void idaInterface::setMaxNonZeros (count_t nonZeroCount)
{
  a1.reserve (nonZeroCount);
  a1.clear ();
}


double * idaInterface::state_data ()
{
  return NVECTOR_DATA (use_omp, state);
}
double * idaInterface::deriv_data ()
{
  return NVECTOR_DATA (use_omp, deriv);
}
double * idaInterface::type_data ()
{
  return NVECTOR_DATA (use_omp, types);
}

double idaInterface::get (const std::string &param) const
{
  long int val = -1;
  if ((param == "resevals")||(param == "iterationcount"))
    {
      IDAGetNumResEvals (solverMem, &val);
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
      IDASlsGetNumJacEvals (solverMem, &val);
#else
      IDADlsGetNumJacEvals (solverMem, &val);
#endif
    }
  else
    {
      IDADlsGetNumJacEvals (solverMem, &val);
    }

  return static_cast<double> (val);
}


// output solver stats
void idaInterface::logSolverStats (int logLevel, bool iconly) const
{
  if (!initialized)
    {
      return;
    }
  long int nni = 0, nje = 0;
  int retval, klast, kcur;
  long int nst, nre, nreLS, netf, ncfn, nge;
  realtype tolsfac, hlast, hcur;

  std::string logstr = "";

  retval = IDAGetNumResEvals (solverMem, &nre);
  check_flag (&retval, "IDAGetNumResEvals", 1);
  retval = IDADlsGetNumJacEvals (solverMem, &nje);
  check_flag (&retval, "IDADlsGetNumJacEvals", 1);
  retval = IDAGetNumNonlinSolvIters (solverMem, &nni);
  check_flag (&retval, "IDAGetNumNonlinSolvIters", 1);
  retval = IDAGetNumNonlinSolvConvFails (solverMem, &ncfn);
  check_flag (&retval, "IDAGetNumNonlinSolvConvFails", 1);
  if (!iconly)
    {
      retval = IDAGetNumSteps (solverMem, &nst);
      check_flag (&retval, "IDAGetNumSteps", 1);
      retval = IDAGetNumErrTestFails (solverMem, &netf);
      check_flag (&retval, "IDAGetNumErrTestFails", 1);
      retval = IDADlsGetNumResEvals (solverMem, &nreLS);
      check_flag (&retval, "IDADlsGetNumResEvals", 1);
      retval = IDAGetNumGEvals (solverMem, &nge);
      check_flag (&retval, "IDAGetNumGEvals", 1);
      retval = IDAGetCurrentOrder (solverMem, &kcur);
      retval = IDAGetCurrentStep (solverMem, &hcur);
      retval = IDAGetLastOrder (solverMem, &klast);
      retval = IDAGetLastStep (solverMem, &hlast);
      retval = IDAGetTolScaleFactor (solverMem, &tolsfac);

      logstr = "IDA Run Statistics: \n";

      logstr += "Number of steps                    = " + std::to_string (nst) + '\n';
      logstr += "Number of residual evaluations     = " + std::to_string (nre) + '\n';
      logstr += "Number of Jacobian evaluations     = " + std::to_string (nje) + '\n';
      logstr += "Number of nonlinear iterations     = " + std::to_string (nni) + '\n';
      logstr += "Number of error test failures      = " + std::to_string (netf) + '\n';
      logstr += "Number of nonlinear conv. failures = " + std::to_string (ncfn) + '\n';
      logstr += "Number of root fn. evaluations     = " + std::to_string (nge) + '\n';
      logstr += "Current order used                 = " + std::to_string (kcur) + '\n';
      logstr += "Current step                       = " + std::to_string (hcur) + '\n';
      logstr += "Last order used                    = " + std::to_string (klast) + '\n';
      logstr += "Last step                          = " + std::to_string (hlast) + '\n';
      logstr += "Tolerance scale factor             = " + std::to_string (tolsfac) + '\n';
    }
  else
    {
      logstr = "IDACalcIC Statistics: \n";
      logstr += "Number of residual evaluations     = " + std::to_string (nre) + '\n';
      logstr += "Number of Jacobian evaluations     = " + std::to_string (nje) + '\n';
      logstr += "Number of nonlinear iterations     = " + std::to_string (nni) + '\n';
      logstr += "Number of nonlinear conv. failures = " + std::to_string (ncfn) + '\n';
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


void idaInterface::logErrorWeights (int logLevel) const
{
  realtype *eldata, *ewdata;

  eldata = NVECTOR_DATA (use_omp, ele);
  ewdata = NVECTOR_DATA (use_omp, eweight);
  IDAGetErrWeights (solverMem, eweight);
  IDAGetEstLocalErrors (solverMem, ele);
  std::string logstr = "Error Weight\tEstimated Local Errors\n";
  for (count_t kk = 0; kk < svsize; ++kk)
    {
      logstr += std::to_string (kk) + ':' + std::to_string (ewdata[kk]) + '\t' + std::to_string ( eldata[kk]) + '\n';
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


static const std::map<int, std::string> idaRetCodes {
  {
    IDA_MEM_NULL, "The solver memory argument was NULL"
  },
  {
    IDA_ILL_INPUT, "One of the function inputs is illegal"
  },
  {
    IDA_NO_MALLOC, "The solver memory was not allocated by a call to IDAMalloc"
  },
  {
    IDA_TOO_MUCH_WORK, "The solver took mxstep internal steps but could not reach tout"
  },
  {
    IDA_TOO_MUCH_ACC, "The solver could not satisfy the accuracy demanded by the user for some internal step"
  },
  {
    IDA_ERR_FAIL, "Error test failures occurred too many times during one internal time step or minimum step size was reached"
  },
  {
    IDA_CONV_FAIL, "Convergence test failures occurred too many times during one internal time step or minimum step size was reached"
  },
  {
    IDA_LINIT_FAIL, "The linear solver's initialization function failed"
  },
  {
    IDA_LSETUP_FAIL, "The linear solver's setup function failed in an unrecoverable manner"
  },
  {
    IDA_LSOLVE_FAIL, "The linear solver's solve function failed in an unrecoverable manner"
  },
  {
    IDA_RES_FAIL, "The user - provided residual function failed in an unrecoverable manner"
  },
  {
    IDA_CONSTR_FAIL, "12	The inequality constraints were violated and the solver was unable to recover"
  },
  {
    IDA_MEM_FAIL, "14	A memory allocation failed"
  },
  {
    IDA_BAD_T, "The time t is outside the last step taken"
  },
  {
    IDA_BAD_EWT, "Zero value of some error weight component"
  },
  {
    IDA_FIRST_RES_FAIL, "The user - provided residual function failed recoverably on the first call"
  },
  {
    IDA_LINESEARCH_FAIL, "The line search failed"
  },
  {
    IDA_NO_RECOVERY, "The residual function, linear solver setup function, or linear solver solve function had a recoverable failure, but IDACalcIC could not recover"
  },
  {
    IDA_RTFUNC_FAIL, "The rootfinding function failed in an unrecoverable manner"
  },
  {
    IDA_REP_RES_ERR, "The user-provided residual function repeatedly returned a recoverable error flag, but the solver was unable to recover"
  },
  {
    IDA_MEM_FAIL, "Memory Allocation failed"
  },
  {
    IDA_BAD_K, "Bad K"
  },
  {
    IDA_BAD_DKY, "Bad DKY"
  },
};

int idaInterface::initialize (double t0)
{
  if (!allocated)
    {
      printf ("ERROR,  ida data not allocated\n");
      return -2;
    }
  int retval;
  auto jsize = m_gds->jacSize (mode);

  // initializeB IDA - Sundials

  retval = IDASetUserData (solverMem, (void *)this);
  if (check_flag (&retval, "IDASetUserData", 1))
    {
      return(1);
    }

  //guess an initial condition
  m_gds->guess (t0, NVECTOR_DATA (use_omp, state), NVECTOR_DATA (use_omp, deriv),mode);

  retval = IDAInit (solverMem, idaFunc, t0, state, deriv);
  if (check_flag (&retval, "IDAInit", 1))
    {
      return(1);
    }

  if (rootCount > 0)
    {
      rootsfound.resize (rootCount);
      retval = IDARootInit (solverMem, rootCount, idaRootFunc);
      if (check_flag (&retval, "IDARootInit", 1))
        {
          return(1);
        }
    }

  N_VConst (tolerance, abstols);

  retval = IDASVtolerances (solverMem, tolerance / 100, abstols);
  if (check_flag (&retval, "IDASVtolerances", 1))
    {
      return(1);
    }

  retval = IDASetMaxNumSteps (solverMem, 1500);
  if (check_flag (&retval, "IDASetMaxNumSteps", 1))
    {
      return(1);
    }
#ifdef KLU_ENABLE
  if (dense)
    {
      retval = IDADense (solverMem, svsize);
      if (check_flag (&retval, "IDADense", 1))
        {
          return(1);
        }

      retval = IDADlsSetDenseJacFn (solverMem, idaJacDense);
      if (check_flag (&retval, "IDADlsSetDenseJacFn", 1))
        {
          return(1);
        }
    }
  else
    {

      retval = IDAKLU (solverMem, svsize, jsize);
      if (check_flag (&retval, "IDAKLU", 1))
        {
          return(1);
        }

      retval = IDASlsSetSparseJacFn (solverMem, idaJacSparse);
      if (check_flag (&retval, "IDASlsSetSparseJacFn", 1))
        {
          return(1);
        }
    }
#else
  retval = IDADense (solverMem, svsize);
  if (check_flag (&retval, "IDADense", 1))
    {
      return(1);
    }

  retval = IDADlsSetDenseJacFn (solverMem, idaJacDense);
  if (check_flag (&retval, "IDADlsSetDenseJacFn", 1))
    {
      return(1);
    }
  #endif





  retval = IDASetMaxNonlinIters (solverMem, 20);
  if (check_flag (&retval, "IDASetMaxNonlinIters", 1))
    {
      return(1);
    }

  m_gds->getVariableType (type_data (), mode);

  retval = IDASetId (solverMem, types);
  if (check_flag (&retval, "IDASetId", 1))
    {
      return(1);
    }

  retval = IDASetErrHandlerFn (solverMem, sundialsErrorHandlerFunc, (void *)this);
  if (check_flag (&retval, "IDASetErrHandlerFn", 1))
    {
      return(1);
    }

  setConstraints ();

  initialized = true;
  return FUNCTION_EXECUTION_SUCCESS;

}

int idaInterface::sparseReInit (sparse_reinit_modes sparseReinitMode)
{
#ifdef KLU_ENABLE
  if (dense)
    {
      return FUNCTION_EXECUTION_SUCCESS;
    }
  else
    {
      int kinmode = (sparseReinitMode == sparse_reinit_modes::refactor) ? 1 : 2;
      int retval = IDAKLUReInit (solverMem, static_cast<int> (svsize), static_cast<int> (a1.capacity ()), kinmode);
      if (check_flag (&retval, "IDAKLUReInit", 1))
        {
          return(FUNCTION_EXECUTION_FAILURE);
        }
    }
#endif
  return FUNCTION_EXECUTION_SUCCESS;
}



int idaInterface::setRootFinding (count_t numRoots)
{
  if (numRoots != rootsfound.size ())
    {
      rootsfound.resize (numRoots);
    }
  rootCount = numRoots;
  int retval = IDARootInit (solverMem, numRoots, idaRootFunc);
  if (check_flag (&retval, "IDARootInit", 1))
    {
      return(retval);
    }
  return FUNCTION_EXECUTION_SUCCESS;
}

#define SHOW_MISSING_ELEMENTS 0

int idaInterface::calcIC (double t0, double tstep0, ic_modes initCondMode, bool constraints)
{
  int retval;
  ++icCount;
  assert (icCount < 200);
  if (initCondMode == ic_modes::fixed_masked_and_deriv) //mainly for use upon startup from steady state
    {
      //do a series of steps to ensure the orginal algebraic states are fixed and the derivatives are fixed
      useMask = true;
      loadMaskElements ();
      if (!dense)
        {
          sparseReInit (sparse_reinit_modes::refactor);
        }
      retval = IDACalcIC (solverMem, IDA_Y_INIT, t0 + tstep0); //IDA_Y_INIT

      //retval = IDACalcIC (solverMem, IDA_YA_YDP_INIT, t0 + tstep0); //IDA_YA_YDP_INIT
      //   getCurrentData();
      //  printStates(true);
      if (check_flag (&retval, "IDACalcIC", 1))
        {
          //if the solver failed with error code -14 then we probably have a singular matrix
          //then locate the singular elements and fix them so the problem is valid
          if (retval == IDA_NO_RECOVERY)
            {
              auto mvec = findMissing (&a1);
              if (mvec.size () > 0)
                {
                  double *lstate = NV_DATA_S (state);
                  for (auto &me : mvec)
                    {
                      maskElements.push_back (me);
                      tempState[me] = lstate[me];
                    }

                  if (!dense)
                    {
                      sparseReInit (sparse_reinit_modes::refactor);
                    }
                  retval = IDACalcIC (solverMem, IDA_Y_INIT, t0 + tstep0); //IDA_Y_INIT
                  if (retval == 0)
                    {
                      return 0;
                    }
                  else
                    {
                      return SOLVER_INVALID_STATE_ERROR;
                    }
                }
            }
          else
            {
              switch (retval)
                {
                case 0:                 //no error
                  break;
                case IDA_REP_RES_ERR:
                case IDA_NO_RECOVERY:
                  retval = SOLVER_INVALID_STATE_ERROR;
                  break;
                default:
                  break;
                }
              return retval;
            }
        }
      useMask = false;
      if (!dense)
        {
          sparseReInit (sparse_reinit_modes::refactor);
        }
    }
  else if (initCondMode == ic_modes::fixed_diff)
    {
      retval = IDAReInit (solverMem, t0, state, deriv);

      if (check_flag (&retval, "IDAReInit", 1))
        {

          return retval;
        }
      if (constraints)
        {
          setConstraints ();
        }
      //  printStates();
      retval = IDACalcIC (solverMem, IDA_YA_YDP_INIT, t0 + tstep0); //IDA_YA_YDP_INIT
      if (check_flag (&retval, "IDACalcIC", 1))
        {
#if SHOW_MISSING_ELEMENTS > 0
          auto mvec = findMissing (&a1);
          if (mvec.size () > 0)
            {
              printf ("missing rows in jacobian from calcIC mode 1\n");
            }
#endif
          switch (retval)
            {
            case IDA_REP_RES_ERR:
            case IDA_NO_RECOVERY:
              retval = SOLVER_INVALID_STATE_ERROR;
              break;
            default:
              break;
            }
          return retval;
        }
      // getCurrentData();
      //  printStates();
    }
  return FUNCTION_EXECUTION_SUCCESS;
}

int idaInterface::getCurrentData ()
{
  int retval = IDAGetConsistentIC (solverMem, state, deriv);
  if (check_flag (&retval, "IDAGetConsistentIC", 1))
    {
      return(retval);
    }
  return FUNCTION_EXECUTION_SUCCESS;
}

int idaInterface::solve (double tStop, double &tReturn, step_mode stepMode)
{
  assert (rootCount == m_gds->rootSize (mode));
  ++solverCallCount;
  icCount = 0;
  int retval = IDASolve (solverMem, tStop, &tReturn, state, deriv, (stepMode == step_mode::normal) ? IDA_NORMAL : IDA_ONE_STEP);
  check_flag (&retval, "IDASolve", 1, false);
  switch (retval)
    {
    case 0:       //no error
      break;
    case IDA_ROOT_RETURN:
      retval = SOLVER_ROOT_FOUND;
      break;
    case IDA_REP_RES_ERR:
      retval = SOLVER_INVALID_STATE_ERROR;
      break;
    default:
      break;
    }
  return retval;
}

int idaInterface::getRoots ()
{
  int ret = IDAGetRootInfo (solverMem, rootsfound.data ());
  if (!check_flag (&ret, "IDAGetRootInfo", 1))
    {
      return ret;
    }
  return ret;
}

void idaInterface::setConstraints ()
{
  if (m_gds->hasConstraints ())
    {
      N_VConst (ZERO, consData);
      m_gds->getConstraints (NVECTOR_DATA (use_omp,consData), mode);
      IDASetConstraints (solverMem, consData);
    }
}

void idaInterface::loadMaskElements ()
{
  std::vector<double> mStates (svsize,0.0);
  m_gds->getVoltageStates (mStates.data (),mode);
  m_gds->getAngleStates (mStates.data (),mode);
  maskElements = vecFindgt<double, index_t> (mStates,0.5);
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

// IDA C Functions
int idaFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, N_Vector resid, void *user_data)
{
  idaInterface *sd = reinterpret_cast<idaInterface *> (user_data);
  //printf("time=%f\n", ttime);
  int ret = sd->m_gds->residualFunction (ttime, NVECTOR_DATA (sd->use_omp, state), NVECTOR_DATA (sd->use_omp, dstate_dt), NVECTOR_DATA (sd->use_omp, resid), sd->mode);
  if (sd->useMask)
    {
      double *lstate = NVECTOR_DATA (sd->use_omp, state);
      double *lresid = NVECTOR_DATA (sd->use_omp, resid);
      for (auto &v:sd->maskElements)
        {
          lresid[v] = 100.0 * (lstate[v] - sd->tempState[v]);
        }
    }
#ifdef CAPTURE_STATE_FILE
  saveStateFile (ttime, sd->svsize, NVECTOR_DATA (state), NVECTOR_DATA (dstate_dt), NVECTOR_DATA (resid), "jac_new_state.dat", true);
#endif
  return ret;
}

int idaRootFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, realtype *gout, void *user_data)
{
  idaInterface *sd = reinterpret_cast<idaInterface *> (user_data);
  sd->m_gds->rootFindingFunction (ttime, NVECTOR_DATA (sd->use_omp, state), NVECTOR_DATA (sd->use_omp, dstate_dt), gout, sd->mode);

  return FUNCTION_EXECUTION_SUCCESS;
}

#define CHECK_JACOBIAN 0
int idaJacDense (long int Neq, realtype ttime, realtype cj, N_Vector state, N_Vector dstate_dt, N_Vector /*resid*/, DlsMat J, void *user_data, N_Vector /*tmp1*/, N_Vector /*tmp2*/, N_Vector /*tmp3*/)
{
  index_t kk;
  idaInterface *sd = reinterpret_cast<idaInterface *> (user_data);

  assert (Neq == static_cast<int> (sd->svsize));
  arrayDataSparse *a1 = &(sd->a1);
  sd->m_gds->jacobianFunction (ttime, NVECTOR_DATA (sd->use_omp, state), NVECTOR_DATA (sd->use_omp, dstate_dt), a1,cj, sd->mode);


  if (sd->useMask)
    {
      for (auto &v : sd->maskElements)
        {
          a1->translateRow (v,kNullLocation);
          a1->assign (v,v,100);
        }
      a1->filter ();
    }

  //assign the elements
  for (kk = 0; kk < a1->size (); ++kk)
    {
      DENSE_ELEM (J, a1->rowIndex (kk), a1->colIndex (kk)) += a1->val (kk);
      //DENSE_ELEM (J, a1->rowIndex (kk), a1->colIndex (kk)) += DENSE_ELEM (J, a1->rowIndex (kk), a1->colIndex (kk)) + a1->val (kk);
    }

    #if (CHECK_JACOBIAN > 0)
  auto mv = findMissing (a1);
  for (auto &me : mv)
    {
      printf ("no entries for element %d\n",me);
    }
    #endif
  return FUNCTION_EXECUTION_SUCCESS;
}

//#define CAPTURE_JAC_FILE

#ifdef KLU_ENABLE
int idaJacSparse (realtype ttime, realtype cj, N_Vector state, N_Vector dstate_dt, N_Vector /*resid*/, SlsMat J, void *user_data, N_Vector, N_Vector, N_Vector )
{
  count_t kk;
  count_t colval;

  idaInterface *sd = reinterpret_cast<idaInterface *> (user_data);

  arrayDataSparse *a1 = &(sd->a1);

  sd->m_gds->jacobianFunction (ttime, NVECTOR_DATA (sd->use_omp, state), NVECTOR_DATA (sd->use_omp, dstate_dt), a1,cj, sd->mode);
  a1->sortIndexCol ();
  if (sd->useMask)
    {
      for (auto &v : sd->maskElements)
        {
          a1->translateRow (v,kNullLocation);
          a1->assign (v, v,1);
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
