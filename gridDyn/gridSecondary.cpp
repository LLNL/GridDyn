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
#include "stringOps.h"
#include <cstdio>
#include <iostream>




gridSecondary::gridSecondary (const std::string &objName) : gridObject (objName)
{
}

void gridSecondary::pFlowInitializeA (gridDyn_time time0, unsigned long flags)
{
  if (enabled)
    {

      pFlowObjectInitializeA (time0, flags);
      prevTime = time0;
      updateFlags (false);
      setupPFlowFlags ();
    }
}

void gridSecondary::pFlowInitializeB ()
{
  if (enabled)
    {
      pFlowObjectInitializeB ();
      opFlags.set (pFlow_initialized);
    }
}
void gridSecondary::dynInitializeA (gridDyn_time time0, unsigned long flags)
{
  if (enabled)
    {
      dynObjectInitializeA (time0, flags);
      prevTime = time0;
      updateFlags (true);
      setupDynFlags ();
    }
}

void gridSecondary::dynInitializeB (const IOdata & args, const IOdata & outputSet)
{
  if (enabled)
    {

      auto ns = stateSize (cLocalSolverMode);
      m_state.resize (ns, 0);
      m_dstate_dt.clear ();
      m_dstate_dt.resize (ns, 0);
      dynObjectInitializeB (args, outputSet);
      if (updatePeriod < kHalfBigNum)
        {
          opFlags.set (has_updates);
          nextUpdateTime = prevTime + updatePeriod;
          alert (this, UPDATE_REQUIRED);
        }
      opFlags.set (dyn_initialized);
    }
}


void gridSecondary::pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
		  if (dynamic_cast<gridSubModel *>(subobj))
		  {
			  if (subobj->checkFlag(pflow_init_required))
			  {
				  static_cast<gridSubModel *> (subobj)->initializeA(time0, flags);
			  }
		  }
		  else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->pFlowInitializeA (time0, flags);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->pFlowInitializeA (time0, flags);
            }
        }
    }

}

void gridSecondary::pFlowObjectInitializeB ()
{
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
		  if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->pFlowInitializeB ();
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->pFlowInitializeB ();
            }
        }
    }
}

void gridSecondary::dynObjectInitializeA (gridDyn_time time0, unsigned long flags)
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

void gridSecondary::dynObjectInitializeB (const IOdata & args, const IOdata & outputSet)
{
  if (!subObjectList.empty ())
    {
      IOdata iset;
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->initializeB (args, outputSet, iset);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->dynInitializeB (args, outputSet);
            }
          else if (dynamic_cast<gridPrimary *> (subobj))
            {
              static_cast<gridPrimary *> (subobj)->dynInitializeB (iset);
            }
        }
    }
}

void gridSecondary::preEx (const IOdata & /*args*/, const stateData *, const solverMode & /*sMode*/)
{
}

void gridSecondary::residual (const IOdata & /*args*/, const stateData *, double /*resid*/[], const solverMode & /*sMode*/)
{
}

void gridSecondary::algebraicUpdate (const IOdata & /*args*/, const stateData *, double /*update*/[], const solverMode & /*sMode*/, double /*alpha*/)
{
}

void gridSecondary::derivative (const IOdata & /*args*/, const stateData *, double /*deriv*/[], const solverMode & /*sMode*/)
{
}

double gridSecondary::getRealPower (const IOdata & /*args*/, const stateData *, const solverMode & /*sMode*/)
{
  return 0.0;
}

double gridSecondary::getReactivePower (const IOdata & /*args*/, const stateData *, const solverMode & /*sMode*/)
{
  return 0.0;
}


double gridSecondary::getRealPower () const
{
  return 0.0;
}

double gridSecondary::getReactivePower () const
{
  return 0;
}


void gridSecondary::timestep (gridDyn_time ttime, const IOdata & args, const solverMode & sMode)
{
  prevTime = ttime;
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              static_cast<gridSubModel *> (subobj)->timestep (ttime, args, sMode);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              static_cast<gridSecondary *> (subobj)->timestep (ttime, args, sMode);
            }
        }
    }
}

double gridSecondary::getAdjustableCapacityUp (gridDyn_time /*time*/) const
{
  return 0;
}

double gridSecondary::getAdjustableCapacityDown (gridDyn_time /*time*/) const
{
  return 0;
}

void gridSecondary::jacobianElements (const IOdata & /*args*/, const stateData *, matrixData<double> &, const IOlocs & /*argLocs*/, const solverMode & /*sMode*/)
{
}

void gridSecondary::outputPartialDerivatives  (const IOdata & /*args*/, const stateData *, matrixData<double> &, const solverMode & /*sMode*/)
{
}

