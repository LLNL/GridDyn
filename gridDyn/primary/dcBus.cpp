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
#include "gridArea.h"
#include "generators/gridDynGenerator.h"
#include "loadModels/gridLoad.h"
#include "linkModels/gridLink.h"
#include "dcBus.h"
#include "objectFactoryTemplates.h"
#include "vectorOps.hpp"
#include "linkModels/dcLink.h"
#include "arrayDataSparse.h"
#include "gridCoreTemplates.h"

#include <iostream>
#include <cmath>
#include <utility>

static typeFactory<dcBus> gbf ("bus", stringVec { "dc" "hvdc" });

using namespace gridUnits;

dcBus::dcBus (const std::string &objName) : acBus (objName)
{

}

gridCoreObject *dcBus::clone (gridCoreObject *obj) const
{
  dcBus *nobj = cloneBase<dcBus, acBus> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }

  return nobj;
}


// destructor
dcBus::~dcBus ()
{

}


// add link
int dcBus::add (gridLink *lnk)
{
  if ((lnk->checkFlag (dc_only))||(lnk->checkFlag (dc_capable)))
    {
      return gridBus::add (lnk);
    }
  else
    {
      return (OBJECT_NOT_RECOGNIZED);
    }
}


// initializeB states
void dcBus::pFlowObjectInitializeA (double time0, unsigned long flags)
{
  gridBus::pFlowObjectInitializeA (time0, flags);
}

void dcBus::pFlowObjectInitializeB ()
{
  gridBus::pFlowObjectInitializeB ();

  if (opFlags[use_autogen])
    {
      busController.autogenQ = 0;
      busController.autogenQact = 0;
    }
  angle = 0;
  propogatePower ();
}

void dcBus::loadSizes (const solverMode &sMode, bool dynOnly)
{
  auto so = offsets.getOffsets (sMode);
  if (!enabled)
    {
      so->reset ();
      so->stateLoaded = true;
      so->rjLoaded = true;
      return;
    }
  if (dynOnly)
    {
      if (so->rjLoaded)
        {
          return;
        }
      so->rootAndJacobianCountReset ();
    }
  else
    {
      if (so->stateLoaded)
        {
          return;
        }
      so->reset ();
    }
  if (hasAlgebraic (sMode))
    {
      so->local.aSize = 0;
      so->local.vSize = 1;
      so->local.jacSize = 1 + 2 * static_cast<count_t> (attachedLinks.size ());
    }
  if (dynOnly)
    {
      so->total.algRoots = so->local.algRoots;
      so->total.diffRoots = so->local.diffRoots;
      so->total.jacSize = so->local.jacSize;
    }
  else
    {
      so->localLoad (false);
    }
  for (auto ld : attachedLoads)
    {
      if (!(ld->isLoaded (sMode, dynOnly)))
        {
          ld->loadSizes (sMode, dynOnly);
        }
      if (dynOnly)
        {
          so->addRootAndJacobianSizes (ld->offsets.getOffsets (sMode));
        }
      else
        {
          so->addSizes (ld->offsets.getOffsets (sMode));
        }
    }
  for (auto gen : attachedGens)
    {
      if (!(gen->isLoaded (sMode, dynOnly)))
        {
          gen->loadSizes (sMode, dynOnly);
        }
      if (dynOnly)
        {
          so->addRootAndJacobianSizes (gen->offsets.getOffsets (sMode));
        }
      else
        {
          so->addSizes (gen->offsets.getOffsets (sMode));
        }
    }
  if (!dynOnly)
    {
      so->stateLoaded = true;

    }
  so->rjLoaded = true;

}

