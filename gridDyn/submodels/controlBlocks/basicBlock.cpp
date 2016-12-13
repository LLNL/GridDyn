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

#include "submodels/otherBlocks.h"
#include "objectFactoryTemplates.h"
#include "vectorOps.hpp"
#include "matrixData.h"
#include "stringConversion.h"
#include "gridCoreTemplates.h"

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

static const stringVec stNames {
  "output", "test"
};

basicBlock::basicBlock (const std::string &objName) : gridSubModel (objName)
{
  m_inputSize = 1;

}

basicBlock::basicBlock (double gain, const std::string &objName) : gridSubModel (objName),K (gain)
{
  m_inputSize = 1;
}

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
  return nobj;
}

void basicBlock::objectInitializeA (gridDyn_time /*time0*/, unsigned long /*flags*/)
{
  offsets.local->reset ();
  offsets.unload ();       //unload all the offsets
  offsets.local->local.jacSize = 2;
  if (!opFlags[use_state])
    {
      if (opFlags[differential_input])
        {
          offsets.local->local.diffSize = 1;
          opFlags.set (differential_output);
        }
      else
        {
          offsets.local->local.algSize = 1;
        }
    }
  else if (opFlags[differential_output])
    {
      offsets.local->local.diffSize = 1;
    }
  else
    {
      offsets.local->local.algSize = 1;
    }
  limiter_alg = 0;
  limiter_diff = 0;
  if (opFlags[use_block_limits])
    {
      if (resetLevel < 0)
        {
          resetLevel = (Omin > -kHalfBigNum) ? ((Omax < kHalfBigNum) ? (Omax - Omin) * 0.001 : std::abs (Omin) * 0.001) : std::abs (Omax) * 0.001;
        }

      if (opFlags[differential_output])
        {
          ++offsets.local->local.diffRoots;
          ++offsets.local->local.diffSize;
          limiter_diff = 1;
        }
      else
        {
          ++offsets.local->local.algRoots;
          ++offsets.local->local.algSize;
          limiter_alg = 1;
        }
      offsets.local->local.jacSize += 2;

    }
  if ((opFlags[use_ramp_limits])&& (opFlags[differential_output]))  //ramp limits only work with a differential output state before the limiters
    {
      ++offsets.local->local.diffSize;

      ++offsets.local->local.diffRoots;

      ++limiter_diff;
      offsets.local->local.jacSize += 2;
    }
  if (limiter_alg + limiter_diff > 0)
    {
      opFlags[has_limits] = true;
    }

}
// initial conditions
void basicBlock::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{

  if (outputSet.empty ())
    {
      prevInput = (args[0] + bias);
      if (!opFlags[use_state])
        {
          m_state[limiter_alg + limiter_diff] = (prevInput) * K;
          if (opFlags[use_ramp_limits])
            {
              m_state[limiter_diff - 1] = m_state[limiter_diff];
            }
          if (opFlags[use_block_limits])
            {
              basicBlock::rootCheck (args, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
              m_state[0] = valLimit (m_state[1], Omin, Omax);
            }
        }
      else
        {
          if (opFlags[use_ramp_limits])
            {
              index_t diffOffset = offsets.local->local.algSize + limiter_diff;
              m_state[diffOffset - 1] = m_state[diffOffset];
              if (opFlags[use_block_limits])
                {
                  basicBlock::rootCheck (args, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
                  m_state[0] = valLimit (m_state[diffOffset - 1], Omin, Omax);
                }
            }
          else
            {
              if (opFlags[use_block_limits])
                {
                  if (opFlags[differential_output])
                    {
                      index_t diffOffset = offsets.local->local.algSize;
                      basicBlock::rootCheck (args, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
                      m_state[0] = valLimit (m_state[diffOffset], Omin, Omax);
                    }
                  else
                    {
                      basicBlock::rootCheck (args, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
                      m_state[0] = valLimit (m_state[1], Omin, Omax);
                    }

                }
            }

        }

      fieldSet[0] = m_state[0];
    }
  else
    {
      m_state[0] = outputSet[0];
      if (opFlags[use_block_limits])
        {
          m_state[0] = valLimit (m_state[0], Omin, Omax);
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
                      basicBlock::rootCheck (args, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
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


void basicBlock::timestep (gridDyn_time ttime, const IOdata &args, const solverMode &)
{
  step (ttime, args[0]);
}

static IOdata kNullVec;

double basicBlock::step (gridDyn_time ttime, double input)
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
              if ((ramp <= rampMax) && (ramp >= rampMin))
                {
                  m_state[offset] = m_state[offset + 1];
                }
              else if (ramp > rampMax)
                {
                  m_state[offset] += rampMax * (ttime - prevTime);
                }
              else
                {
                  m_state[offset] += rampMin * (ttime - prevTime);
                }

            }
          else
            {
              rootCheck (kNullVec, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
              m_state[0] = valLimit (m_state[1], Omin, Omax);
            }
        }
    }
  else
    {
      if (opFlags[use_ramp_limits])
        {
          int offset = offsets.getDiffOffset (cLocalSolverMode);
          double ramp = (m_state[offset + 1] - m_state[offset]) / (ttime - prevTime);
          if ((ramp <= rampMax) && (ramp >= rampMin))
            {
              m_state[offset] = m_state[offset + 1];
            }
          else if (ramp > rampMax)
            {
              m_state[offset] += rampMax * (ttime - prevTime);
            }
          else
            {
              m_state[offset] += rampMin * (ttime - prevTime);
            }
        }
      else
        {
          if (opFlags[use_block_limits])
            {
              auto offset = opFlags[differential_output] ? (offsets.getDiffOffset (cLocalSolverMode)) + 1 : 1;
              rootCheck (kNullVec, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
              m_state[offset - 1] = valLimit (m_state[offset], Omin, Omax);
            }

        }
    }
  prevTime = ttime;
  auto offset = opFlags[differential_output] ? (offsets.getDiffOffset (cLocalSolverMode)) : 0;
  return m_state[offset];
}

double basicBlock::getBlockOutput (const stateData &sD, const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD, sMode, this);
  return opFlags[differential_output] ? *Loc.diffStateLoc : *Loc.algStateLoc;
}

double basicBlock::getBlockOutput ()
{
  auto offset = opFlags[differential_output] ? (offsets.getDiffOffset (cLocalSolverMode)) : 0;
  return m_state[offset];
}

void basicBlock::residElements (double input, double didt, const stateData &sD, double resid[], const solverMode &sMode)
{
  if ((opFlags[differential_output]) && (!hasDifferential (sMode)))
    {
      return;
    }

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
        }
    }

}

// residual
void basicBlock::residual (const IOdata &args, const stateData &sD, double resid[], const solverMode &sMode)
{
  residElements (args[0],(args.size () > 1) ? args[1] : 0.0, sD, resid, sMode);

}

void basicBlock::algElements (double input, const stateData &sD, double update[], const solverMode &sMode)
{
  if (opFlags[differential_output])
    {
      return;
    }
  auto offset = offsets.getAlgOffset (sMode) + limiter_alg;

  if (!opFlags[use_state])
    {
      update[offset] = (input + bias) * K;
    }
  if (opFlags[has_limits])
    {
      --offset;
      if (opFlags[outside_lim])
        {
          if (opFlags[trigger_high])
            {
              update[offset] = Omax;
            }
          else
            {
              update[offset] = Omin;
            }
        }
      else
        {
          update[offset] = sD.state[offset + 1];
        }
    }
}

void basicBlock::algebraicUpdate (const IOdata &args, const stateData &sD, double update[], const solverMode &sMode, double /*alpha*/)
{
  algElements (args[0],  sD, update, sMode);
}

void basicBlock::derivElements (double /*input*/, double didt, const stateData &sD, double deriv[], const solverMode &sMode)
{
  if (opFlags[differential_output])
    {
      auto offset = offsets.getDiffOffset (sMode) + limiter_diff;
      if (!opFlags[use_state])
        {
          deriv[offset] = didt * K;
        }
      if (limiter_diff > 0)
        {

          if (opFlags[use_ramp_limits])
            {
              --offset;
              if (opFlags[ramp_limit_triggered])
                {
                  if (opFlags[ramp_limit_triggered_high])
                    {
                      deriv[offset] = rampMax;
                    }
                  else
                    {
                      deriv[offset] = rampMin;
                    }
                }
              else
                {
                  deriv[offset] = sD.dstate_dt[offset + 1];
                }
            }
          if (opFlags[use_block_limits])
            {
              --offset;
              if (opFlags[outside_lim])
                {
                  deriv[offset] = 0;
                }
              else
                {
                  deriv[offset] = sD.dstate_dt[offset + 1];
                }
            }
        }
    }
}
// residual
void basicBlock::derivative (const IOdata &args, const stateData &sD, double deriv[], const solverMode &sMode)
{
  derivElements (args[0], (args.size () > 1) ? args[1] : 0.0,sD, deriv, sMode);

}


void basicBlock::jacElements (double /*input*/, double /*didt*/, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{

  if ((opFlags[differential_output]) && (hasDifferential (sMode)))
    {
      auto offset = offsets.getDiffOffset (sMode) + limiter_diff;
      if (!opFlags[use_state])
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

              if (!opFlags[ramp_limit_triggered])
                {
                  ad.assign (offset, offset + 1, sD.cj);
                }
            }
          if (opFlags[use_block_limits])
            {
              --offset;
              ad.assign (offset, offset, -sD.cj);
              if (!opFlags[outside_lim])
                {
                  ad.assign (offset, offset + 1, sD.cj);
                }
            }
        }
    }
  //Now do the algebraic states if needed
  if ((!opFlags[differential_output]) && (hasAlgebraic (sMode)))
    {
      auto offset = offsets.getAlgOffset (sMode) + limiter_alg;
      if (!opFlags[use_state])
        {
          ad.assignCheckCol (offset, argLoc, K);
          ad.assign (offset, offset, -1);
        }
      if (limiter_alg > 0)
        {
          --offset;
          ad.assign (offset, offset, -1);
          if (!opFlags[outside_lim])
            {
              ad.assign (offset, offset + 1, 1.0);
            }
        }
    }
}


void basicBlock::jacobianElements (const IOdata & args, const stateData &sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode)
{
  jacElements  (args[0], (args.size () > 1) ? args[1] : 0.0,sD, ad, argLocs[0], sMode);

}


void basicBlock::rootTest (const IOdata & /*args*/, const stateData &sD, double root[], const solverMode &sMode)
{
  if (!opFlags[has_limits])
    {
      return;
    }
  int rootOffset = offsets.getRootOffset (sMode);
  if (opFlags[use_ramp_limits])
    {
      auto doffset = offsets.getDiffOffset (sMode);
      if (opFlags[ramp_limit_triggered])
        {
          if (opFlags[ramp_limit_triggered_high])
            {
              root[rootOffset] = sD.state[doffset + 1] - sD.state[doffset] + resetLevel;
            }
          else
            {
              root[rootOffset] = sD.state[doffset] - sD.state[doffset + 1] + resetLevel;
            }
        }
      else
        {
          double val = sD.dstate_dt[doffset];
          root[rootOffset] = std::min (rampMax - val, val - rampMin);
        }
      ++rootOffset;
    }
  if (opFlags[use_block_limits])
    {
      auto offset = offsets.getAlgOffset (sMode);
      double val = sD.state[offset];
      if (opFlags[outside_lim])
        {
          if (opFlags[trigger_high])
            {
              root[rootOffset] = val - Omax + resetLevel;
            }
          else
            {
              root[rootOffset] = Omin - val + resetLevel;
            }
        }
      else
        {
          root[rootOffset] = std::min (Omax - val, val - Omin);
        }
    }

}

change_code basicBlock::rootCheck (const IOdata & /*args*/, const stateData &sD, const solverMode &sMode, check_level_t /*level*/)
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
      auto doffset = offsets.getDiffOffset (sMode);
      if (opFlags[ramp_limit_triggered])
        {
          if (opFlags[ramp_limit_triggered_high])
            {
              if (st[doffset + 1] - st[doffset] + resetLevel < 0.0)
                {
                  m_state[doffset] = m_state[doffset + 1];
                  m_dstate_dt[doffset] = m_dstate_dt[doffset + 1];
                  LOG_DEBUG ("return to within ramp limits");
                  alert (this, JAC_COUNT_INCREASE);
                  opFlags.reset (ramp_limit_triggered);
                  ret = change_code::jacobian_change;
                }

            }
          else
            {
              if (st[doffset] - st[doffset + 1] + resetLevel < 0.0)
                {
                  m_state[doffset] = m_state[doffset + 1];
                  m_dstate_dt[doffset] = m_dstate_dt[doffset + 1];
                  LOG_DEBUG ("return to within ramp limits");
                  alert (this, JAC_COUNT_INCREASE);
                  opFlags.reset (ramp_limit_triggered);
                  ret = change_code::jacobian_change;
                }
            }
        }
      else
        {
          double val = dst[doffset];
          if (val > rampMax)
            {
              opFlags.set (ramp_limit_triggered);
              opFlags.set (ramp_limit_triggered_high);
              m_dstate_dt[doffset] = rampMax;
              LOG_DEBUG ("upper ramp limit hit");
              alert (this, JAC_COUNT_DECREASE);
              ret = change_code::jacobian_change;
            }
          else if (val < rampMin)
            {
              opFlags.set (ramp_limit_triggered);
              m_dstate_dt[doffset] = rampMin;
              LOG_DEBUG ("lower ramp limit hit");
              alert (this, JAC_COUNT_DECREASE);
              ret = change_code::jacobian_change;
            }
        }
    }
  if (opFlags[use_block_limits])
    {
      double val = opFlags[differential_output] ? st[offsets.getDiffOffset (sMode)] : st[1];
      if (opFlags[outside_lim])
        {
          if ((val < Omax - resetLevel) && (val > Omin + resetLevel))
            {
              opFlags.reset (trigger_high);
              opFlags.reset (outside_lim);
              LOG_NORMAL ("return to within limits");
              alert (this, JAC_COUNT_INCREASE);
              ret = change_code::jacobian_change;
            }
        }
      else if ((val > Omax) || (val < Omin))
        {
          opFlags.set (outside_lim);
          LOG_NORMAL ("limit hit");
          alert (this, JAC_COUNT_DECREASE);
          if (val >= Omax)
            {
              opFlags.set (trigger_high);
            }
          ret = change_code::jacobian_change;
        }
    }
  return ret;

}

