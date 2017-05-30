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

#include "sundialsInterface.h"
#include "core/helperTemplates.h"
#include "core/factoryTemplates.h"
#include <cstdio>
#include <cassert>
#include "utilities/stringOps.h"
#include "sundialsMatrixData.h"
#include "utilities/matrixDataFilter.h"
#include "utilities/matrixCreation.h"
#include "simulation/gridDynSimulationFileOps.h"
#include "gridDyn.h"
#include "simulation/diagnostics.h"
#include <cassert>



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
	if ((fullCopy)&&(flags[allocated_flag]))
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
  bool prev_omp = use_omp;
  _unused(prev_omp); //looks unused if OPENMP is not available
  use_omp = flags[use_omp];
  flags.reset(initialized_flag);
  if (state)
    {
      NVECTOR_DESTROY (prev_omp, state);
    }
  state = NVECTOR_NEW (use_omp, stateCount);
  check_flag((void *)state, "NVECTOR_NEW", 0);
	
  if (hasDifferential(mode))
  {
	  if (dstate_dt)
	  {
		  NVECTOR_DESTROY(prev_omp, dstate_dt);
	  }
	  dstate_dt = NVECTOR_NEW(use_omp, stateCount);
	  check_flag((void *)dstate_dt, "NVECTOR_NEW", 0);
	  N_VConst(ZERO, dstate_dt);
  }
  if (abstols)
    {
      NVECTOR_DESTROY (prev_omp, abstols);
    }
  abstols = NVECTOR_NEW (use_omp, stateCount);
  check_flag((void *)abstols, "NVECTOR_NEW", 0);
    
  if (consData)
    {
      NVECTOR_DESTROY (prev_omp, consData);
    }
  consData = NVECTOR_NEW (use_omp, stateCount);
  check_flag((void *)consData, "NVECTOR_NEW", 0);
  

  if (scale)
  {
	  NVECTOR_DESTROY(prev_omp, scale);
  }
  scale = NVECTOR_NEW(use_omp, stateCount);
  check_flag((void *)scale, "NVECTOR_NEW", 0);
 
  N_VConst(ONE, scale);

  if (isDAE(mode))
  {
	  if (types)
	  {
		  NVECTOR_DESTROY(prev_omp, types);
	  }
	  types = NVECTOR_NEW(use_omp, stateCount);
	  check_flag((void *)types, "NVECTOR_NEW", 0);
	 
	  N_VConst(ONE, types);
  }
  
  
  svsize = stateCount;

  flags.set(allocated_flag);
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
  if ((J->indexptrs[0] != 0) || (J->indexptrs[0] > J->NNZ))
    {
      return false;
    }
  if ((J->indexptrs[J->N] <= 0) || (J->indexptrs[J->N] >= J->NNZ))
    {
      return false;
    }
  return true;
}

