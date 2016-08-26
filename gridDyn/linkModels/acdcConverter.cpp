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

#include "acdcConverter.h"
#include "gridArea.h"
#include "vectorOps.hpp"
#include "primary/dcBus.h"
#include "submodels/otherBlocks.h"
#include "gridCoreTemplates.h"
#include "arrayDataSparse.h"


#include <cmath>
#include <cstring>

static const double k3sq2 = (3 * sqrt (2) / kPI);
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
  firingAngleControl->setParent (this);
  powerLevelControl = std::make_shared<pidBlock> (mp_controlKp, mp_controlKi, 0, "powerControl");
  powerLevelControl->setParent (this);
  controlDelay = std::make_shared<delayBlock> (tD, "controlDelay");
  controlDelay->setParent (this);
  subObjectList.push_back (firingAngleControl.get ());
  subObjectList.push_back (powerLevelControl.get ());
  subObjectList.push_back (controlDelay.get ());
}

acdcConverter::~acdcConverter ()
{
}

gridCoreObject *acdcConverter::clone (gridCoreObject *obj) const
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
  nobj->tapAngle = tapAngle;
  nobj->maxAngle = maxAngle;
  nobj->minAngle = minAngle;
  firingAngleControl->clone (nobj->firingAngleControl.get ());
  powerLevelControl->clone (nobj->powerLevelControl.get ());
  controlDelay->clone (nobj->controlDelay.get ());
  return nobj;
}

double  acdcConverter::timestep (double ttime, const solverMode &)
{

  if (!enabled)
    {
      return 0;

    }
  updateLocalCache ();

  /*if (scheduled)
  {
  Psched=sched->timestepP(time);
  }*/
  prevTime = ttime;
  return linkInfo.P1;
}

//it may make more sense to have the dc bus as bus 1 but then the equations wouldn't be symmetric with the the rectifier
int acdcConverter::updateBus (gridBus *bus, index_t /*busnumber*/)
{
  if (dynamic_cast<dcBus *> (bus))
    {
      return gridLink::updateBus (bus, 2); //bus 2 must be the dc bus
    }
  else
    {
      return gridLink::updateBus (bus, 1);
    }
}

void acdcConverter::followNetwork (int /*network*/, std::queue<gridBus *> & /*bstk*/)
{
  //network disconnect for acdc converters
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
int  acdcConverter::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;

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
          out = INVALID_PARAMETER_VALUE;
        }
    }
  else
    {
      out = gridLink::set (param, val);
    }
  return out;
}

int  acdcConverter::set (const std::string &param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;

  if (param == "r")
    {
      r = val;
    }
  else if ((param == "l") || (param == "x"))
    {
      x = val;
      // set line admittance
    }
  else if ((param == "p") || (param == "pset"))
    {
      Pset = unitConversion (val, unitType, puMW, systemBasePower);
      Pset = (Pset < 0) ? dirMult * Pset : Pset;
      opFlags.set (fixed_target_power);
      control_mode = control_mode_t::power;
      if (opFlags.test (dyn_initialized))
        {
          tap = linkInfo.v2 * linkInfo.v1 / Pset;
        }
    }
  else if ((param == "tapi")||(param == "mi")||(param == "tap"))
    {
      tap = val;
      baseTap = val;
    }
  else if (param == "tapangle")
    {
      tapAngle = val;
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
      out = gridLink::set (param, val, unitType);
    }
  return out;
}


void acdcConverter::pFlowObjectInitializeA (double /*time0*/, unsigned long /*flags*/)
{

  double v1 = B1->getVoltage ();
  double v2 = B2->getVoltage ();
  if (opFlags.test (fixed_target_power))
    {
      Idc = Pset / v2;
    }
  else
    {
      Idc = v1 / tap;
    }
  tapAngle = (v1 + 3 / kPI * x * Idc) / (k3sq2 * v1);
  offsets.local->local.algSize = 1;
  offsets.local->local.jacSize = 4;

}

void acdcConverter::dynObjectInitializeA (double time0, unsigned long flags)
{
  updateLocalCache ();
  if (opFlags.test (fixed_target_power))
    {
      tap = linkInfo.v2 * linkInfo.v1 / Pset;
    }
  baseTap = tap;
  firingAngleControl->initializeA (time0,flags);

  vTarget = B2->getVoltage ();
  powerLevelControl->initializeA (time0,flags);
  controlDelay->initializeA (time0,flags);
  if (tD < 0.0001) //check if control are needed
    {
      controlDelay->disable ();
    }
  if (control_mode != control_mode_t::voltage)
    {
      powerLevelControl->disable ();
      controlDelay->disable ();
    }
  offsets.local->local.algSize = 1;
  offsets.local->local.jacSize = 4;

}

