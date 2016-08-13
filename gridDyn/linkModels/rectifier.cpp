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

#include "rectifier.h"
#include "gridArea.h"
#include "vectorOps.hpp"
#include "dcBus.h"


#include <cmath>

static const double k3sq2 = (3 * sqrt (2) / kPI);
static const double k3sq2sq = k3sq2 * k3sq2;

using namespace gridUnits;
rectifier::rectifier (double rP, double xP)
{
  r = rP;
  x = xP;
  m_name = "rectifier_" + std::to_string (id);
  opFlags.set (dc_capable);
  opFlags.set (adjustable_P);
  opFlags.set (adjustable_Q);
  tap = kBigNum;
}

rectifier::rectifier ()
{
  m_name = "rectifier_" + std::to_string (id);
  opFlags.set (dc_capable);
  opFlags.set (adjustable_P);
  opFlags.set (adjustable_Q);
  tap = kBigNum;
}

rectifier::~rectifier ()
{
}

gridCoreObject *rectifier::clone (gridCoreObject *obj, bool fullCopy)
{
  rectifier *nobj;
  if (obj == nullptr)
    {
      nobj = new rectifier ();
    }
  else
    {
      nobj = dynamic_cast<rectifier *> (obj);
      if (nobj == nullptr)
        {
          //if we can't cast the pointer clone at the next lower level
          gridLink::clone (obj, fullCopy);
          return obj;
        }
    }
  gridLink::clone (nobj, fullCopy);
  nobj->Idcmax = Idcmax;
  nobj->Idcmin = Idcmin;
  nobj->Pset = Pset;
  nobj->alphaMax = alphaMax;
  nobj->alphaMin = alphaMin;
  nobj->mp_Ki = mp_Ki;
  nobj->mp_Kp = mp_Kp;

  return nobj;
}

double  rectifier::timestep (double ttime, const solverMode &sMode)
{

  if (!enabled)
    {
      return 0;

    }
  computePower ();

  /*if (scheduled)
  {
  Psched=sched->timestepP(time);
  }*/
  return LinkInfo.Pik;
}


int rectifier::updateBus (gridBus *bus, int busnumber)
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

void rectifier::followNetwork (int network)
{
  //network disconnect at this point cross dc to ac path so no network transfer
}

double rectifier::getMaxTransfer ()
{
  if (!isConnected ())
    {
      return 0;
    }
  if (Erating > 0)
    {
      return Erating;
    }
  else if (ratingB > 0)
    {
      return ratingB;
    }
  else if (ratingA > 0)
    {
      return ratingA;
    }
  else
    {
      return kBigNum;
    }
}

// set properties
int  rectifier::set (std::string param, std::string val)
{

  return gridLink::set (param, val);
}

int  rectifier::set (std::string param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;

  makeLowerCase (param);
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
      Pset = unitConversion (val, unitType, puMW, m_basePower);
      opFlags.set (fixed_target_power);
      if (dyn_initialized)
        {
          tap = LinkInfo.v2 * LinkInfo.v1 / Pset;
        }
    }
  else if ((param == "tapr") || (param == "mr"))
    {
      tap = val;
    }
  else if ((param == "idcmax")||(param == "imax"))
    {
      Idcmax = val;
    }
  else if ((param == "idcmin") || (param == "imin"))
    {
      Idcmin = val;
    }
  else if ((param == "alphamax")||(param == "anglemax"))
    {
      alphaMax = val;
    }
  else if ((param == "alphamin") || (param == "anglemin"))
    {
      alphaMin = val;
    }
  else if ((param == "ki") || (param == "igain")||(param == "i"))
    {
      mp_Ki = val;
    }
  else if ((param == "kp") || (param == "pgain"))
    {
      mp_Kp = val;
    }
  else if ((param == "t") || (param == "tp"))
    {
      mp_Ki = 1.0 / val;
    }
  else
    {
      out = gridLink::set (param, val, unitType);
    }
  return out;
}


void rectifier::pFlowInitializeA (double time0, double abstime0, unsigned long flags)
{
  m_state.resize (3);

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
  m_state[0] = (v1 + 3 / kPI * x * Idc) / (k3sq2 * v1);
  m_state[1] = Idc;
}

void rectifier::dynInitializeA (double time0, double abstime0, unsigned long flags)
{
  m_dstate_dt.resize (3);
  computePower ();

  m_state[1] = Idc;
  m_dstate_dt[2] = 0;
  m_state[2] = m_state[0];

  if (opFlags.test (fixed_target_power))
    {
      tap = LinkInfo.v2 * LinkInfo.v1 / Pset;
    }

}


