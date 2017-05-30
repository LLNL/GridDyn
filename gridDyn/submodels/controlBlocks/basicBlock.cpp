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

#include "submodels/otherBlocks.h"
#include "core/objectFactoryTemplates.h"
#include "utilities/vectorOps.hpp"
#include  "utilities/matrixData.h"
#include "utilities/stringConversion.h"
#include "core/coreObjectTemplates.h"
#include "valueLimiter.h"
#include "rampLimiter.h"

//object factory statements
static typeFactory<basicBlock> bbof ("controlblock", stringVec { "basic", "gain" }, "basic");
static childTypeFactory<controlBlock, basicBlock> cbof ("controlblock", "control");
static childTypeFactory<deadbandBlock, basicBlock> dbbof ("controlblock", stringVec {"deadband","db"} );
static childTypeFactory<delayBlock, basicBlock> dbof ("controlblock", stringVec { "delay", "filter" });
static childTypeFactory<pidBlock, basicBlock> pidbof ("controlblock", "pid");
static childTypeFactory<integralBlock, basicBlock> ibof ("controlblock", stringVec { "integrator", "integral" });
static childTypeFactory<functionBlock, basicBlock> fbof ("controlblock", stringVec { "function", "func" });
static childTypeFactory<lutBlock,basicBlock> lutbof ("controlblock", stringVec { "lut", "lookuptable"});
static childTypeFactory<derivativeBlock,basicBlock> derbof ("controlblock", stringVec { "der", "derivative", "deriv" });
static childTypeFactory<filteredDerivativeBlock,basicBlock> fderbof ("controlblock", stringVec { "fder", "filtered_deriv","filtered_derivative" });


basicBlock::basicBlock (const std::string &objName) : gridSubModel (objName)
{
  m_inputSize = 1;

}

basicBlock::basicBlock (double gain, const std::string &objName) : gridSubModel (objName),K (gain)
{
  m_inputSize = 1;
}

basicBlock::~basicBlock() = default;

coreObject *basicBlock::clone (coreObject *obj) const
{
  basicBlock *nobj = cloneBase<basicBlock, gridSubModel> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }

  nobj->K = K;
  nobj->bias = bias;
  nobj->Omax = Omax;
  nobj->Omin = Omin;
  nobj->rampMax = rampMax;
  nobj->rampMin = rampMin;
  nobj->resetLevel = resetLevel;
  nobj->limiter_alg = limiter_alg;
  nobj->limiter_diff = limiter_diff;
  nobj->outputName = outputName;
  return nobj;
}

void basicBlock::dynObjectInitializeA (coreTime /*time0*/, unsigned long /*flags*/)
{
	auto &lcinfo = offsets.local();
  lcinfo.reset ();
  offsets.unload ();       //unload all the offsets
  
  lcinfo.local.jacSize = 2;
  if (!opFlags[use_state])
    {
	  if (opFlags[use_direct])
	  {//In use direct mode it just processes the input also ignore the gain and bias
		  lcinfo.local.jacSize = 0;
	  }
	  else
	  {
		  if (opFlags[differential_input])
		  {
			  lcinfo.local.diffSize = 1;
			  opFlags.set(differential_output);
		  }
		  else
		  {
			  lcinfo.local.algSize = 1;
		  }
	  }
      
    }
  else if (opFlags[differential_output])
    {
	  lcinfo.local.diffSize = 1;
    }
  else
    {
	  lcinfo.local.algSize = 1;
    }
  limiter_alg = 0;
  limiter_diff = 0;

  if (resetLevel < 0)
  {
	  resetLevel = computeDefaultResetLevel();
  }
  if (opFlags[use_block_limits])
    {
      

      if (opFlags[differential_output])
        {
          ++(lcinfo.local.diffRoots);
          ++(lcinfo.local.diffSize);
          limiter_diff = 1;
        }
      else
        {
          ++(lcinfo.local.algRoots);
          ++(lcinfo.local.algSize);
          limiter_alg = 1;
        }
	  lcinfo.local.jacSize += 2;
	  vLimiter = std::make_unique<valueLimiter>(Omin,Omax);
	  vLimiter->setResetLevel(resetLevel);
    }
  if ((opFlags[use_ramp_limits])&& (opFlags[differential_output]))  //ramp limits only work with a differential output state before the limiters
    {
	 
      ++lcinfo.local.diffSize;

      ++lcinfo.local.diffRoots;

      ++limiter_diff;
	  lcinfo.local.jacSize += 2;
	  rLimiter = std::make_unique<rampLimiter>(rampMin, rampMax);
	  rLimiter->setResetLevel(resetLevel);
    }
  if (limiter_alg + limiter_diff > 0)
    {
      opFlags[has_limits] = true;
    }
  if (opFlags[differential_input])
  {
	  m_inputSize = 2;
  }
  
}

