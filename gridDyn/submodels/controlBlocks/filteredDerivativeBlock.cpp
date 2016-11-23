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
#include "core/gridDynExceptions.h"
#include "objectFactory.h"
#include "matrixData.h"

#include <cmath>
#include <algorithm>


filteredDerivativeBlock::filteredDerivativeBlock (const std::string &objName) : basicBlock (objName)
{
  opFlags.set (use_state);
  opFlags.set (differential_output);
}

filteredDerivativeBlock::filteredDerivativeBlock (double t1, double t2,const std::string &objName) : basicBlock (objName), m_T1 (t1), m_T2 (t2)
{
  opFlags.set (use_state);
  opFlags.set (differential_output);
}

gridCoreObject *filteredDerivativeBlock::clone (gridCoreObject *obj) const
{
  filteredDerivativeBlock *nobj;
  if (obj == nullptr)
    {
      nobj = new filteredDerivativeBlock ();
    }
  else
    {
      nobj = dynamic_cast<filteredDerivativeBlock *> (obj);
      if (nobj == nullptr)
        {
          basicBlock::clone (obj);
          return obj;
        }
    }
  basicBlock::clone (nobj);
  nobj->m_T1 = m_T1;
  nobj->m_T2 = m_T2;
  return nobj;
}

void filteredDerivativeBlock::objectInitializeA (gridDyn_time time0, unsigned long flags)
{
  basicBlock::objectInitializeA (time0, flags);
  offsets.local->local.diffSize++;
  offsets.local->local.jacSize += 2;

}

void filteredDerivativeBlock::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{
  index_t loc = limiter_diff;              //can't have a ramp limiter
  if (outputSet.empty ())
    {
      m_state[loc + 1] = K * (args[0] + bias);
      m_state[loc] = 0;
      if (limiter_diff > 0)
        {
          basicBlock::objectInitializeB (args, outputSet, fieldSet);
        }
    }
  else
    {
      basicBlock::objectInitializeB (args, outputSet, fieldSet);
      m_state[loc] = outputSet[0];
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

double filteredDerivativeBlock::step (gridDyn_time ttime, double inputA)
{

  index_t loc = limiter_diff;
  double dt = ttime - prevTime;

  double input = inputA + bias;
  if ((dt >= fabs (5.0 * m_T1))&& (dt >= fabs (5.0 * m_T2)))
    {
      m_state[loc + 1] = K * input;
      m_state[loc] = 0;
    }
  else
    {
      double tstep = 0.05 * std::min (m_T1, m_T2);
      double ct = prevTime + tstep;
      double in = prevInput;
      double pin = prevInput;
      double ival = m_state[loc + 1];
      double ival2 = m_state[loc];
      double der1;
      while (ct < ttime)
        {
          in = in + (input - prevInput) / dt * tstep;
          der1 = K / m_T1 * ((pin + in) / 2.0 - ival);
          ival = ival + der1 * tstep;
          ival2 = ival2 + (der1 - ival2) / m_T2;
          ct += tstep;
          pin = in;
        }
      m_state[loc + 1] = ival + K / m_T1 * ((pin + input) / 2.0 - ival) * (ttime - ct + tstep);
      m_state[loc] = ival2 + (K / m_T1 * ((pin + input) / 2.0 - ival) - ival2) / m_T2;
    }
  prevInput = input;
  double out;
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


void filteredDerivativeBlock::derivElements (double input, double /*didt*/, const stateData *sD, double deriv[], const solverMode &sMode)
{
  auto offset = offsets.getDiffOffset (sMode) + limiter_diff;

  deriv[offset + 1] = (K * (input + bias) - sD->state[offset + 1]) / m_T1;
  deriv[offset] = (sD->dstate_dt[offset + 1] - sD->state[offset]) / m_T2;
}

void filteredDerivativeBlock::jacElements (double input, double didt, const stateData *sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{
  if (!hasDifferential (sMode))
    {
      return;
    }
  auto offset = offsets.getDiffOffset (sMode) + limiter_diff;

  ad.assignCheckCol (offset + 1, argLoc, K / m_T1);
  ad.assign (offset + 1, offset + 1, -1 / m_T1 - sD->cj);

  ad.assign (offset, offset + 1, sD->cj / m_T2);
  ad.assign (offset, offset, -1 / m_T2 - sD->cj);

  if (limiter_diff > 0)
    {
      basicBlock::jacElements (input, didt, sD, ad, argLoc, sMode);
    }

}


// set parameters
void filteredDerivativeBlock::set (const std::string &param, const std::string &val)
{
  basicBlock::set (param, val);
}

void filteredDerivativeBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "t1")
    {
      m_T1 = val;
    }
  else if (param == "t2")
    {
      if (std::abs (val) < kMin_Res)
        {
		  throw(invalidParameterValue());
        }
      else
        {
          m_T2 = val;
        }
    }
  else
    {
      basicBlock::set (param, val, unitType);
    }

}

stringVec filteredDerivativeBlock::localStateNames () const
{
  switch (limiter_diff)
    {
    case 0:
      return {
               "deriv","filter"
      };
    case 1:
      if (opFlags[use_block_limits])
        {
          return {
                   "limited", "deriv","filter"
          };
        }
      else
        {
          return {
                   "ramp_limited", "deriv","filter"
          };
        }
    default:      //should be 0, 1 or 2 so this should run with 2
      return {
               "limited","ramp_limited", "deriv","filter"
      };
    }

}