void basicBlock::rootTrigger (gridDyn_time /*ttime*/, const IOdata & /*args*/, const std::vector<int> &rootMask, const solverMode &sMode)
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
          auto doffset = offsets.getDiffOffset (cLocalSolverMode);
          if (opFlags[ramp_limit_triggered])
            {

              m_state[doffset] = m_state[doffset + 1];
              m_dstate_dt[doffset] = m_dstate_dt[doffset + 1];
              LOG_DEBUG ("return to within ramp limits");
              alert (this, JAC_COUNT_INCREASE);
            }
          else
            {
              double val = m_dstate_dt[doffset];
              opFlags.set (ramp_limit_triggered_high, (val >= rampMax - resetLevel));
              m_dstate_dt[doffset] = m_dstate_dt[doffset + 1];
              LOG_DEBUG ("ramp limit hit");
              alert (this, JAC_COUNT_DECREASE);
            }
          opFlags.flip (ramp_limit_triggered);
        }
      ++roffset;
    }
  if (opFlags[use_block_limits])
    {
      if (!rootMask[roffset])
        {
          return;
        }
      if (opFlags[outside_lim])
        {
          m_state[0] = (opFlags[differential_output]) ? m_state[offsets.getDiffOffset (cLocalSolverMode)] : m_state[1];
          LOG_DEBUG ("return to within limits");
          alert (this, JAC_COUNT_INCREASE);
        }
      else
        {
          opFlags.set (trigger_high, (m_state[0] >= Omax - resetLevel));
          m_state[0] = valLimit (m_state[0], Omin, Omax);
          LOG_DEBUG ("limit hit");
          alert (this, JAC_COUNT_DECREASE);
        }
      opFlags.flip (outside_lim);
    }
}

