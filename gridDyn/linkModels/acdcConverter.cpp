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

#include "acdcConverter.h"
#include "gridArea.h"
#include "utilities/vectorOps.hpp"
#include "primary/dcBus.h"
#include "submodels/otherBlocks.h"
#include "core/coreObjectTemplates.h"
#include "utilities/matrixDataSparse.h"
#include "core/coreExceptions.h"


#include <cmath>
#include <cstring>

static const double k3sq2 = (3.0 * sqrt (2.0) / kPI);
static const double k3sq2sq = k3sq2 * k3sq2;

using namespace gridUnits;
const std::string rect = "rectifier_$";
const std::string inv = "inverter_$";
const std::string bidir = "acdcConveter_$";

const std::string &modeToName (acdcConverter::mode_t mode,const std::string &name)
{
  if (!name.empty ())
    {
      return name;
    }
  switch (mode)
    {
    case acdcConverter::mode_t::rectifier:
      return rect;
    case acdcConverter::mode_t::inverter:
      return inv;
    case acdcConverter::mode_t::bidirectional:
    default:
      return bidir;
    }
}

acdcConverter::acdcConverter (double rP, double xP, const std::string &objName) : gridLink (objName),r (rP),x (xP)
{
  buildSubsystem ();
}

acdcConverter::acdcConverter (mode_t opType, const std::string &objName) : gridLink (modeToName (opType,objName)),type(opType)
{
  if (opType == mode_t::inverter)
    {
      dirMult = -1.0;
    }
  buildSubsystem ();
}
acdcConverter::acdcConverter (const std::string &objName) : gridLink (objName)
{

  buildSubsystem ();
}

void acdcConverter::buildSubsystem ()
{
  tap = kBigNum;
  opFlags.set (dc_capable);
  opFlags.set (adjustable_P);
  opFlags.set (adjustable_Q);
  firingAngleControl = std::make_shared<pidBlock> (-dirMult * mp_Kp, -dirMult * mp_Ki, 0, "angleControl");
  firingAngleControl->addOwningReference();
  addSubObject(firingAngleControl.get());
  powerLevelControl = std::make_shared<pidBlock> (mp_controlKp, mp_controlKi, 0, "powerControl");
  powerLevelControl->addOwningReference();
  addSubObject(powerLevelControl.get());
  controlDelay = std::make_shared<delayBlock> (tD, "controlDelay");
  controlDelay->addOwningReference();
  addSubObject(controlDelay.get());
}

acdcConverter::~acdcConverter ()
{
}

coreObject *acdcConverter::clone (coreObject *obj) const
{
  acdcConverter *nobj = cloneBase<acdcConverter, gridLink> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }
  nobj->Idcmax = Idcmax;
  nobj->Idcmin = Idcmin;
  nobj->mp_Ki = mp_Ki;
  nobj->mp_Kp = mp_Kp;
  nobj->mp_controlKi = mp_controlKi;
  nobj->mp_controlKp = mp_controlKp;
  nobj->tD = tD;
  nobj->control_mode = control_mode;
  nobj->vTarget = vTarget;
  nobj->type = type;
  nobj->dirMult = dirMult;
  nobj->tap = tap;
  nobj->angle = angle;
  nobj->maxAngle = maxAngle;
  nobj->minAngle = minAngle;
  firingAngleControl->clone (nobj->firingAngleControl.get ());
  powerLevelControl->clone (nobj->powerLevelControl.get ());
  controlDelay->clone (nobj->controlDelay.get ());
  return nobj;
}

void  acdcConverter::timestep (coreTime ttime, const IOdata & /*inputs*/, const solverMode &)
{
	//TODO: this function is incorrect
  if (!isEnabled())
    {
      return;

    }
  updateLocalCache ();

  /*if (scheduled)
  {
  Psched=sched->timestepP(time);
  }*/
  prevTime = ttime;
}

//it may make more sense to have the dc bus as bus 1 but then the equations wouldn't be symmetric with the the rectifier
void acdcConverter::updateBus (gridBus *bus, index_t /*busnumber*/)
{
  if (dynamic_cast<dcBus *> (bus))
    {
      gridLink::updateBus (bus, 2); //bus 2 must be the dc bus
    }
  else
    {
      gridLink::updateBus (bus, 1);
    }
}


double acdcConverter::getMaxTransfer () const
{
  if (!isConnected ())
    {
      return 0;
    }
  else
    {
      return linkInfo.v2 * (std::max)(std::abs (Idcmax),std::abs (Idcmin));
    }
}

