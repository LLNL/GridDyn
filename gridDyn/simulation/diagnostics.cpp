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

#include "simulation/diagnostics.h"
#include "gridDyn.h"

#include "solvers/solverInterface.h"

#include "utilities/vectorOps.hpp"
#include "utilities/matrixDataSparse.h"
#include "utilities/gridRandom.h"
#include <cassert>

std::pair<double,int> checkResid (gridDynSimulation *gds, coreTime time, const solverMode &sMode)
{
  return checkResid (gds, time, gds->getSolverInterface (sMode));
}

std::pair<double, int> checkResid (gridDynSimulation *gds, const std::shared_ptr<solverInterface> &sd)
{
  return checkResid (gds, gds->getCurrentTime (), sd);
}

std::pair<double, int> checkResid (gridDynSimulation *gds, coreTime time, const std::shared_ptr<solverInterface> &sd)
{
  const solverMode &sMode = sd->getSolverMode ();
  std::vector<double> resid;
  double *dstate_dt = nullptr;
  auto kSize = sd->size ();
  resid.resize (kSize, 0);
  double *state = sd->state_data ();
  assert (kSize == const_cast<const gridDynSimulation *>(gds)->stateSize (sMode));
  if (!isAlgebraicOnly (sMode))
    {
      dstate_dt = sd->deriv_data ();
    }

  gds->residualFunction (time, state, dstate_dt, resid.data (), sMode);
  auto rb = resid.begin ();
  std::vector<double> signs (kSize, 0);
  if (isDAE (sMode))
    {
      signs.assign (kSize, 1);
      gds->getVariableType (signs.data (), sMode);
    }
  double *sdata = signs.data ();
  while (rb != resid.end ())
    {
      if (*sdata == 1)
        {
          *rb = 0.0;
        }
      ++sdata;
      ++rb;
    }
  return absMaxLoc(resid);
 
}


