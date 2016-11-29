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

// headers
#include "linkModels/zBreaker.h"
#include "gridCoreTemplates.h"
#include "gridBus.h"
#include "stringOps.h"
#include "core/gridDynExceptions.h"

#include "objectFactoryTemplates.h"

#include <cstring>

using namespace gridUnits;

static typeFactory<zBreaker> glf ("link", stringVec { "zbreaker", "zline"});

zBreaker::zBreaker (const std::string &objName) : gridLink (objName)
{
	opFlags.set(network_connected);
}

gridCoreObject * zBreaker::clone (gridCoreObject *obj) const
{
  zBreaker *lnk = cloneBase<zBreaker, gridLink> (this, obj);
  if (lnk == nullptr)
    {
      return obj;
    }
  return lnk;
}
// parameter set functions

void zBreaker::set (const std::string &param, const std::string &val)
{

  if (param == "status")
    {
      auto nval = convertToLowerCase (val);
      if ((nval == "in") || (nval == "closed") || (nval == "connected"))
        {
          switchMode (0,false);
        }
      else if ((nval == "out")||(nval == "open")||(nval == "disconnected"))
        {
          switchMode (0, true);
        }
      else
        {
		  throw(invalidParameterValue());
        }
    }
  else
    {
      gridLink::set (param,val);
    }
}

void zBreaker::set (const std::string &param, double val, units_t unitType)
{

  if (param == "status")
    {
      switchMode (0, (val > 0.1));
    }
  else
    {
      gridLink::set (param,val,unitType);
    }

}

void zBreaker::pFlowObjectInitializeA (gridDyn_time /*time0*/, unsigned long /*flags*/)
{
  if (isConnected ())
    {
      if (!merged)
        {
          merge ();
        }
    }
  else if (merged)
    {
      unmerge ();
    }

}

void zBreaker::dynObjectInitializeA (gridDyn_time /*time0*/, unsigned long /*flags*/)
{
  if (isConnected ())
    {
      if (!merged)
        {
          merge ();
        }
    }
  else if (merged)
    {
      unmerge ();
    }

}

void zBreaker::switchMode (index_t /*num*/, bool mode)
{
  //TODO:PT: this shouldn't cause enable disable, I need to replace this with some of the checks for enabled disable
  if (mode == opFlags[switch1_open_flag])
    {
      return;
    }

  opFlags.flip (switch1_open_flag);
  opFlags.flip (switch2_open_flag);
  if (opFlags[pFlow_initialized])
    {
      if (linkInfo.v1 < 0.2)
        {
          alert (this, POTENTIAL_FAULT_CHANGE);
        }
      if (isConnected ())
        {
          merge ();
        }
      else
        {
          unmerge ();
        }
    }


  /*if (opFlags[switch2_open_flag])
  {
  enable();
  opFlags.reset(switch2_open_flag);
  }
  else
  {
  disable();
  opFlags.set(switch2_open_flag);
  }*/

}


void zBreaker::updateLocalCache ()
{
  if (!enabled)
    {
      return;
    }
  linkInfo.v1 = B1->getVoltage ();
  linkInfo.v2 = linkInfo.v1;
}
void zBreaker::updateLocalCache (const stateData *sD, const solverMode &)
{
  if (!enabled)
    {
      return;
    }
  if ((linkInfo.seqID == sD->seqID) && (sD->seqID != 0))
    {
      return;
    }
  std::memset (&linkInfo, 0, sizeof(linkI));
  linkInfo.seqID = sD->seqID;
  linkInfo.v1 = B1->getVoltage ();
  linkInfo.v2 = linkInfo.v1;
}

double zBreaker::quickupdateP ()
{
  return 0;
}

void zBreaker::checkMerge ()
{
  if (isConnected ())
    {
      merge ();
    }
  else
    {
      merged = false;
    }
}
void zBreaker::merge ()
{
  B1->mergeBus (B2);
  merged = true;
}

void zBreaker::unmerge ()
{
  B1->unmergeBus (B2);
  merged = false;
}