// set properties
void  acdcConverter::set (const std::string &param,  const std::string &val)
{
  
  if (param == "mode")
    {
      if (val == "rectifier")
        {
          type = mode_t::rectifier;
          if (dirMult < 0)
            {
              firingAngleControl->set ("p",-mp_Kp);
              firingAngleControl->set ("i", -mp_Ki);
            }
          dirMult = 1.0;
        }
      else if (val == "inverter")
        {
          type = mode_t::inverter;
          if (dirMult > 0)
            {
              firingAngleControl->set ("p", mp_Kp);
              firingAngleControl->set ("i", mp_Ki);
            }
          dirMult = -1.0;
        }
      else if (val == "bidirectional")
        {
          type = mode_t::bidirectional;
          if (dirMult < 0)
            {
              firingAngleControl->set ("p", -mp_Kp);
              firingAngleControl->set ("i", -mp_Ki);
            }
          dirMult = 1.0;
        }
      else
        {
		  throw(invalidParameterValue());
        }
    }
  else
    {
      gridLink::set (param, val);
    }
 
}

void  acdcConverter::set (const std::string &param, double val, units_t unitType)
{

  if (param == "r")
    {
      r = val;
    }
  else if ((param == "l") || (param == "x"))
    {
      x = val;
    }
  else if ((param == "p") || (param == "pset"))
    {
      Pset = unitConversion (val, unitType, puMW, systemBasePower);
      Pset = (Pset < 0) ? dirMult * Pset : Pset;
      opFlags.set (fixed_target_power);
      control_mode = control_mode_t::power;
      if (opFlags[dyn_initialized])
        {
          tap = linkInfo.v2 * linkInfo.v1 / Pset;
        }
    }
  else if ((param == "tapi")||(param == "mi")||(param == "tap"))
    {
      tap = val;
      baseTap = val;
    }
  else if ((param=="angle")||(param == "tapangle")||(param=="alpha")||(param=="gamma"))
    {
      angle = val;
    }
  else if ((param == "idcmax") || (param == "imax"))
    {
      Idcmax = val;
      powerLevelControl->set ("omax",Idcmax);
    }
  else if ((param == "idcmin") || (param == "imin"))
    {
      Idcmin = val;
      powerLevelControl->set ("omax", Idcmin);
    }
  else if ((param == "gammamax") || (param == "alphamax") || (param == "anglemax") || (param == "maxangle"))
    {
      maxAngle = val;
      if (type == mode_t::inverter)
        {
          firingAngleControl->set ("max", cos (kPI - maxAngle));
        }
      else
        {
          firingAngleControl->set ("max", cos (maxAngle));
        }
    }
  else if ((param == "gammamin") || (param == "alphamin") || (param == "anglemin") || (param == "minangle"))
    {
      minAngle = val;
      if (type == mode_t::inverter)
        {
          firingAngleControl->set ("min", cos (kPI - minAngle));
        }
      else
        {
          firingAngleControl->set ("min", cos (minAngle));
        }
    }
  else if ((param == "ki") || (param == "igain") || (param == "i"))
    {
      mp_Ki = val;
      firingAngleControl->set ("i",dirMult * val);
    }
  else if ((param == "kp") || (param == "pgain"))
    {
      mp_Kp = val;
      firingAngleControl->set ("p", dirMult * val);
    }
  else if ((param == "t") || (param == "tp"))
    {
      mp_Ki = 1.0 / val;
      firingAngleControl->set ("i", val);
    }
  else if ((param == "controlki") || (param == "controli"))
    {
      mp_controlKi = val;
      powerLevelControl->set ("i", val);
    }
  else if ((param == "controlkp") || (param == "controlgain"))
    {
      mp_controlKp = val;
      powerLevelControl->set ("p", val);
    }
  else if ((param == "tm")||(param == "td"))
    {
      tD = val;
      controlDelay->set ("t", tD);
    }
  else
    {
      gridLink::set (param, val, unitType);
    }

}


void acdcConverter::pFlowObjectInitializeA (coreTime /*time0*/, unsigned long /*flags*/)
{

  double v1 = B1->getVoltage ();
  double v2 = B2->getVoltage ();
  if (opFlags[fixed_target_power])
    {
      Idc = Pset / v2;
    }
  else
    {
      Idc = v1 / tap;
    }
  angle = (v1 + 3 / kPI * x * Idc) / (k3sq2 * v1);
  updateLocalCache();
  offsets.local().local.algSize = 1;
  offsets.local().local.jacSize = 4;

}

