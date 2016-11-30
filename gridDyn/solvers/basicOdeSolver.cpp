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
#include "gridDyn.h"
#include "core/helperTemplates.h"
#include "vectorOps.hpp"
#include <algorithm>
#include <cmath>

basicOdeSolver::basicOdeSolver(const std::string &objName) : solverInterface(objName)
{
	mode.dynamic = true;
	mode.differential = true;
	mode.algebraic = false;
}
basicOdeSolver::basicOdeSolver(gridDynSimulation *gds, const solverMode& sMode) : solverInterface(gds, sMode)
{
}

std::shared_ptr<solverInterface> basicOdeSolver::clone(std::shared_ptr<solverInterface> si, bool fullCopy) const
{
	auto rp = cloneBase<basicOdeSolver, solverInterface>(this, si, fullCopy);
	if (!rp)
	{
		return si;
	}
	rp->deltaT = deltaT;
	return rp;
}
double * basicOdeSolver::state_data()
{
	return state.data();
}
double * basicOdeSolver::deriv_data()
{
	return deriv.data();
}
double * basicOdeSolver::type_data()
{
	return type.data();
}
const double * basicOdeSolver::state_data() const
{
	return state.data();
}
const double * basicOdeSolver::deriv_data() const
{
	return deriv.data();
}
const double * basicOdeSolver::type_data() const
{
	return type.data();
}

void basicOdeSolver::allocate(count_t stateCount, count_t numRoots)
{
	// load the vectors
	if (stateCount != svsize)
	{
		state.resize(stateCount);
		deriv.resize(stateCount);
		state2.resize(stateCount);
		svsize = stateCount;
		initialized = false;
		allocated = true;
		rootsfound.resize(numRoots);
	}
}

void basicOdeSolver::initialize(gridDyn_time t0)
{
	if (!allocated)
	{
		throw(InvalidSolverOperation(-2));
	}
	initialized = true;
	solverCallCount = 0;
	solveTime = t0;
}

double basicOdeSolver::get(const std::string & param) const
{
	if (param == "deltat")
	{
		return deltaT;
	}
	else
	{
		return solverInterface::get(param);
	}

}
void basicOdeSolver::set(const std::string &param, const std::string &val)
{

	if (param[0] == '#')
	{
		
	}
	else
	{
		solverInterface::set(param, val);
	}

}
void basicOdeSolver::set(const std::string &param, double val)
{

	if ((param == "delta")||(param=="deltat")||(param=="step")||(param=="steptime"))
	{
		deltaT = val;
	}
	else
	{
		solverInterface::set(param, val);
	}

}

int basicOdeSolver::solve(gridDyn_time tStop, gridDyn_time &tReturn, step_mode stepMode)
{
	if( solveTime==tStop)
	{
		tReturn = tStop;
		return FUNCTION_EXECUTION_SUCCESS;
	}
	gridDyn_time Tstep = (std::min)(deltaT, tStop - solveTime);
	if (mode.pairedOffsetIndex != kNullLocation)
	{
		int ret = m_gds->dynAlgebraicSolve(solveTime, state.data(), deriv.data(), mode);
		if (ret<FUNCTION_EXECUTION_SUCCESS)
		{
			return ret;
		}
	}
	m_gds->derivativeFunction(solveTime, state.data(), deriv.data(), mode);
	std::transform(state.begin(), state.end(), deriv.begin(), state.begin(), [Tstep](double a, double b) {return fma(Tstep, b, a); });
	solveTime += Tstep;
	//if we are in single step mode don't go into the loop
	if( stepMode==step_mode::normal)
	{
		while (solveTime < tStop)
		{
			Tstep = (std::min)(deltaT, tStop - solveTime);
			if (mode.pairedOffsetIndex != kNullLocation)
			{
				int ret = m_gds->dynAlgebraicSolve(solveTime, state.data(), deriv.data(), mode);
				if (ret<FUNCTION_EXECUTION_SUCCESS)
				{
					return ret;
				}
			}
			m_gds->derivativeFunction(solveTime, state.data(), deriv.data(), mode);
			std::transform(state.begin(), state.end(), deriv.begin(), state.begin(), [Tstep](double a, double b) {return fma(Tstep, b, a); });
			solveTime += Tstep;
		}
		
	}
	tReturn = solveTime;
	return FUNCTION_EXECUTION_SUCCESS;
}

