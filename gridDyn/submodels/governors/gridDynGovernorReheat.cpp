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

#include "submodels/otherGovernors.h"
#include "generators/gridDynGenerator.h"
#include "objectFactory.h"
#include "gridBus.h"
#include "arrayData.h"
#include "gridCoreTemplates.h"

using namespace gridUnits;

gridDynGovernorReheat::gridDynGovernorReheat (const std::string &objName) : gridDynGovernor (objName)
{
  // default values
  K = 16.667;
  T1 = 0.1;
  T2 = 0.45;
  T3 = 0.05;
  T4 = 12.0;
  T5 = 50.0;
  offsets.local->local.algSize = 1;
  offsets.local->local.diffSize = 3;
  offsets.local->local.jacSize = 12;
}

gridCoreObject *gridDynGovernorReheat::clone (gridCoreObject *obj) const
{
  gridDynGovernorReheat *gov = cloneBase<gridDynGovernorReheat, gridDynGovernor> (this, obj);
  if (gov == nullptr)
    {
      return obj;
    }

  gov->T3 = T3;
  gov->T4 = T4;
  gov->T5 = T5;

  return gov;
}

// destructor
gridDynGovernorReheat::~gridDynGovernorReheat ()
{

}

// initial conditions
void gridDynGovernorReheat::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
  if (Wref < 0)
    {
      Wref = m_baseFreq;
    }

  double P = outputSet[0];
  //double omega = getControlFrequency (args);
  double omega = args[govOmegaInLocation];
  if (P > Pmax)
    {
      P = Pmax;
    }
  else if (P < Pmin)
    {
      P = Pmin;
    }
  m_state[3] = P;
  m_state[2] = (1.0 - T3 / T2) * P;
  m_state[1] = (1.0 - T4 / T5) * (m_state[2] + T3 / T2 * m_state[3]);
  m_state[0] = P;



  inputSet.resize (2);
  inputSet[1] = P - K * (Wref - omega);      //Pset

}


// residual
void gridDynGovernorReheat::residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode)
{

  auto offset = offsets.getAlgOffset (sMode);
  const double *gsp = sD->dstate_dt + offset;
  if (isAlgebraicOnly (sMode))
    {
      gsp = sD->dstate_dt + offsets.getAlgOffset (solverMode (4));
      resid[offset + 0] = gsp[1] + T4 / T5 * (gsp[2] + T3 / T2 * gsp[3]) - sD->state[offset];
      return;
    }
  const double *gs = sD->state + offset;

  //double omega = getControlFrequency (args);
  double omega = args[govOmegaInLocation];

  double Tin = args[govpSetInLocation] + K * (Wref - omega);
  if (Tin > Pmax)
    {
      Tin = Pmax;
    }
  else if (Tin < Pmin)
    {
      Tin = Pmin;
    }
  resid[offset + 3] = (Tin - gs[3]) / T1 - gsp[3];
  resid[offset + 2] = ((1 - T3 / T2) * gs[3] - gs[2]) / T2 - gsp[2];
  resid[offset + 1] = ((1.0 - T4 / T5) * (gs[2] + T3 / T2 * gs[3]) - gs[1]) / T5 - gsp[1];
  resid[offset + 0] = gs[1] + T4 / T5 * (gs[2] + T3 / T2 * gs[3]) - gs[0];
}

void gridDynGovernorReheat::derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
  auto offset = offsets.getAlgOffset (sMode);
  const double *gs = sD->state + offset;
  //double omega = getControlFrequency (args);
  double omega = args[govOmegaInLocation];
  double Tin = args[govpSetInLocation] + K * (Wref - omega);
  if (Tin > Pmax)
    {
      Tin = Pmax;
    }
  else if (Tin < Pmin)
    {
      Tin = Pmin;
    }
  deriv[offset + 3] = (Tin - gs[3]) / T1;
  deriv[offset + 2] = ((1 - T3 / T2) * gs[3] - gs[2]) / T2;
  deriv[offset + 1] = ((1.0 - T4 / T5) * (gs[2] + T3 / T2 * gs[3]) - gs[1]) / T5;
}



void gridDynGovernorReheat::jacobianElements (const IOdata &args, const stateData *sD, arrayData<double> *ad,  const IOlocs &argLocs, const solverMode &sMode)
{
  auto offset = offsets.getAlgOffset  (sMode);
  if (isAlgebraicOnly (sMode))
    {
      ad->assign (offset, offset, -1);
      return;
    }


  //double omega = getControlFrequency (args);
  double omega = args[govOmegaInLocation];
  bool limitTin = false;
  double Tin = args[govpSetInLocation] + K * (Wref - omega);
  if (Tin > Pmax)
    {
      Tin = Pmax;
      limitTin = true;
    }
  else if (Tin < Pmin)
    {
      Tin = Pmin;
      limitTin = true;
    }
  int refI = offset;
  //use the ad->assign Macro defined in basicDefs
  // ad->assign(arrayIndex, RowIndex, ColIndex, value)
  bool linkOmega = (argLocs[govOmegaInLocation] != kNullLocation);

  /* if (opFlags.test (uses_deadband))
     {
       if (!opFlags.test (outside_deadband))
         {
           linkOmega = false;
         }
     }
         */
  if (limitTin)
    {
      ad->assign (refI + 3, refI + 3, -1.0 / T1 - sD->cj);
    }
  else
    {
      if (linkOmega)
        {
          ad->assign (refI + 3, argLocs[govOmegaInLocation], K / T1);
        }
      if (argLocs[govpSetInLocation] != kNullLocation)
        {
          ad->assign (refI + 3, argLocs[govpSetInLocation], 1.0 / T1);
        }
      ad->assign (refI + 3, refI + 3, -1.0 / T1 - sD->cj);
    }
  // tg2

  ad->assign (refI + 2, refI + 3, (1.0 - T3 / T2) * 1.0 / T2);
  ad->assign (refI + 2, refI + 2, -1.0 / T2 - sD->cj);
  //Tg3
  ad->assign (refI + 1, refI + 2, (1.0 - T4 / T5) / T5);
  ad->assign (refI + 1, refI + 3, ((1.0 - T4 / T5) * T3 / T2) / T5);
  ad->assign (refI + 1, refI + 1, -1.0 / T5 - sD->cj);
  //Tout
  ad->assign (refI, refI, -1);
  ad->assign (refI, refI + 1, 1);
  ad->assign (refI, refI + 2, T4 / T5);
  ad->assign (refI, refI + 3, T4 / T5 * (T3 / T2));

}


index_t gridDynGovernorReheat::findIndex (const std::string &field, const solverMode &) const
{
  index_t ret = kInvalidLocation;
  if (field == "pm")
    {
      ret = 0;
    }
  else if (field == "tg1")
    {
      ret = 3;
    }
  else if (field == "tg2")
    {
      ret = 2;
    }
  else if (field == "tg3")
    {
      ret = 1;
    }
  return ret;
}

// set parameters
int gridDynGovernorReheat::set (const std::string &param,  const std::string &val)
{
  return gridCoreObject::set (param, val);
}

int gridDynGovernorReheat::set (const std::string &param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;

  //param   = gridDynSimulation::toLower(param);
  if ((param == "ts") || (param == "t1"))
    {
      T1 = val;
    }
  else if ((param == "tc") || (param == "t2"))
    {
      T2 = val;
    }
  else if (param == "t3")
    {
      T3 = val;
    }
  else if (param == "t4")
    {
      T4 = val;
    }
  else if (param == "t5")
    {
      T5 = val;
    }
  else
    {
      out = gridDynGovernor::set (param, val, unitType);
    }

  return out;
}








