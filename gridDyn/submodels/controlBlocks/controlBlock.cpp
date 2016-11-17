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


controlBlock::controlBlock (const std::string &objName) : basicBlock (objName)
{

  opFlags.set (use_state);
}
controlBlock::controlBlock (double t1, const std::string &objName) : basicBlock (objName), m_T1 (t1)
{

  opFlags.set (use_state);
}
controlBlock::controlBlock (double t1,double t2, const std::string &objName) : basicBlock (objName), m_T1 (t1),m_T2 (t2)
{

  opFlags.set (use_state);
}

gridCoreObject *controlBlock::clone (gridCoreObject *obj) const
{
  controlBlock *nobj;
  if (obj == nullptr)
    {
      nobj = new controlBlock ();
    }
  else
    {
      nobj = dynamic_cast<controlBlock *> (obj);
      if (nobj == nullptr)
        {
          basicBlock::clone (obj);
          return obj;
        }
    }
  basicBlock::clone (nobj);

  nobj->m_T2 = m_T2;
  nobj->m_T1 = m_T2;
  return nobj;
}
//set up the number of states
void controlBlock::objectInitializeA (double time0, unsigned long flags)
{
  if (opFlags[differential_input])
    {
      opFlags.set (differential_output);
    }
  basicBlock::objectInitializeA (time0, flags);

  offsets.local->local.diffSize += 1;
  offsets.local->local.jacSize += 6;
}
// initial conditions
void controlBlock::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{
  if (opFlags[has_limits])
    {
      basicBlock::objectInitializeB (args, outputSet, fieldSet);
    }
  if (outputSet.empty ())
    {
      m_state[limiter_alg + 1] = K * (1.0 - m_T2 / m_T1) * (args[0] + bias);
      m_state[limiter_alg] = K * (args[0] + bias);

      fieldSet[0] = m_state[0];
      prevInput = args[0] + bias;
    }
  else
    {
      m_state[limiter_alg] = outputSet[0];
      m_state[limiter_alg + 1] = (1.0 - m_T2 / m_T1) * outputSet[0] / K;
      fieldSet[0] = outputSet[0] / K - bias;
      prevInput = outputSet[0] / K;
    }

}


void controlBlock::algElements (double input, const stateData *sD, double update[], const solverMode &sMode)
{
  if (!opFlags[differential_input])
    {
      Lp Loc = offsets.getLocations (sD, update, sMode, this);

      Loc.destLoc[limiter_alg] = Loc.diffStateLoc[0] + m_T2 / m_T1 * (input + bias) * K;
      if (limiter_alg > 0)
        {
          basicBlock::algElements (input, sD, update, sMode);
        }
    }

}

void controlBlock::derivElements (double input, double didt, const stateData *sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD, deriv, sMode, this);
  if (opFlags[differential_input])
    {
      Loc.destDiffLoc[limiter_diff] = Loc.dstateLoc[limiter_diff + 1] + m_T2 / m_T1 * didt * K;
      Loc.destDiffLoc[limiter_diff + 1] = (K * (input + bias) - Loc.diffStateLoc[limiter_diff]) / m_T1;
      if (limiter_diff > 0)
        {
          basicBlock::derivElements (input, didt, sD, deriv, sMode);
        }
    }
  else
    {
      Loc.destDiffLoc[0] = (K * (input + bias) - Loc.algStateLoc[limiter_alg]) / m_T1;
    }

}


void controlBlock::jacElements (double input, double didt, const stateData *sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{

  Lp Loc = offsets.getLocations  (sD, sMode, this);
  if (opFlags[differential_input])
    {

    }
  else
    {
      if (hasAlgebraic (sMode))
        {
          ad.assign (Loc.algOffset + limiter_alg, Loc.algOffset + limiter_alg, -1);

          ad.assignCheckCol (Loc.algOffset + limiter_alg, argLoc, K * m_T2 / m_T1);
          if (limiter_alg > 0)
            {
              basicBlock::jacElements (input, didt, sD, ad, argLoc, sMode);
            }
          if (hasDifferential (sMode))
            {
              ad.assign (Loc.algOffset + limiter_alg, Loc.diffOffset, 1);
            }
        }

      if (hasDifferential (sMode))
        {
          ad.assignCheckCol (Loc.diffOffset, argLoc, K / m_T1);
          if (hasAlgebraic (sMode))
            {
              ad.assign (Loc.diffOffset, Loc.algOffset + limiter_alg, -1 / m_T1);
            }
          ad.assign (Loc.diffOffset, Loc.diffOffset, -sD->cj);
        }
    }



}

double controlBlock::step (double ttime, double inputA)
{

  double dt = ttime - prevTime;
  double out;
  double input = inputA + bias;
  double ival,ival2;
  if (dt >= fabs (5.0 * m_T1))
    {
      m_state[limiter_alg + limiter_diff + 1] = K * (1.0 - m_T2 / m_T1) * (input);
    }
  else
    {
      double tstep = 0.05 * m_T1;
      double ct = prevTime + tstep;
      double in = prevInput;
      double pin = prevInput;
      ival = m_state[limiter_alg + limiter_diff + 1];
      ival2 = m_state[limiter_alg + limiter_diff];
      while (ct < ttime)
        {
          in = in + (input - prevInput) / dt * tstep;
          ival = ival + 1.0 / m_T1 * (K * (pin + in) / 2.0 - ival2) * tstep;
          ival2 = ival + K * m_T2 / m_T1 * (input);
          ct += tstep;
          pin = in;
        }
      m_state[limiter_alg + limiter_diff + 1] = ival + 1.0 / m_T1 * (K * (pin + input) / 2.0 - ival2) * (ttime - ct + tstep);
    }
  m_state[limiter_alg + limiter_diff] = m_state[limiter_alg + limiter_diff + 1] + K * m_T2 / m_T1 * (input);

  prevInput = input;
  if (opFlags[has_limits])
    {
      out = basicBlock::step (ttime, input);
    }
  else
    {
      out = m_state[0];
      prevTime = ttime;
      m_output = out;
    }
  return out;
}

index_t controlBlock::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  if (field == "m1")
    {
      ret = offsets.getDiffOffset (sMode);
    }
  else
    {
      ret = basicBlock::findIndex (field,sMode);
    }
  return ret;
}

// set parameters
void controlBlock::set (const std::string &param,  const std::string &val)
{
  basicBlock::set (param, val);
}

void controlBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  //param   = gridDynSimulation::toLower(param);

  if ((param == "t1") || (param == "t"))
    {
      m_T1 = val;
    }
  else if (param == "t2")
    {
      m_T2 = val;
    }
  else
    {
      basicBlock::set (param, val, unitType);
    }

}

stringVec controlBlock::localStateNames () const
{
  stringVec out (stateSize (cLocalSolverMode));
  int loc = 0;
  if (opFlags[use_block_limits])
    {
      out[loc++] = "limiter_out";
    }
  if (opFlags[use_ramp_limits])
    {
      out[loc++] = "ramp_limiter_out";
    }
  out[loc++] = "output";
  out[loc] = "intermediate";
  return out;
}

