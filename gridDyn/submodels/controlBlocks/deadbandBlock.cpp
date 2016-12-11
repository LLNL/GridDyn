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

#include "submodels/gridControlBlocks.h"
#include "vectorOps.hpp"
#include "matrixData.h"
#include "stringOps.h"

deadbandBlock::deadbandBlock (const std::string &objName) : basicBlock (objName)
{
  opFlags.set (use_state);
}

deadbandBlock::deadbandBlock (double db, const std::string &objName) : basicBlock (objName), deadbandHigh (db),deadbandLow (-db)
{
  opFlags.set (use_state);
}

coreObject *deadbandBlock::clone (coreObject *obj) const
{
  deadbandBlock *nobj;
  if (obj == nullptr)
    {
      nobj = new deadbandBlock ();
    }
  else
    {
      nobj = dynamic_cast<deadbandBlock *> (obj);
      if (nobj == nullptr)
        {
          basicBlock::clone (obj);
          return obj;
        }
    }
  basicBlock::clone (nobj);
  nobj->deadbandHigh = deadbandHigh;
  nobj->deadbandLow = deadbandLow;
  nobj->deadbandLevel = deadbandLevel;
  nobj->rampUpband = rampUpband;
  nobj->rampDownband = rampDownband;
  nobj->resetHigh = resetHigh;
  nobj->resetLow = resetLow;
  return nobj;
}