change_code dcBus::powerFlowAdjust (unsigned long flags, check_level_t level)
{

  auto out = change_code::no_change;
  //genP and genQ are defined negative for producing power so we flip the signs here
  S.genP = -S.genP;
  if (!CHECK_CONTROLFLAG (flags, ignore_bus_limits))
    {

      switch (type)
        {

        case busType::SLK:
        case busType::afix:


          if (S.genP < busController.Pmin)
            {
              S.genP = busController.Pmin;
              if (attachedGens.size () == 1)
                {
                  attachedGens[0]->set ("p", S.genP);
                }
              else
                {
                  //TODO:: PT figure out what to do here
                  // for (auto &gen : attachedGens)
                  //  {
                  // gen->set ("p", gen->getGeneration);
                  //   }
                }
              type = busType::PQ;
              alert (this, JAC_COUNT_CHANGE);
              out = change_code::jacobian_change;
              if (prevType == busType::SLK)
                {
                  alert (this, SLACK_BUS_CHANGE);
                }
            }
          else if (S.genP > busController.Pmax)
            {
              S.genP = busController.Pmax;
              type = busType::PQ;
              if (attachedGens.size () == 1)
                {
                  attachedGens[0]->set ("p", S.genP);
                }
              else
                {
                  //TODO::PT figure out what to do in this case
                  //for (auto &gen : attachedGens)
                  //  {
                  // gen->set ("p", gen->Pmax);
                  //  }
                }
              alert (this, JAC_COUNT_CHANGE);
              out = change_code::jacobian_change;
              if (prevType == busType::SLK)
                {
                  alert (this, SLACK_BUS_CHANGE);
                }
            }
          break;
        default:
          break;
        }
    }
  auto args = getOutputs (nullptr, cLocalSolverMode);
  for (auto &gen : attachedGens)
    {
      if (gen->checkFlag (has_powerflow_adjustments))
        {
          auto iret = gen->powerFlowAdjust (args, flags,level);
          if (iret > out)
            {
              out = iret;
            }
        }
    }
  for (auto &ld : attachedLoads)
    {
      if (ld->checkFlag (has_powerflow_adjustments))
        {
          auto iret = ld->powerFlowAdjust (args, flags,level);
          if (iret > out)
            {
              out = iret;
            }
        }
    }
  //genP and genQ are defined negative for producing power so we flip the signs here
  S.genP = -S.genP;
  return out;

}
/*function to check the currect status for any limit violations*/
void dcBus::pFlowCheck (std::vector<violation> &Violation_vector)
{
  gridBus::pFlowCheck (Violation_vector);

}

// initializeB states for dynamic solution
void dcBus::dynObjectInitializeA (double time0, unsigned long flags)
{
  return gridBus::dynObjectInitializeA (time0, flags);

}

// initializeB states for dynamic solution part 2  //final clean up
void dcBus::dynObjectInitializeB (IOdata &outputSet)
{

  gridBus::dynInitializeB (outputSet);
  S.genQ = 0;
  angle = 0;

  busController.autogenQact = 0;

}

double dcBus::timestep (double ttime, const solverMode &sMode)
{
  gridBus::timestep (ttime, sMode);

  return 0.0;
}


// set properties
int dcBus::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;
  out = gridBus::set (param, val);
  return out;

}

int dcBus::set (const std::string &param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;

  if (param[0] == '#')
    {

    }
  else
    {
      out = gridBus::set (param, val, unitType);
    }
  return out;

}