void rectifier::loadSizes (const solverMode &sMode, bool dynOnly)
{
  auto so = offsets.getOffsets (sMode);
  if (isDynamic (sMode))
    {
      so->diffSize = 1;
      so->algSize = 2;
      so->jacSize = 6;
    }
  else
    {
      so->algSize = 1;
      so->jacSize = 3;
    }
  so->rjLoaded = true;
  so->stateLoaded = true;
}


void rectifier::ioPartialDerivatives (int busId, const stateData *sD, arrayData *ad, const std::vector<int> &argLocs, const solverMode &sMode)
{
  if (!(enabled))
    {
      return;
    }
  computePower (sD, sMode);

  double P1V2 = 0.0, P2V1 = 0.0;


  //int mode = B1->getMode(sMode) * 4 + B2->getMode(sMode);

  int algOffset = offsets.getAlgOffset (sMode);

  //LinkInfo.Pik = LinkInfo.v2 * Idc;
  // LinkInfo.Pki = -LinkInfo.Pik;
  //double sr = k3sq2*LinkInfo.v1*Idc;

  //LinkInfo.Qik = std::sqrt(sr*sr - LinkInfo.Pik*LinkInfo.Pik);

  //	ad->assign(B1Voffset, B2Voffset, Q1V2);
  //	ad->assign(B2Voffset, B1Voffset, Q2V1);
  if (isDynamic (sMode))
    {
      if (busId == B2->getID ())
        {
          ad->assignCheck (PoutLocation, argLocs[voltageInLocation], -Idc);
        }
      else
        {
          ad->assignCheck (QoutLocation, argLocs[voltageInLocation], Idc * k3sq2sq * LinkInfo.v1 / std::sqrt (k3sq2sq * LinkInfo.v1 * LinkInfo.v1 - LinkInfo.v2 * LinkInfo.v2));
        }
    }
  else
    {
      /*
      Idc = (opFlags.test(fixed_target_power)) ? Pset / LinkInfo.v2 : LinkInfo.v1 / tap;


      LinkInfo.Pik = LinkInfo.v2 * Idc;
      LinkInfo.Pki = -LinkInfo.Pik;
      double sr = k3sq2*LinkInfo.v1*Idc;

      LinkInfo.Qik = std::sqrt(sr*sr - LinkInfo.Pik*LinkInfo.Pik);
      */
      if (busId == B2->getID ())
        {
          if (!opFlags.test (fixed_target_power))
            {
              ad->assignCheck (PoutLocation, B1Voffset, -LinkInfo.v2 / tap);
            }
        }
      else
        {
          double temp = std::sqrt (k3sq2sq * LinkInfo.v1 * LinkInfo.v1 - LinkInfo.v2 * LinkInfo.v2);
          if (opFlags.test (fixed_target_power))
            {
              ad->assignCheck (QoutLocation, B2Voffset, -Pset / temp - Pset * temp / (LinkInfo.v2 * LinkInfo.v2));
            }
          else
            {
              ad->assignCheck (PoutLocation, B2Voffset, LinkInfo.v1 / tap);
              ad->assignCheck (QoutLocation, B2Voffset, -LinkInfo.v1 / tap * LinkInfo.v2 / temp);
            }
        }
    }
}


void rectifier::jacobianElements (const stateData *sD, arrayData *ad, const solverMode &sMode)
{
  int B1Voffset = B1->offsets.getVOffset (sMode);
  int B2Voffset = B2->offsets.getVOffset (sMode);
  if (isDynamic (sMode))
    {
      auto Loc = offsets.getLocations (sD, nullptr, sMode, this);
      int refAlg = Loc.algOffset;

      if (refAlg >= 0)
        {
          ad->assignCheck (refAlg,B1Voffset,mp_Kp / tap);
          ad->assign (refAlg, refAlg, -1);
          ad->assign (refAlg,refAlg + 1,-mp_Kp);

          ad->assignCheck (refAlg + 1, B1Voffset, k3sq2 * Loc.algStateLoc[0]);
          ad->assignCheck (refAlg + 1,B2Voffset,-1);
          ad->assign (refAlg + 1, refAlg, k3sq2 * LinkInfo.v1);
          ad->assign (refAlg + 1, refAlg + 1, -3 / kPI * x);
        }
      int refDiff = Loc.diffOffset;
      if (refDiff >= 0)
        {
          ad->assign (refAlg,refDiff,1);
          ad->assign (refDiff,B1Voffset,mp_Ki / tap);
          ad->assign (refDiff,refAlg,-mp_Ki);
          ad->assign (refDiff,refDiff,-sD->cj);
        }

    }
  else
    {
      int offset = offsets.getAlgOffset (sMode);

      //resid[offset] = k3sq2*LinkInfo.v1*sD->state[offset] - 3 / kPI*x*Idc - LinkInfo.v2;
      ad->assign (offset, offset, k3sq2 * LinkInfo.v1);
      if (opFlags.test (fixed_target_power))
        {
          ad->assignCheck (offset, B1Voffset, k3sq2 * sD->state[offset]);
          ad->assignCheck (offset, B2Voffset, 3 / kPI * x * Pset / (LinkInfo.v2 * LinkInfo.v2) - 1);
        }
      else
        {
          ad->assignCheck (offset, B1Voffset, k3sq2 * sD->state[offset] - 3 / kPI * x / tap);
          ad->assignCheck (offset, B2Voffset, -1);
        }
    }
}