int JacobianCheck (gridDynSimulation *gds, const solverMode &queryMode, double jactol, bool useStateNames)
{

  if (isDynamic (queryMode))
    {
      if (gds->currentProcessState () < gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED)
        {
          return -1;
        }
    }
  else if (gds->currentProcessState () < gridDynSimulation::gridState_t::INITIALIZED)
    {
      return -1;
    }
  int errors = 0;
  auto sd = gds->getSolverInterface (queryMode);
  const solverMode &sMode = sd->getSolverMode();
  gds->getSolverReady (sd);
  auto nsize = sd->size ();

  if (nsize == 0)
    {
      return 0;
    }
  double *state = sd->state_data ();
  double *dstate = sd->deriv_data ();

  coreTime timeCurr = gds->getCurrentTime ();
  if ((gds->currentProcessState () <= gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED)&&(timeCurr<=gds->getStartTime()))
    {
      gds->guess (timeCurr, state, dstate, sd->getSolverMode());
    }


  std::vector<double> nstate (nsize);
  std::vector<double> ndstate (nsize);
  std::copy (state, state + nsize, nstate.data ());
  if (dstate)
    {
      std::copy (dstate, dstate + nsize, ndstate.data ());
    }

  matrixDataSparse<double> tad, tad2, ad;
  tad.reserve (gds->jacSize (sMode));
  ad.reserve (gds->jacSize (sMode));
  tad2.reserve (gds->jacSize (sMode));
  double delta = 1e-8;
  double delta2 = 1e-10;

  //matrixDataSparse b2;
  if (jactol < 0)        //make sure the tolerance is positive
    {
      jactol = jac_check_tol;
    }

  std::vector<double> resid (nsize);
  std::vector<double> resid2 (nsize);
  stateData sD (timeCurr, nstate.data (), ndstate.data ());
  if (sMode.pairedOffsetIndex != kNullLocation)
    {
      gds->fillExtraStateData (sD, sMode);
    }
  if (isDifferentialOnly (sMode))
    {
      sD.cj = 0.0;
      gds->derivative (noInputs, sD, resid.data (), sMode);
    }
  else
    {
      sD.cj = 100;
      gds->residual (noInputs, sD, resid.data (), sMode);
    }

  gds->jacobianFunction (timeCurr, nstate.data (), ndstate.data (), ad, sD.cj, sMode);

  stringVec stv;
  if (useStateNames)
    {
      gds->getStateName (stv, sMode);
    }

  for (index_t kk = 0; kk < nsize; ++kk)
    {
      nstate[kk] += delta;
      if (isDifferentialOnly (sMode))
        {
          gds->derivative (noInputs, sD, resid2.data (), sMode);
        }
      else
        {
          gds->residual (noInputs, sD, resid2.data (), sMode);
        }

      //find the changed elements
      for (index_t pp = 0; pp < nsize; ++pp)
        {
          if (std::abs (resid[pp] - resid2[pp]) > delta * jactol / 2)
            {
              tad.assign (pp, kk, (resid2[pp] - resid[pp]) / delta);
            }
        }
      nstate[kk] -= delta;
      nstate[kk] += delta2;
      if (isDifferentialOnly (sMode))
        {
          gds->derivative (noInputs, sD, resid2.data (), sMode);
        }
      else
        {
          gds->residual (noInputs, sD, resid2.data (), sMode);
        }

      for (index_t pp = 0; pp < nsize; ++pp)
        {
          if (std::abs (resid[pp] - resid2[pp]) > delta2 * jactol / 2)
            {
              tad2.assign (pp, kk, (resid2[pp] - resid[pp]) / delta2);
            }
        }
      nstate[kk] -= delta2;
      //find the Jacobian elements dependent on the derivatives
      if (isDAE (sMode))
        {
          ndstate[kk] += delta;
          gds->residual (noInputs, sD, resid2.data (), sMode);
          //find the changed elements
          for (index_t pp = 0; pp < nsize; ++pp)
            {
              if (std::abs (resid[pp] - resid2[pp]) > delta * jactol / 2)
                {
                  tad.assign (pp, kk, (resid2[pp] - resid[pp]) / delta * sD.cj);
                }
            }
          ndstate[kk] -= delta;
          ndstate[kk] += delta2;
          gds->residual (noInputs, sD, resid2.data (), sMode);
          for (index_t pp = 0; pp < nsize; ++pp)
            {
              if (std::abs (resid[pp] - resid2[pp]) > delta2 * jactol / 2)
                {
                  tad2.assign (pp, kk, (resid2[pp] - resid[pp]) / delta2 * sD.cj);
                }
            }
          ndstate[kk] -= delta2;
        }
    }


  ad.compact ();

  tad.compact ();

  tad2.compact ();

  ad.start();
  for (index_t nn = 0; nn < ad.size (); ++nn)
    {
	  auto el = ad.next();
      index_t rowk = el.row;
      index_t colk = el.col;
      double val1 = el.data;
      double val2 = tad.at (rowk, colk);
      double val3 = tad2.at (rowk, colk);

      if ((std::abs (val1 - val2) > jactol) && (std::abs (val1 - val3) > jactol) && (std::abs ((val1 - val2) / std::max (std::abs (val1), std::abs (val2))) > 2e-4))
        {
          //convergence
          if ((((std::abs (val3 - val1) / std::abs (val2 - val1)) > 10.0) && (std::abs (val1) < jactol)) || ((std::abs (val3) / std::abs (val2)) > 100.0))
            {
              continue;
            }
          if ((std::abs (val3 - val1) / std::abs (val2 - val1)) > 30.0)
            {
              continue;
            }
          //oscillatory convergence
          if ((val3 / val2 < 0) && (val1 > std::min (val3, val2)) && (val1 < std::max (val3, val2)) && (std::abs (val1) < jactol))
            {
              continue;
            }
			//big number tolerance
			if ((std::abs(val1)>10)&&(std::abs(val2)>10))
			{
				if (std::abs(val2 - val1)<jactol*val1/10.0)
				{
					continue;
				}
			}
          ++errors;
          if ((std::abs (val1) > 0.001) || (std::abs (val2) > 0.001))
            {
              printf ("Mismatched Jacobian A [%u,%u] jac=%5.4f, a1=%5.4f a2=%5.4f %4.2f%%\n", static_cast<unsigned int> (rowk), static_cast<unsigned int> (colk), val1, val2, val3, std::abs ((val1 - val2) / val1) * 100);
            }
          else
            {
              printf ("Mismatched Jacobian A [%u,%u] jac=%6e, a1=%6e a2=%6e %4.2f%%\n", static_cast<unsigned int> (rowk), static_cast<unsigned int> (colk), val1, val2, val3, std::abs ((val1 - val2) / val1) * 100);
            }
        }
    }

  tad.start();

  for (index_t nn = 0; nn < tad.size (); ++nn)
    {
	  auto el = tad.next();
      index_t rowk = el.row;
      index_t colk = el.col;


      double val1 = ad.at (rowk, colk);
      double val2 = el.data;
      double val3 = tad2.at (rowk, colk);
      if (val1 != 0)
        {
          continue;
        }
      if ((std::abs (val1 - val2) > jactol) && (std::abs (val1 - val3) > jactol) && (std::abs ((val1 - val2) / std::max (std::abs (val1), std::abs (val2))) > 2e-4))
        {
          //convergence
          if ((((std::abs (val3) / std::abs (val2)) > 10.0) && (std::abs (val1) < jactol)) || ((std::abs (val3) / std::abs (val2)) > 100.0))
            {
              continue;
            }
          //oscillatory convergence
          if ((val3 / val2 < 0) && (val1 > std::min (val3, val2)) && (val1 < std::max (val3, val2)) && (std::abs (val1) < jactol))
            {
              continue;
            }
          if ((std::abs (val3 - val1) / std::abs (val2 - val1)) > 30.0)
            {
              continue;
            }
          ++errors;
          printf ("Mismatched Jacobian B [%u,%u] jac=%f, a1=%f a2=%5f %4.2f%%\n", static_cast<unsigned int> (rowk), static_cast<unsigned int> (colk), val1, val2, val3, std::abs ((val2 - val1) / val2) * 100);
        }
    }

  return errors;
}


