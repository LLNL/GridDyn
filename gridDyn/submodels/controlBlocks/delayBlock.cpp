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
#include "core/gridDynExceptions.h"
#include "matrixData.h"

#include <cmath>

delayBlock::delayBlock (const std::string &objName) : basicBlock (objName)
{
  opFlags.set (differential_output);
  opFlags.set (use_state);
}

delayBlock::delayBlock (double t1, const std::string &objName) : basicBlock (objName), m_T1 (t1)
{
  if (std::abs (m_T1) < kMin_Res)
    {
      opFlags.set (simplified);
    }
  else
    {
      opFlags.set (differential_output);
      opFlags.set (use_state);
    }


}
delayBlock::delayBlock (double t1, double gain, const std::string &objName) : basicBlock (gain,objName),m_T1 (t1)
{
  if (std::abs (m_T1) < kMin_Res)
    {
      opFlags.set (simplified);
    }
  else
    {
      opFlags.set (differential_output);
      opFlags.set (use_state);
    }


}

gridCoreObject *delayBlock::clone (gridCoreObject *obj) const
{
  delayBlock *nobj;
  if (obj == nullptr)
    {
      nobj = new delayBlock ();
    }
  else
    {
      nobj = dynamic_cast<delayBlock *> (obj);
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

void delayBlock::objectInitializeA (double time0, unsigned long flags)
{
  if ((m_T1 < kMin_Res)||(opFlags[simplified]))
    {
      opFlags.set (simplified);
      opFlags.reset (differential_output);
      opFlags.reset (use_state);
    }

  basicBlock::objectInitializeA (time0, flags);
}

void delayBlock::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
  basicBlock::objectInitializeB (args,outputSet,inputSet);
  if (args.empty ())
    {
      m_state[limiter_diff] = outputSet[0];
    }
  else
    {
      m_state[limiter_diff] = K * (args[0] + bias);
    }
}

double delayBlock::step (double ttime, double inputA)
{
  if (opFlags[simplified])
    {
      return basicBlock::step (ttime, inputA);
    }
  double dt = ttime - prevTime;

  double input = (inputA + bias);
  index_t loc = limiter_diff;
  if (dt >= fabs (5.0 * m_T1))
    {
      m_state[loc] = K * input;
    }
  else if (dt <= std::abs (0.05 * m_T1))
    {
      m_state[loc] = m_state[loc] + 1.0 / m_T1 * (K * (input + prevInput) / 2.0 - m_state[loc]) * dt;
    }
  else
    {
      double tstep = 0.05 * m_T1;
      double ct = prevTime + tstep;
      double in = prevInput;
      double pin = prevInput;
      double ival = m_state[loc];
      while (ct < ttime)
        {
          in = in + (input - prevInput) / dt * tstep;
          ival = ival + 1.0 / m_T1 * (K * (pin + in) / 2.0 - ival) * tstep;
          ct += tstep;
          pin = in;
        }
      m_state[loc] = ival + 1.0 / m_T1 * (K * (pin + input) / 2.0 - ival) * (ttime - ct + tstep);
    }
  prevInput = input;
  double out;
  if (loc > 0)
    {
      out = basicBlock::step (ttime, input);
    }
  else
    {
      out = m_state[loc];
      prevTime = ttime;
      m_output = out;
    }
  return out;
}

void delayBlock::derivElements (double input, double didt, const stateData *sD, double deriv[], const solverMode &sMode)
{
  auto offset = offsets.getDiffOffset (sMode) + limiter_diff;

  deriv[offset] =  (K * (input + bias) - sD->state[offset]) / m_T1;
  if (limiter_diff > 0)
    {
      basicBlock::derivElements (input, didt, sD, deriv, sMode);
    }
}


void delayBlock::jacElements (double input, double didt, const stateData *sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{
  if ((isAlgebraicOnly (sMode))||(opFlags[simplified]))
    {
      basicBlock::jacElements (input, didt, sD, ad, argLoc, sMode);
      return;
    }
  auto offset = offsets.getDiffOffset (sMode) + limiter_diff;
  ad.assignCheck (offset, argLoc, K / m_T1);
  ad.assign (offset, offset, -1.0 / m_T1 - sD->cj);
  basicBlock::jacElements (input,didt, sD,ad,argLoc,sMode);
}


// set parameters
void delayBlock::set (const std::string &param,  const std::string &val)
{
  return gridCoreObject::set (param, val);
}

void delayBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  //param   = gridDynSimulation::toLower(param);

  if ((param == "t1") || (param == "t"))
    {
      if (opFlags[dyn_initialized])
        {
          if (opFlags[simplified])
            {
              m_T1 = val;                    //doesn't matter, parameter doesn't get used in simplified mode
            }
          else
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