static IOdata zVec {
  0.0,0.0,0.0
};

static const IOdata nvec {};

void acdcConverter::dynObjectInitializeB (IOdata & /*fieldSet*/)
{
  IOdata out (1);
  firingAngleControl->initializeB (zVec,{tapAngle},out);
  if (control_mode == control_mode_t::voltage)
    {
      vTarget = B2->getVoltage ();
      powerLevelControl->initializeB (zVec, nvec, out);
      if (tD > 0.0001)
        {
          controlDelay->initializeB (out, nvec, out);
        }

    }

}

void acdcConverter::loadSizes (const solverMode &sMode, bool dynOnly)
{
  gridObject::loadSizes (sMode,dynOnly);
}


void acdcConverter::ioPartialDerivatives (index_t busId, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
  if  (!(enabled))
    {
      return;
    }
  if (argLocs[voltageInLocation] == kNullLocation)
    {
      return;
    }
  updateLocalCache (sD, sMode);
  //int mode = B1->getMode(sMode) * 4 + B2->getMode(sMode);

  /*
  linkInfo.P1 = dirMult*linkInfo.v2 * Idc;
  linkInfo.P2 = -linkInfo.P1;
  double sr = k3sq2*linkInfo.v1*Idc;

  linkInfo.Q1 = -std::sqrt(sr*sr - linkInfo.P1*linkInfo.P1);
  */
  index_t vLoc = argLocs[voltageInLocation];
  if (isDynamic (sMode))
    {
      if (busId == B2->getID ())
        {
          ad->assign (PoutLocation, vLoc, -dirMult * Idc);
        }
      else
        {
          ad->assign (QoutLocation, vLoc, -Idc * k3sq2sq * linkInfo.v1 / std::sqrt (k3sq2sq * linkInfo.v1 * linkInfo.v1 - linkInfo.v2 * linkInfo.v2));
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
              ad->assign (PoutLocation, vLoc, dirMult * linkInfo.v1 / tap);
            }
        }
      else
        {
          double temp = std::sqrt (k3sq2sq * linkInfo.v1 * linkInfo.v1 - linkInfo.v2 * linkInfo.v2);
          if (opFlags[fixed_target_power])
            {
              ad->assign (QoutLocation, vLoc, -k3sq2sq * Pset * linkInfo.v1 / (linkInfo.v2 * temp));
            }
          else
            {
              ad->assign (PoutLocation, vLoc, -dirMult * linkInfo.v2 / tap);
              ad->assign (QoutLocation, vLoc, -1 / tap * temp - linkInfo.v1 * linkInfo.v1 / (tap * temp) * k3sq2sq);
            }
        }
    }
}

void acdcConverter::outputPartialDerivatives (index_t busId, const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{
  if (!(enabled))
    {
      return;
    }
  updateLocalCache (sD, sMode);



  //int mode = B1->getMode(sMode) * 4 + B2->getMode(sMode);
  auto B1Voffset = B1->offsets.getVOffset (sMode);
  auto B2Voffset = B2->offsets.getVOffset (sMode);

  auto algOffset = offsets.getAlgOffset (sMode);



  //	ad->assign(B1Voffset, B2Voffset, Q1V2);
  //	ad->assign(B2Voffset, B1Voffset, Q2V1);
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
          ad->assignCheckCol (PoutLocation, algOffset, -dirMult * linkInfo.v2);
        }
      else
        {
          ad->assignCheckCol (PoutLocation, B2Voffset, dirMult * Idc);
          ad->assign (PoutLocation, algOffset, dirMult * linkInfo.v2);
          ad->assignCheckCol (QoutLocation, B2Voffset, -1 * (-Idc * linkInfo.v2 / std::sqrt (k3sq2 * k3sq2 * linkInfo.v1 * linkInfo.v1 - linkInfo.v2 * linkInfo.v2)));
          ad->assign (QoutLocation, algOffset, linkInfo.Q1 / Idc);
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
          if (!opFlags.test (fixed_target_power))
            {
              ad->assignCheckCol (PoutLocation, B1Voffset, dirMult * linkInfo.v2 / tap);
            }
        }
      else
        {
          if (B2Voffset != kNullLocation)
            {
              double temp = std::sqrt (k3sq2sq * linkInfo.v1 * linkInfo.v1 - linkInfo.v2 * linkInfo.v2);
              if (opFlags[fixed_target_power])
                {
                  ad->assignCheckCol (QoutLocation, B2Voffset, Pset / temp + Pset * temp / (linkInfo.v2 * linkInfo.v2));
                }
              else
                {
                  ad->assignCheckCol (PoutLocation, B2Voffset, -dirMult * linkInfo.v1 / tap);
                  ad->assignCheckCol (QoutLocation, B2Voffset, linkInfo.v1 / tap * linkInfo.v2 / temp);
                }
            }

        }
    }
}