index_t basicBlock::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  if (field == "out")
    {
      ret = offsets.getAlgOffset (sMode);
    }
  else if (field == "block_limit")
    {
      if (opFlags[use_block_limits])
        {
          if (opFlags[differential_output])
            {
              ret = offsets.getDiffOffset (sMode);
            }
          else
            {
              ret = offsets.getAlgOffset (sMode);
              ret = (ret != kNullLocation) ? ret + 1 : ret;
            }
        }
    }
  return ret;
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
              objectInitializeA (prevTime, 0);
              alert (this, STATE_COUNT_CHANGE);
              LOG_WARNING ("changing object state computations during simulation triggers solver reset");
            }
        }
      else
        {
          opFlags[simplified] = val;
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
      if (!opFlags[dyn_initialized])
        {
          opFlags.set (use_block_limits);
        }
    }
  else if ((param == "omin") || (param == "min"))
    {
      Omin = val;
      if (!opFlags[dyn_initialized])
        {
          opFlags.set (use_block_limits);
        }
    }
  else if (param == "limit")
    {
      Omax = val;
      Omin = -val;
      if (!opFlags[dyn_initialized])
        {
          opFlags.set (use_block_limits);
        }
    }
  else if (param == "rampmax")
    {
      rampMax = val;
      if (!opFlags[dyn_initialized])
        {
          opFlags.set (use_ramp_limits);
        }
    }
  else if (param == "rampmin")
    {
      rampMin = val;
      if (!opFlags[dyn_initialized])
        {
          opFlags.set (use_ramp_limits);
        }
    }
  else if (param == "ramplimit")
    {
      rampMin = -val;
      rampMax = val;
      if (!opFlags[dyn_initialized])
        {
          opFlags.set (use_ramp_limits);
        }
    }

  else if (param == "resetlevel")
    {
      resetLevel = val;
    }
  else
    {
      gridSubModel::set (param, val, unitType);
    }

}


