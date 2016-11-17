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
#include "vectorOps.hpp"
#include "matrixData.h"
#include "gridCoreTemplates.h"

pidBlock::pidBlock (const std::string &objName) : basicBlock (objName)
{
  opFlags.set (use_state);
  opFlags.set (differential_output);
}

pidBlock::pidBlock (double P, double I, double D,const std::string &objName) : basicBlock (objName), m_P (P),m_I (I),m_D (D)
{
  opFlags.set (use_state);
  opFlags.set(differential_output);
  if (D != 0)
    {
      no_D = false;
    }
}

gridCoreObject *pidBlock::clone (gridCoreObject *obj) const
{
	pidBlock *nobj = cloneBase<pidBlock, basicBlock>(this, obj);
  if (nobj == nullptr)
    {
	  return obj;
    }
  nobj->m_P = m_P;
  nobj->m_I = m_I;
  nobj->m_D = m_D;
  nobj->m_T1 = m_T1;
  nobj->iv = iv;
  nobj->no_D = no_D;
  return nobj;
}

void pidBlock::objectInitializeA (double time0, unsigned long flags)
{
  basicBlock::objectInitializeA (time0, flags);
  offsets.local->local.diffSize += 2;
  offsets.local->local.jacSize += 8;
}

// initial conditions
/*in local layout algebraic states come first then differential
0 is PID output
1 is derivative
[limiter_diff] states for the limiters
[limiter_diff] filtered PID output--this is what goes into the limiters
[limiter_diff+1] is the derivative filter
[limiter_diff+2] is the integral calculation
*/
void pidBlock::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{
  double in = (args.empty ()) ? 0 : args[0] + bias;
  basicBlock::objectInitializeB (args, outputSet, fieldSet);
  if (outputSet.empty ())
    {
      m_state[limiter_diff + 2] = iv;
      m_dstate_dt[limiter_diff + 2] = in * m_I; //integral
      m_state[limiter_diff + 1] = in; //derivative uses a filter function
      m_state[limiter_diff] = K * (m_P * in + m_state[limiter_diff + 2]); //differential should be 0
      //m_state[1] should be 0

      fieldSet[0] = m_state[0];
    }
  else
    {

      m_dstate_dt[limiter_diff + 2] = m_I * in; //rate of change of integral value
      m_state[limiter_diff + 1] = in;     //derivative uses a filter function
      m_state[limiter_diff] = outputSet[0];
      //m_state[1] should be 0
      if (m_I != 0)
        {
          m_state[limiter_diff + 2] = (m_state[limiter_diff] / K - m_P * (in)); //note: differential component assumed 0
        }
      else if (in != 0.0)
        {
          m_state[limiter_diff + 2] = 0;
          bias += m_state[limiter_diff] / K / m_P - in;
          in = args[0] + bias;
          m_dstate_dt[limiter_diff + 2] = m_I * in; //integral
          m_state[limiter_diff + 2] = in; //derivative uses a filter function
        }
      else
        {
          m_state[limiter_diff + 1] = 0;
        }
      fieldSet[0] = m_state[0];
    }
  prevInput = in + bias;
}

void pidBlock::derivElements (double input, double didt, const stateData *sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD, deriv, sMode, this);
  Loc.destDiffLoc[limiter_diff + 2] = m_I * (input + bias);
  Loc.destDiffLoc[limiter_diff + 1] = (no_D) ? 0 : (m_D * (input + bias) - Loc.diffStateLoc[limiter_diff + 1]) / m_T1;

  Loc.destDiffLoc[limiter_diff] = (K * (m_P * (input + bias) + Loc.dstateLoc[limiter_diff + 1] + Loc.diffStateLoc[limiter_diff + 2]) - Loc.diffStateLoc[limiter_diff]) / m_Td;
  if (limiter_diff > 0)
    {
      basicBlock::derivElements (input, didt, sD, deriv, sMode);
    }
}