void acdcConverter::dynObjectInitializeA (coreTime time0, unsigned long flags)
{
  updateLocalCache ();
  if (opFlags[fixed_target_power])
    {
      tap = linkInfo.v2 * linkInfo.v1 / Pset;
    }
  baseTap = tap;
  firingAngleControl->dynInitializeA (time0,flags);

  vTarget = B2->getVoltage ();
  powerLevelControl->dynInitializeA (time0,flags);
  controlDelay->dynInitializeA (time0,flags);
  if (tD < 0.0001) //check if control are needed
    {
      controlDelay->disable ();
    }
  if (control_mode != control_mode_t::voltage)
    {
      powerLevelControl->disable ();
      controlDelay->disable ();
    }
  offsets.local().local.algSize = 1;
  offsets.local().local.jacSize = 4;

}

static IOdata zVec {
  0.0,0.0,0.0
};


void acdcConverter::dynObjectInitializeB (const IOdata & /*inputs*/, const IOdata & /*desiredOutput*/, IOdata & fieldSet)
{

  firingAngleControl->dynInitializeB (noInputs,{angle},fieldSet);
  if (control_mode == control_mode_t::voltage)
    {
      vTarget = B2->getVoltage ();
	  powerLevelControl->dynInitializeB({ vTarget }, noInputs, fieldSet);
      if (tD > 0.0001)
        {
          controlDelay->dynInitializeB (fieldSet, noInputs, fieldSet);
        }

    }

}

void acdcConverter::loadSizes (const solverMode &sMode, bool dynOnly)
{
  gridObject::loadSizes (sMode,dynOnly);
}


