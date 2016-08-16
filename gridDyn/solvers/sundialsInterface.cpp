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

#include <cstdio>
#include <cassert>


//int kinsolFunc (N_Vector u, N_Vector f, void *user_data);
//int kinsolJacDense (long int N, N_Vector u, N_Vector f, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);
//#ifdef KLU_ENABLE
//int kinsolJacSparse (N_Vector u, N_Vector f, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);
//#endif
//int kinsolAlgFunc (N_Vector u, N_Vector f, void *user_data);
//int kinsolAlgJacDense (long int N, N_Vector u, N_Vector f, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);

sundialsInterface::sundialsInterface ()
{
  tolerance = 1e-8;
}

sundialsInterface::sundialsInterface (gridDynSimulation *gds, const solverMode& sMode) : solverInterface (gds, sMode)
{
  tolerance = 1e-8;
}

sundialsInterface::~sundialsInterface ()
{
  // clear variables for IDA to use
  if (initialized)
    {

      NVECTOR_DESTROY (use_omp, state);
      NVECTOR_DESTROY (use_omp, dstate_dt);
      NVECTOR_DESTROY (use_omp, abstols);
      NVECTOR_DESTROY (use_omp, consData);

    }
}


int sundialsInterface::allocate (count_t stateCount, count_t /*numroots*/)
{
  // load the vectors
  if (stateCount == svsize)
    {
      return FUNCTION_EXECUTION_SUCCESS;
    }
  initialized = false;
  if (state)
    {
      NVECTOR_DESTROY (use_omp, state);
    }
  state = NVECTOR_NEW (use_omp, stateCount);
  if (check_flag ((void *)state, "NVECTOR_NEW", 0))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  if (abstols)
    {
      NVECTOR_DESTROY (use_omp, abstols);
    }
  abstols = NVECTOR_NEW (use_omp, stateCount);
  if (check_flag ((void *)abstols, "NVECTOR_NEW", 0))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  if (consData)
    {
      NVECTOR_DESTROY (use_omp, consData);
    }
  consData = NVECTOR_NEW (use_omp, stateCount);
  if (check_flag ((void *)consData, "NVECTOR_NEW", 0))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }

  svsize = stateCount;

  allocated = true;
  return FUNCTION_EXECUTION_SUCCESS;
}


void sundialsInterface::setMaxNonZeros (count_t nonZeroCount)
{
  maxNNZ = nonZeroCount;
  nnz = nonZeroCount;
}


double * sundialsInterface::state_data ()
{
  return NVECTOR_DATA (use_omp, state);
}

double * sundialsInterface::deriv_data ()
{
  return NVECTOR_DATA (use_omp, dstate_dt);
}

// output solver stats
void sundialsInterface::logSolverStats (int /*logLevel*/, bool /*iconly*/) const
{

}



int sundialsInterface::initialize (double /*t0*/)
{
  return FUNCTION_EXECUTION_FAILURE;
}


int sundialsInterface::sparseReInit (sparse_reinit_modes /*sparseReinitMode*/)
{
  return FUNCTION_EXECUTION_FAILURE;
}


double sundialsInterface::get (const std::string &param) const
{
  long int val = -1;
  if (param == "solvercount")
    {
      val = solverCallCount;
    }
  else if (param == "jaccallcount")
    {
      val = jacCallCount;
    }

  return static_cast<double> (val);
}

void sundialsInterface::setConstraints ()
{

}


int sundialsInterface::solve (double /*tStop*/, double & /*tReturn*/, step_mode /*mode*/)
{
  return FUNCTION_EXECUTION_FAILURE;
}


#ifdef KLU_ENABLE
bool isSlsMatSetup (SlsMat J)
{
  if ((J->colptrs[0] != 0) || (J->colptrs[0] > J->NNZ))
    {
      return false;
    }
  if ((J->colptrs[J->N] <= 0) || (J->colptrs[J->N] >= J->NNZ))
    {
      return false;
    }
  return true;
}

void arrayDataToSlsMat (arrayData<double> *ad, SlsMat J,count_t svsize)
{
  count_t colval = 0;
  J->colptrs[0] = colval;

  ad->compact ();

  //SlsSetToZero(J);
  int sz = static_cast<int> (ad->size ());
  ad->start ();
  int zcnt = 0;
  for (int kk = 0; kk < sz; ++kk)
    {
      auto tp = ad->next ();
      //	  printf("kk: %d  dataval: %f  rowind: %d   colind: %d \n ", kk, a1->val(kk), a1->rowIndex(kk), a1->colIndex(kk));
      if (tp.col > colval)
        {
          colval++;
          J->colptrs[colval] = kk;
        }
	  if (tp.col < colval)
	  {
		  ++zcnt;
	  }
      J->data[kk] = tp.data;
      J->rowvals[kk] = tp.row;
    }
  if (colval + 1 != svsize)
    {
      printf ("sz=%d, svsize=%d, colval+1=%d zcnt=%d\n", sz, svsize, colval + 1,zcnt);
    }
  assert (colval + 1 == svsize);
  J->colptrs[colval + 1] = sz;
}

#endif

//Error handling function for Kinsol
void sundialsErrorHandlerFunc (int error_code, const char *module, const char *function, char *msg, void *user_data)
{
  if (error_code == 0)
    {
      return;
    }
  solverInterface *sd = reinterpret_cast<solverInterface *> (user_data);
  std::string message = "SUNDIALS ERROR(" + std::to_string (error_code) + ") in Module (" + std::string (module) + ") function " + std::string (function) + "::" + std::string (msg);
  sd->logMessage (error_code, message);
}