double basicBlock::getRateInput(const IOdata &inputs) const
{
	return (inputs.size() > 1) ? inputs[1] : 0.0;
}

double basicBlock::computeDefaultResetLevel()
{
	double rlevel = 0.001;
	if (Omax < kHalfBigNum)
	{
		if (Omin > -kHalfBigNum)
		{
			rlevel = (Omax - Omin) * 0.001;
		}
		else
		{
			rlevel = std::abs(Omax) * 0.001;
		}
	}
	else if(Omin > -kHalfBigNum)
	{
		rlevel = std::abs(Omin) * 0.001;
	}
	
	return rlevel;
}
// initial conditions
void basicBlock::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{

  if (desiredOutput.empty ())
    {
      prevInput = (inputs[0] + bias);
      if (!opFlags[use_state])
        {
		  if (!opFlags[use_direct])
		  {
			  m_state[limiter_alg + limiter_diff] = (prevInput)* K;
			  if (opFlags[use_ramp_limits])
			  {
				  m_state[limiter_diff - 1] = m_state[limiter_diff];
			  }
			  if (opFlags[use_block_limits])
			  {
				  basicBlock::rootCheck(inputs, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
				  m_state[0] = vLimiter->clampOutput(m_state[1]);
			  }
		  }
          
        }
      else
        {
          if (opFlags[use_ramp_limits])
            {
              index_t diffOffset = offsets.local().local.algSize + limiter_diff;
              m_state[diffOffset - 1] = m_state[diffOffset];
              if (opFlags[use_block_limits])
                {
                  basicBlock::rootCheck (inputs, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
				  m_state[0] = vLimiter->clampOutput(m_state[diffOffset - 1]);
                }
            }
          else
            {
              if (opFlags[use_block_limits])
                {
                  if (opFlags[differential_output])
                    {
                      index_t diffOffset = offsets.local().local.algSize;
                      basicBlock::rootCheck (inputs, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
                      m_state[0] = vLimiter->clampOutput(m_state[diffOffset]);
                    }
                  else
                    {
                      basicBlock::rootCheck (inputs, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
					  m_state[0]=vLimiter->clampOutput(m_state[1]);
                    }

                }
            }

        }

      fieldSet[0] = m_state[0];
    }
  else
    {
      m_state[0] = desiredOutput[0];
      if (opFlags[use_block_limits])
        {
		  m_state[0] = vLimiter->clampOutput(m_state[0]);
        }

      if (!opFlags[use_state])
        {
          if (opFlags[use_block_limits])
            {
              m_state[1] = m_state[0];
            }
          if (opFlags[use_ramp_limits])
            {
              m_state[2] = m_state[0];              //we know the layout in this case
            }
        }
      else
        {

          if (opFlags[use_ramp_limits])
            {
              index_t diffOffset = offsets.getDiffOffset (cLocalSolverMode) + limiter_diff;

              if (opFlags[use_block_limits])
                {
                  m_state[limiter_diff - 1] = m_state[0];
                }
              m_state[diffOffset] = m_state[0];
              m_state[diffOffset + 1] = m_state[0];
            }
          else
            {
              if (opFlags[use_block_limits])
                {
                  if (opFlags[differential_output])
                    {
                      index_t diffOffset = offsets.getDiffOffset (cLocalSolverMode) + limiter_diff;
                      basicBlock::rootCheck (inputs, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
                      m_state[diffOffset] = m_state[0];
                    }
                  else
                    {
                      m_state[1] = m_state[0];
                    }

                }
            }

        }
      fieldSet[0] = m_state[0] / K - bias;
      prevInput = fieldSet[0] + bias;
    }


}


void basicBlock::timestep (coreTime ttime, const IOdata &inputs, const solverMode &)
{
  step (ttime, inputs[0]);
}

static IOdata kNullVec;

double basicBlock::step (coreTime ttime, double input)
{

  if (!opFlags[use_state])
    {
      m_state[limiter_alg + limiter_diff] = (input + bias) * K;

      if (opFlags[has_limits])
        {
          if (opFlags[use_ramp_limits])
            {
              int offset = offsets.getDiffOffset (cLocalSolverMode);
              double ramp = (m_state[offset + 1] - m_state[offset]) / (ttime - prevTime);
			  double cramp = rLimiter->clampOutputRamp(ramp);
              if (cramp==ramp)
                {
                  m_state[offset] = m_state[offset + 1];
                }
              else 
                {
                  m_state[offset] += cramp * (ttime - prevTime);
                }
            }
          else
            {
			  rootCheck({ input }, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
              m_state[0] = vLimiter->clampOutput (m_state[1]);
            }
        }
    }
  else
    {
      if (opFlags[use_ramp_limits])
        {
          int offset = offsets.getDiffOffset (cLocalSolverMode);
          double ramp = (m_state[offset + 1] - m_state[offset]) / (ttime - prevTime);
		  double cramp = rLimiter->clampOutputRamp(ramp);
		  if (cramp == ramp)
            {
              m_state[offset] = m_state[offset + 1];
            }
		  else
		  {
			  m_state[offset] += ramp * (ttime - prevTime);
		  }
        }
      else
        {
          if (opFlags[use_block_limits])
            {
              auto offset = opFlags[differential_output] ? (offsets.getDiffOffset (cLocalSolverMode)) + 1 : 1;
              rootCheck (kNullVec, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
              m_state[offset - 1] = vLimiter->clampOutput (m_state[offset]);
            }

        }
    }
  prevTime = ttime;
  auto offset = opFlags[differential_output] ? (offsets.getDiffOffset (cLocalSolverMode)) : 0;
  return m_state[offset];
}

double basicBlock::getBlockOutput (const stateData &sD, const solverMode &sMode) const
{
  Lp Loc = offsets.getLocations (sD, sMode, this);
  return opFlags[differential_output] ? *Loc.diffStateLoc : *Loc.algStateLoc;
}

double basicBlock::getBlockOutput () const
{
  auto offset = opFlags[differential_output] ? (offsets.getDiffOffset (cLocalSolverMode)) : 0;
  return m_state[offset];
}

double basicBlock::getBlockDoutDt(const stateData &sD, const solverMode &sMode) const
{
	if (opFlags[differential_output])
	{
		Lp Loc = offsets.getLocations(sD, sMode, this);
		return *Loc.dstateLoc;
	}
	return 0.0;
	
}

double basicBlock::getBlockDoutDt() const
{
	if (opFlags[differential_output])
	{
		auto offset = opFlags[differential_output] ? (offsets.getDiffOffset(cLocalSolverMode)) : 0;
		return m_dstate_dt[offset];
	}
	return 0.0;
}

void basicBlock::residElements (double input, double didt, const stateData &sD, double resid[], const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (so->total.diffSize > 0)
    {
      derivElements (input, didt, sD, resid, sMode);
      for (index_t ii = 0; ii < so->total.diffSize; ++ii)
        {
          resid[so->diffOffset + ii] -= sD.dstate_dt[so->diffOffset + ii];
        }
    }


  if (so->total.algSize > 0)
    {
      algElements (input, sD, resid, sMode);
      for (index_t ii = 0; ii < so->total.algSize; ++ii)
        {
		  
          resid[so->algOffset + ii] -= sD.state[so->algOffset + ii];
		  /*if ((vLimiter) && (vLimiter->isActive()))
		  {
			  printf("%d:%d:%f input=%f , state=%f resid=%f\n", sD.seqID, ii,static_cast<double>(sD.time), input, sD.state[so->algOffset + ii], resid[so->algOffset + ii]);
		  }
		  */
        }
    }

}

void basicBlock::limiterResidElements(double input, double didt, const stateData &sD, double resid[], const solverMode &sMode)
{
	if (opFlags[differential_output])
	{
		auto offset = offsets.getDiffOffset(sMode) + limiter_diff;
		double testVal = getTestRate(didt, sD.dstate_dt[offset]);
		
		if (limiter_diff > 0)
		{
			if (opFlags[use_ramp_limits])
			{
				--offset;
				resid[offset] = rLimiter->deriv(testVal);
				testVal = resid[offset];
				resid[offset] -= sD.dstate_dt[offset];
			}
			if (opFlags[use_block_limits])
			{
				resid[offset - 1] = vLimiter->deriv(testVal)-sD.dstate_dt[offset-1];

			}
		}
	}
	else
	{
		auto offset = offsets.getAlgOffset(sMode) + limiter_alg;
		double testVal = getTestValue(input, sD.state[offset]);
		if (opFlags[has_limits])
		{
			resid[offset - 1] = vLimiter->output(testVal)-sD.state[offset-1];
		}
	}
	

	auto so = offsets.getOffsets(sMode);
	if (so->total.diffSize > 0)
	{
		derivElements(input, didt, sD, resid, sMode);
		for (index_t ii = 0; ii < so->total.diffSize; ++ii)
		{
			resid[so->diffOffset + ii] -= sD.dstate_dt[so->diffOffset + ii];
		}
	}


	if (so->total.algSize > 0)
	{
		algElements(input, sD, resid, sMode);
		for (index_t ii = 0; ii < so->total.algSize; ++ii)
		{
			resid[so->algOffset + ii] -= sD.state[so->algOffset + ii];
		}
	}

}
// residual
void basicBlock::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  residElements (inputs[0],getRateInput(inputs), sD, resid, sMode);

}

bool basicBlock::hasValueState() const
{
	return (!((opFlags[use_state]) || (opFlags[use_direct])));
}

double basicBlock::getTestValue(double input, double currentState) const
{
	double testVal;
	if (opFlags[use_state])
	{
		testVal = currentState;
	}
	else if (opFlags[use_direct])
	{
		testVal = input*K;
	}
	else
	{
		testVal = (input + bias) * K;
	}
	return testVal;
}

double basicBlock::getTestRate(double didt, double currentStateRate) const
{
	double testRate;
	if (opFlags[use_state])
	{
		testRate = currentStateRate;
	}
	else 
	{
		testRate = didt*K;
	}
	return testRate;
}

void basicBlock::algElements (double input, const stateData &sD, double update[], const solverMode &sMode)
{
  if (opFlags[differential_output])
    {
      return;
    }
  
  auto offset = offsets.getAlgOffset(sMode) + limiter_alg;
  double testVal = getTestValue(input, sD.state[offset]);
  if (hasValueState())
  {
	  update[offset] = testVal;
	  testVal = sD.state[offset]; //need to alter the testVal for the next check otherwise the residual fails to check properly
  }
  if (opFlags[has_limits])
  {
	  update[offset - 1] = vLimiter->output(testVal);
  }
  
}

void basicBlock::algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double /*alpha*/)
{
  algElements (inputs[0],  sD, update, sMode);
}

void basicBlock::derivElements (double /*input*/, double didt, const stateData &sD, double deriv[], const solverMode &sMode)
{
	if (opFlags[differential_output])
	{

		auto offset = offsets.getDiffOffset(sMode) + limiter_diff;
		double testVal = getTestRate(didt, sD.dstate_dt[offset]);
		if (hasValueState())
		{
			deriv[offset] = testVal;
		}
		if (limiter_diff > 0)
		{
			if (opFlags[use_ramp_limits])
			{
				--offset;
				deriv[offset] = rLimiter->output(testVal);
				testVal = deriv[offset];
			}
			if (opFlags[use_block_limits])
			{
				deriv[offset - 1] = vLimiter->deriv(testVal);
			}
		}
	}
}
// residual
void basicBlock::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  derivElements (inputs[0], getRateInput(inputs),sD, deriv, sMode);
}


void basicBlock::jacElements (double /*input*/, double /*didt*/, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{

  if ((opFlags[differential_output]) && (hasDifferential (sMode)))
    {
      auto offset = offsets.getDiffOffset (sMode) + limiter_diff;
      if (hasValueState())
        {
          ad.assignCheckCol (offset, argLoc, K * sD.cj);
          ad.assign (offset, offset, -sD.cj);
        }
      if (limiter_diff > 0)
        {
          if (opFlags[use_ramp_limits])
            {
              --offset;
              ad.assign (offset, offset, -sD.cj);
			  if (opFlags[use_direct])
			  {
				  ad.assignCheckCol(offset, argLoc, K*sD.cj*rLimiter->DoutDin());
			  }
			  else
			  {
				  ad.assign(offset, offset + 1, sD.cj*rLimiter->DoutDin());
			  }
			  
            }
          if (opFlags[use_block_limits])
            {
              --offset;
              ad.assign (offset, offset, -sD.cj);

			  if ((opFlags[use_direct])&&(!opFlags[use_ramp_limits]))
			  {
				  ad.assignCheckCol(offset, argLoc, K*sD.cj*vLimiter->DoutDin());
			  }
			  else
			  {
				  ad.assign(offset, offset + 1, sD.cj*vLimiter->DoutDin());
			  }
            }
        }
    }
  //Now do the algebraic states if needed
  if ((!opFlags[differential_output]) && (hasAlgebraic (sMode)))
    {
      auto offset = offsets.getAlgOffset (sMode) + limiter_alg;
      if (hasValueState())
        {
          ad.assignCheckCol (offset, argLoc, K);
          ad.assign (offset, offset, -1.0);
        }
      if (limiter_alg > 0)
        {
          --offset;
          ad.assign (offset, offset, -1.0);
		  if (opFlags[use_direct])
		  {
			  ad.assign(offset, argLoc, K*vLimiter->DoutDin());
		  }
		  else
		  {
			  ad.assign(offset, offset + 1, vLimiter->DoutDin());
		  }
         
        }
    }
}


void basicBlock::jacobianElements (const IOdata & inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{
  jacElements  (inputs[0], getRateInput(inputs),sD, ad, inputLocs[0], sMode);

}

double basicBlock::getLimiterTestValue(double input, const stateData &sD, const solverMode &sMode)
{

	auto offset= (opFlags[differential_output])?offsets.getDiffOffset(sMode): offsets.getAlgOffset(sMode);
	auto stateVal = (sD.empty()) ? m_state[1] : sD.state[offset + 1];
	if (hasValueState() || opFlags[use_ramp_limits])
	{
		return stateVal;
	}
	else
	{
		return getTestValue(input, stateVal);
	}
	
}

void basicBlock::rootTest (const IOdata & inputs, const stateData &sD, double root[], const solverMode &sMode)
{
  if (!opFlags[has_limits])
    {
      return;
    }
  int rootOffset = offsets.getRootOffset (sMode);
  if (opFlags[use_ramp_limits])
  {
	  auto doffset = offsets.getDiffOffset(sMode) + limiter_diff;
	  double testRate = getTestRate(inputs[1], sD.dstate_dt[doffset]);
	  double testVal = getTestValue(inputs[0], sD.state[doffset]);
	  root[rootOffset] = rLimiter->limitCheck(sD.state[doffset], testVal, testRate);
	  ++rootOffset;
  }

  if (opFlags[use_block_limits])
    {
	  double val = getLimiterTestValue(inputs[0], sD, sMode);
	  root[rootOffset] = vLimiter->limitCheck(val);
    }

}

change_code basicBlock::rootCheck (const IOdata & inputs, const stateData &sD, const solverMode &sMode, check_level_t /*level*/)
{
  change_code ret = change_code::no_change;
  if (!opFlags[has_limits])
    {
      return ret;
    }
  const double *st = ((!sD.empty()) ? sD.state : m_state.data ());
  const double *dst = ((!sD.empty()) ? sD.dstate_dt : m_dstate_dt.data ());
  if (opFlags[use_ramp_limits])
  {
		  auto doffset = offsets.getDiffOffset(sMode);
		  double testRate = getTestRate(getRateInput(inputs), dst[doffset]);
		  double testVal = getTestValue(inputs[0], st[doffset]);
		  double vval = rLimiter->limitCheck(st[doffset], testVal, testRate);
		  if (vval < 0.0)
		  {
			  rLimiter->changeLimitActivation(testRate);
			  ret = change_code::non_state_change;
		  }
  }
  if (opFlags[use_block_limits])
  {
	  double val = getLimiterTestValue(inputs[0], sD, sMode);
		  double vval = vLimiter->limitCheck(val);
		  if (vval < 0.0)
		  {
			  vLimiter->changeLimitActivation(val);
			  ret = change_code::non_state_change;
		  }
	  }

  return ret;

}

void basicBlock::rootTrigger (coreTime /*ttime*/, const IOdata & inputs, const std::vector<int> &rootMask, const solverMode &sMode)
{
  if (!opFlags[has_limits])
    {
      return;
    }
  auto roffset = offsets.getRootOffset (sMode);

  if (opFlags[use_ramp_limits])
    {
	  if (rootMask[roffset])
	  {
		  auto doffset = offsets.getDiffOffset(cLocalSolverMode);
		  double testRate = getTestRate(getRateInput(inputs), m_dstate_dt[doffset]);
		  rLimiter->changeLimitActivation(testRate);
		  //ret = change_code::non_state_change;
	  }
      ++roffset;
    }
  if (opFlags[use_block_limits])
    {
      if (!rootMask[roffset])
        {
          return;
        }
	  double val = getLimiterTestValue(inputs.empty()?m_state[0]:inputs[0], emptyStateData, sMode);
	 
	  vLimiter->changeLimitActivation(val);
	  m_state[0] = vLimiter->output(val);
    }
}

void basicBlock::setFlag (const std::string &flag, bool val)
{

  if (flag == "use_limits")
    {
      if (!opFlags[dyn_initialized])
        {
          opFlags[has_limits] = val;
          opFlags[use_block_limits] = val;
          opFlags[use_ramp_limits] = val;
        }
    }
  else if (flag == "simplified")
    {

      if (opFlags[dyn_initialized])
        {
          if (opFlags[simplified] != val)              //this is probably not the best thing to be changing after initialization
            {
              opFlags[simplified] = val;
              dynObjectInitializeA (prevTime, 0);
              alert (this, STATE_COUNT_CHANGE);
              LOG_WARNING ("changing object state computations during simulation triggers solver reset");
            }
        }
      else
        {
          opFlags[simplified] = val;
        }
    }
  else if (flag == "use_direct")
  {
	  if (!opFlags[dyn_initialized])
	  {
		  opFlags[use_direct] = val;
	  }
  }
  else if (flag == "differential_input")
    {
      if (!opFlags[dyn_initialized])
        {
          opFlags[differential_input] = val;
        }
    }
  else if (flag == "use_ramp_limits")
    {
      if (!opFlags[dyn_initialized])
        {
          opFlags[use_ramp_limits] = val;
        }
    }
  else
    {
      gridSubModel::setFlag (flag, val);
    }

}


// set parameters
void basicBlock::set (const std::string &param,  const std::string &val)
{
  gridSubModel::set (param, val);
}

void basicBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{
 
  //param   = gridDynSimulation::toLower(param);

  if ((param == "k")||(param == "gain"))
    {
      K = val;
    }
  else if ((param == "bias") || (param == "b"))
    {
      bias = val;
    }
  else if ((param == "omax") || (param == "max"))
    {
      Omax = val;
	  valLimiterUpdate();
      
    }
  else if ((param == "omin") || (param == "min"))
    {
      Omin = val;
	  valLimiterUpdate();
    }
  else if (param == "limit")
    {
      Omax = val;
      Omin = -val;
	  valLimiterUpdate();
    }
  else if (param == "rampmax")
    {
      rampMax = val;
	  rampLimiterUpdate();
    }
  else if (param == "rampmin")
    {
      rampMin = val;
	  rampLimiterUpdate();
    }
  else if (param == "ramplimit")
    {
      rampMin = -val;
      rampMax = val;
	  rampLimiterUpdate();
    }

  else if (param == "resetlevel")
    {
      resetLevel = val;
	  if (vLimiter)
	  {
		  vLimiter->setResetLevel(val);
	  }
	  if (rLimiter)
	  {
		  rLimiter->setResetLevel(val);
	  }
    }
  else
    {
      gridSubModel::set (param, val, unitType);
    }

}

double basicBlock::get(const std::string &param, gridUnits::units_t unitType) const
{
	if (param == "maxstepsize")
	{
		return kBigNum;
	}
	else
	{
		return gridSubModel::get(param, unitType);
	}
}

void basicBlock::valLimiterUpdate()
{
	if (!opFlags[dyn_initialized])
	{
		opFlags.set(use_block_limits);
	}
	else
	{
		if (vLimiter)
		{
			vLimiter->setLimits(Omin, Omax);
		}
	}
}

void basicBlock::rampLimiterUpdate()
{
	if (!opFlags[dyn_initialized])
	{
		opFlags.set(use_ramp_limits);
	}
	else
	{
		if (rLimiter)
		{
			rLimiter->setLimits(rampMin, rampMax);
		}
	}
}
std::unique_ptr<basicBlock> make_block (const std::string &blockstr)
{
  auto posp1 = blockstr.find_first_of ('(');
  auto posp2 = blockstr.find_last_of (')');
  auto fstr = blockstr.substr (0, posp1 - 1);
  auto argstr = blockstr.substr (posp1 + 1, posp2 - posp1 - 1);

  auto inputs = str2vector (argstr,kNullVal);
  auto tail = blockstr.substr (posp2 + 2);
  auto tailArgs = stringOps::splitline (tail);
  stringOps::trim(tailArgs);
  double gain = 1.0;
  posp1 = fstr.find_first_of ('*');
  std::unique_ptr<basicBlock> ret;
  if (posp1 == std::string::npos)
    {
      makeLowerCase (fstr);
    }
  else
    {
      gain = numeric_conversion (fstr,1.0);           //purposely not using numeric_conversionComplete to just get the first number
      fstr = convertToLowerCase (fstr.substr (posp1 + 1));

    }
  if (fstr == "basic")
    {
      ret = std::make_unique<basicBlock> (gain);
    }
  else if ((fstr == "der") || (fstr == "derivative"))
    {
      if (inputs.empty ())
        {
          ret = std::make_unique<derivativeBlock> ();
        }
      else
        {
          ret = std::make_unique<derivativeBlock> (inputs[0]);
        }
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if ((fstr == "integral")||(fstr == "integrator"))
    {
      ret = std::make_unique<integralBlock> (gain);
    }
  else if (fstr == "control")
    {
      if (inputs.empty ())
        {
          ret = std::make_unique<controlBlock> ();
        }
      else if (inputs.size () == 1)
        {
          ret = std::make_unique<controlBlock> (inputs[0]);
        }
      else
        {
          ret = std::make_unique<controlBlock> (inputs[0],inputs[1]);
        }
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if (fstr == "delay")
    {
      if (inputs.empty ())
        {
          ret = std::make_unique<delayBlock> ();
        }
      else
        {
          ret = std::make_unique<delayBlock> (inputs[0]);
        }
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if (fstr == "deadband")
    {
      if (inputs.empty ())
        {
          ret = std::make_unique<deadbandBlock> ();
        }
      else
        {
          ret = std::make_unique<deadbandBlock> (inputs[0]);
        }
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if (fstr == "pid")
    {
      double p = 1.0, i = 0.0, d = 0.0;
      if (inputs.size () > 0)
        {
          p = inputs[0];
        }
      if (tailArgs.size () > 1)
        {
          i = inputs[1];
        }
      if (inputs.size () > 2)
        {
          d = inputs[2];
        }

      ret = std::make_unique<pidBlock> (p,i,d);
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if (fstr == "function")
    {
      if (argstr.empty ())
        {
          ret = std::make_unique<functionBlock> ();
        }
      else
        {
          ret = std::make_unique<functionBlock> (argstr);
        }
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else
    {
      return ret;
    }
  //process any additional parameters
  if (!tailArgs.empty ())
    {
      for (auto &ta : tailArgs)
        {
          auto eloc = ta.find_first_of ('=');
          if (eloc == std::string::npos)
            {
              ret->setFlag (ta, true);
            }
          else
            {
              std::string param = ta.substr (0, eloc);
              double val = numeric_conversionComplete (ta.substr (eloc + 1), kNullVal);
              if (val == kNullVal)
                {
                  ret->set (param, ta.substr (eloc + 1));
                }
              else
                {
                  ret->set (param, val);
                }
            }
        }
    }
  return ret;
}

stringVec basicBlock::localStateNames () const
{
	stringVec stNames;
	if (opFlags[use_block_limits])
	{
		stNames.push_back("block_limits");
	}
	if (opFlags[use_ramp_limits])
	{
		stNames.push_back("ramp_limits");
	}
	if (hasValueState())
	{
		stNames.push_back("test");
	}
	if (!stNames.empty())
	{
		stNames[0] = outputName;
	}
	return stNames;
}
