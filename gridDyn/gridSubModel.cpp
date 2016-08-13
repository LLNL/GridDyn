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

#include "gridObjects.h"
#include <cstdio>
#include <iostream>
#include "arrayData.h"
#include "stringOps.h"


gridSubModel::gridSubModel (const std::string &objName) : gridObject (objName)
{
  opFlags.set (no_pflow_states);
}


void gridSubModel::initializeA (double time0, unsigned long flags)
{
  if (enabled)
    {
      objectInitializeA (time0, flags);

      auto so = offsets.getOffsets (cLocalSolverMode);
      if (subObjectList.empty ())
        {
          so->localLoad (true);
        }
      else
        {
          loadSizes (cLocalSolverMode);
        }

      so->setOffset (0);
      prevTime = time0;
      updateFlags (true);
      setupDynFlags ();
    }
}

void gridSubModel::initializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
  if (enabled)
    {
      //make sure the state vectors are sized properly
      auto ns = stateSize (cLocalSolverMode);
      m_state.resize (ns, 0);
      m_dstate_dt.clear ();
      m_dstate_dt.resize (ns, 0);

      objectInitializeB (args, outputSet, inputSet);
      if (updatePeriod < kHalfBigNum)
        {
          opFlags.set (has_updates);
          nextUpdateTime = prevTime + updatePeriod;
          alert (this, UPDATE_REQUIRED);
        }
      opFlags.set (dyn_initialized);
    }
}

void gridSubModel::objectInitializeA (double time0, unsigned long flags)
{

  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->initializeA (time0, flags);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->dynInitializeA (time0, flags);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->dynInitializeA (time0, flags);
            }
        }
    }
}

void gridSubModel::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{

  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->initializeB (args, outputSet,inputSet);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->dynInitializeB (args, outputSet);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->dynInitializeB (inputSet);
            }
        }
    }
}

double gridSubModel::timestep (double ttime, const IOdata & /*args*/, const solverMode &)
{
  prevTime = ttime;
  return m_output;
}

void gridSubModel::rootTest (const IOdata & /*args*/, const stateData *, double /*roots*/[], const solverMode &)
{
}

void gridSubModel::rootTrigger (double ttime, const IOdata & args, const std::vector<int> & rootMask, const solverMode & sMode)
{
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          if ((!subobj)||(!(subobj->checkFlag (has_roots))))
            {
              continue;
            }
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->rootTrigger (ttime,args,rootMask,sMode);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->rootTrigger (ttime, args, rootMask, sMode);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->rootTrigger (ttime, rootMask, sMode);
            }
        }
    }
}

change_code gridSubModel::rootCheck (const IOdata & args, const stateData *sD, const solverMode &sMode, check_level_t level)
{
  auto ret = change_code::no_change;
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          if ((!subobj)||(!(subobj->checkFlag (has_roots))))
            {
              continue;
            }
          auto iret = change_code::no_change;
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              iret = static_cast<gridSubModel *> (subobj)->rootCheck (args, sD, sMode,level);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              iret = static_cast<gridSecondary *> (subobj)->rootCheck (args, sD, sMode, level);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              iret = static_cast<gridPrimary *> (subobj)->rootCheck (sD, sMode, level);
            }
          if (iret > ret)
            {
              ret = iret;
            }
        }
    }
  return ret;
}

void gridSubModel::residual (const IOdata & /*args*/, const stateData *, double /*resid*/[], const solverMode &)
{
}

void gridSubModel::derivative (const IOdata & /*args*/, const stateData *, double /*deriv*/[], const solverMode & /*sMode*/)
{
}

void gridSubModel::algebraicUpdate (const IOdata & /*args*/, const stateData *, double /*update*/ [], const solverMode &, double /*alpha*/)
{

}

void gridSubModel::jacobianElements (const IOdata & /*args*/, const stateData *, arrayData<double> *, const IOlocs & /*argLocs*/, const solverMode & /*sMode*/)
{
}

void gridSubModel::ioPartialDerivatives  (const IOdata & /*args*/, const stateData *, arrayData<double> *, const IOlocs & /*argLocs*/, const solverMode & /*sMode*/)
{
}