int residualCheck (gridDynSimulation *gds, const solverMode &sMode, double residTol, bool useStateNames)
{
  return residualCheck (gds, gds->getCurrentTime (), sMode, residTol, useStateNames);
}

int residualCheck (gridDynSimulation *gds, coreTime time, const solverMode &sMode, double residTol, bool useStateNames)
{
  if (isDynamic (sMode))
    {
      if (gds->currentProcessState () < gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED)
        {

          return -1;
        }
    }
  else if (gds->currentProcessState ()  < gridDynSimulation::gridState_t::INITIALIZED)
    {
      return -1;
    }
  stringVec stv;
  if (useStateNames)
    {
      gds->getStateName (stv, sMode);
    }
  int errors = 0;
  auto sd = gds->getSolverInterface (sMode);
  double *state = sd->state_data ();
  size_t nsize = sd->size ();
  assert (nsize == const_cast<const gridDynSimulation *>(gds)->stateSize (sMode));
  if (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED)
    {
      //sMode must be power flow or dc power flow to get here
      gds->guess (time, state, nullptr, sMode);
    }

  std::vector<double> resid (nsize);
  stateData sD (time, sd->state_data ());
  if (residTol < 0)       //make sure the tolerance is positive
    {
      residTol = resid_check_tol;
    }

  sD.dstate_dt = (isDAE (sMode)) ? sd->deriv_data () : nullptr;

  gds->residual (noInputs, sD, resid.data (), sMode);
  for (size_t kk = 0; kk < nsize; ++kk)
    {
      if (std::abs (resid[kk]) > residTol)
        {
          if (useStateNames)
            {
              printf ("non zero resid[%d](%s)=%6e\n", static_cast<int> (kk), stv[kk].c_str (), resid[kk]);
            }
          else
            {
              printf ("non-zeros resid[%d]=%6e\n", static_cast<int> (kk), resid[kk]);
            }
          ++errors;
        }
    }
  return errors;
}


