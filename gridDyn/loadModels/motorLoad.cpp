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

#include "loadModels/motorLoad.h"
#include "gridBus.h"
#include "objectFactoryTemplates.h"
#include "gridCoreTemplates.h"
#include "matrixData.h"
#include "core/gridDynExceptions.h"

#include <iostream>
#include <cmath>

using namespace gridUnits;

//setup the load object factories
static typeFactory<motorLoad> mlf1 ("load", stringVec { "motor", "motor1" });

static typeFactory<motorLoad3> mlf3 ("load", stringVec { "motor3", "motorIII","m3" });

static typeFactory<motorLoad5> mlf5 ("load", stringVec { "motor5", "motorIV","m5" });


static const double cSmallDiff = 1e-7;
motorLoad::motorLoad (const std::string &objName) : gridLoad (objName)
{
  // default values
  P = -kBigNum;
  H = 3;
  x = 0.15;
  r = 0.01;
  opFlags.set (no_pqvoltage_limit);
  opFlags.set (has_dyn_states);
}

gridCoreObject *motorLoad::clone (gridCoreObject *obj) const
{
  motorLoad *ld = cloneBase<motorLoad, gridLoad> (this, obj);
  if (!(ld))
    {
      return obj;
    }
  ld->r1 = r1;
  ld->x1 = x1;
  ld->xm = xm;
  ld->a = a;
  ld->b = b;
  ld->c = c;
  ld->Vcontrol = Vcontrol;
  ld->mBase = mBase;
  return ld;
}

void motorLoad::pFlowObjectInitializeA (double time0, unsigned long flags)
{
  m_state.resize (1);
  if (opFlags[init_transient])
    {
      if (init_slip >= 0)
        {
          m_state[0] = init_slip;
        }
      else
        {
          m_state[0] = 1.0;
        }
    }
  else if (init_slip > 0)
    {
      m_state[0] = init_slip;
    }
  else
    {
      m_state[0] = 0.03;
    }
  scale = mBase / systemBasePower;
  //these parameters need to be ignored for the time being
  Vpqmin = -1.0;
  Vpqmax = kBigNum;
  return gridLoad::pFlowObjectInitializeA (time0,flags);
}

void motorLoad::dynObjectInitializeA (double time0, unsigned long flags)
{
  opFlags.set (has_roots);
  return gridLoad::dynObjectInitializeA (time0, flags);
}

void motorLoad::dynObjectInitializeB (const IOdata & /*args*/, const IOdata & /*outputSet*/)
{

  m_dstate_dt[0] = 0;

  if (opFlags[init_transient])
    {
      P = mechPower (m_state[0]);
    }
  else
    {

    }

}



void motorLoad::loadSizes (const solverMode &sMode, bool dynOnly)
{
  auto so = offsets.getOffsets (sMode);
  if (dynOnly)
    {
      so->total.jacSize = 4;
      if ((opFlags[stalled]) && (opFlags[resettable]))
        {
          so->total.algRoots = 1;
        }
      else
        {
          so->total.diffRoots = 1;
        }
      so->rjLoaded = true;
    }
  else
    {
      so->reset ();
      if (isDynamic (sMode))
        {
          if ((opFlags[stalled]) && (opFlags[resettable]))
            {
              so->total.diffRoots = 1;
            }
          else
            {
              so->total.algRoots = 1;
            }
          if (!isAlgebraicOnly (sMode))
            {
              so->total.diffSize = 1;
              so->total.jacSize = 4;

            }
        }
      else if (!opFlags[init_transient])
        {
          so->total.algSize = 1;
          so->total.jacSize = 4;
        }
      so->rjLoaded = true;
      so->stateLoaded = true;
    }
}

// set properties
void motorLoad::set (const std::string &param,  const std::string &val)
{


  if (param[0] == '#')
    {

    }
  else
    {
      gridLoad::set (param, val);
    }

}

void motorLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  bool slipCheck = false;

  if (param.size () == 1)
    {
      switch (param[0])
        {
        case 'h':
          H = val;
          break;
        case 'p':
          P = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
          if (mBase < 0)
            {
              mBase = P * systemBasePower;
              scale = P;
            }
          alpha = P / scale;
          a = alpha - b - c;
          slipCheck = true;
          break;
        case 'b':
          b = val;
          alpha = a + b + c;
          beta = -b - 2 * c;
          slipCheck = true;
          break;
        case 'a':
          a = val;
          alpha = a + b + c;
          slipCheck = true;
          break;
        case 'c':
          c = val;
          gamma = c;
          slipCheck = true;
          break;
        default:
			throw(unrecognizedParameter());

        }
    }
  else
    {
      if (param == "r1")
        {
          r1 = val;
        }
      else if (param == "x1")
        {
          x1 = val;
        }
      else if (param == "xm")
        {
          xm = val;
        }

      else if (param == "alpha")
        {
          alpha = val;
          a = alpha - b - c;
          slipCheck = true;
        }
      else if (param == "beta")
        {
          beta = val;
          b = -beta - 2 * c;
          a = alpha - b - c;
          slipCheck = true;
        }
      else if (param == "gamma")
        {
          gamma = val;
          c = gamma;
          slipCheck = true;
        }
      else if ((param == "base") || (param == "mbase") || (param == "rating"))
        {
          mBase = unitConversion (val, unitType, MVAR, systemBasePower, baseVoltage);
        }
      else if (param == "Vcontrol")
        {
          Vcontrol = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
          slipCheck = true;
        }
      else
        {
          gridLoad::set (param, val, unitType);
        }
    }

  if (slipCheck)
    {
      if (opFlags[stalled])
        {
          rootCheck (bus->getOutputs (nullptr,cLocalSolverMode),nullptr,cLocalSolverMode, check_level_t::reversable_only);
        }
    }

}

void motorLoad::setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      if (isAlgebraicOnly (sMode))
        {
          return;
        }

      auto offset = offsets.getDiffOffset (sMode);
      m_state[0] = state[offset];
      m_dstate_dt[0] = dstate_dt[offset];
    }
  else if (!opFlags[init_transient])
    {
      auto offset = offsets.getAlgOffset (sMode);
      m_state[0] = state[offset];
    }
  prevTime = ttime;
}

void motorLoad::guess (double /*ttime*/, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
	  if (hasDifferential(sMode))
	  {
		  auto offset = offsets.getDiffOffset(sMode);
		  state[offset] = m_state[0];
		  dstate_dt[offset] = m_dstate_dt[0];
	  }
    }
  else if (!opFlags[init_transient])
    {
      auto offset = offsets.getAlgOffset (sMode);
      state[offset] = m_state[0];
    }
}

// residual
void motorLoad::residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
	  if (hasDifferential(sMode))
	  {
		  derivative(args, sD, resid, sMode);
		  auto offset = offsets.getDiffOffset(sMode);
		  resid[offset] -= sD->dstate_dt[offset];
	  }
    }
  else if (!opFlags[init_transient])
    {
      auto offset = offsets.getAlgOffset (sMode);
      double slip = sD->state[offset];
      resid[offset] = mechPower (slip) - rPower (args[voltageInLocation], slip);
      // printf("slip=%f mpower=%f, rPower=%f\n", slip, mechPower(slip), rPower(args[voltageInLocation], slip));
    }

}

void motorLoad::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  if (isDynamic (sMode))
    {
      if (isAlgebraicOnly (sMode))
        {
          return;
        }

      auto offset = offsets.getDiffOffset (sMode);
      stNames[offset] = prefix + name + ":slip";
    }
  else if (!opFlags[init_transient])
    {
      auto offset = offsets.getAlgOffset (sMode);
      stNames[offset] = prefix + name + ":slip";
    }
  else
    {
      return;
    }

}
void motorLoad::timestep (double ttime, const IOdata &args, const solverMode &)
{
  double dt = ttime - prevTime;
  motorLoad::derivative (args, nullptr, m_dstate_dt.data (), cLocalSolverMode);
  m_state[0] += dt * m_dstate_dt[0];
}

void motorLoad::derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{
  auto offset = offsets.getDiffOffset (sMode);
  double slip = (sD) ? sD->state[offset] : m_state[0];
  double V = args[voltageInLocation];

  deriv[offset] = (opFlags[stalled]) ? 0 : (0.5 / H * (mechPower (slip) - rPower (V * Vcontrol, slip)));


}