void acdcConverter::ioPartialDerivatives (index_t busId, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{
  if  (!(isEnabled()))
    {
      return;
    }
  if (inputLocs[voltageInLocation] == kNullLocation)
    {
      return;
    }
  updateLocalCache (noInputs, sD, sMode);
  //int mode = B1->getMode(sMode) * 4 + B2->getMode(sMode);

  /*
  linkInfo.P1 = dirMult*linkInfo.v2 * Idc;
  linkInfo.P2 = -linkInfo.P1;
  double sr = k3sq2*linkInfo.v1*Idc;

  linkInfo.Q1 = -std::sqrt(sr*sr - linkInfo.P1*linkInfo.P1);
  */
  index_t vLoc = inputLocs[voltageInLocation];
  if (isDynamic (sMode))
    {
      if (busId == B2->getID ())
        {
          ad.assign (PoutLocation, vLoc, -dirMult * Idc);
        }
      else
        {
          ad.assign (QoutLocation, vLoc, -Idc * k3sq2sq * linkInfo.v1 / std::sqrt (k3sq2sq * linkInfo.v1 * linkInfo.v1 - linkInfo.v2 * linkInfo.v2));
        }
    }
  else
    {
      /*
      Idc = (opFlags[fixed_target_power]) ? Pset / linkInfo.v2 : linkInfo.v1 / tap;


      linkInfo.P1 = linkInfo.v2 * Idc;
      linkInfo.P2 = -linkInfo.P1;
      double sr = k3sq2*linkInfo.v1*Idc;

      linkInfo.Q1 = std::sqrt(sr*sr - linkInfo.P1*linkInfo.P1);
      */
      if (busId == B2->getID ())
        {
          if (!opFlags[fixed_target_power])
            {
              ad.assign (PoutLocation, vLoc, dirMult * linkInfo.v1 / tap);
            }
        }
      else
        {
          double temp = std::sqrt (k3sq2sq * linkInfo.v1 * linkInfo.v1 - linkInfo.v2 * linkInfo.v2);
          if (opFlags[fixed_target_power])
            {
              ad.assign (QoutLocation, vLoc, -k3sq2sq * Pset * linkInfo.v1 / (linkInfo.v2 * temp));
            }
          else
            {
              ad.assign (PoutLocation, vLoc, -dirMult * linkInfo.v2 / tap);
              ad.assign (QoutLocation, vLoc, -1.0 / tap * temp - linkInfo.v1 * linkInfo.v1 / (tap * temp) * k3sq2sq);
            }
        }
    }
}

void acdcConverter::outputPartialDerivatives (index_t busId, const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
{
  if (!(isEnabled()))
    {
      return;
    }
  updateLocalCache (noInputs, sD, sMode);



  //int mode = B1->getMode(sMode) * 4 + B2->getMode(sMode);
  auto B1Voffset = B1->getOutputLoc(sMode, voltageInLocation);
  auto B2Voffset = B2->getOutputLoc(sMode, voltageInLocation);

  auto algOffset = offsets.getAlgOffset (sMode);



  //	ad.assign(B1Voffset, B2Voffset, Q1V2);
  //	ad.assign(B2Voffset, B1Voffset, Q2V1);
  if (isDynamic (sMode))
    {
      /*
        linkInfo.P1 = dirMult*linkInfo.v2 * Idc;
        linkInfo.P2 = -linkInfo.P1;
        double sr = k3sq2*linkInfo.v1*Idc;

        linkInfo.Q1 = -std::sqrt(sr*sr - linkInfo.P1*linkInfo.P1);
        */
      if (busId == B2->getID ())
        {
          ad.assignCheckCol (PoutLocation, algOffset, -dirMult * linkInfo.v2);
        }
      else
        {
          ad.assignCheckCol (PoutLocation, B2Voffset, dirMult * Idc);
          ad.assign (PoutLocation, algOffset, dirMult * linkInfo.v2);
          ad.assignCheckCol (QoutLocation, B2Voffset, -1 * (-Idc * linkInfo.v2 / std::sqrt (k3sq2 * k3sq2 * linkInfo.v1 * linkInfo.v1 - linkInfo.v2 * linkInfo.v2)));
          ad.assign (QoutLocation, algOffset, linkFlows.Q1 / Idc);
        }
    }
  else
    {
      /*
      Idc = (opFlags[fixed_target_power]) ? Pset / linkInfo.v2 : linkInfo.v1 / tap;


      linkInfo.P1 = linkInfo.v2 * Idc;
      linkInfo.P2 = -linkInfo.P1;
      double sr = k3sq2*linkInfo.v1*Idc;

      linkInfo.Q1 = std::sqrt(sr*sr - linkInfo.P1*linkInfo.P1);
      */
      if (busId == B2->getID ())
        {
          if (!opFlags[fixed_target_power])
            {
              ad.assignCheckCol (PoutLocation, B1Voffset, dirMult * linkInfo.v2 / tap);
            }
        }
      else
        {
          if (B2Voffset != kNullLocation)
            {
              double temp = std::sqrt (k3sq2sq * linkInfo.v1 * linkInfo.v1 - linkInfo.v2 * linkInfo.v2);
              if (opFlags[fixed_target_power])
                {
                  ad.assignCheckCol (QoutLocation, B2Voffset, Pset / temp + Pset * temp / (linkInfo.v2 * linkInfo.v2));
                }
              else
                {
                  ad.assignCheckCol (PoutLocation, B2Voffset, -dirMult * linkInfo.v1 / tap);
                  ad.assignCheckCol (QoutLocation, B2Voffset, linkInfo.v1 / tap * linkInfo.v2 / temp);
                }
            }

        }
    }
}

count_t acdcConverter::outputDependencyCount(index_t /*num*/, const solverMode &sMode) const
{
	if (isDynamic(sMode))
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

void acdcConverter::jacobianElements (const IOdata & /*inputs*/, const stateData &sD, matrixData<double> &ad, const IOlocs & /*inputLocs*/, const solverMode &sMode)
{
	auto B1Loc=B1->getOutputLocs(sMode);
  auto B1Voffset = B1Loc[voltageInLocation];
  auto B2Loc = B2->getOutputLocs(sMode);
  auto B2Voffset = B2Loc[voltageInLocation];
  updateLocalCache(noInputs, sD, sMode);
  if (isDynamic (sMode))
    {
      auto Loc = offsets.getLocations (sD, sMode,this);
      auto refAlg = Loc.algOffset;
      IOlocs argL {
        B2Voffset
      };
      IOdata a1 {
        linkInfo.v2 - vTarget
      };

      index_t refLoc;
      matrixDataSparse<double> tad1,tad2;

      if  (refAlg != kNullLocation)
        {
          if (control_mode == control_mode_t::voltage)
            {
			  double Padj;
			  if (tD>0.0001)
			  {
				 Padj = controlDelay->getOutput(a1, sD, sMode);
				  refLoc = controlDelay->getOutputLoc(sMode);
			  }
			  else
			  {
				  Padj = powerLevelControl->getOutput(a1, sD, sMode);
				  refLoc = powerLevelControl->getOutputLoc(sMode);
			  }
             
              tap = baseTap / (1.0 + dirMult * baseTap * Padj);
              tad2.assign (0, refLoc, -dirMult * linkInfo.v1);
            }
          double I0 = linkInfo.v1 / tap;
          a1[0] = Loc.algStateLoc[0] - I0;
          double cA = firingAngleControl->getOutput (a1, sD, sMode);
		  refLoc = firingAngleControl->getOutputLoc(sMode);
          ad.assignCheckCol (refAlg, B1Voffset, k3sq2 * cA);
          ad.assignCheckCol (refAlg, B2Voffset, -1);
          ad.assign (refAlg, refAlg, -3 / kPI * x);
          ad.assign (refAlg, refLoc, k3sq2 * linkInfo.v1);

          tad2.assign (0, refAlg, 1);
        }
      //manage the input for the
      argL[0] = 0;
      firingAngleControl->jacobianElements (a1, sD, tad1, argL, sMode);

      tad2.assign (0, B1Voffset, -(1.0 / tap));
      tad1.cascade (tad2, 0);
      ad.merge (tad1);

      if (control_mode == control_mode_t::voltage)
        {
          a1[0] = linkInfo.v2 - vTarget;
          argL[0] = B2Voffset;
          powerLevelControl->jacobianElements (a1,sD,ad,argL,sMode);
          if (tD > 0.0001)
            {
              a1[0] = powerLevelControl->getOutput (a1, sD, sMode);
              argL[0] = powerLevelControl->getOutputLoc(sMode);
              controlDelay->jacobianElements (a1,sD,ad,argL,sMode);
            }


        }

    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);

      //resid[offset] = k3sq2*linkInfo.v1*sD.state[offset] - 3 / kPI*x*Idc - linkInfo.v2;

      ad.assign (offset, offset, k3sq2 * linkInfo.v1);
      if (opFlags[fixed_target_power])
        {
          ad.assignCheckCol (offset, B1Voffset, k3sq2 * sD.state[offset]);
          ad.assignCheckCol (offset, B2Voffset, 3.0 / kPI * x * Pset / (linkInfo.v2 * linkInfo.v2) - 1.0);
        }
      else
        {
          ad.assignCheckCol (offset, B1Voffset, k3sq2 * sD.state[offset] - 3 / kPI * x / tap);
          ad.assignCheckCol (offset, B2Voffset, -1);
        }
    }
}

void acdcConverter::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{

  updateLocalCache (inputs, sD, sMode);
  if (isDynamic (sMode))
    {

      auto Loc = offsets.getLocations (sD, resid, sMode, this);
      IOdata a {
        linkInfo.v2 - vTarget
      };
      if (control_mode == control_mode_t::voltage)
        {
          double Padj = (tD > 0.0001) ? controlDelay->getOutput (a, sD, sMode) : powerLevelControl->getOutput (a, sD, sMode);
          tap = baseTap / (1.0 + dirMult * baseTap * Padj);
        }
      double I0 = linkInfo.v1 / tap;
      a[0] = Loc.algStateLoc[0] - I0;
      auto cA = firingAngleControl->getOutput (a, sD, sMode);
      Loc.destLoc[0] = k3sq2 * linkInfo.v1 * cA - 3.0 / kPI * x * Loc.algStateLoc[0] - linkInfo.v2;

      firingAngleControl->residual (a,sD,resid,sMode);
      if (control_mode == control_mode_t::voltage)
        {
          a[0] = linkInfo.v2 - vTarget;
          powerLevelControl->residual (a,sD,resid,sMode);
          if (tD > 0.0001)
            {
             
              a[0] = powerLevelControl->getOutput (a, sD, sMode);
              controlDelay->residual (a,sD,resid,sMode);
              
            }

        }
      
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      Idc = (opFlags[fixed_target_power]) ? Pset / linkInfo.v2 : linkInfo.v1 / tap;

      resid[offset] = k3sq2 * linkInfo.v1 * sD.state[offset] - 3 / kPI * x * Idc - linkInfo.v2;
    }
}

void acdcConverter::setState (coreTime ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      Idc = state[offsets.getAlgOffset (sMode)];
      for (auto &sub : getSubObjects())
        {
          if (sub->isEnabled())
            {
              sub->setState (ttime, state, dstate_dt, sMode);
            }
        }
      angle = firingAngleControl->getOutput ();
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      angle = state[offset];
      if (opFlags[fixed_target_power])
        {
          Idc = Pset / B2->getVoltage (state, sMode);
        }
      else
        {
          Idc = B1->getVoltage (state, sMode) / tap;
        }
    }
  prevTime = ttime;
  updateLocalCache ();

}

void acdcConverter::guess (coreTime ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      state[offsets.getAlgOffset (sMode)] = Idc;
      for (auto &sub:getSubObjects())
        {
          if (sub->isEnabled())
            {
              sub->guess (ttime, state, dstate_dt, sMode);
            }
        }
    }
  else
    {
      state[offsets.getAlgOffset (sMode)] = angle;
    }

}