change_code gridSubModel::powerFlowAdjust (const IOdata & args, unsigned long flags, check_level_t level)
{
  auto ret = change_code::no_change;
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          if ((!subobj)||(!(subobj->checkFlag (has_powerflow_adjustments))))
            {
              continue;
            }
          auto iret = change_code::no_change;
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              iret = static_cast<gridSubModel *> (subobj)->powerFlowAdjust (args, flags, level);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              iret = static_cast<gridSecondary *> (subobj)->powerFlowAdjust (args, flags,level);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              iret = static_cast<gridPrimary *> (subobj)->powerFlowAdjust (flags,level);
            }
          if (iret > ret)
            {
              ret = iret;
            }
        }
    }
  return ret;
}

double gridSubModel::getOutput (const IOdata & /*args*/, const stateData *sD, const solverMode &sMode, index_t Num) const
{
  if (Num > m_outputSize)
    {
      return kNullVal;
    }
  Lp Loc = offsets.getLocations  (sD, sMode, this);
  if (opFlags[differential_output])
    {
      if (Loc.diffSize > Num)
        {
          return Loc.diffStateLoc[Num];
        }
      else
        {
          return kNullVal;
        }
    }
  else
    { //if differential flag was not specified try algebraic state values then differential

      if (Loc.algSize > Num)
        {
          return Loc.algStateLoc[Num];
        }
      else if (Loc.diffSize + Loc.algSize > Num)
        {
          return Loc.diffStateLoc[Num - Loc.algSize];
        }
      else
        {
          return (m_state.size () > Num) ? m_state[Num] : m_output;
        }
    }

}

static const IOdata kNullVec;

double gridSubModel::getOutput (index_t Num) const
{
  return getOutput (kNullVec, nullptr, cLocalSolverMode, Num);
}

double gridSubModel::getOutputLoc (const IOdata & /*args*/, const stateData *sD, const solverMode &sMode, index_t &currentLoc, index_t Num) const
{
  if (Num > m_outputSize)
    {
      currentLoc = kNullLocation;
      return kNullVal;
    }
  if (isLocal (sMode))
    {
      currentLoc = kNullLocation;
      return getOutput (Num);
    }
  else if (sD)
    {
      Lp Loc = offsets.getLocations (sD, sMode,  this);
      if (opFlags[differential_output])
        {
          if (Loc.diffSize > Num)
            {
              currentLoc = Loc.algOffset + Num;
              return Loc.diffStateLoc[Num];
            }
          else
            {
              currentLoc = kNullLocation;
              return (m_state.size () > Num) ? m_state[Num] : m_output;
            }
        }
      else
        {         //if differential flag was not specified try algebraic state values then differential

          if (Loc.algSize > Num)
            {
              currentLoc = Loc.algOffset + Num;
              return Loc.algStateLoc[Num];
            }
          else if (Loc.diffSize + Loc.algSize > Num)
            {
              currentLoc = Loc.diffOffset - Loc.algSize + Num;
              return Loc.diffStateLoc[Num - Loc.algSize];
            }
          else
            {
              currentLoc = kNullLocation;
              return (m_state.size () > Num) ? m_state[Num] : m_output;
            }
        }
    }
  else
    {
      if (opFlags[differential_output])
        {
          currentLoc = offsets.getDiffOffset (sMode) + Num;
        }
      else
        {
          auto so = offsets.getOffsets (sMode);
          if (so->total.algSize > Num)
            {
              currentLoc = so->algOffset + Num;
            }
          else if (so->total.diffSize + so->total.algSize > Num)
            {
              currentLoc = so->diffOffset - so->total.algSize + Num;
            }
          else
            {
              currentLoc = kNullLocation;
            }
        }
      return getOutput (Num);
    }
}


IOdata gridSubModel::getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode)
{
  IOdata mout (m_outputSize);
  for (count_t pp = 0; pp < m_outputSize; ++pp)
    {
      mout[pp] = getOutput (args, sD, sMode, pp);
    }
  return mout;

}

//static IOdata kNullVec;

IOlocs gridSubModel::getOutputLocs  (const solverMode &sMode) const
{
  IOlocs oloc  (m_outputSize);

  if (!isLocal (sMode))
    {
      for (count_t pp = 0; pp < m_outputSize; ++pp)
        {
          getOutputLoc (kNullVec, nullptr, sMode, (oloc[pp]),pp);
        }
    }
  else
    {
      for (count_t pp = 0; pp < m_outputSize; ++pp)
        {
          oloc[pp] = kNullLocation;
        }
    }
  return oloc;

}