void deadbandBlock::objectInitializeA (gridDyn_time time0, unsigned long flags)
{
  basicBlock::objectInitializeA (time0,flags);
  if (deadbandLow < deadbandHigh)     //this means it was set to some value
    {
      opFlags[has_roots] = true;
      offsets.local->local.algRoots++;
      opFlags.set (has_alg_roots);
      opFlags[uses_deadband] = true;
    }
  if (resetHigh < -kHalfBigNum)  //this means we are using default
    {
      resetHigh = deadbandHigh - 1e-6;
    }
  if (resetLow > kHalfBigNum)  //this means we are using default
    {
      resetLow = deadbandLow + 1e-6;
    }

}
// initial conditions
void deadbandBlock::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{

  if (outputSet.empty ())
    {
      m_state[limiter_alg] = deadbandLevel;
      rootCheck (args,emptyStateData,cLocalSolverMode, check_level_t::reversable_only);
      m_state[limiter_alg] = K * computeValue (args[0] + bias);
      if (limiter_alg > 0)
        {
          basicBlock::rootCheck (args,emptyStateData,cLocalSolverMode, check_level_t::reversable_only);
        }
    }

  else
    {
      if (limiter_alg > 0)
        {
          basicBlock::rootCheck (args, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
        }
      dbstate = deadbandstate_t::normal;
      double ival = m_state[limiter_alg] / K;
      if (std::abs (ival - deadbandLevel) > 1e-6)
        {
          if (opFlags[uses_shiftedoutput])
            {
              dbstate = deadbandstate_t::shifted;
              if (ival > deadbandLevel)
                {
                  fieldSet[0] = (deadbandHigh - deadbandLevel) + ival;
                }
              else
                {
                  fieldSet[0] = ival - (deadbandLevel - deadbandLow);
                }
            }
          else if ((ival > deadbandHigh + rampUpband) || (ival < deadbandLow - rampUpband))
            {
              dbstate = deadbandstate_t::outside;
              fieldSet[0] = ival;
            }
          else if (rampUpband > 0)
            {
              dbstate = deadbandstate_t::rampup;
              if (ival > deadbandLevel)
                {
                  fieldSet[0] = deadbandHigh + (ival - deadbandLevel) / (deadbandHigh + rampUpband - deadbandLevel) * rampUpband;
                }
              else
                {
                  fieldSet[0] = deadbandLow - (deadbandLevel - ival) / (deadbandLevel - deadbandLow - rampUpband) * rampUpband;
                }
            }
          else
            {
              dbstate = deadbandstate_t::outside;
              fieldSet[0] = ival;
            }
        }
      else
        {
          fieldSet[0] = deadbandLevel;
        }
      fieldSet[0] -= bias;
    }

}

double deadbandBlock::computeValue (double input)
{
  double out = input;
  switch (dbstate)
    {
    case deadbandstate_t::normal:
      out = deadbandLevel;
      break;
    case deadbandstate_t::outside:
      out = input;
      break;
    case deadbandstate_t::shifted:
      if (input > deadbandLevel)
        {
          out = input - (deadbandHigh - deadbandLevel);
        }
      else
        {
          out = input + (deadbandLevel - deadbandLow);
        }
      break;
    case deadbandstate_t::rampup:
      if (input > deadbandLevel)
        {
          double tband = deadbandHigh - deadbandLevel + rampUpband;
          out = deadbandLevel + (input - deadbandHigh) / rampUpband * tband;
        }
      else
        {
          double tband = deadbandLevel - deadbandLow + rampUpband;
          out = deadbandLevel - (deadbandLow - input) / rampUpband * tband;
        }
      break;
    case deadbandstate_t::rampdown:
      if (input > deadbandLevel)
        {
          double tband = deadbandHigh - deadbandLevel + rampDownband;
          out = deadbandLevel + (input - deadbandHigh) / rampDownband * tband;
        }
      else
        {
          double tband = deadbandLevel - deadbandLow + rampDownband;
          out = deadbandLevel - (deadbandLow - input) / rampDownband * tband;
        }
      break;
    }
  return out;
}

double deadbandBlock::computeDoutDin (double input)
{
  double out = 0.0;
  switch (dbstate)
    {
    case deadbandstate_t::normal:
      break;
    case deadbandstate_t::outside:
    case deadbandstate_t::shifted:
      out = 1.0;
      break;
    case deadbandstate_t::rampup:
      if (input > deadbandLevel)
        {
          double tband = deadbandHigh - deadbandLevel + rampUpband;
          out = tband / rampUpband;
        }
      else
        {
          double tband = deadbandLevel - deadbandLow + rampUpband;
          out = tband / rampUpband;
        }
      break;
    case deadbandstate_t::rampdown:
      if (input > deadbandLevel)
        {
          double tband = deadbandHigh - deadbandLevel + rampDownband;
          out = tband / rampDownband;
        }
      else
        {
          double tband = deadbandLevel - deadbandLow + rampDownband;
          out = tband / rampDownband;
        }
      break;
    }
  return out;
}
double deadbandBlock::step (gridDyn_time ttime, double input)
{
  rootCheck ({ input }, emptyStateData, cLocalSolverMode, check_level_t::reversable_only);
  m_state[limiter_alg] = K * computeValue (input + bias);
  if (limiter_alg > 0)
    {
      basicBlock::step (ttime,input);
    }
  else
    {
      prevTime = ttime;
      m_output = m_state[0];
    }

  return m_state[0];
}

void deadbandBlock::derivElements (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode)
{
  if (opFlags[differential_input])
    {
      auto offset = offsets.getDiffOffset (sMode) + limiter_diff;
      double ival = input + bias;
      deriv[offset] = K * computeDoutDin (ival) * didt;

      if (limiter_diff > 0)
        {
          return basicBlock::derivElements (input, didt,sD, deriv, sMode);
        }
    }
}

void deadbandBlock::algElements (double input, const stateData &sD, double update[], const solverMode &sMode)
{
  if (!opFlags[differential_input])
    {
      auto offset = offsets.getAlgOffset (sMode) + limiter_alg;
      double ival = input + bias;
      update[offset] = K * computeValue (ival);
      //printf("db %f intput=%f val=%f dbstate=%d\n", sD.time, ival, update[offset], static_cast<int>(dbstate));
      if (limiter_alg > 0)
        {
          return basicBlock::algElements (input, sD, update, sMode);
        }
    }
}


void deadbandBlock::jacElements (double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{
  if ((!opFlags[differential_input])&& (hasAlgebraic (sMode)))
    {
      auto offset = offsets.getAlgOffset (sMode) + limiter_alg;
      ad.assign (offset, offset, -1.0);
      double dido = K * computeDoutDin (input + bias);

      if (argLoc != kNullLocation)
        {
          ad.assign (offset, argLoc, dido);
        }
      if (limiter_alg > 0)
        {
          basicBlock::jacElements (input, didt, sD, ad, argLoc, sMode);
        }
    }
  else if ((opFlags[differential_input]) && (hasDifferential (sMode)))
    {
      auto offset = offsets.getDiffOffset (sMode) + limiter_diff;
      ad.assign (offset, offset, -sD.cj);
      double dido = K * computeDoutDin (input + bias);

      if (argLoc != kNullLocation)
        {
          ad.assign (offset, argLoc, dido * sD.cj);
        }

      if (limiter_diff > 0)
        {
          basicBlock::jacElements (input, didt, sD, ad, argLoc, sMode);
        }
    }
}


void deadbandBlock::rootTest (const IOdata &args, const stateData &sD, double root[], const solverMode &sMode)
{

  if (limiter_alg + limiter_diff > 0)
    {
      basicBlock::rootTest (args,sD,root,sMode);
    }
  if (opFlags[uses_deadband])
    {
      int rootOffset = offsets.getRootOffset (sMode) + limiter_alg + limiter_diff;

      double ival = args[0] + bias;
      // double prevInput = ival;
      switch (dbstate)
        {
        case deadbandstate_t::normal:
          root[rootOffset] = std::min (deadbandHigh - ival, ival - deadbandLow);
          if (ival > deadbandHigh)
            {
              opFlags.set (dbtrigger_high);
            }
          break;
        case deadbandstate_t::outside:
          if (opFlags[dbtrigger_high])
            {
              root[rootOffset] = ival - resetHigh;
            }
          else
            {
              root[rootOffset] = resetLow - ival;
            }
          break;
        case deadbandstate_t::shifted:
          if (opFlags[dbtrigger_high])
            {
              root[rootOffset] = ival - deadbandHigh;
            }
          else
            {
              root[rootOffset] = deadbandLow - ival;
            }
          break;
          break;

        case deadbandstate_t::rampup:
          if (opFlags[dbtrigger_high])
            {
              root[rootOffset] = std::min (deadbandHigh + rampUpband - ival, ival - deadbandHigh);
            }
          else
            {
              root[rootOffset] = std::min (deadbandLow - ival, ival - deadbandLow - rampUpband);
            }
          break;
        case deadbandstate_t::rampdown:
          if (opFlags[dbtrigger_high])
            {
              root[rootOffset] = std::min (ival - resetHigh - rampDownband, resetHigh - ival);
            }
          else
            {
              root[rootOffset] = std::min (ival - resetLow, resetLow + rampDownband - ival);
            }
          break;
        }

    }

}


void deadbandBlock::rootTrigger (gridDyn_time ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
  auto rootOffset = offsets.getRootOffset (sMode);
  if (limiter_alg + limiter_diff > 0)
    {
      if (rootMask[rootOffset])
        {
          basicBlock::rootTrigger (ttime, args, rootMask, sMode);
        }
      else if (rootMask[rootOffset + limiter_alg + limiter_diff - 1])
        {
          basicBlock::rootTrigger (ttime, args, rootMask, sMode);
        }
      rootOffset += limiter_alg + limiter_diff;
    }

  if (opFlags[uses_deadband])
    {

      if (!(rootMask[rootOffset]))
        {
          return;
        }

      switch (dbstate)
        {
        case deadbandstate_t::normal:
          if (opFlags[uses_shiftedoutput])
            {
              dbstate = deadbandstate_t::shifted;
            }
          else if (rampUpband > 0)
            {
              dbstate = deadbandstate_t::rampup;
            }
          else
            {
              dbstate = deadbandstate_t::outside;
            }
          break;
        case deadbandstate_t::outside:
          if (rampDownband > 0)
            {
              dbstate = deadbandstate_t::rampdown;
            }
          else
            {
              dbstate = deadbandstate_t::normal;
            }
          break;
        case deadbandstate_t::shifted:
          dbstate = deadbandstate_t::normal;
          break;

        case deadbandstate_t::rampup:
          if ((prevInput >= deadbandHigh + rampUpband) || (prevInput <= deadbandLow - rampUpband))
            {
              dbstate = deadbandstate_t::outside;
            }
          else
            {
              dbstate = deadbandstate_t::normal;
            }
          break;
        case deadbandstate_t::rampdown:
          if ((prevInput >= resetHigh) || (prevInput <= resetLow))
            {
              dbstate = deadbandstate_t::outside;
            }
          else
            {
              dbstate = deadbandstate_t::normal;
            }
          break;
        }
      if (dbstate == deadbandstate_t::normal)
        {
          opFlags.reset (dbtrigger_high);
        }
    }
}

change_code deadbandBlock::rootCheck (const IOdata &args, const stateData &sD, const solverMode &sMode, check_level_t /*level*/)
{
  change_code ret = change_code::no_change;
  auto cstate = dbstate;
  if (opFlags[uses_deadband])
    {
      double ival = args[0] + bias;
      switch (dbstate)
        {
        case deadbandstate_t::normal:
          if (std::min (deadbandHigh - ival, ival - deadbandLow) < 0)
            {
              ret = change_code::parameter_change;
              if (opFlags[uses_shiftedoutput])
                {
                  dbstate = deadbandstate_t::shifted;
                }
              else if (rampUpband > 0)
                {
                  dbstate = deadbandstate_t::rampup;
                }
              else
                {
                  dbstate = deadbandstate_t::outside;
                }
              if (ival > deadbandHigh)
                {
                  opFlags.set (dbtrigger_high);
                }
              else
                {
                  opFlags.reset (dbtrigger_high);
                }
            }
          break;
        case deadbandstate_t::outside:
          if (opFlags[dbtrigger_high])
            {
              if (ival < resetHigh)
                {
                  if (rampDownband > 0)
                    {
                      if (ival < resetHigh - rampDownband)
                        {
                          dbstate = deadbandstate_t::normal;
                          ret = change_code::parameter_change;
                        }
                      else
                        {
                          dbstate = deadbandstate_t::rampdown;
                        }
                    }
                  else
                    {
                      dbstate = deadbandstate_t::normal;
                      ret = change_code::parameter_change;
                    }
                }
            }
          else
            {
              if (ival > resetLow)
                {
                  if (rampDownband > 0)
                    {
                      if (ival > resetLow + rampDownband)
                        {
                          dbstate = deadbandstate_t::normal;
                          ret = change_code::parameter_change;
                        }
                      else
                        {
                          dbstate = deadbandstate_t::rampdown;
                        }
                    }
                  else
                    {
                      dbstate = deadbandstate_t::normal;
                      ret = change_code::parameter_change;
                    }
                }
            }
          break;
        case deadbandstate_t::shifted:
          if ((ival < deadbandHigh)&&(ival > deadbandLow))
            {
              dbstate = deadbandstate_t::normal;
              ret = change_code::parameter_change;
            }
          break;

        case deadbandstate_t::rampup:
          if (opFlags[dbtrigger_high])
            {
              if (ival > deadbandHigh + rampUpband)
                {
                  dbstate = deadbandstate_t::outside;

                }
              else if (ival < deadbandHigh)
                {
                  dbstate = deadbandstate_t::normal;
                  ret = change_code::parameter_change;
                }
            }
          else
            {
              if (ival < deadbandLow - rampUpband)
                {
                  dbstate = deadbandstate_t::outside;
                }
              else if (ival > deadbandLow)
                {
                  dbstate = deadbandstate_t::normal;
                  ret = change_code::parameter_change;
                }
            }
          break;
        case deadbandstate_t::rampdown:
          if (opFlags[dbtrigger_high])
            {
              if (ival > deadbandHigh + rampDownband)
                {
                  dbstate = deadbandstate_t::outside;

                }
              else if (ival < deadbandHigh)
                {
                  dbstate = deadbandstate_t::normal;
                  ret = change_code::parameter_change;
                }
            }
          else
            {
              if (ival < deadbandLow - rampDownband)
                {
                  dbstate = deadbandstate_t::outside;
                }
              else if (ival > deadbandLow)
                {
                  dbstate = deadbandstate_t::normal;
                  ret = change_code::parameter_change;
                }
            }
          break;
        }

    }
  if (limiter_alg > 0)
    {
      auto iret = basicBlock::rootCheck (args, sD, sMode, check_level_t::reversable_only);
      ret = std::max (ret,iret);
    }
  if (cstate != dbstate)  //we may run through multiple categories so we need to do this recursively
    {
      auto iret = rootCheck (args,sD,sMode, check_level_t::reversable_only);
      ret = std::max (ret, iret);
    }
  return ret;
}

void deadbandBlock::setFlag (const std::string &flag, bool val)
{
  if (flag == "shifted")
    {
      opFlags.set (uses_shiftedoutput,val);
    }
  else if (flag == "unshifted")
    {
      opFlags.set (uses_shiftedoutput,!val);
    }
  else if (flag == "no_down_deadband")
    {
      resetLow = deadbandLevel;
      resetHigh = deadbandLevel;
    }
  else
    {
	  basicBlock::setFlag(flag, val);
    }

}
// set parameters
void deadbandBlock::set (const std::string &param,  const std::string &val)
{

  if (param[0] == '#')
    {

    }
  else
    {
      basicBlock::set (param, val);
    }

}

void deadbandBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if ((param == "level") || (param == "dblevel")||(param == "deadbandlevel"))
    {
      deadbandLevel = val;
    }
  else if ((param == "deadband")||(param == "db"))
    {
      deadbandHigh = deadbandLevel + val;
      deadbandLow = deadbandLevel - val;
    }
  else if ((param == "deadbandhigh") || (param == "dbhigh")||(param == "high"))
    {
      if (val > deadbandLevel)
        {
          deadbandHigh = val;
        }
      else
        {
          deadbandHigh = deadbandLevel + val;
        }
    }
  else if ((param == "deadbandlow") || (param == "dblow")||(param == "low"))
    {
      if (val < deadbandLevel)
        {
          deadbandLow = val;
        }
      else
        {
          deadbandLow = deadbandLevel - val;
        }
    }
  else if ((param == "rampband")||(param == "ramp"))
    {
      rampUpband = val;
      rampDownband = val;

    }
  else if ((param == "rampupband")||(param == "rampup"))
    {
      rampUpband = val;

    }
  else if ((param == "rampdownband") || (param == "rampdown"))
    {
      rampDownband = val;
    }
  else if ((param == "resetlevel")||(param == "reset"))
    {
      resetHigh = deadbandLevel + val;
      resetLow = deadbandLevel - val;
    }
  else if (param == "resethigh")
    {
      resetHigh = val;
    }
  else if (param == "resetlow")
    {
      resetLow = val;
    }
  else
    {
      basicBlock::set (param, val, unitType);
    }


}