int algebraicCheck (gridDynSimulation *gds, coreTime time, const solverMode &sMode, double algtol, bool useStateNames)
{
  if (isDynamic (sMode))
    {
      if (gds->currentProcessState () < gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED)
        {

          return -1;
        }
    }
  else if (gds->currentProcessState ()  < gridDynSimulation::gridState_t::INITIALIZED)
    {
      return -1;
    }
  stringVec stv;
  if (useStateNames)
    {
      gds->getStateName (stv, sMode);
    }

  auto sd = gds->getSolverInterface (sMode);
  double *state = sd->state_data ();
  size_t nsize = sd->size ();
  assert (nsize == const_cast<const gridDynSimulation *>(gds)->stateSize (sMode));
  if (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED)
    {
      //sMode must be power flow or dc power flow to get here
      gds->guess (time, state, nullptr, sMode);
    }
  else
    {
      gds->guess (time, state, sd->deriv_data (), sMode);
    }
  std::vector<double> update (nsize);

  if (algtol < 0)             //make sure the tolerance is positive
    {
      algtol = resid_check_tol;
    }
  stateData sD (time, sd->state_data ());
  sD.dstate_dt = (isDAE (sMode)) ? sd->deriv_data () : nullptr;

  gds->algebraicUpdate (noInputs, sD, update.data (), sMode,1.0);
  std::vector<double> vtype (nsize);

  gds->getVariableType (vtype.data (), sMode);
  int errors = 0;
  for (size_t kk = 0; kk < nsize; ++kk)
    {
      if (vtype[kk] > 0.01)
        {
          continue;
        }
      if (std::abs (update[kk] - sD.state[kk]) > algtol)
        {
          if (useStateNames)
            {
              printf ("mismatching updates[%d](%s)=%6e vs %6e\n", static_cast<int> (kk), stv[kk].c_str (), update[kk], sD.state[kk]);
            }
          else
            {
              printf ("mismatching updates[%d]=%6e vs %6e\n", static_cast<int> (kk), update[kk], sD.state[kk]);
            }
          ++errors;
        }
    }
  return errors;

}


int derivativeCheck (gridDynSimulation *gds, coreTime time, const solverMode &sMode, double derivtol, bool useStateNames)
{
  if (hasDifferential (sMode))
    {
      if (gds->currentProcessState () < gridDynSimulation::gridState_t::DYNAMIC_INITIALIZED)
        {

          return -1;
        }
    }
  else if (gds->currentProcessState ()  < gridDynSimulation::gridState_t::INITIALIZED)
    {
      return -1;
    }
  stringVec stv;
  if (useStateNames)
    {
      gds->getStateName (stv, sMode);
    }
  int errors = 0;
  auto sd = gds->getSolverInterface (sMode);
  double *state = sd->state_data ();
  size_t nsize = sd->size ();
  assert (nsize == const_cast<const gridDynSimulation *>(gds)->stateSize (sMode));
  if (gds->currentProcessState () == gridDynSimulation::gridState_t::INITIALIZED)
    {
      //sMode must be power flow or dc power flow to get here
      gds->guess (time, state, nullptr, sMode);
    }
  else
    {
      gds->guess (time, state, sd->deriv_data (), sMode);
    }
  std::vector<double> deriv (nsize);

  if (derivtol < 0)             //make sure the tolerance is positive
    {
      derivtol = resid_check_tol;
    }
  stateData sD (time, sd->state_data (), sd->deriv_data());

  gds->derivative (noInputs, sD, deriv.data (), sMode);
  std::vector<double> vtype (nsize);

  gds->getVariableType (vtype.data (), sMode);
  for (size_t kk = 0; kk < nsize; ++kk)
    {
      if (vtype[kk] < 0.1)
        {
          continue;
        }
      if (std::abs (deriv[kk] - sD.dstate_dt[kk]) > derivtol)
        {
          if (useStateNames)
            {
              printf ("mismatching derivative[%d](%s)=%6e\n", static_cast<int> (kk), stv[kk].c_str (), deriv[kk]);
            }
          else
            {
              printf ("mismatching derivative[%d]=%6e\n", static_cast<int> (kk), deriv[kk]);
            }
          ++errors;
        }
    }
  return errors;

}


