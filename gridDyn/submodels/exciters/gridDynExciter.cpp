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

#include "submodels/gridDynExciter.h"
#include "generators/gridDynGenerator.h"
#include "gridBus.h"
#include "objectFactoryTemplates.h"
#include "arrayData.h"
#include "gridCoreTemplates.h"

#include <cmath>

// note that there is only 1 dynamic state since V_R = E_f


//setup the object factories
static childTypeFactory<gridDynExciterDC1A, gridDynExciter> gfe1 ("exciter", "dc1a");
static childTypeFactory<gridDynExciterDC2A, gridDynExciter> gfe2 ("exciter", "dc2a");
static childTypeFactory<gridDynExciterIEEEtype1, gridDynExciter> gfet1 ("exciter", "type1");
static typeFactory<gridDynExciter> gf ("exciter", stringVec { "basic", "fast" }, "type1"); //setup type 1 as the default
static childTypeFactory<gridDynExciterIEEEtype2, gridDynExciter> gfet2 ("exciter", "type2");

gridDynExciter::gridDynExciter (const std::string &objName) : gridSubModel (objName)
{

}

//cloning function
gridCoreObject *gridDynExciter::clone (gridCoreObject *obj) const
{
  gridDynExciter *gdE = cloneBase<gridDynExciter, gridSubModel> (this, obj);
  if (gdE == nullptr)
    {
      return obj;
    }

  gdE->Ka = Ka;
  gdE->Ta = Ta;
  gdE->Vrmin = Vrmin;
  gdE->Vrmax = Vrmax;
  gdE->Vref = Vref;
  return gdE;
}


void gridDynExciter::objectInitializeA (double /*time0*/, unsigned long /*flags*/)
{
  offsets.local->local.diffSize = 1;
  offsets.local->local.jacSize = 4;
  checkForLimits ();
}

void gridDynExciter::checkForLimits ()
{
  if ((Vrmin > -21) || (Vrmax < 21))
    {
      offsets.local->local.algRoots = 1;
    }
}
// initial conditions
void gridDynExciter::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{
  double *gs = m_state.data ();
  double V = args[voltageInLocation];
  if (outputSet.empty () || (outputSet[0] == kNullVal))
    {
      gs[0] = (Vref + vBias - V) / Ka;
      fieldSet[0] = gs[0];
    }
  else
    {
      gs[0] = outputSet[0];

      vBias = V - Vref + gs[0] / Ka;
      fieldSet[exciterVsetInLocation] = Vref;
    }


}

// residual
void gridDynExciter::residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode)
{
  if (isAlgebraicOnly (sMode))
    {
      return;
    }
  auto offset = offsets.getDiffOffset (sMode);
  const double *es = sD->state + offset;
  const double *esp = sD->dstate_dt + offset;
  double *rv = resid + offset;
  if (opFlags[outside_vlim])
    {
      if (opFlags[etrigger_high])
        {
          rv[0] = esp[0];
        }
      else
        {
          rv[0] = esp[0];
        }
    }
  else
    {
      rv[0] = (-es[0] + Ka * (Vref + vBias - args[voltageInLocation])) / Ta - esp[0];
    }

}

void gridDynExciter::derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{

  Lp Loc = offsets.getLocations (sD,deriv, sMode, this);
  const double *es = Loc.diffStateLoc;
  double *d = Loc.destDiffLoc;
  if (opFlags[outside_vlim])
    {
      d[0] = 0.0;
    }
  else
    {
      d[0] = (-es[0] + Ka * (Vref + vBias - args[voltageInLocation])) / Ta;
    }
}

// Jacobian
void gridDynExciter::jacobianElements (const IOdata & /*args*/, const stateData *sD,
                                       arrayData<double> *ad,
                                       const IOlocs &argLocs, const solverMode &sMode)
{
  if  (isAlgebraicOnly (sMode))
    {
      return;
    }
  auto offset = offsets.getDiffOffset (sMode);

  // Ef (Vr)
  if (opFlags[outside_vlim])
    {
      ad->assign (offset, offset, sD->cj);
    }
  else
    {
      ad->assign (offset, offset, -1.0 / Ta - sD->cj);
      ad->assignCheckCol (offset, argLocs[voltageInLocation], -Ka / Ta);
    }

  //printf("%f\n",sD->cj);

}