void  acdcConverter::updateLocalCache (const IOdata &, const stateData &sD, const solverMode &sMode)
{
  if (!sD.updateRequired(linkInfo.seqID))
    {
      return;
    }

  if (!isEnabled())
    {
      return;
    }
  std::memset (&linkInfo, 0, sizeof(linkI));
  linkInfo.seqID = sD.seqID;


  linkInfo.v1 = B1->getVoltage (sD, sMode);
  linkInfo.v2 = B2->getVoltage (sD, sMode);

  if (isDynamic (sMode))
    {
      auto Loc = offsets.getLocations  (sD, sMode, this);
      Idc = Loc.algStateLoc[0];
    }
  else
    {
      Idc = opFlags[fixed_target_power]? Pset / linkInfo.v2 : linkInfo.v1 / tap;
		
    }

  linkFlows.P1 = dirMult * linkInfo.v2 * Idc;
  linkFlows.P2 = -linkFlows.P1;
  double sr = k3sq2 * linkInfo.v1 * Idc;

  linkFlows.Q1 = -std::sqrt (sr * sr - linkFlows.P1 * linkFlows.P1);

  
  //Q2 is 0 since bus k is a DC bus.
	/*
  if (type == mode_t::inverter)
    {
      printf ("inv sid=%d P1=%f P2=%f Q1=%f\n", linkInfo.seqID, linkFlows.P1, linkFlows.P2, linkFlows.Q1);
    }
  else
    {
      printf ("rect sid=%d P1=%f P2=%f Q1=%f\n", linkInfo.seqID, linkFlows.P1, linkFlows.P2, linkFlows.Q1);
    }
*	 */
}

