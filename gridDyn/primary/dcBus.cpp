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
#include "gridCoreTemplates.h"
#include "matrixDataCompact.h"
#include "core/gridDynExceptions.h"
#include "stringOps.h"

#include <iostream>

static typeFactory<dcBus> gbf ("bus", stringVec { "dc" "hvdc" });

using namespace gridUnits;

dcBus::dcBus (const std::string &objName) : gridBus (objName), busController(this)
{

}

coreObject *dcBus::clone (coreObject *obj) const
{
  dcBus *nobj = cloneBase<dcBus, gridBus> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }
  nobj->vTarget = vTarget;
  nobj->participation = participation;
  return nobj;
}


// destructor
dcBus::~dcBus ()
{

}


// add link
void dcBus::add (gridLink *lnk)
{
  if ((lnk->checkFlag (dc_only))||(lnk->checkFlag (dc_capable)))
    {
      return gridBus::add (lnk);
    }

  throw(invalidObjectException(this));
}


// initializeB states
void dcBus::pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  gridBus::pFlowObjectInitializeA (time0, flags);
}

void dcBus::pFlowObjectInitializeB ()
{
  gridBus::pFlowObjectInitializeB ();

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
          so->addRootAndJacobianSizes (ld->getOffsets (sMode));
        }
      else
        {
          so->addSizes (ld->getOffsets (sMode));
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
          so->addRootAndJacobianSizes (gen->getOffsets (sMode));
        }
      else
        {
          so->addSizes (gen->getOffsets (sMode));
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
  auto args = getOutputs (emptyStateData, cLocalSolverMode);
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
/*function to check the current status for any limit violations*/
void dcBus::pFlowCheck (std::vector<violation> &Violation_vector)
{
  gridBus::pFlowCheck (Violation_vector);

}

// initializeB states for dynamic solution
void dcBus::dynObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  return gridBus::dynObjectInitializeA (time0, flags);

}

// initializeB states for dynamic solution part 2  //final clean up
void dcBus::dynObjectInitializeB (IOdata &outputSet)
{

  gridBus::dynObjectInitializeB (outputSet);
  S.genQ = 0;
  angle = 0;


}

void dcBus::timestep (gridDyn_time ttime, const solverMode &sMode)
{
  gridBus::timestep (ttime, sMode);

}


// set properties
void dcBus::set (const std::string &param,  const std::string &vali)
{
  auto val = convertToLowerCase(vali);
  if ((param == "type") || (param == "bustype") || (param == "pflowtype"))
  {


      if ((val == "slk") || (val == "swing") || (val == "slack"))
      {
          type = busType::SLK;
          prevType = busType::SLK;
      }
      else if (val == "pv")
      {
          type = busType::PV;
          prevType = busType::PV;
      }
      else if (val == "pq")
      {
          type = busType::PQ;
          prevType = busType::PQ;
      }
      else if ((val == "dynslk") || (val == "inf") || (val == "infinite"))
      {
          type = busType::SLK;
          prevType = busType::SLK;
          dynType = dynBusType::dynSLK;
      }
      else if ((val == "fixedangle") || (val == "fixangle") || (val == "ref"))
      {
          dynType = dynBusType::fixAngle;
      }
      else if ((val == "fixedvoltage") || (val == "fixvoltage"))
      {
          dynType = dynBusType::fixVoltage;
      }
      else if (val == "afix")
      {
          type = busType::afix;
          prevType = busType::afix;
      }
      else if (val == "normal")
      {
          dynType = dynBusType::normal;
      }
      else
      {
		  throw(invalidParameterValue());
      }

  }
  else if (param == "dyntype")
  {
      if ((val == "dynslk") || (val == "inf") || (val == "slk"))
      {
          dynType = dynBusType::dynSLK;
          type = busType::SLK;
      }
      else if ((val == "fixedangle") || (val == "fixangle") || (val == "ref"))
      {
          dynType = dynBusType::fixAngle;
      }
      else if ((val == "fixedvoltage") || (val == "fixvoltage"))
      {
          dynType = dynBusType::fixVoltage;
      }
      else if ((val == "normal") || (val == "pq"))
      {
          dynType = dynBusType::normal;
      }
      else
      {
		  throw(invalidParameterValue());
      }
  }
  else
  {
      gridBus::set(param, val);
  }
  

}