void gridDynExciter::rootTest (const IOdata &args, const stateData *sD, double root[],  const solverMode &sMode)
{
  auto offset = offsets.getDiffOffset (sMode);
  int rootOffset = offsets.getRootOffset (sMode);
  double Efield = sD->state[offset];

  if (opFlags[outside_vlim])
    {
      root[rootOffset] = Vref + vBias - args[voltageInLocation];
    }
  else
    {
      root[rootOffset] = std::min (Vrmax - Efield, Efield - Vrmin) + 0.0001;
      if (Efield > Vrmax)
        {
          opFlags.set (etrigger_high);
        }
    }


}


void gridDynExciter::rootTrigger (double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
  int rootOffset = offsets.getRootOffset (sMode);
  if (rootMask[rootOffset])
    {

      if (opFlags[outside_vlim])
        {
          LOG_NORMAL ("root trigger back in bounds");
          alert (this, JAC_COUNT_INCREASE);
          opFlags.reset (outside_vlim);
          opFlags.reset (etrigger_high);
        }
      else
        {
          opFlags.set (outside_vlim);
          if (opFlags[etrigger_high])
            {
              LOG_NORMAL ("root trigger above bounds");
              m_state[limitState] -= 0.0001;
            }
          else
            {
              LOG_NORMAL ("root trigger below bounds");
              m_state[limitState] += 0.0001;
            }
          alert (this, JAC_COUNT_DECREASE);
        }
      stateData sD;
      sD.time = ttime;
      sD.state = m_state.data ();
      derivative (args, &sD, m_dstate_dt.data (), cLocalSolverMode);
    }
}

change_code gridDynExciter::rootCheck ( const IOdata &args, const stateData *, const solverMode &, check_level_t /*level*/)
{

  double Efield = m_state[0];
  double test;
  change_code ret = change_code::no_change;
  if (opFlags[outside_vlim])
    {
      test = Vref + vBias - args[voltageInLocation];
      if (opFlags[etrigger_high])
        {
          if (test < 0)
            {
              opFlags.reset (outside_vlim);
              opFlags.reset (etrigger_high);
              alert (this, JAC_COUNT_INCREASE);
              ret = change_code::jacobian_change;
            }
        }
      else
        {
          if (test > 0)
            {
              opFlags.reset (outside_vlim);
              alert (this, JAC_COUNT_INCREASE);
              ret = change_code::jacobian_change;
            }
        }
    }
  else
    {
      if (Efield > Vrmax + 0.0001)
        {
          opFlags.set (etrigger_high);
          opFlags.set (outside_vlim);
          m_state[0] = Vrmax;
          alert (this, JAC_COUNT_DECREASE);
          ret = change_code::jacobian_change;
        }
      else if (Efield < Vrmin - 0.0001)
        {
          opFlags.set (outside_vlim);
          m_state[0] = Vrmin;
          alert (this, JAC_COUNT_DECREASE);
          ret = change_code::jacobian_change;
        }
    }
  return ret;

}

static const stringVec exciterFields {
  "ef"
};

stringVec gridDynExciter::localStateNames () const
{
  return exciterFields;
}


int gridDynExciter::set (const std::string &param,  const std::string &val)
{
  return gridCoreObject::set (param, val);
}

// set parameters
int gridDynExciter::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;

  if (param == "vref")
    {
      Vref = val;
    }
  else if (param == "ka")
    {
      Ka = val;
    }
  else if (param == "ta")
    {
      Ta = val;
    }
  else if ((param == "vrmax")|| (param == "urmax"))
    {
      Vrmax = val;
    }
  else if ((param == "vrmin") || (param == "urmin"))
    {
      Vrmin = val;
    }
  else if (param == "vbias")
    {
      vBias = val;
    }
  else
    {
      out = gridSubModel::set (param, val, unitType);
    }

  return out;
}