void rectifier::residual (const stateData *sD, double resid[], const solverMode &sMode)
{

  computePower (sD, sMode);
  if (isDynamic (sMode))
    {
      auto Loc = offsets.getLocations (sD, resid, sMode, this);
      Loc.destLoc[0] = Loc.diffStateLoc[0] + mp_Kp * (LinkInfo.v1 / tap - Loc.algStateLoc[1]) - Loc.algStateLoc[0];
      Loc.destLoc[1] = k3sq2 * LinkInfo.v1 * Loc.algStateLoc[0] - 3 / kPI * x * Loc.algStateLoc[1] - LinkInfo.v2;
      if (isAlgebraic (sMode))
        {
          return;
        }
      Loc.destDiffLoc[0] = mp_Ki * (LinkInfo.v1 / tap - Loc.algStateLoc[1]) - Loc.dstateLoc[0];
    }
  else
    {
      int offset = offsets.getAlgOffset (sMode);
      Idc = (opFlags.test (fixed_target_power)) ? Pset / LinkInfo.v2 : LinkInfo.v1 / tap;

      resid[offset] = k3sq2 * LinkInfo.v1 * sD->state[offset] - 3 / kPI * x * Idc - LinkInfo.v2;
    }
}

void rectifier::setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      gridObject::setState (ttime, state, dstate_dt, sMode);
      stateData sD;
      sD.state = state;
      sD.dstate_dt = dstate_dt;
      computePower (&sD, sMode);
    }
  else
    {
      int offset = offsets.getAlgOffset (sMode);
      m_state[0] = state[offset];
      if (opFlags.test (fixed_target_power))
        {
          m_state[1] = Pset / B2->getVoltage (state, sMode);
        }
      else
        {
          m_state[1] = B1->getVoltage (state, sMode) / tap;
        }
      Idc = m_state[1];
    }

}

void rectifier::guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      gridObject::guess (ttime, state, dstate_dt, sMode);
    }
  else
    {
      state[offsets.getAlgOffset (sMode)] = m_state[0];
    }

}


void  rectifier::computePower (const stateData *sD, const solverMode &sMode)
{
  if ((LinkInfo.seqID == sD->seqID) && (sD->seqID != 0))
    {
      return;
    }

  if (!enabled)
    {
      return;
    }
  std::memset (&LinkInfo, 0, sizeof(linkP));


  LinkInfo.v1 = B1->getVoltage (sD->state, sMode);
  LinkInfo.v2 = B2->getVoltage (sD->state, sMode);


  if (isDynamic (sMode))
    {
      auto Loc = offsets.getLocations (sD, nullptr, sMode, this);
      Idc = Loc.algStateLoc[1];
    }
  else
    {
      Idc = (opFlags.test (fixed_target_power)) ? Pset / LinkInfo.v2 : LinkInfo.v1 / tap;
    }

  LinkInfo.Pik = LinkInfo.v2 * Idc;
  LinkInfo.Pki = -LinkInfo.Pik;
  double sr = k3sq2 * LinkInfo.v1 * Idc;

  LinkInfo.Qik = std::sqrt (sr * sr - LinkInfo.Pik * LinkInfo.Pik);
  //Qki is 0 since bus k is a DC bus.

}

void  rectifier::computePower ()
{

  std::memset (&LinkInfo, 0, sizeof(linkP));

  if (enabled)
    {
      LinkInfo.v1 = B1->getVoltage ();
      LinkInfo.v2 = B2->getVoltage ();
      LinkInfo.Pik = LinkInfo.v2 * Idc;
      LinkInfo.Pki = -LinkInfo.Pik;
      double sr = k3sq2 * LinkInfo.v1 * Idc;

      LinkInfo.Qik = std::sqrt (sr * sr - LinkInfo.Pik * LinkInfo.Pik);
    }
}


void rectifier::fixPower (double power, int terminal)
{
  Pset = power;
  opFlags.set (fixed_target_power);
  Idc = Pset / B2->getVoltage ();
  computePower ();
}
