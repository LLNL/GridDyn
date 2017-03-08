/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
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
#include "utilities/vectorOps.hpp"
#include  "utilities/matrixData.h"
#include "utilities/functionInterpreter.h"
#include "utilities/stringOps.h"
#include "core/coreObjectTemplates.h"

functionBlock::functionBlock () : basicBlock ("functionBlock_#")
{
  offsets.local().local.algSize = 2;
  offsets.local().local.diffSize = 0;
  opFlags.set (use_state);
  offsets.local().local.jacSize = 3;
}

functionBlock::functionBlock (const std::string &fname) : basicBlock ("functionBlock_#")
{
  offsets.local().local.algSize = 2;
  offsets.local().local.diffSize = 0;
  opFlags.set (use_state);
  offsets.local().local.jacSize = 3;
  setFunction (fname);
}

coreObject *functionBlock::clone (coreObject *obj) const
{
  functionBlock *nobj = cloneBase<functionBlock, basicBlock> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }
  nobj->fptr = fptr;
  nobj->gain = gain;
  nobj->bias = bias;
  return nobj;
}


// initial conditions
void functionBlock::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{

  if (desiredOutput.empty ())
    {
      if (opFlags[uses_constantarg])
        {
          m_state[limiter_alg] = K * fptr2 (gain * (inputs[0] + bias),arg2);
        }
      else
        {
          m_state[limiter_alg] = K * fptr (gain * (inputs[0] + bias));
        }
      basicBlock::dynObjectInitializeB (inputs, desiredOutput, fieldSet);
    }
  else
    {
      basicBlock::dynObjectInitializeB (inputs, desiredOutput, fieldSet);

    }

}


void functionBlock::algElements (double input, const stateData &sD, double update[], const solverMode &sMode)
{
  auto offset = offsets.getAlgOffset (sMode) + limiter_alg;
  if (opFlags[uses_constantarg])
    {
      update[offset] = K * fptr2 (gain * (input + bias), arg2);
    }
  else
    {
      update[offset] = K * fptr (gain * (input + bias));
    }
  if (limiter_alg > 0)
    {
      basicBlock::algElements (input,  sD, update, sMode);
    }
}



void functionBlock::jacElements (double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{

  auto offset = offsets.getAlgOffset (sMode) + limiter_alg;
  //use the ad.assign Macro defined in basicDefs
  // ad.assign(arrayIndex, RowIndex, ColIndex, value)
  if (opFlags[uses_constantarg])
    {
      double temp1 = fptr2 (gain * (input + bias),arg2);
      double temp2 = fptr2 (gain * (input + 1e-8 + bias),arg2);
      ad.assignCheck (offset, argLoc, K * (temp2 - temp1) / 1e-8);
    }
  else
    {
      double temp1 = fptr (gain * (input + bias));
      double temp2 = fptr (gain * (input + 1e-8 + bias));
      ad.assignCheck (offset, argLoc, K * (temp2 - temp1) / 1e-8);
    }
  ad.assign (offset, offset, -1);
  if (limiter_alg > 0)
    {
      basicBlock::jacElements (input, didt, sD, ad, argLoc, sMode);
    }

}

double functionBlock::step (coreTime ttime, double input)
{
  if (opFlags[uses_constantarg])
    {
      m_state[limiter_alg] = K * fptr2 (gain * (input + bias), arg2);
    }
  else
    {
      m_state[limiter_alg] = K * fptr (gain * (input + bias));
    }
  if ( limiter_alg > 0)
    {
      basicBlock::step (ttime, input);
    }
  m_output = m_state[0];
  prevTime = ttime;
  return m_state[0];

}

// set parameters
void functionBlock::set (const std::string &param,  const std::string &val)
{

  if ((param == "function")||(param == "func"))
    {
      auto v2 = convertToLowerCase (val);
      setFunction (v2);
    }
  else
    {
      basicBlock::set (param, val);
    }
}

void functionBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "gain")
    {
      gain = val;
    }
  else if (param == "arg")
    {
      arg2 = val;
    }
  else
    {
      basicBlock::set (param, val, unitType);
    }

}



void functionBlock::setFunction (const std::string &fname)
{
  if (isFunctionName (fname, function_type::arg))
    {
      fptr = get1ArgFunction (fname);
      opFlags.reset (uses_constantarg);
    }
  else if (isFunctionName (fname, function_type::arg2))
    {
      fptr2 = get2ArgFunction (fname);
      opFlags.set (uses_constantarg);
    }
}

/*
double functionBlock::currentValue(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
  Lp Loc;
  offsets.getLocations(sD, sMode, &Loc, this);
  double val = Loc.algStateLoc[1];
  if (!inputs.empty())
  {
    if (opFlags[uses_constantarg])
    {
      val = fptr2(gain*(inputs[0] + bias), arg2);
    }
    else
    {
      val = fptr(gain*(inputs[0] + bias));
    }
  }
  return basicBlock::currentValue({ val }, sD, sMode);
}

double functionBlock::currentValue() const
{
  return m_state[0];
}
*/