void pidBlock::jacElements (double input, double didt, const stateData *sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD, nullptr, sMode,this);
  //adjust the offset to account for the limiter states;
  Loc.diffOffset += limiter_diff;
  if (hasDifferential (sMode))
    {
      //  Loc.destDiffLoc[limiter_diff] = (K*(m_P*(input + bias) + Loc.dstateLoc[limiter_diff + 1] + Loc.diffStateLoc[limiter_diff + 2]) - Loc.diffStateLoc[limiter_diff]) / m_Td;
      if (opFlags[has_limits])
        {
          basicBlock::jacElements (input, didt, sD, ad, argLoc, sMode);
        }

      ad.assign (Loc.diffOffset, Loc.diffOffset, -1.0 / m_Td - sD->cj);
      ad.assign (Loc.diffOffset, Loc.diffOffset + 1, K * sD->cj / m_Td);

      ad.assign (Loc.diffOffset, Loc.diffOffset + 2, K / m_Td);
      ad.assignCheckCol (Loc.diffOffset, argLoc, K * m_P / m_Td);
      if (no_D)
        {
          ad.assign (Loc.diffOffset + 1, Loc.diffOffset + 1, -sD->cj);
        }
      else
        {
          ad.assignCheckCol (Loc.diffOffset + 1, argLoc, m_D / m_T1);
          ad.assign (Loc.diffOffset + 1, Loc.diffOffset + 1, -1.0 / m_T1 - sD->cj);
        }

      ad.assignCheckCol (Loc.diffOffset + 2, argLoc, m_I);
      ad.assign (Loc.diffOffset + 2, Loc.diffOffset + 2, -sD->cj);
    }
}

double pidBlock::step (double ttime, double inputA)
{
  double dt = ttime - prevTime;
  double input = inputA + bias;
  //integral state

  //derivative state
  if (dt >= fabs (5.0 * std::max (m_T1,m_Td)))
    {
      m_state[limiter_diff + 2] = m_state[limiter_diff + 2] + m_I * (input + prevInput) / 2.0 * dt;
      m_state[limiter_diff + 1] = input;
      m_state[limiter_diff] = K * (m_P * input + m_state[limiter_diff + 2]);
    }
  else
    {
      double tstep = 0.05 * std::min (m_T1,m_Td);
      double ct = prevTime + tstep;
      double in = prevInput;
      double pin = prevInput;
      double ival_int = m_state[limiter_diff + 2];
      double ival_der = m_state[limiter_diff + 1];
      double ival_out = m_state[limiter_diff];
      double didt = (input - prevInput) / dt;
      double der;
      while (ct < ttime)
        {
          in = in + didt * tstep;
          ival_int += m_I * (in + pin) / 2 * tstep;
          der = (no_D) ? 0 : 1.0 / m_T1 * (m_D * (pin + in) / 2.0 - ival_der);
          ival_der += der * tstep;
          ival_out += (K * (m_P * in + der + ival_int) - ival_out) / m_Td * tstep;
          ct += tstep;
          pin = in;
        }
      m_state[limiter_diff + 2] = ival_int + m_I * (pin + input) / 2.0 * (ttime - ct + tstep);
      der = (no_D) ? 0 : 1.0 / m_T1 * (m_D * (pin + input) / 2.0 - ival_der);
      m_state[limiter_diff + 1] = ival_der + der * (ttime - ct + tstep);
      m_state[limiter_diff] = ival_out + (K * (m_P * input + der + m_state[limiter_diff + 2]) - ival_out) / m_Td * (ttime - ct + tstep);
    }
  prevInput = input;


  if (opFlags[has_limits])
    {
      basicBlock::step (ttime, input);
    }
  else
    {
      prevTime = ttime;
      m_output = m_state[0];
    }
  return m_output;
}


index_t pidBlock::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  if (field == "integral")
    {
      ret = offsets.getDiffOffset (sMode);
      ret = (ret != kNullLocation) ? ret + 1 : ret;
    }
  else if (field == "derivative")
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
void pidBlock::set (const std::string &param,  const std::string &val)
{
  basicBlock::set (param, val);
}

void pidBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if ((param == "p") || (param == "proportional"))
    {
      m_P = val;
    }
  else if ((param == "i") || (param == "integral"))
    {
      m_I = val;
    }
  else if ((param == "d") || (param == "derivative"))
    {
      m_D = val;
      no_D = (m_D == 0.0);

    }
  else if ((param == "t") || (param == "t1"))
    {
      m_T1 = val;
    }
  else if ((param == "iv")||(param == "initial_value"))
    {
      iv = val;
    }
  else
    {
      basicBlock::set (param, val, unitType);
    }

 
}

stringVec pidBlock::localStateNames () const
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
  out[loc++] = "deriv_delay";
  out[loc] = "integral";
  return out;

}