void dynamicSolverConvergenceTest(gridDynSimulation *gds, const solverMode &sMode, const std::string &file, unsigned int pts, int mode)
{
	auto sd = gds->getSolverInterface(sMode);
	auto ssize = sd->size();

	double *state = sd->state_data();
	std::ofstream  bFile(file.c_str(), std::ios::out | std::ios::binary);

	std::vector<double> baseState(ssize, 0);

	std::vector<double> tempState(baseState);

	std::copy(state, state + ssize, baseState.begin());
	std::vector<double> vStates(ssize, 0);
	std::vector<double> nStates(ssize, -kBigNum);
	gds->getVoltageStates(vStates.data(), sMode);
	bFile.write((char *)(&ssize), sizeof(int));
	double inc = 1.51 / static_cast<double> (pts);
	double limitVal = 1.51;
	bFile.write((char *)(&inc), sizeof(double));
	bFile.write((char *)(&limitVal), sizeof(double));
	auto vsi = vecFindgt(vStates, 0.5);
	auto lstate = vsi.back();
	size_t cvs = vsi.size();
	bFile.write((char *)(&cvs), sizeof(size_t));

	auto tempLevel = sd->get("printlevel");
	sd->set("printLevel", "error");

	switch (mode)
	{
	case 0: //sequential all points
	default:
		for (auto &v : vsi)
		{
			state[v] = 1e-12;
		}

		while (state[lstate] < limitVal)
		{
			bFile.write((char *)state, ssize * sizeof(double));
			std::copy(state, state + ssize, tempState.begin());
			int retval = sd->calcIC(gds->getCurrentTime(), 0.001, solverInterface::ic_modes::fixed_diff, true);
			if (retval < 0)
			{
				double rval2 = retval;
				bFile.write((char *)(&rval2), sizeof(double));
				std::copy(tempState.begin(), tempState.begin() + ssize, state);
			}
			else
			{
				sd->getCurrentData();
				bFile.write((char *)(state), ssize * sizeof(double));
				std::copy(tempState.begin(), tempState.begin() + ssize, state);
			}

			state[vsi[0]] += inc;
			int ctr = 0;
			while (state[vsi[ctr]] > limitVal)
			{
				state[vsi[ctr]] = 1e-12;

				++ctr;
				state[vsi[ctr]] += inc;
				printf("inc %d-%f\n", ctr, state[vsi[ctr]]);
				if (ctr == static_cast<int> (cvs) - 1)
				{
					break;
				}
			}

		}
		break;
	case 1://random points
	{
		gridRandom rng(gridRandom::dist_type_t::uniform,0.0,1.51);
		std::vector<double> rvals(cvs);
		for (index_t kk = 0; kk < pts; ++kk)
		{
			rng.getNewValues(rvals, static_cast<count_t> (cvs));
			for (size_t jj = 0; jj < cvs; ++jj)
			{
				state[vsi[jj]] = rvals[jj];
			}
			bFile.write((char *)state, ssize * sizeof(double));
			std::copy(state, state + ssize, tempState.begin());
			int retval = sd->calcIC(gds->getCurrentTime(), 0.001, solverInterface::ic_modes::fixed_diff, true);
			if (retval < 0)
			{
				double rval2 = retval;
				bFile.write((char *)(&rval2), sizeof(double));
				std::copy(tempState.begin(), tempState.begin() + ssize, state);
			}
			else
			{
				sd->getCurrentData();
				bFile.write((char *)(state), ssize * sizeof(double));
				std::copy(tempState.begin(), tempState.begin() + ssize, state);
			}

		}
	}
	break;
	case 2:  // all the same sequence of points
		for (double val = 1e-12; val < 1.51; val += inc)
		{
			for (auto &v : vsi)
			{
				state[v] = val;
			}
			bFile.write((char *)state, ssize * sizeof(double));
			std::copy(state, state + ssize, tempState.begin());
			int retval = sd->calcIC(gds->getCurrentTime(), 0.001, solverInterface::ic_modes::fixed_diff, true);
			if (retval < 0)
			{
				double rval2 = retval;
				bFile.write((char *)(&rval2), sizeof(double));
				std::copy(tempState.begin(), tempState.begin() + ssize, state);
			}
			else
			{
				sd->getCurrentData();
				bFile.write((char *)(state), ssize * sizeof(double));
				std::copy(tempState.begin(), tempState.begin() + ssize, state);
			}
		}
		break;
	case 3://specific points
	{
		std::vector<std::vector<double>> ptsv
		{ {
				1, 1, 1
			},
			{
				0.5, 0.5, 0.5
			} };

		for (auto &v : ptsv)
		{
			for (size_t mm = 0; ((mm < v.size()) && (mm < cvs)); ++mm)
			{
				state[vsi[mm]] = v[mm];
			}
			std::copy(state, state + ssize, tempState.begin());
			sd->calcIC(gds->getCurrentTime(), 0.001, solverInterface::ic_modes::fixed_diff, true);
			std::copy(tempState.begin(), tempState.begin() + ssize, state);
		}
	}
	break;
	}

	sd->set("printLevel", tempLevel);
	std::copy(baseState.begin(), baseState.begin() + ssize, state);
}

