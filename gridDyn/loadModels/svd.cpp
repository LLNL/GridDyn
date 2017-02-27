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

#include "svd.h"
#include "core/objectFactoryTemplates.h"
#include "core/coreObjectTemplates.h"
#include "stringConversion.h"
#include <cmath>

static typeFactory<svd> svdld ("load", stringVec {"svd", "switched shunt","switchedshunt","ssd"});

using namespace gridUnits;


svd::svd (const std::string &objName) : gridRampLoad (objName)
{

}

svd::svd (double rP, double rQ, const std::string &objName) : gridRampLoad (rP,rQ,objName)
{
  opFlags.set (adjustable_Q);
}


svd::~svd ()
{
}

coreObject *svd::clone (coreObject *obj) const
{
  svd *ld = cloneBase<svd, gridRampLoad> (this, obj);
  if (ld == nullptr)
    {
      return obj;
    }

  ld->Qmin = Qmin;
  ld->Qmax = Qmax;
  ld->Vmin = Vmin;
  ld->Vmax = Vmax;

  ld->Qlow = Qlow;
  ld->Qhigh = Qhigh;
  ld->currentStep = currentStep;
  ld->stepCount = stepCount;
  ld->Cblocks = Cblocks;
  ld->participation = participation;
  return ld;
}

void svd::setControlBus (gridBus *cBus)
{
  if (cBus)
    {
      controlBus = cBus;
    }
}

void svd::setLoad (double level, units_t unitType)
{
  double dlevel = unitConversion (level,unitType,puMW,systemBasePower);
  int setLevel = checkSetting (dlevel);
  if (setLevel >= 0)
    {
      setYq(dlevel);
    }

}

void svd::setLoad (double Plevel, double Qlevel, units_t unitType)
{
  setYp(unitConversion (Plevel,unitType,puMW,systemBasePower));
  double dlevel = unitConversion (Qlevel, unitType, puMW, systemBasePower);
  int setLevel = checkSetting (dlevel);
  if (setLevel >= 0)
    {
      setYq(dlevel);
    }
}

int svd::checkSetting (double level)
{
  if (level == 0)
    {
      return 0;
    }
  if (opFlags[continuous_flag])
    {
      if ((level >= Qlow)&&(level <= Qhigh))
        {
          return 1;
        }
      else
        {
          return -1;
        }
    }
  else
    {
      int setting = 0;
      auto block = Cblocks.begin ();
      double totalQ = Qlow;
      while (std::abs (totalQ) < std::abs (level))
        {
          for (int kk = 0; kk < (*block).first; ++kk)
            {
              totalQ += (*block).second;
              ++setting;
              if (std::abs (totalQ - level) < 0.00001)
                {
                  return setting;
                }
            }
          ++block;
          if (block == Cblocks.end ())
            {
              break;
            }
        }
      if (std::abs (totalQ) > std::abs (level))
        {
          LOG_WARNING ("unable to match requested level");
        }

      return setting;
    }
}

void svd::updateSetting (int step)
{
  if (step <= 0)
    {
      currentStep = checkSetting (step);
      setYq(Qlow);
    }
  else if (step >= stepCount)
    {
      currentStep = stepCount;
      setYq(Qhigh);
    }
  else
    {
      auto block = Cblocks.begin ();
      int scount = 0;
      double qlevel = Qlow;
      while (step > scount + (*block).first)
        {
          scount += (*block).first;
          qlevel += (*block).second;
          ++block;
          if (block == Cblocks.end ())
            {
              break;
            }
        }
      qlevel += (step - scount) * (*block).second;
      currentStep = step;
      setYq(qlevel);
    }
}

void svd::pFlowObjectInitializeA (coreTime time0, unsigned long flags)
{
  if (opFlags[continuous_flag])
    {
      if (!opFlags[locked_flag])
        {
          opFlags.set (has_pflow_states);
          opFlags.set (has_powerflow_adjustments);
        }
    }
  else
    {
      if (!opFlags[locked_flag])
        {
          opFlags.set (has_powerflow_adjustments);
        }
    }
  return zipLoad::pFlowObjectInitializeA (time0, flags);
}

void svd::dynObjectInitializeA (coreTime time0,  unsigned long flags)
{
  return zipLoad::dynObjectInitializeA (time0, flags);
}

void svd::dynObjectInitializeB (const IOdata & /*inputs*/, const IOdata & /*desiredOutput*/, IOdata &/*fieldSet*/)
{

}

void svd::setState (coreTime /*ttime*/, const double /*state*/[], const double /*dstate_dt*/[], const solverMode &)
{
}

void svd::guess (coreTime /*ttime*/, double /*state*/[], double /*dstate_dt*/[], const solverMode & /*sMode*/)
{
}

change_code svd::powerFlowAdjust (const IOdata & /*inputs */, unsigned long /*flags*/, check_level_t /*level*/)
{
  return change_code::no_change;
}

