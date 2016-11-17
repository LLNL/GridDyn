/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
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
#include "timeSeries.h"
#include "matrixData.h"
#include "stringOps.h"
#include <utility>

lutBlock::lutBlock (const std::string &objName) : basicBlock (objName)
{
  opFlags.set (use_state);
}


gridCoreObject *lutBlock::clone (gridCoreObject *obj) const
{
  lutBlock *nobj;
  if (obj == nullptr)
    {
      nobj = new lutBlock ();
    }
  else
    {
      nobj = dynamic_cast<lutBlock *> (obj);
      if (nobj == nullptr)
        {
          basicBlock::clone (obj);
          return obj;
        }
    }
  basicBlock::clone (nobj);
  nobj->lut = lut;
  nobj->b = b;
  nobj->m = 0;
  nobj->vlower = vlower;
  nobj->vupper = vupper;
  nobj->lindex = lindex;
  return nobj;
}


// initial conditions
void lutBlock::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{

  if (outputSet.empty ())
    {
      m_state[limiter_alg] = K * computeValue (args[0] + bias);
      basicBlock::objectInitializeB (args, outputSet, fieldSet);
    }
  else
    {
      //TOOD:: PT figure out how to invert the lookup table
      basicBlock::objectInitializeB (args, outputSet, fieldSet);

    }

}

void lutBlock::algElements (double input, const stateData *sD, double update[], const solverMode &sMode)
{
  auto offset = offsets.getAlgOffset (sMode) + limiter_alg;
  update[offset] = K * computeValue (input + bias);
  if (limiter_alg > 0)
    {
      return basicBlock::algElements (input, sD, update, sMode);
    }
}

void lutBlock::jacElements (double input, double didt, const stateData *sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{

  auto offset = offsets.getAlgOffset (sMode) + limiter_alg;
  //use the ad.assign Macro defined in basicDefs
  // ad.assign(arrayIndex, RowIndex, ColIndex, value)
  ad.assignCheck (offset, argLoc, K * m);
  ad.assign (offset, offset, -1);
  if (limiter_alg > 0)
    {
      basicBlock::jacElements (input, didt, sD, ad, argLoc, sMode);
    }
}



// set parameters
void lutBlock::set (const std::string &param,  const std::string &val)
{

  if (param == "lut")
    {
      auto v2 = str2vector (val,-kBigNum,";,:");
      lut.clear ();
      lut.push_back (std::make_pair (-kBigNum,0.0));
      lut.push_back (std::make_pair (kBigNum,0.0));
      for (size_t mm = 0; mm < v2.size (); mm += 2)
        {
          lut.push_back (std::make_pair (v2[mm],v2[mm + 1]));
        }
      sort (lut.begin (),lut.end ());
      lut[0].second = lut[1].second;
      (*lut.end ()).second = (*(lut.end () - 1)).second;
    }
  else if (param == "element")
    {
      auto v2 = str2vector (val, -kBigNum, ";,:");
      for (size_t mm = 0; mm < v2.size (); mm += 2)
        {
          lut.push_back (std::make_pair (v2[mm], v2[mm + 1]));
        }
      sort (lut.begin (), lut.end ());
      lut[0].second = lut[1].second;
      (*lut.end ()).second = (*(lut.end () - 1)).second;
    }
  else if (param == "file")
    {
      std::string temp = val;
      timeSeries ts (temp);
     
      lut.clear ();
      lut.push_back (std::make_pair (-kBigNum, 0.0));
      lut.push_back (std::make_pair (kBigNum, 0.0));
      for (size_t pp = 0; pp < ts.count; ++pp)
        {
          lut.push_back (std::make_pair (ts.time[pp], ts.data[pp]));
        }
      sort (lut.begin (), lut.end ());
      lut[0].second = lut[1].second;
      (*lut.end ()).second = (*(lut.end () - 1)).second;
    }
  else
    {
      basicBlock::set (param, val);
    }
 
}

void lutBlock::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param[0] == '#')
    {

    }
  else
    {
      basicBlock::set (param, val, unitType);
    }

}

double lutBlock::step (double ttime, double input)
{

  m_state[limiter_alg] = K * computeValue (input + bias);

  if (limiter_alg > 0)
    {
      basicBlock::step (ttime, input);
    }
  else
    {
      m_output = m_state[0];
      prevTime = ttime;
    }

  return m_state[0];
}

double lutBlock::computeValue (double input)
{
  if (input > vupper)
    {
      ++lindex;
      while (lut[lindex].first < input)
        {
          ++lindex;
        }
      vlower = lut[lindex - 1].first;
      vupper = lut[lindex].first;
      m = (lut[lindex].second - lut[lindex - 1].second) / (vupper - vlower);
      b = lut[lindex - 1].second;
    }
  else if (input < vlower)
    {
      --lindex;
      while (lut[lindex].first > input)
        {
          --lindex;
        }
      vlower = lut[lindex - 1].first;
      vupper = lut[lindex].first;
      m = (lut[lindex].second - lut[lindex - 1].second) / (vupper - vlower);
      b = lut[lindex - 1].second;
    }
  return (input - vlower) * m + b;
}

/*
double lutBlock::currentValue(const IOdata &args, const stateData *sD, const solverMode &sMode) const
{
  Lp Loc;
  offsets.getLocations(sD, sMode, &Loc, this);
  double val = Loc.algStateLoc[1];
  if (!args.empty())
  {
    val=computeValue(args[0]+bias);
  }
  return basicBlock::currentValue({ val }, sD, sMode);
}

double lutBlock::currentValue() const
{
  return m_state[0];
}
*/