std::vector<int> getRowCounts(matrixData<double> &ad)
{
	std::vector<int> rowcnt(ad.rowLimit());
	int sz = static_cast<int> (ad.size());
	ad.start();
	int ii = 0;
	while (ii < sz)
	{
		auto el = ad.next();
		++rowcnt[el.row];
		++ii;
	}
	return rowcnt;
}

std::vector<index_t> getLocalStates(const gridObject *obj,const solverMode &sMode)
{
	std::vector<index_t> st;
	auto off=obj->getOffsets(sMode);
	for (index_t ii = 0; ii < off->local.algSize; ++ii)
	{
		st.push_back(off->algOffset + ii);
	}
	for (index_t ii = 0; ii < off->local.diffSize; ++ii)
	{
		st.push_back(off->diffOffset + ii);
	}
	for (index_t ii = 0; ii < off->local.vSize; ++ii)
	{
		st.push_back(off->vOffset + ii);
	}
	for (index_t ii = 0; ii < off->local.aSize; ++ii)
	{
		st.push_back(off->aOffset + ii);
	}
	return st;
}

//helper class for aggregating information
class objectCountInfo
{
public:
	std::string name;
	count_t totalStates=0;
	count_t localStates=0;
	count_t localJacListed=0;
	count_t totalJacListed=0;
	count_t localJacActual=0;
	count_t totalJacActual=0;
	std::vector<objectCountInfo> subObjectInfo;
};
/** function to get the actual Jacobian information about an object*/
objectCountInfo getObjectInformation(const gridObject *obj, const solverMode &sMode, const std::vector<int> &rowCount)
{
	objectCountInfo objI;
	objI.name = obj->getName();
	objI.totalStates = obj->stateSize(sMode);
	
	auto lcStates = getLocalStates(obj, sMode);
	objI.localStates = static_cast<count_t>(lcStates.size());
	objI.totalJacListed = obj->jacSize(sMode);
	for (auto &st : lcStates)
	{
		objI.localJacActual += rowCount[st];
	}

	auto subobj = obj->getSubObject("subobject", 0);
	int ii=0;
	while (subobj != nullptr)
	{
		objI.subObjectInfo.push_back(getObjectInformation(static_cast<gridObject *>(subobj), sMode, rowCount));
		++ii;
		subobj = obj->getSubObject("subobject", ii);
	}
	objI.localJacListed = objI.totalJacListed;
	objI.totalJacActual = objI.localJacActual;
	for (auto &sui : objI.subObjectInfo)
	{
		objI.localJacListed -= sui.totalJacListed;
		objI.totalJacActual += sui.totalJacActual;
	}
	return objI;

}