void svd::reset (reset_levels /*level*/)
{
}

//for identifying which variables are algebraic vs differential
void svd::getVariableType (double /*sdata*/[], const solverMode & /*sMode*/)
{
}

void svd::set (const std::string &param,  const std::string &val)
{
  if ((param == "blocks")||(param == "block"))
    {
      auto bin = stringOps::splitline (val);
      for (size_t kk = 0; kk < bin.size () - 1; ++kk)
        {
          int cnt = numeric_conversion<int> (bin[kk],0);
          double bsize = numeric_conversion (bin[kk + 1],0.0);
          if (cnt > 0)
            {
              addBlock (cnt,bsize);
            }
        }
    }
  else if (param == "mode")
    {
      auto v2 = convertToLowerCase (val);
      if ((v2 == "manual")||(v2 == "locked"))
        {
          opFlags.set (locked_flag);
        }
      if ((v2 == "cont")||(v2 == "continuous"))
        {
          opFlags.set (continuous_flag, true);
          opFlags.reset (locked_flag);
        }
      else if ((v2 == "stepped")||(v2 == "discrete"))
        {
          opFlags.reset (continuous_flag);
          opFlags.reset (locked_flag);
        }
    }
  else if (param == "control")
    {
      auto v2 = convertToLowerCase (val);
      if (v2 == "reactive")
        {
          opFlags.set (reactive_control_flag, true);
        }
    }
  else
    {
      zipLoad::set (param,val);
    }


}
void svd::set (const std::string &param, double val, units_t unitType)
{


  if (param == "qlow")
    {
      Qlow = unitConversion (val,unitType,puMW,systemBasePower,baseVoltage);
    }
  else if (param == "qhigh")
    {
      Qhigh = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "qmin")
    {
      Qmin = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  if (param == "qmax")
    {
      Qmax = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "vmax")
    {
      Vmax = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
    }
  else if (param == "vmin")
    {
      Vmin = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
    }
  else if (param == "yq")
    {
      double temp = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      setLoad (temp);
    }
  else if (param == "step")
    {
      updateSetting (static_cast<int> (val));
    }
  else if (param == "participation")
    {
      participation = val;
    }
  else if (param == "block")
    {
      if (Cblocks.size () == 1)
        {
          if (Cblocks[0].second == 0)
            {
              Cblocks[0].second = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
              Qhigh = Qlow + Cblocks[0].first * Cblocks[0].second;
              stepCount = Cblocks[0].first;
            }
          else
            {
              addBlock (1,val,unitType);
            }
        }
      else
        {
          addBlock (1, val, unitType);
        }
    }
  else if (param == "count")
    {
      if (Cblocks.size () < 2)
        {
          if (Cblocks.empty ())
            {
              addBlock (static_cast<int> (val),0);
            }
          else
            {
              Cblocks[0].first = static_cast<int> (val);
              Qhigh = Qlow + Cblocks[0].first * Cblocks[0].second;
              stepCount = Cblocks[0].first;
            }
        }
    }
  else if (param.substr (0,5) == "block")
    {

    }
  else if (param.substr (0, 5) == "count")
    {

    }
  else
    {
      zipLoad::set (param, val, unitType);
    }


}

void svd::addBlock (int steps, double Qstep, gridUnits::units_t unitType)
{
  Qstep = gridUnits::unitConversion (Qstep,unitType,gridUnits::puMW,systemBasePower);
  Cblocks.push_back (std::make_pair (steps,Qstep));
  Qhigh += steps * Qstep;
  stepCount += steps;
}

void svd::residual (const IOdata & /*inputs*/, const stateData &, double /*resid*/[], const solverMode &)
{
}

void svd::derivative (const IOdata & /*inputs*/, const stateData &, double /*deriv*/[], const solverMode &)
{
}

void svd::outputPartialDerivatives (const IOdata & /*inputs*/, const stateData &, matrixData<double> &, const solverMode &)
{
}

void svd::jacobianElements (const IOdata & /*inputs*/, const stateData &, matrixData<double> &, const IOlocs & /*inputLocs*/, const solverMode &)
{
}
void svd::getStateName  (stringVec & /*stNames*/, const solverMode &, const std::string & /*prefix*/) const
{

}

void svd::timestep (coreTime /*ttime*/, const IOdata & /*inputs*/, const solverMode &)
{
  
}

void svd::rootTest (const IOdata & /*inputs*/, const stateData &, double /*roots*/[], const solverMode &)
{
}

void svd::rootTrigger (coreTime /*ttime*/, const IOdata & /*inputs*/, const std::vector<int> & /*rootMask*/, const solverMode &)
{
}

change_code svd::rootCheck ( const IOdata & /*inputs*/, const stateData &, const solverMode &, check_level_t /*level*/)
{
  return change_code::no_change;
}