double gridSubModel::getDoutdt (const stateData *sD, const solverMode &sMode, index_t Num)
{
  if (Num > m_outputSize)
    {
      return kNullVal;
    }
  Lp Loc = offsets.getLocations  (sD, sMode, this);
  if (opFlags[differential_output])
    {
      return Loc.dstateLoc[Num];
    }
  else
    {
      if (Loc.algSize > Num)
        {
          return 0;
        }
      else if (Loc.diffSize + Loc.algSize > Num)
        {
          return Loc.dstateLoc[Num - Loc.algSize];
        }
      else
        {
          return 0;
        }
    }

}

void gridSubModel::outputPartialDerivatives (const IOdata & /*args*/, const stateData *, arrayData<double> *ad, const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (opFlags[differential_output])
    {
      ad->assignCheck (0, so->diffOffset, 1);
    }
  else
    {
      if (so->total.algSize > 0)
        {
          ad->assignCheck (0, so->algOffset, 1);
        }
      else if (so->total.diffSize > 0)
        {
          ad->assignCheck (0, so->diffOffset, 1);
        }
    }

}

static const stringVec emptyStr {};

stringVec gridSubModel::localStateNames () const
{
  return emptyStr;
}


index_t gridSubModel::findIndex (const std::string & field, const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);
  if (field.compare (0, 3, "alg") == 0)
    {
      auto num = static_cast<index_t> (trailingStringInt (field, 0));
      if (so->total.algSize > num)
        {
          if (so->algOffset != kNullLocation)
            {
              return so->algOffset + num;
            }
          else
            {
              return kNullLocation;
            }
        }
      else if (opFlags[dyn_initialized] == false)
        {
          return kNullLocation;
        }
      else
        {
          return kInvalidLocation;
        }
    }
  if (field.compare (0, 4, "diff") == 0)
    {
      auto num = static_cast<index_t> (trailingStringInt (field, 0));
      if (so->total.diffSize > num)
        {
          if (so->diffOffset != kNullLocation)
            {
              return so->diffOffset + num;
            }
          else
            {
              return kNullLocation;
            }
        }
      else if (opFlags[dyn_initialized] == false)
        {
          return kNullLocation;
        }
      else
        {
          return kInvalidLocation;
        }
    }
  auto stateNames = localStateNames ();
  for (index_t nn = 0; nn < stateNames.size (); ++nn)
    {
      if (field == stateNames[nn])
        {
          if (nn < offsets.local->local.algSize)
            {
              if (so->algOffset != kNullLocation)
                {
                  return so->algOffset + nn;
                }
              else
                {
                  return kNullLocation;
                }
            }
          else if (nn - offsets.local->local.algSize < offsets.local->local.diffSize)
            {
              if (so->diffOffset != kNullLocation)
                {
                  return so->diffOffset + nn - offsets.local->local.algSize;
                }
              else
                {
                  return kNullLocation;
                }
            }
          else if (opFlags[dyn_initialized] == false)
            {
              return kNullLocation;
            }
          else
            {
              return kInvalidLocation;
            }
        }
    }
  return kInvalidLocation;
}

void gridSubModel::updateLocalCache (const IOdata & /*args*/, const stateData *, const solverMode &)
{
  //nothing to cache by default
}

void gridSubModel::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  auto stateNames = localStateNames ();
  //create a default set of state names that we will overwrite if able to do so
  gridObject::getStateName (stNames, sMode, prefix);

  if (stateNames.empty ())
    {
      return;
    }
  auto so = offsets.getOffsets (sMode);
  int mxsize = offsets.maxIndex (sMode);
  std::string prefix2 = prefix + name + ':';
  count_t kk;
  if (mxsize > 0)
    {
      if (static_cast<int> (stNames.size ()) < mxsize)
        {
          stNames.resize (mxsize);
        }
    }
  else
    {
      return;
    }
  if (hasAlgebraic (sMode))
    {

      for (kk = 0; (kk < so->local.algSize)&&(kk < stateNames.size ()); kk++)
        {
          stNames[so->algOffset + kk] =
            prefix2 + stateNames[kk];
        }
    }
  if (!isAlgebraicOnly (sMode))
    {
      if (so->local.diffSize > 0)
        {
          auto doffset = offsets.local->total.algSize;
          for (kk = 0; (kk < so->local.diffSize)&&(kk + doffset < stateNames.size ()); kk++)
            {
              stNames[so->diffOffset + kk] =
                prefix2 + stateNames[kk + doffset];
            }
        }
    }

}

