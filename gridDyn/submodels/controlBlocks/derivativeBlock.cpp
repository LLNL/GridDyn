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
#include "objectFactory.h"
#include "core/gridDynExceptions.h"
#include "matrixData.h"

#include <cmath>

static const stringVec stNames {
  "output", "deriv","delayI"
};

derivativeBlock::derivativeBlock (const std::string &objName) : basicBlock (objName)
{
  opFlags.set (use_state);
}

derivativeBlock::derivativeBlock (double t1, const std::string &objName) : basicBlock (objName), m_T1 (t1)
{
  opFlags.set (use_state);
}

gridCoreObject *derivativeBlock::clone (gridCoreObject *obj) const
{
  derivativeBlock *nobj;
  if (obj == nullptr)
    {
      nobj = new derivativeBlock ();
    }
  else
    {
      nobj = dynamic_cast<derivativeBlock *> (obj);
      if (nobj == nullptr)
        {
          basicBlock::clone (obj);
          return obj;
        }
    }
  basicBlock::clone (nobj);
  nobj->m_T1 = m_T1;

  return nobj;
}

void derivativeBlock::objectInitializeA (double time0, unsigned long flags)
{
  basicBlock::objectInitializeA (time0, flags);
  offsets.local->local.diffSize++;
  offsets.local->local.jacSize += 2;

}

void derivativeBlock::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{
  index_t loc = limiter_alg;        //can't have a ramp limiter
  if (outputSet.empty ())
    {
      m_state[loc + 1] = K * (args[0] + bias);
      basicBlock::objectInitializeB (args, outputSet, fieldSet);
      m_state[loc] = 0;
    }
  else
    {
      basicBlock::objectInitializeB (args, outputSet, fieldSet);
      m_dstate_dt[loc + 1] = outputSet[0];
      if (std::abs (m_dstate_dt[loc + 1]) < 1e-7)
        {
          m_state[loc + 1] = K * (args[0] + bias);
        }
      else
        {
          m_state[loc + 1] = (m_state[loc] - m_dstate_dt[loc + 1] * m_T1);
        }
    }

}

double derivativeBlock::step (double ttime, double inputA)
{

  index_t loc = limiter_alg;
  double dt = ttime - prevTime;
  double out;
  double input = inputA + bias;
  double ival;
  if (dt >= fabs (5.0 * m_T1))
    {
      m_state[loc + 1] = K * input;
      m_state[loc] = 0;
    }
  else if (dt <= fabs (0.05 * m_T1))
    {
      m_state[loc + 1] = m_state[loc + 1] + 1.0 / m_T1 * (K * (input + prevInput) / 2.0 - m_state[loc + 1]) * dt;
      m_state[loc] = 1.0 / m_T1 * (K * (input + prevInput) / 2.0 - m_state[loc + 1]);
    }
  else
    {
      double tstep = 0.05 * m_T1;
      double ct = prevTime + tstep;
      double in = prevInput;
      double pin = prevInput;
      ival = m_state[loc + 1];
      while (ct < ttime)
        {
          in = in + (input - prevInput) / dt * tstep;
          ival = ival + K / m_T1 * ((pin + in) / 2.0 - ival) * tstep;
          ct += tstep;
          pin = in;
        }
      m_state[loc + 1] = ival + K / m_T1 * ((pin + input) / 2.0 - ival) * (ttime - ct + tstep);
      m_state[loc] = K / m_T1 * ((pin + input) / 2.0 - ival);
    }
  prevInput = input;
  if (loc > 0)
    {
      out = basicBlock::step (ttime, inputA);
    }
  else
    {
      out = m_state[0];
      prevTime = ttime;
      m_output = out;
    }
  return out;
}

void derivativeBlock::algElements (double input, const stateData *sD, double update[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD, update, sMode, this);
  Loc.destLoc[limiter_alg] = Loc.dstateLoc[0];
//	update[Aoffset + limiter_alg] = sD->state[Aoffset + limiter_alg] - sD->dstate_dt[offset];
  basicBlock::algElements (input,  sD, update, sMode);
}


void derivativeBlock::derivElements (double input, double /*didt*/, const stateData *sD, double deriv[], const solverMode &sMode)
{
  auto offset = offsets.getDiffOffset (sMode); //limiter diff must be 0 since the output is algebraic

  deriv[offset] = (K * (input + bias) - sD->state[offset]) / m_T1;
}

void derivativeBlock::jacElements (double input, double didt, const stateData *sD, matrixData<double> *ad, index_t argLoc, const solverMode &sMode)
{
  auto offset = offsets.getDiffOffset (sMode);
  if (hasDifferential (sMode))
    {
      ad->assignCheck (offset, argLoc, K / m_T1);
      ad->assign (offset, offset, -1.0 / m_T1 - sD->cj);
    }
  else
    {
      offset = kNullLocation;
    }
  if (hasAlgebraic (sMode))
    {
      auto Aoffset = offsets.getAlgOffset (sMode) + limiter_alg;
      ad->assignCheck (Aoffset, offset, sD->cj);
      ad->assign (Aoffset, Aoffset, -1);
      if (limiter_alg > 0)
        {
          basicBlock::jacElements (input, didt, sD, ad, argLoc, sMode);
        }
    }


}


// set parameters
void derivativeBlock::set (const std::string &param,  const std::string &val)
{
  basicBlock::set (param, val);
}

void derivativeBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  //param   = gridDynSimulation::toLower(param);

  if ((param == "t1") || (param == "t"))
    {
      if (std::abs (val) < kMin_Res)
        {
		  throw(invalidParameterValue());
        }
      else
        {
          m_T1 = val;
        }
    }
  else
    {
      basicBlock::set (param, val, unitType);
    }

}

stringVec derivativeBlock::localStateNames () const
{
  return stNames;
}