void motorLoad::jacobianElements (const IOdata &args, const stateData *sD, matrixData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
  if  (isDynamic (sMode))
    {
	  if (hasDifferential(sMode))
	  {
		  auto offset = offsets.getDiffOffset(sMode);
		  double slip = sD->state[offset];
		  double V = args[voltageInLocation];
		  if (opFlags[stalled])
		  {
			  ad->assign(offset, offset, -sD->cj);
		  }
		  else
		  {
			  ad->assignCheck(offset, argLocs[voltageInLocation], -(1.0 / H) * (V * Vcontrol * Vcontrol * r1 * slip / (r1 * r1 + slip * slip * (x + x1) * (x + x1))));
			  //this is a really ugly looking derivative so I am computing it numerically
			  double test1 = 0.5 / H * (mechPower(slip) - rPower(V, slip));
			  double test2 = 0.5 / H * (mechPower(slip + cSmallDiff) - rPower(V * Vcontrol, slip + cSmallDiff));
			  ad->assign(offset, offset, (test2 - test1) / cSmallDiff - sD->cj);
		  }
	  }
    }
  else if (!opFlags[init_transient])
    {
      int offset = offsets.getAlgOffset (sMode);
      double slip = sD->state[offset];
      double V = args[voltageInLocation];

      double t1 = rPower (V * Vcontrol, slip);
      double t3 = rPower (V * Vcontrol, slip + cSmallDiff);
      ad->assign (offset, offset, dmechds (slip) - (t3 - t1) / cSmallDiff);


      ad->assignCheck (offset,argLocs[voltageInLocation],-2 * t1 / V);

    }
}

void motorLoad::outputPartialDerivatives (const IOdata &args, const stateData *sD, matrixData<double> *ad, const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      if (isAlgebraicOnly (sMode))
        {
          return;
        }

      auto offset = offsets.getDiffOffset (sMode);
      double slip = sD->state[offset];
      double V = args[voltageInLocation];

      ad->assign (PoutLocation, offset, scale * (rPower (V * Vcontrol, slip + cSmallDiff) - rPower (V * Vcontrol, slip)) / cSmallDiff);

      ad->assign (QoutLocation, offset, scale * (qPower (V * Vcontrol, slip + cSmallDiff) - qPower (V * Vcontrol, slip)) / cSmallDiff);
    }
  else if (!opFlags[init_transient])
    {
      auto offset = offsets.getAlgOffset (sMode);
      double slip = sD->state[offset];
      double V = args[voltageInLocation];
      ad->assign (QoutLocation, offset, scale * (qPower (V * Vcontrol, slip + cSmallDiff) - qPower (V * Vcontrol, slip)) / cSmallDiff);
      ad->assign (PoutLocation, offset, scale * (rPower (V * Vcontrol, slip + cSmallDiff) - rPower (V * Vcontrol, slip)) / cSmallDiff);
    }
}

void motorLoad::ioPartialDerivatives (const IOdata &args, const stateData *sD, matrixData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
  if  (argLocs[voltageInLocation] != kNullLocation)
    {
      double slip = m_state[0];
      double V = args[voltageInLocation];
      if (isDynamic (sMode))
        {
          Lp Loc = offsets.getLocations  (sD, sMode, this);
          slip = Loc.diffStateLoc[0];
        }
      else if (!opFlags[init_transient])
        {
          slip = sD->state[offsets.getAlgOffset (sMode)];
        }
      double temp = V * slip / (r1 * r1 + slip * slip * (x + x1) * (x + x1));
      ad->assign (PoutLocation, argLocs[voltageInLocation], scale * (2 * r1 * temp));
      ad->assign (QoutLocation, argLocs[voltageInLocation], scale * (2 * V / xm + 2 * slip * (x + x1) * temp));
    }
}

index_t motorLoad::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  if (field == "slip")
    {
      ret = offsets.getDiffOffset (sMode);
    }
  return ret;
}



void motorLoad::rootTest (const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations  (sD, sMode, this);
  double slip = Loc.diffStateLoc[0];
  auto ro = offsets.getRootOffset (sMode);
  if (opFlags[stalled])
    {
      roots[ro] = rPower (args[voltageInLocation] * Vcontrol, 1.0) - mechPower (1.0);
    }
  else
    {
      roots[ro] = 1.0 - slip;
    }
}