void gridSecondary::ioPartialDerivatives (const IOdata & /*args*/, const stateData *, matrixData<double> &, const IOlocs & /*argLocs*/, const solverMode & /*sMode*/)
{
}

void gridSecondary::rootTest  (const IOdata & /*args*/, const stateData *, double /*roots*/[], const solverMode & /*sMode*/)
{
}

void gridSecondary::rootTrigger (gridDyn_time /*ttime*/, const IOdata & /*args*/, const std::vector<int> & /*rootMask*/, const solverMode & /*sMode*/)
{
}

change_code gridSecondary::rootCheck (const IOdata & /*args*/, const stateData *, const solverMode &, check_level_t /*level*/)
{
  return change_code::no_change;
}

void gridSecondary::reset (reset_levels /*level*/)
{
}

change_code gridSecondary::powerFlowAdjust (const IOdata & /*args*/, unsigned long /*flags*/, check_level_t /*level*/)
{
  return change_code::no_change;
}

double gridSecondary::getDoutdt (const IOdata & /*args*/, const stateData *, const solverMode &, index_t /*num*/)
{
  return 0.0;
}



static const IOlocs NullLocVec {
  kNullLocation, kNullLocation
};

IOlocs gridSecondary::getOutputLocs  (const solverMode &) const
{
  return NullLocVec;
}

index_t gridSecondary::getOutputLoc ( const solverMode &, index_t /*num*/)
{
	return kNullLocation;
}

double gridSecondary::getOutput (const IOdata &args, const stateData *sD, const solverMode &sMode, index_t num)
{
  if (num == PoutLocation)
    {
      return getRealPower (args, sD, sMode);
    }
  else if (num == QoutLocation)
    {
      return getReactivePower (args, sD, sMode);
    }
  else
    {
      return kNullVal;
    }
}


double gridSecondary::getOutput (index_t num) const
{
  if (num == PoutLocation)
    {
      return getRealPower ();
    }
  else if (num == QoutLocation)
    {
      return getReactivePower ();
    }
  else
    {
      return kNullVal;
    }
}

IOdata gridSecondary::getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode)
{
  IOdata out (2);
  out[PoutLocation] = getRealPower (args, sD, sMode);
  out[QoutLocation] = getReactivePower (args, sD, sMode);
  return out;
}

IOdata gridSecondary::predictOutputs (double /*ptime*/, const IOdata &args, const stateData *sD, const solverMode &sMode)
{
  IOdata out (2);
  out[PoutLocation] = getRealPower (args, sD, sMode);
  out[QoutLocation] = getReactivePower (args, sD, sMode);
  return out;
}



index_t gridSecondary::findIndex (const std::string & field, const solverMode & sMode) const
{
  std::string ifield;

  index_t ret = kInvalidLocation;
  int num = trailingStringInt (field, ifield, -1);
  if (num >= 0)
    {
      auto so = offsets.getOffsets (sMode);

      if (ifield == "state")
        {
          if (num < static_cast<int> (so->total.algSize))
            {
              if (so->algOffset != kNullLocation)
                {
                  ret = so->algOffset + num;
                }
              else
                {
                  ret = kNullLocation;
                }
            }
          else if (num < static_cast<int> (so->total.algSize + so->total.diffSize))
            {
              if (so->diffOffset != kNullLocation)
                {
                  ret = so->diffOffset + (num - so->total.algSize);
                }
              else
                {
                  ret = kNullLocation;
                }
            }
        }
      else if (ifield == "alg")
        {
          if (num < static_cast<int> (so->total.algSize))
            {
              if (so->algOffset != kNullLocation)
                {
                  ret = so->algOffset + num;
                }
              else
                {
                  ret = kNullLocation;
                }
            }
        }
      else if (ifield == "diff")
        {
          if (num < static_cast<int> (so->total.diffSize))
            {
              if (so->diffOffset != kNullLocation)
                {
                  ret = so->diffOffset + num;
                }
              else
                {
                  ret = kNullLocation;
                }
            }
        }
    }
  else
    {
      for (auto &subobj : subObjectList)
        {
          if (dynamic_cast<gridSubModel *> (subobj))
            {
              ret = static_cast<gridSubModel *> (subobj)->findIndex (field, sMode);
            }
          else if (dynamic_cast<gridSecondary *> (subobj))
            {
              ret = static_cast<gridSecondary *> (subobj)->findIndex (field, sMode);
            }
          if (ret != kInvalidLocation)
            {
              break;
            }

        }
    }



  return ret;
}


void gridSecondary::updateLocalCache(const IOdata & /*args*/, const stateData *, const solverMode &)
{
	//nothing to cache by default
}