void  acdcConverter::updateLocalCache ()
{

  std::memset (&linkInfo, 0, sizeof(linkI));

  if (isEnabled())
    {
      linkInfo.v1 = B1->getVoltage ();
      linkInfo.v2 = B2->getVoltage ();
	  linkFlows.P1 = dirMult * linkInfo.v2 * Idc;
	  linkFlows.P2 = -linkFlows.P1;
      double sr = k3sq2 * linkInfo.v1 * Idc;

	  linkFlows.Q1 = -std::sqrt (sr * sr - linkFlows.P1 * linkFlows.P1);
      
    }
}

int acdcConverter::fixRealPower (double power, index_t /*measureTerminal*/, index_t fixedTerminal, gridUnits::units_t unitType)
{
  if (fixedTerminal != 1)
    {
      Pset = (power < 0) ? dirMult * power : power;
      Pset = unitConversion (Pset,unitType,puMW,systemBasePower);
      opFlags.set (fixed_target_power);
      Idc = Pset / B2->getVoltage ();
      updateLocalCache ();
      return 1;
    }
  return 0;
}

int acdcConverter::fixPower (double /*power*/, double /*qpower*/, index_t /*measureTerminal*/,index_t /*fixedTerminal*/, gridUnits::units_t /*unitType*/)
{
  return 0;
}


void acdcConverter::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  auto offset = offsets.getAlgOffset (sMode);

  std::string prefix2 = prefix + getName() + ':';

  if (isDynamic (sMode))
    {
      if (offset > 0)
        {
          stNames[offset] = prefix2 + "Idc";
        }
      firingAngleControl->getStateName (stNames,sMode,prefix2);
      if (powerLevelControl->isEnabled())
        {
          powerLevelControl->getStateName (stNames,sMode,prefix2);
          if (controlDelay->isEnabled())
            {
              controlDelay->getStateName (stNames,sMode,prefix2);
            }
        }
    }
  else
    {
      stNames[offset] = prefix2 + "cos(firing_angle)";
    }

}
