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


#include "controlSystem.h"
#include "submodels/gridControlBlocks.h"
#include "gridCoreTemplates.h"
#include "core/gridDynExceptions.h"

controlSystem::controlSystem (const std::string &objName) : gridSubModel (objName)
{

}

controlSystem::~controlSystem ()
{

}

gridCoreObject * controlSystem::clone (gridCoreObject *obj) const
{
  controlSystem *cs = cloneBase<controlSystem, gridSubModel> (this, obj);
  if (cs == nullptr)
    {
      return obj;
    }
  return cs;
}


void controlSystem::add (gridCoreObject *obj)
{
  if (dynamic_cast<basicBlock *> (obj))
    {
      add (static_cast<basicBlock *> (obj));
    }
  else
    {
	  throw(invalidObjectException(this));
    }
}

void controlSystem::add (basicBlock *blk)
{
  blk->setParent (this);
  blk->set ("basepower", systemBasePower);
  blocks.push_back (blk);
  blk->locIndex = static_cast<index_t> (blocks.size ()) - 1;
  subObjectList.push_back (blk);
}

void controlSystem::objectInitializeA (gridDyn_time time0, unsigned long flags)
{
  for (auto &bb : blocks)
    {
      bb->initializeA (time0, flags);
    }
}
void controlSystem::objectInitializeB (const IOdata & /*args*/, const IOdata & /*outputSet*/, IOdata & /*inputSet*/)
{

}

void controlSystem::set (const std::string &param, const std::string &val)
{
  if (param[0] == '#')
    {

    }
  else
    {
      gridSubModel::set (param, val);
    }
}

void controlSystem::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param[0] == '#')
    {

    }
  else
    {
      gridSubModel::set (param, val,unitType);
    }

}

index_t controlSystem::findIndex (const std::string & /*field*/, const solverMode &) const
{
  return kInvalidLocation;
}

void controlSystem::residual (const IOdata & /*args*/, const stateData *, double /*resid*/[], const solverMode &)
{

}

void controlSystem::jacobianElements (const IOdata & /*args*/, const stateData *,
                                      matrixData<double> &,
                                      const IOlocs & /*argLocs*/, const solverMode & /*sMode*/)
{

}

void controlSystem::timestep (gridDyn_time /*ttime*/, const IOdata & /*args*/, const solverMode & /*sMode*/)
{
  
}

void controlSystem::rootTest (const IOdata & /*args*/, const stateData *, double /*roots*/[], const solverMode & /*sMode*/)
{

}

void controlSystem::rootTrigger (gridDyn_time /*ttime*/, const IOdata & /*args*/, const std::vector<int> & /*rootMask*/, const solverMode & /*sMode*/)
{

}

change_code controlSystem::rootCheck (const IOdata & /*args*/, const stateData *, const solverMode & /*sMode*/, check_level_t /*level*/)
{
  return change_code::no_change;
}
//virtual void setTime(gridDyn_time time){prevTime=time;};