void matrixDataToSlsMat (matrixData<double> &ad, SlsMat J,count_t svsize)
{
  count_t indval = 0;
  J->indexptrs[0] = indval;
  
  
  ad.compact ();
  assert(J->NNZ >= static_cast<int>(ad.size()));
  int sz = static_cast<int> (ad.size());
	/*
  auto itel = ad.begin();
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
	

  ad.start ();
  for (int kk = 0; kk < sz; ++kk)
    {
      auto tp = ad.next ();
      //	  printf("kk: %d  dataval: %f  rowind: %d   colind: %d \n ", kk, a1->val(kk), a1->rowIndex(kk), a1->colIndex(kk));
      if (tp.row > indval)
        {
          indval++;
          J->indexptrs[indval] = kk;
		  assert(tp.row == indval);
        }
	
      J->data[kk] = tp.data;
      J->indexvals[kk] = tp.col;
    }
	
  if (indval + 1 != svsize)
    {
      printf ("sz=%d, svsize=%d, colval+1=%d\n", sz, svsize, indval + 1);
    }
  assert (indval + 1 == svsize);
  J->indexptrs[indval + 1] = sz;
}

#endif

//Error handling function for Sundials
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

#ifdef KLU_ENABLE
int sundialsJacSparse(realtype ttime, realtype cj, N_Vector state, N_Vector dstate_dt, SlsMat J, void *user_data, N_Vector, N_Vector, N_Vector)
{

	sundialsInterface *sd = reinterpret_cast<sundialsInterface *> (user_data);

	if ((sd->jacCallCount == 0) || (!isSlsMatSetup(J)))
	{
		auto a1 = makeSparseMatrix(sd->svsize, sd->maxNNZ);

		a1->setRowLimit(sd->svsize);
		a1->setColLimit(sd->svsize);

		if (sd->flags[useMask_flag])
		{
			matrixDataFilter<double> filterAd(*(a1.get()));
			filterAd.addFilter(sd->maskElements);
			sd->m_gds->jacobianFunction(ttime, NVECTOR_DATA(sd->use_omp, state), NVECTOR_DATA(sd->use_omp, dstate_dt), filterAd, cj, sd->mode);
			for (auto &v : sd->maskElements)
			{
				a1->assign(v, v, 1.0);
			}
		}
		else
		{
			sd->m_gds->jacobianFunction(ttime, NVECTOR_DATA(sd->use_omp, state), NVECTOR_DATA(sd->use_omp, dstate_dt), *(a1.get()), cj, sd->mode);
		}

		++sd->jacCallCount;
#ifdef _DEBUG
		if (J->NNZ < static_cast<int>(a1->size()))
		{
			a1->compact();
			if (J->NNZ < static_cast<int>(a1->size()))
			{
				jacobianAnalysis(*(a1.get()), sd->m_gds, sd->mode, 5);
			}
		}
#endif
		matrixDataToSlsMat(*(a1.get()), J, sd->svsize);
		sd->nnz = a1->size();
		if (sd->flags[fileCapture_flag])
		{
			if (!sd->jacFile.empty())
			{
				long int val = static_cast<long int>(sd->get("nliterations"));
				writeArray(ttime, 1, val, sd->mode.offsetIndex, *(a1.get()), sd->jacFile);
			}
		}
	}
	else
	{
		//if it isn't the first we can use the SUNDIALS arraySparse object
		sundialsMatrixDataSparseRow a1(J);
		if (sd->flags[useMask_flag])
		{
			matrixDataFilter<double> filterAd(a1);
			filterAd.addFilter(sd->maskElements);
			sd->m_gds->jacobianFunction(ttime, NVECTOR_DATA(sd->use_omp, state), NVECTOR_DATA(sd->use_omp, dstate_dt), filterAd, cj, sd->mode);
			for (auto &v : sd->maskElements)
			{
				a1.assign(v, v, 1.0);
			}
		}
		else
		{
			sd->m_gds->jacobianFunction(ttime, NVECTOR_DATA(sd->use_omp, state), NVECTOR_DATA(sd->use_omp, dstate_dt), a1, cj, sd->mode);
		}

		sd->jacCallCount++;
		if (sd->flags[fileCapture_flag])
		{
			if (!sd->jacFile.empty())
			{
				writeArray(ttime, 1, sd->jacCallCount, sd->mode.offsetIndex, a1, sd->jacFile);
			}
		}
	}
	/*
	matrixDataSparse<double> &a1 = sd->a1;

	sd->m_gds->jacobianFunction (ttime, NVECTOR_DATA(sd->use_omp, state), NVECTOR_DATA(sd->use_omp, dstate_dt), a1,cj, sd->mode);
	a1.sortIndexCol ();
	if (sd->flags[useMask_flag])
	{
	for (auto &v : sd->maskElements)
	{
	a1.translateRow (v,kNullLocation);
	a1.assign (v, v,1);
	}
	a1.filter ();
	a1.sortIndexCol ();
	}
	a1.compact ();

	SlsSetToZero (J);

	count_t colval = 0;
	J->colptrs[0] = colval;
	for (index_t kk = 0; kk < a1.size (); ++kk)
	{
	//	  printf("kk: %d  dataval: %f  rowind: %d   colind: %d \n ", kk, a1->val(kk), a1->rowIndex(kk), a1->colIndex(kk));
	if (a1.colIndex (kk) > colval)
	{
	colval++;
	J->colptrs[colval] = static_cast<int> (kk);
	}
	J->data[kk] = a1.val (kk);
	J->rowvals[kk] = a1.rowIndex (kk);
	}
	J->colptrs[colval + 1] = static_cast<int> (a1.size ());

	if (sd->flags[fileCapture_flag])
	{
	if (!sd->jacFile.empty())
	{
	long int val = 0;
	IDAGetNumNonlinSolvIters(sd->solverMem, &val);
	writeArray(sd->solveTime, 1, val, sd->mode.offsetIndex, a1, sd->jacFile);
	}
	}
	*/
#if (CHECK_JACOBIAN > 0)
	auto mv = findMissing(a1);
	for (auto &me : mv)
	{
		printf("no entries for element %d\n", me);
	}
#endif
	return 0;
}

#endif

#define CHECK_JACOBIAN 0
int sundialsJacDense(long int Neq, realtype ttime, realtype cj, N_Vector state, N_Vector dstate_dt, DlsMat J, void *user_data, N_Vector /*tmp1*/, N_Vector /*tmp2*/, N_Vector /*tmp3*/)
{
	sundialsInterface *sd = reinterpret_cast<sundialsInterface *> (user_data);

	assert(Neq == static_cast<int> (sd->svsize));
	_unused(Neq);
	sundialsMatrixDataDense a1(J);
	if (sd->flags[useMask_flag])
	{
		matrixDataFilter<double> filterAd(a1);
		filterAd.addFilter(sd->maskElements);
		sd->m_gds->jacobianFunction(ttime, NVECTOR_DATA(sd->use_omp, state), NVECTOR_DATA(sd->use_omp, dstate_dt), filterAd, cj, sd->mode);
		for (auto &v : sd->maskElements)
		{
			a1.assign(v, v, 1.0);
		}
	}
	else
	{
		sd->m_gds->jacobianFunction(ttime, NVECTOR_DATA(sd->use_omp, state), NVECTOR_DATA(sd->use_omp, dstate_dt), a1, cj, sd->mode);
	}
	
	++sd->jacCallCount;
	if (sd->flags[fileCapture_flag])
	{
		if (!sd->jacFile.empty())
		{
			writeArray(ttime, 1, sd->jacCallCount, sd->mode.offsetIndex, a1, sd->jacFile);
		}
	}
	

#if (CHECK_JACOBIAN > 0)
	auto mv = findMissing(a1);
	for (auto &me : mv)
	{
		printf("no entries for element %d\n", me);
	}
#endif
	return FUNCTION_EXECUTION_SUCCESS;
}