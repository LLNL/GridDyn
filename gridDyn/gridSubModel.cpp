/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include "gridObjects.h"
#include <cstdio>
#include <iostream>
#include  "utilities/matrixData.h"
#include "utilities/stringOps.h"


gridSubModel::gridSubModel (const std::string &objName) : gridObject (objName)
{
  opFlags.set (no_pflow_states);
}

void gridSubModel::pFlowInitializeA(coreTime time0, unsigned long flags)
{
	gridObject::pFlowInitializeA(time0, flags);

}

void gridSubModel::pFlowInitializeB()
{
	gridObject::pFlowInitializeB();
}

void gridSubModel::dynInitializeA (coreTime time0, unsigned long flags)
{
  if (isEnabled())
    {
      dynObjectInitializeA (time0, flags);

      auto so = offsets.getOffsets (cLocalSolverMode);
      if (getSubObjects().empty ())
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

void gridSubModel::dynInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet)
{
  if (isEnabled())
    {
      //make sure the state vectors are sized properly
	  auto ns = offsets.local().local.totalSize();
      m_state.resize (ns, 0);
      m_dstate_dt.clear ();
      m_dstate_dt.resize (ns, 0);

      dynObjectInitializeB (inputs, desiredOutput, inputSet);
      if (updatePeriod < maxTime)
        {
          opFlags.set (has_updates);
          nextUpdateTime = prevTime + updatePeriod;
          alert (this, UPDATE_REQUIRED);
        }
      opFlags.set (dyn_initialized);
    }
}


















