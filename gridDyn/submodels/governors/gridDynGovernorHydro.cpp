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
#include "gridBus.h"
#include "arrayData.h"
#include "gridCoreTemplates.h"

gridDynGovernorHydro::gridDynGovernorHydro (const std::string &objName) : gridDynGovernorIeeeSimple (objName)
{
  // default values
  K     = 0;
  T1    = 0;
  T2    = 0;
  T3    = 0;
  Pup   = kBigNum;
  Pdown = kBigNum;
  Pmax  = kBigNum;
  Pmin  = 0;
  Pset    = 0;
  offsets.local->local.diffSize = 2;
  offsets.local->local.jacSize = 5;
}

gridCoreObject *gridDynGovernorHydro::clone (gridCoreObject *obj) const
{
  gridDynGovernorHydro *gov = cloneBase<gridDynGovernorHydro, gridDynGovernorIeeeSimple> (this, obj);
  if (gov == nullptr)
    {
      return obj;
    }

  gov->K     = K;
  gov->T1    = T1;
  gov->T2    = T2;
  gov->T3    = T3;
  gov->Pup   = Pup;
  gov->Pdown = Pdown;
  gov->Pmax  = Pmax;
  gov->Pmin  = Pmin;
  gov->Pset    = Pset;
  return gov;
}

// destructor
gridDynGovernorHydro::~gridDynGovernorHydro ()
{

}

// initial conditions
void gridDynGovernorHydro::objectInitializeB (const IOdata & /*args*/, const IOdata &outputSet, IOdata & /*fieldSet*/)
{
  m_state[1] = 0;
  m_state[0] = outputSet[0];
  auto genObj = find ("gen");
  if (genObj)
    {
      genObj->get ("pset");
    }

}


// residual
void gridDynGovernorHydro::residual (const IOdata & /*args*/, const stateData *, double resid[],  const solverMode &sMode)
{
  auto offset = offsets.getAlgOffset (sMode);
  resid[offset] = 0;
  resid[offset + 1] = 0;
}


void gridDynGovernorHydro::jacobianElements (const IOdata & /*args*/, const stateData *sD, arrayData<double> *ad,  const IOlocs & /*argLocs*/, const solverMode &sMode)
{
  if  (isAlgebraicOnly (sMode))
    {
      return;
    }
  auto offset = offsets.getAlgOffset (sMode);
  auto refI = offset;
  //use the ad->assign Macro defined in basicDefs
  // ad->assign(arrayIndex, RowIndex, ColIndex, value)
  int omegaLoc = -1;

  // Pm
  if (omegaLoc >= 0)
    {
      ad->assign ( refI, omegaLoc, -K * T2 / (T1 * T3));

    }
  ad->assign (refI,refI,-1 / T3 - sD->cj);
  ad->assign (refI,refI + 1,-K / T3);
  // X
  if (omegaLoc >= 0)
    {
      ad->assign ( refI + 1, omegaLoc, (T1 - T2) / (T1 * T1));
    }

  ad->assign (refI + 1,refI + 1,-1 / T1 - sD->cj);

}



index_t gridDynGovernorHydro::findIndex (const std::string &field, const solverMode &) const
{
  index_t ret = kInvalidLocation;
  if (field == "pm")
    {
      ret = 0;
    }
  else if (field == "x")
    {
      ret = 1;
    }
  return ret;
}

// set parameters
int gridDynGovernorHydro::set (const std::string &param,  const std::string &val)
{
  return gridCoreObject::set (param, val);
}

int gridDynGovernorHydro::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;

  //param   = gridDynSimulation::toLower(param);
  if (param == "k")
    {
      K = val;
    }
  else if (param == "t1")
    {
      T1 = val;
    }
  else if (param == "t2")
    {
      T2 = val;
    }
  else if (param == "t3")
    {
      T3 = val;
    }
  else if (param == "pup")
    {
      Pup = val;
    }
  else if (param == "pdown")
    {
      Pdown = val;
    }
  else if (param == "pmax")
    {
      Pmax = val;
    }
  else if (param == "pmin")
    {
      Pmin = val;
    }
  else
    {
      out = gridCoreObject::set (param,val,unitType);
    }

  return out;
}