void motorLoad::rootTrigger (double /*ttime*/, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
  if (!rootMask[offsets.getRootOffset (sMode)])
    {
      return;
    }
  if (opFlags[stalled])
    {
      if (args[voltageInLocation] > 0.5)
        {
          opFlags.reset (stalled);
          alert (this,JAC_COUNT_INCREASE);
          m_state[0] = 1.0 - 1e-7;
        }
    }
  else
    {
      opFlags.set (stalled);
      alert (this, JAC_COUNT_DECREASE);
      m_state[0] = 1.0;
    }
}

change_code motorLoad::rootCheck (const IOdata &args, const stateData *, const solverMode &, check_level_t /*level*/)
{
  if (opFlags[stalled])
    {
      if (rPower (args[voltageInLocation] * Vcontrol, 1.0) - mechPower (1.0) > 0)
        {
          opFlags.reset (stalled);
          alert (this, JAC_COUNT_INCREASE);
          return change_code::jacobian_change;
        }
    }
  return change_code::no_change;
}



double motorLoad::getRealPower () const
{
  const double V = bus->getVoltage ();

  double slip = m_state[0];
  return rPower (V * Vcontrol, slip) * scale;
}

double motorLoad::getReactivePower () const
{
  const double V = bus->getVoltage ();

  double slip = m_state[0];

  return qPower (V * Vcontrol, slip) * scale;
}

double motorLoad::getRealPower (const IOdata &args, const stateData *sD, const solverMode &sMode)
{
  const double V = args[voltageInLocation];

  //double Ptemp;
  if (isDynamic (sMode))
    {
      Lp Loc = offsets.getLocations  (sD, sMode, this);

      double slip = Loc.diffStateLoc[0];
      Pout = rPower (V * Vcontrol, slip);
    }
  else if (opFlags[init_transient])
    {
      double slip = m_state[0];
      Pout = rPower (V * Vcontrol,slip);
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      double slip = sD->state[offset];
      Pout = rPower (V * Vcontrol, slip);
    }

  return Pout * scale;
}

double motorLoad::getReactivePower (const IOdata &args, const stateData *sD, const solverMode &sMode)
{
  double V = args[voltageInLocation];
  // double Qtemp;
  if (isDynamic (sMode))
    {
      Lp Loc = offsets.getLocations  (sD, sMode, this);

      double slip = Loc.diffStateLoc[0];
      Qout = qPower (V, slip);
    }
  else if (opFlags[init_transient])
    {
      double slip = m_state[0];
      Qout = qPower (V * Vcontrol, slip);
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      double slip = sD->state[offset];
      Qout = qPower (V * Vcontrol,slip);
    }
  return Qout * scale;

}

double motorLoad::getRealPower (const double V) const
{
  double slip = m_state[0];

  return rPower (V * Vcontrol, slip) * scale;
}

double motorLoad::getReactivePower (double V) const
{
  double slip = m_state[0];
  return qPower (V * Vcontrol, slip) * scale;
}


double motorLoad::mechPower (double slip) const
{
  double Tm = alpha + beta * slip + gamma * slip * slip;
  return Tm;
}

double motorLoad::dmechds (double slip) const
{
  double Tmds = beta + 2 * gamma * slip;
  return Tmds;
}

double motorLoad::computeSlip (double Ptarget) const
{
  if (gamma == 0)
    {
	  return (beta == 0) ? 0.05 : (Ptarget - alpha) / beta;
    }
  else
    {
      double out1 = (-beta + std::sqrt (beta * beta - 4.0 * gamma * (alpha - Ptarget))) / (2.0 * gamma);
      double out2 = (-beta - std::sqrt (beta * beta - 4.0 * gamma * (alpha - Ptarget))) / (2.0 * gamma);

      if ((out1 >= 0)&&(out1 <= 1.0))
        {
          return out1;
        }
      else
        {
          return out2;
        }
    }
}


double motorLoad::rPower (double vin, double slip) const
{
  double out = r1 * vin * vin * slip / (r1 * r1 + slip * slip * (x + x1) * (x + x1));
  return out;
}
double motorLoad::qPower (double vin, double slip) const
{
  double xs2 = (x + x1) * slip * slip;
  double out = vin * vin * (1.0 / xm +  xs2 / (r1 * r1 + xs2 * (x + x1)));
  return out;
}