void dcBus::set (const std::string &param, double val, units_t unitType)
{

  if (param[0] == '#')
    {

    }
  else
    {
      gridBus::set (param, val, unitType);
    }

}

void dcBus::getStateName(stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
    if (hasAlgebraic(sMode))
    {
        auto Voffset = offsets.getVOffset(sMode);
        

        count_t bst = 0;
        if (stNames.size() < Voffset + 1)
        {
            stNames.resize(Voffset + 1);
        }
        if (Voffset != kNullLocation)
        {
            stNames[Voffset] = name + ":voltage";
            ++bst;
        }
        
        if (stateSize(sMode) == bst)
        {
            return;
        }
    }
    gridBus::getStateName(stNames, sMode, prefix);
    
}

// pass the solution
void dcBus::setState(gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{

    auto Voffset = offsets.getVOffset(sMode);


    if (isDAE(sMode))
    {
        if (Voffset != kNullLocation)
        {
            voltage = state[Voffset];
            m_dstate_dt[voltageInLocation] = dstate_dt[Voffset];
        }
    }
    else if (hasAlgebraic(sMode))
    {
        if (Voffset != kNullLocation)
        {
            if (ttime > prevTime)
            {
                //m_dstate_dt[voltageInLocation] = (state[Voffset] - m_state[voltageInLocation]) / (ttime - lastSetTime);
            }
            voltage = state[Voffset];
        }
        
    }
    gridBus::setState(ttime, state, dstate_dt, sMode);

}

void dcBus::guess(gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode)
{

    auto Voffset = offsets.getVOffset(sMode);
    

    if (!opFlags[slave_bus])
    {
        if (Voffset != kNullLocation)
        {
            state[Voffset] = voltage;

            if (hasDifferential(sMode))
            {
                dstate_dt[Voffset] = 0.0;
            }
        }
        
    }
    gridBus::guess(ttime, state, dstate_dt, sMode);
    

}

// residual
void dcBus::residual (const stateData &sD, double resid[], const solverMode &sMode)
{
  gridBus::residual(sD, resid, sMode);
  auto Voffset = offsets.getVOffset (sMode);
  // output

  if (Voffset != kNullLocation)
    {
      if (useVoltage (sMode))
        {
          resid[Voffset] = S.sumP ();
        }
      else
        {
          resid[Voffset] = sD.state[Voffset] - voltage;
        }
    }
 
  //printf("[%d] Bus %d V=%f theta=%f\n", seqID, id, v1,t1);
 

}

static const IOlocs inLoc{
    0,1,2
};

void dcBus::computeDerivatives(const stateData &sD, const solverMode &sMode)
{
    matrixDataCompact<2, 3> partDeriv;
    if (!isConnected())
    {
        return;
    }
    partDeriv.clear();

    for (auto &link : attachedLinks)
    {
        if (link->enabled)
        {
            link->updateLocalCache(sD, sMode);
            link->ioPartialDerivatives(getID(), sD, partDeriv, inLoc, sMode);
        }
    }
    if (!isExtended(sMode))
    {
        for (auto &gen : attachedGens)
        {
            if (gen->isConnected())
            {
				gen->updateLocalCache(outputs, sD, sMode);
                gen->ioPartialDerivatives(outputs, sD, partDeriv, inLoc, sMode);
            }
        }
        for (auto &load : attachedLoads)
        {
            if (load->isConnected())
            {
				load->updateLocalCache(outputs, sD, sMode);
                load->ioPartialDerivatives(outputs, sD, partDeriv, inLoc, sMode);
            }
        }
    }
    dVdP = partDeriv.at(PoutLocation, voltageInLocation);

}
// Jacobian
void dcBus::jacobianElements (const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
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
          ad.assign (Voffset, Voffset, dVdP);
          argLocs[voltageInLocation] = Voffset;
        }
      else
        {
          ad.assign (Voffset, Voffset, 1.0);
          argLocs[voltageInLocation] = kNullLocation;
        }
    }

  //matrixDataSparse od;
  od.setArray (ad);
  od.setTranslation (PoutLocation, useVoltage (sMode) ? argLocs[voltageInLocation] : kNullLocation);
  for (auto &gen : attachedGens)
    {
      if ((gen->jacSize (sMode) > 0) && (gen->enabled))
        {
          gen->jacobianElements (args, sD, ad, argLocs, sMode);
          gen->outputPartialDerivatives (args, sD, od, sMode);
        }
    }
  for (auto &load : attachedLoads)
    {
      if ((load->jacSize (sMode) > 0) && (load->enabled))
        {
          load->jacobianElements (args, sD, ad, argLocs, sMode);
          load->outputPartialDerivatives (args, sD, od, sMode);
        }

    }
  int gid = getID ();
  for (auto &link : attachedLinks)
    {
      link->outputPartialDerivatives (gid, sD, od, sMode);
    }
  /*if (argLocs[voltageInLocation] != kNullLocation)
    {
      if (useVoltage (sMode))
        {
          ad.copyTranslateRow (&od, PoutLocation, argLocs[voltageInLocation]);
        }
    }
        */
}