std::shared_ptr<basicBlock> make_block (const std::string &blockstr)
{
  auto posp1 = blockstr.find_first_of ('(');
  auto posp2 = blockstr.find_last_of (')');
  auto fstr = blockstr.substr (0, posp1 - 1);
  auto argstr = blockstr.substr (posp1 + 1, posp2 - posp1 - 1);

  auto args = str2vector (argstr,kNullVal);
  auto tail = blockstr.substr (posp2 + 2);
  auto tailArgs = splitlineTrim (tail);

  double gain = 1.0;
  posp1 = fstr.find_first_of ('*');
  std::shared_ptr<basicBlock> ret;
  if (posp1 == std::string::npos)
    {
      makeLowerCase (fstr);
    }
  else
    {
      gain = doubleRead (fstr);           //purposely not using doubleReadComplete to just get the first number
      fstr = convertToLowerCase (fstr.substr (posp1 + 1));

    }
  if (fstr == "basic")
    {
      ret = std::make_shared<basicBlock> (gain);
    }
  else if ((fstr == "der") || (fstr == "derivative"))
    {
      if (args.empty ())
        {
          ret = std::make_shared<derivativeBlock> ();
        }
      else
        {
          ret = std::make_shared<derivativeBlock> (args[0]);
        }
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if ((fstr == "integral")||(fstr == "integrator"))
    {
      ret = std::make_shared<integralBlock> (gain);
    }
  else if (fstr == "control")
    {
      if (args.empty ())
        {
          ret = std::make_shared<controlBlock> ();
        }
      else if (args.size () == 1)
        {
          ret = std::make_shared<controlBlock> (args[0]);
        }
      else
        {
          ret = std::make_shared<controlBlock> (args[0],args[1]);
        }
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if (fstr == "delay")
    {
      if (args.empty ())
        {
          ret = std::make_shared<delayBlock> ();
        }
      else
        {
          ret = std::make_shared<delayBlock> (args[0]);
        }
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if (fstr == "deadband")
    {
      if (args.empty ())
        {
          ret = std::make_shared<deadbandBlock> ();
        }
      else
        {
          ret = std::make_shared<deadbandBlock> (args[0]);
        }
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if (fstr == "pid")
    {
      double p = 1.0, i = 0.0, d = 0.0;
      if (args.size () > 0)
        {
          p = args[0];
        }
      if (tailArgs.size () > 1)
        {
          i = args[1];
        }
      if (args.size () > 2)
        {
          d = args[2];
        }

      ret = std::make_shared<pidBlock> (p,i,d);
      if (gain != 1.0)
        {
          ret->set ("gain", gain);
        }
    }
  else if (fstr == "function")
    {
      if (argstr.empty ())
        {
          ret = std::make_shared<functionBlock> ();
        }
      else
        {
          ret = std::make_shared<functionBlock> (argstr);
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
              double val = doubleReadComplete (ta.substr (eloc + 1), kNullVal);
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
	return{ outputName,"test" };
}