void acdcConverter::jacobianElements (const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{
  auto B1Voffset = B1->offsets.getVOffset (sMode);
  auto B2Voffset = B2->offsets.getVOffset (sMode);
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
      double I0;
      index_t refLoc;
      arrayDataSparse tad1,tad2;
      double Padj = 0;
      if  (refAlg != kNullLocation)
        {
          if (control_mode == control_mode_t::voltage)
            {
              Padj = (tD > 0.0001) ? controlDelay->getOutputLoc (a1, sD, sMode, refLoc) : powerLevelControl->getOutputLoc (a1, sD, sMode, refLoc);
              tap = baseTap / (1.0 + dirMult * baseTap * Padj);
              tad2.assign (0, refLoc, -dirMult * linkInfo.v1);
            }
          I0 = linkInfo.v1 / tap;
          a1[0] = Loc.algStateLoc[0] - I0;
          double cA = firingAngleControl->getOutputLoc (a1, sD, sMode, refLoc);

          ad->assignCheckCol (refAlg, B1Voffset, k3sq2 * cA);
          ad->assignCheckCol (refAlg, B2Voffset, -1);
          ad->assign (refAlg, refAlg, -3 / kPI * x);
          ad->assign (refAlg, refLoc, k3sq2 * linkInfo.v1);

          tad2.assign (0, refAlg, 1);
        }
      //manage the input for the
      argL[0] = 0;
      firingAngleControl->jacobianElements (a1, sD, &tad1, argL, sMode);

      tad2.assign (0, B1Voffset, -(1.0 / tap));
      tad1.cascade (&tad2, 0);
      ad->merge (&tad1);

      if (control_mode == control_mode_t::voltage)
        {
          a1[0] = linkInfo.v2 - vTarget;
          argL[0] = B2Voffset;
          powerLevelControl->jacobianElements (a1,sD,ad,argL,sMode);
          if (tD > 0.0001)
            {
              a1[0] = powerLevelControl->getOutputLoc (a1, sD, sMode, refLoc);
              argL[0] = refLoc;
              controlDelay->jacobianElements (a1,sD,ad,argL,sMode);
            }


        }

    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);

      //resid[offset] = k3sq2*linkInfo.v1*sD->state[offset] - 3 / kPI*x*Idc - linkInfo.v2;
      ad->assign (offset, offset, k3sq2 * linkInfo.v1);
      if (opFlags[fixed_target_power])
        {
          ad->assignCheckCol (offset, B1Voffset, k3sq2 * sD->state[offset]);
          ad->assignCheckCol (offset, B2Voffset, 3 / kPI * x * Pset / (linkInfo.v2 * linkInfo.v2) - 1);
        }
      else
        {
          ad->assignCheckCol (offset, B1Voffset, k3sq2 * sD->state[offset] - 3 / kPI * x / tap);
          ad->assignCheckCol (offset, B2Voffset, -1);
        }
    }
}

void acdcConverter::residual (const stateData *sD, double resid[], const solverMode &sMode)
{

  updateLocalCache (sD, sMode);
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
              printf ("vc t=%f v=%f vdiff=%f ",sD->time,linkInfo.v2, a[0]);
              a[0] = powerLevelControl->getOutput (a, sD, sMode);
              controlDelay->residual (a,sD,resid,sMode);
              printf ("alevel=%f\n",a[0]);
            }

        }
      else
        {
          printf ("novc t=%f v=%f vdiff=%f ", sD->time, linkInfo.v2, a[0]);
        }
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      Idc = (opFlags.test (fixed_target_power)) ? Pset / linkInfo.v2 : linkInfo.v1 / tap;

      resid[offset] = k3sq2 * linkInfo.v1 * sD->state[offset] - 3 / kPI * x * Idc - linkInfo.v2;
    }
}