IOlocs dcBus::getOutputLocs(const solverMode &sMode) const
{
	return{ useVoltage(sMode) ? offsets.getVOffset(sMode) : kNullLocation,kNullLocation,kNullLocation };
}

index_t dcBus::getOutputLoc(const solverMode &sMode, index_t num) const
{
	if (num==voltageInLocation)
	{
		return useVoltage(sMode) ? offsets.getVOffset(sMode) : kNullLocation;
	}
	return kNullLocation;
}

//TODO:: PT write this function
void dcBus::converge (gridDyn_time, double [], double [], const solverMode &, converge_mode,double)
//void dcBus::converge (const gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode, double tol, int mode)
{

}

int dcBus::getMode (const solverMode &sMode) const
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


double dcBus::getVoltage(const double state[], const solverMode &sMode) const
{
	if (isLocal(sMode))
	{
		return voltage;
	}
	if (useVoltage(sMode))
	{
		auto Voffset = offsets.getVOffset(sMode);
		return (Voffset != kNullLocation) ? state[Voffset] : voltage;
	}
	return voltage;
}

double dcBus::getVoltage(const stateData &sD, const solverMode &sMode) const
{
	if (isLocal(sMode))
	{
		return voltage;
	}
	if (useVoltage(sMode))
	{
		auto Voffset = offsets.getVOffset(sMode);
		return (Voffset != kNullLocation) ? sD.state[Voffset] : voltage;
	}
	return voltage;
	
}

bool dcBus::useVoltage (const solverMode &sMode) const
{
  bool ret = true;
  if (isDifferentialOnly (sMode))
    {
      ret = false;
    }
  else if (isDynamic (sMode))
    {
      if ((dynType == dynBusType::fixVoltage)||(dynType==dynBusType::dynSLK))
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

  computePowerAdjustments();
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

void dcBus::computePowerAdjustments()
{
	//declaring an embedded function
	auto cid = getID();

	S.reset();

	for (auto &link : attachedLinks)
	{
		if ((link->isConnected()) && (!busController.hasAdjustments(link->getID())))
		{
			S.linkP += link->getRealPower(cid);
		}

	}
	for (auto &load : attachedLoads)
	{
		if ((load->isConnected()) && (!busController.hasAdjustments(load->getID())))
		{
			S.loadP += load->getRealPower(voltage);
		}
	}
	for (auto &gen : attachedGens)
	{
		if ((gen->isConnected()) && (!busController.hasAdjustments(gen->getID())))
		{
			S.genP += gen->getRealPower();
		}
	}
}