void printObjCountInfo(const objectCountInfo &oi, int clevel, int maxLevel)
{
	for (int ii = 0; ii < clevel;++ii)
	{
		printf("  ");
	}
	printf("%s:: st %d(%d) list %d(%d) NNZ %d(%d)\n", oi.name.c_str(),
		oi.totalStates, oi.localStates, oi.totalJacListed, oi.localJacListed,
		oi.totalJacActual, oi.localJacActual);
	if (clevel < maxLevel)
	{
		for (auto &soi : oi.subObjectInfo)
		{
			printObjCountInfo(soi, clevel + 1, maxLevel);
		}
	}
}

void jacobianAnalysis(matrixData<double> &ad, gridDynSimulation *gds, const solverMode &sMode, int level)
{
	auto rc = getRowCounts(ad);
	auto oi = getObjectInformation(gds, sMode, rc);
	printObjCountInfo(oi, 0, level);

}


bool checkObjectEquivalence(const coreObject *obj1, const coreObject *obj2, bool printMessage)
{
	if (!((obj1) && (obj2)))
	{
		if (printMessage)
		{
			printf("at least one object is null\n");
		}
		return false;
	}
	if (typeid(*obj1) != typeid(*obj2))
	{
		if (printMessage)
		{
			printf("object 1 name (%s) not matching type of object 2(%s)\n", obj1->getName().c_str(), obj2->getName().c_str());
		}
		return false;
	}
	if (obj1->getName() != obj2->getName())
	{
		if (printMessage)
		{
			printf("object 1 name (%s) not matching object 2(%s)\n", obj1->getName().c_str(), obj2->getName().c_str());
		}
		return false;
	}
	if (obj1->getParent())
	{
		if (obj2->getParent())
		{
			if (obj1->getParent()->getName() != obj2->getParent()->getName())
			{	//these do not affect equivalence but should be noted
				if (printMessage)
				{
					printf("object 1 (%s) has a different parent than object 2(%s)\n", obj1->getName().c_str(), obj2->getName().c_str());
				
				}
			}
		}
		else
		{
			if (printMessage)
			{
				printf("object 1 (%s) has a different parent than object 2(%s)\n", obj1->getName().c_str(), obj2->getName().c_str());
			}
			
		}
	}
	if (obj1== obj2)
	{//these do not affect equivalence but should be noted
		if (printMessage)
		{
			printf("object 1 and object 2 (%s) have same id\n", obj1->getName().c_str());
		}
		return true;
	}
	if (obj1->get("subobjectcount") != obj2->get("subobjectcount"))
	{
		if (printMessage)
		{
			printf("object 1 (%s) has a different number of subobjects than object 2(%s)\n", obj1->getName().c_str(), obj2->getName().c_str());
		}
		return false;
	}
	int ii = 0;
	coreObject *sub1 = obj1->getSubObject("subobject", ii);
	bool result = true;
	while (sub1)
	{
		coreObject *sub2 = obj2->find(sub1->getName());
		if (!(sub2))
		{
			if (printMessage)
			{
				printf("object 2 (%s) does not have a subobject named %s\n", obj1->getName().c_str(), sub1->getName().c_str());
			}
			continue;
		}
		auto res = checkObjectEquivalence(sub1, sub2, printMessage);
		++ii;
		sub1 = obj1->getSubObject("subobject", ii);
		if (!res)
		{
			result = false;
		}
	}
	
	return result;
}