void acdcConverter::setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      Idc = state[offsets.getAlgOffset (sMode)];
      for (auto &sub : subObjectList)
        {
          if (sub->enabled)
            {
              sub->setState (ttime, state, dstate_dt, sMode);
            }
        }
      tapAngle = firingAngleControl->getOutput ();
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      tapAngle = state[offset];
      if (opFlags.test (fixed_target_power))
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

void acdcConverter::guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      state[offsets.getAlgOffset (sMode)] = Idc;
      for (auto &sub:subObjectList)
        {
          if (sub->enabled)
            {
              sub->guess (ttime, state, dstate_dt, sMode);
            }
        }
    }
  else
    {
      state[offsets.getAlgOffset (sMode)] = tapAngle;
    }

}


void  acdcConverter::updateLocalCache (const stateData *sD, const solverMode &sMode)
{
  if ((linkInfo.seqID == sD->seqID) && (sD->seqID != 0))
    {
      return;
    }

  if (!enabled)
    {
      return;
    }
  std::memset (&linkInfo, 0, sizeof(convLinkInfo));
  linkInfo.seqID = sD->seqID;

  linkInfo.v1 = B1->getVoltage (sD->state, sMode);
  linkInfo.v2 = B2->getVoltage (sD->state, sMode);


  if (isDynamic (sMode))
    {
      auto Loc = offsets.getLocations  (sD, sMode, this);
      Idc = Loc.algStateLoc[0];
    }
  else
    {
      Idc = (opFlags.test (fixed_target_power)) ? Pset / linkInfo.v2 : linkInfo.v1 / tap;
    }

  linkInfo.P1 = dirMult * linkInfo.v2 * Idc;
  linkInfo.P2 = -linkInfo.P1;
  double sr = k3sq2 * linkInfo.v1 * Idc;

  linkInfo.Q1 = -std::sqrt (sr * sr - linkInfo.P1 * linkInfo.P1);
  //Q2 is 0 since bus k is a DC bus.
  if (type == mode_t::inverter)
    {
      printf ("inv sid=%d P1=%f P2=%f\n", linkInfo.seqID, linkInfo.P1, linkInfo.P2);
    }
  else
    {
      printf ("rect sid=%d P1=%f P2=%f\n", linkInfo.seqID, linkInfo.P1, linkInfo.P2);
    }
}

void  acdcConverter::updateLocalCache ()
{

  std::memset (&linkInfo, 0, sizeof(convLinkInfo));

  if (enabled)
    {
      linkInfo.v1 = B1->getVoltage ();
      linkInfo.v2 = B2->getVoltage ();
      linkInfo.P1 = dirMult * linkInfo.v2 * Idc;
      linkInfo.P2 = -linkInfo.P1;
      double sr = k3sq2 * linkInfo.v1 * Idc;

      linkInfo.Q1 = -std::sqrt (sr * sr - linkInfo.P1 * linkInfo.P1);
      if (type == mode_t::inverter)
        {
          printf ("inv t=%f P1=%f P2=%f\n", prevTime, linkInfo.P1, linkInfo.P2);
        }
      else
        {
          printf ("rect t=%f P1=%f P2=%f\n", prevTime, linkInfo.P1, linkInfo.P2);
        }
    }
}

int acdcConverter::fixRealPower (double power, index_t /*mterminal*/, index_t fixedTerminal, gridUnits::units_t unitType)
{
  if (fixedTerminal == 2)
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

int acdcConverter::fixPower (double /*power*/, double /*qpower*/, index_t /*mterminal*/,index_t /*fixedTerminal*/, gridUnits::units_t /*unitType*/)
{
  return 0;
}


void acdcConverter::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  auto offset = offsets.getAlgOffset (sMode);

  std::string prefix2 = prefix + name + ':';

  if (isDynamic (sMode))
    {
      if (offset > 0)
        {
          stNames[offset] = prefix2 + "Idc";
        }
      firingAngleControl->getStateName (stNames,sMode,prefix2);
      if (powerLevelControl->enabled)
        {
          powerLevelControl->getStateName (stNames,sMode,prefix2);
          if (controlDelay->enabled)
            {
              controlDelay->getStateName (stNames,sMode,prefix2);
            }
        }
    }
  else
    {
      stNames[offset] = prefix2 + "firing_angle";
    }

}