// residual
void dcBus::residual (const stateData *sD, double resid[], const solverMode &sMode)
{

  auto Voffset = offsets.getVOffset (sMode);

  updateLocalCache (sD, sMode);

  // output

  if (Voffset != kNullLocation)
    {
      if (useVoltage (sMode))
        {
          resid[Voffset] = S.sumP ();
        }
      else
        {
          resid[Voffset] = sD->state[Voffset] - voltage;
        }
    }

  //printf("[%d] Bus %d V=%f theta=%f\n", seqID, id, v1,t1);
  auto args = getOutputs (sD, sMode);

  for (auto &gen : attachedGens)
    {
      if ((gen->stateSize (sMode) > 0) && (gen->enabled))
        {
          gen->residual (args, sD, resid, sMode);
        }
    }
  for (auto &load : attachedLoads)
    {
      if ((load->stateSize (sMode) > 0) && (load->enabled))
        {
          load->residual (args, sD, resid, sMode);
        }
    }

}
// jacobian
void dcBus::jacobianElements (const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{

  auto args = getOutputs (sD, sMode);

  //kinsolJacDense(state, J, ind, true);

  auto Voffset = offsets.getVOffset (sMode);
  // import bus values (current theta and voltage)

  computeDerivatives (sD, sMode);
  auto argLocs = getOutputLocs (sMode);
  //printf("t=%f,id=%d, dpdt=%f, dpdv=%f, dqdt=%f, dqdv=%f\n", ttime, id, Ptii, Pvii, Qvii, Qtii);
  if (Voffset != kNullLocation)
    {
      if (useVoltage (sMode))
        {
          ad->assign (Voffset, Voffset, partDeriv.at (PoutLocation, voltageInLocation));
        }
      else
        {
          ad->assign (Voffset, Voffset, 1);
        }
    }

  //arrayDataSparse od;
  od.setArray (ad);
  od.setTranslation (QoutLocation, useVoltage (sMode) ? argLocs[voltageInLocation] : kNullLocation);
  for (auto &gen : attachedGens)
    {
      if ((gen->jacSize (sMode) > 0) && (gen->enabled))
        {
          gen->jacobianElements (args, sD, ad, argLocs, sMode);
          gen->outputPartialDerivatives (args, sD, &od, sMode);
        }
    }
  for (auto &load : attachedLoads)
    {
      if ((load->jacSize (sMode) > 0) && (load->enabled))
        {
          load->jacobianElements (args, sD, ad, argLocs, sMode);
          load->outputPartialDerivatives (args, sD, &od, sMode);
        }

    }
  int gid = getID ();
  for (auto &link : attachedLinks)
    {
      link->outputPartialDerivatives (gid, sD, &od, sMode);
    }
  /*if (argLocs[voltageInLocation] != kNullLocation)
    {
      if (useVoltage (sMode))
        {
          ad->copyTranslateRow (&od, PoutLocation, argLocs[voltageInLocation]);
        }
    }
        */
}

double dcBus::getAngle (const stateData *, const solverMode &) const
{
  return 0;
}

//TODO:: PT write this function
void dcBus::converge (double, double [], double [], const solverMode &, converge_mode,double)
//void dcBus::converge (const double ttime, double state[], double dstate_dt[], const solverMode &sMode, double tol, int mode)
{

}

int dcBus::getMode (const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      if (isDifferentialOnly (sMode))
        {
          return 3;
        }
      else
        {
          return (static_cast<int> (dynType) | 1);
        }
    }
  else
    {
      return (static_cast<int> (type) | 1);
    }
}

bool dcBus::useVoltage (const solverMode &sMode)
{
  bool ret = true;
  if (isDifferentialOnly (sMode))
    {
      ret = false;
    }
  else if (isDynamic (sMode))
    {
      if (dynType == dynBusType::fixVoltage)
        {
          ret = false;
        }
    }
  else
    {
      if ((type == busType::PV) || (type == busType::SLK))
        {
          ret = false;
        }
    }

  return ret;
}

int dcBus::propogatePower ( bool makeSlack)
{
  int ret = 0;
  if (makeSlack)
    {
      prevType = type;
      type = busType::SLK;
    }

  updateLocalCache ();
  int unfixed = 0;
  gridLink *dc1 = nullptr;
  for (auto &lnk:attachedLinks)
    {
      if (!(lnk->checkFlag (gridLink::fixed_target_power)))
        {
          ++unfixed;
          dc1 = lnk;
        }
    }
  if (unfixed == 1)
    {
      ret = dc1->fixRealPower (dc1->getRealPower (getID ()) - (S.sumP ()),getID ());
    }
  return ret;
}
