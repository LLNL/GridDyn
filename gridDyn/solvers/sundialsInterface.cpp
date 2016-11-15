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
#include "core/helperTemplates.h"
#include "core/factoryTemplates.h"
#include "basicDefs.h"
#include <cstdio>
#include <cassert>
#include "stringOps.h"


static childClassFactory<kinsolInterface, solverInterface> kinFactory(stringVec{ "kinsol","algebraic" });
static childClassFactory<idaInterface, solverInterface> idaFactory(stringVec{ "ida","dae","dynamic" });
#ifdef LOAD_CVODE
static childClassFactory<cvodeInterface, solverInterface > cvodeFactory(stringVec{ "cvode","dyndiff","differential" });
#endif

#ifdef LOAD_ARKODE
static childClassFactory<arkodeInterface, solverInterface> arkodeFactory(stringVec{ "arkode" });
#endif

sundialsInterface::sundialsInterface (const std::string &objName):solverInterface(objName)
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
	if (state)
	{
		NVECTOR_DESTROY(use_omp, state);
	}
	  if (dstate_dt)
	  {
		  NVECTOR_DESTROY(use_omp, dstate_dt);
	  }
	  if (abstols)
	  {
		  NVECTOR_DESTROY(use_omp, abstols);
	  }
	  if (consData)
	  {
		  NVECTOR_DESTROY(use_omp, consData);
	  }

}

std::shared_ptr<solverInterface> sundialsInterface::clone(std::shared_ptr<solverInterface> si, bool fullCopy) const
{
	auto rp = cloneBase<sundialsInterface, solverInterface>(this, si, fullCopy);
	if (!rp)
	{
		return si;
	}
	rp->maxNNZ = maxNNZ;
	if ((fullCopy)&&(allocated))
	{
		auto tols = NVECTOR_DATA(use_omp, abstols);
		std::copy(tols, tols + svsize, NVECTOR_DATA(use_omp, rp->abstols));
		auto cons = NVECTOR_DATA(use_omp, consData);
		std::copy(cons, cons + svsize, NVECTOR_DATA(use_omp, rp->consData));
		auto sc = NVECTOR_DATA(use_omp, scale);
		std::copy(sc, sc + svsize, NVECTOR_DATA(use_omp, rp->scale));
	}
	return rp;
}

void sundialsInterface::allocate (count_t stateCount, count_t /*numroots*/)
{
  // load the vectors
  if (stateCount == svsize)
    {
      return;
    }
  initialized = false;
  if (state)
    {
      NVECTOR_DESTROY (use_omp, state);
    }
  state = NVECTOR_NEW (use_omp, stateCount);
  check_flag((void *)state, "NVECTOR_NEW", 0);
	
  if (hasDifferential(mode))
  {
	  if (dstate_dt)
	  {
		  NVECTOR_DESTROY(use_omp, dstate_dt);
	  }
	  dstate_dt = NVECTOR_NEW(use_omp, stateCount);
	  check_flag((void *)dstate_dt, "NVECTOR_NEW", 0);
	  N_VConst(ZERO, dstate_dt);
  }
  if (abstols)
    {
      NVECTOR_DESTROY (use_omp, abstols);
    }
  abstols = NVECTOR_NEW (use_omp, stateCount);
  check_flag((void *)abstols, "NVECTOR_NEW", 0);
    
  if (consData)
    {
      NVECTOR_DESTROY (use_omp, consData);
    }
  consData = NVECTOR_NEW (use_omp, stateCount);
  check_flag((void *)consData, "NVECTOR_NEW", 0);
  

  if (scale)
  {
	  NVECTOR_DESTROY(use_omp, scale);
  }
  scale = NVECTOR_NEW(use_omp, stateCount);
  check_flag((void *)scale, "NVECTOR_NEW", 0);
 
  N_VConst(ONE, scale);

  if (isDAE(mode))
  {
	  if (types)
	  {
		  NVECTOR_DESTROY(use_omp, types);
	  }
	  types = NVECTOR_NEW(use_omp, stateCount);
	  check_flag((void *)types, "NVECTOR_NEW", 0);
	 
	  N_VConst(ONE, types);
  }
  
  
  svsize = stateCount;

  allocated = true;
}


void sundialsInterface::setMaxNonZeros (count_t nonZeroCount)
{
  maxNNZ = nonZeroCount;
  nnz = nonZeroCount;
}


double * sundialsInterface::state_data ()
{
  return (state)? NVECTOR_DATA (use_omp, state):nullptr;
}

double * sundialsInterface::deriv_data ()
{
	return (dstate_dt) ? NVECTOR_DATA(use_omp, dstate_dt) : nullptr;
}

const double * sundialsInterface::state_data() const
{
	return (state) ?  NVECTOR_DATA(use_omp, state) : nullptr;
}

const double * sundialsInterface::deriv_data() const
{
	return (dstate_dt) ? NVECTOR_DATA(use_omp, dstate_dt) : nullptr;
}
// output solver stats

double * sundialsInterface::type_data()
{
	return (types) ? NVECTOR_DATA(use_omp, types) : nullptr;
}

const double * sundialsInterface::type_data() const
{
	return (types) ? NVECTOR_DATA(use_omp, types) : nullptr;
}

double sundialsInterface::get (const std::string &param) const
{

  if (param == "maxnnz")
    {
	  return static_cast<double>(maxNNZ);
    }
  else
  {
	  return solverInterface::get(param);
  }
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

void matrixDataToSlsMat (matrixData<double> *ad, SlsMat J,count_t svsize)
{
  count_t colval = 0;
  J->colptrs[0] = colval;

  ad->compact ();

  int sz = static_cast<int> (ad->size());
	/*
  auto itel = ad->begin();
  for (int kk = 0; kk < sz; ++kk)
  {
	  auto tp = *itel;
	  //	  printf("kk: %d  dataval: %f  rowind: %d   colind: %d \n ", kk, a1->val(kk), a1->rowIndex(kk), a1->colIndex(kk));
	  if (tp.col > colval)
	  {
		  colval++;
		  J->colptrs[colval] = kk;
	  }

	  J->data[kk] = tp.data;
	  J->rowvals[kk] = tp.row;
	  ++itel;
  }
*/
  //SlsSetToZero(J);
	

  ad->start ();
  for (int kk = 0; kk < sz; ++kk)
    {
      auto tp = ad->next ();
      //	  printf("kk: %d  dataval: %f  rowind: %d   colind: %d \n ", kk, a1->val(kk), a1->rowIndex(kk), a1->colIndex(kk));
      if (tp.col > colval)
        {
          colval++;
          J->colptrs[colval] = kk;
        }
	
      J->data[kk] = tp.data;
      J->rowvals[kk] = tp.row;
    }
	
  if (colval + 1 != svsize)
    {
      printf ("sz=%d, svsize=%d, colval+1=%d\n", sz, svsize, colval + 1);
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

