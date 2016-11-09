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

#include "loadModels/gridLoad.h"
#include "loadModels/otherLoads.h"
#include "objectFactoryTemplates.h"
#include "core/gridDynExceptions.h"
#include "gridBus.h"
#include "gridCoreTemplates.h"
#include "matrixData.h"

#include <iostream>
#include <cmath>

static gridBus defBus (1.0,0);

using namespace gridUnits;

//setup the load object factories
static typeFactory<gridLoad> glf ("load", stringVec { "basic", "zip" }, "basic"); //set basic to the default
static childTypeFactory<gridPulseLoad, gridLoad> glfp ("load", "pulse");
static childTypeFactory<gridSineLoad, gridLoad> cfgsl ("load", stringVec {"sine","sin","sinusoidal"});
static childTypeFactory<gridRampLoad, gridLoad> glfr ("load", "ramp");
static childTypeFactory<gridRandomLoad, gridLoad> glfrand ("load", stringVec {"random","rand"});
static childTypeFactory<gridFileLoad, gridLoad> glfld ("load", "file");

static childTypeFactory<exponentialLoad,gridLoad> glexp ("load", "exponential");
static childTypeFactory<fDepLoad,gridLoad> glfd ("load", "fdep");


std::atomic<count_t> gridLoad::loadCount(0);
gridLoad::gridLoad (const std::string &objName) : gridSecondary (objName),bus (&defBus)
{
  constructionHelper ();
}

gridLoad::gridLoad (double rP, double rQ, const std::string &objName) : gridSecondary (objName), bus (&defBus),P (rP),Q (rQ)
{
  constructionHelper ();

}

void gridLoad::constructionHelper ()
{
  // default values
  ++loadCount;
  id = loadCount;
  updateName ();
}

gridLoad::~gridLoad ()
{
}

void gridLoad::setTime (double time)
{
  prevTime = time;
  lastTime = time;
}
gridCoreObject *gridLoad::clone (gridCoreObject *obj) const
{
  gridLoad *nobj = cloneBaseFactory<gridLoad, gridSecondary> (this, obj, &glf);
  if (!(nobj) )
    {
      return obj;
    }
  nobj->Psched = Psched;
  nobj->P = P;
  nobj->Q = Q;
  nobj->Yp = Yp;
  nobj->Yq = Yq;
  nobj->Iq = Iq;
  nobj->Ip = Ip;
  nobj->r = r;
  nobj->x = x;
  nobj->pfq = pfq;
  nobj->Pout = Pout;
  nobj->dPdf = dPdf;
  nobj->M = M;
  nobj->H = H;
  nobj->Vpqmax = Vpqmax;
  nobj->Vpqmin = Vpqmin;
  nobj->lastTime = lastTime;
  return nobj;
}

void gridLoad::pFlowObjectInitializeA (double /*time0*/, unsigned long /*flags*/)
{
  bus = static_cast<gridBus *> (find ("bus"));
  if (!bus)
    {
      bus = &defBus;
    }
  Psched = getRealPower ();
  dPdf = -H / 30 * Psched;

#ifdef SGS_DEBUG
  std::cout << "SGS : " << prevTime << " : " << name << " gridLoad::pFlowInitializeA realPower = " << getRealPower () << " reactive power = " << getReactivePower () << '\n';
#endif

}


void gridLoad::setLoad (double level, units_t unitType )
{
  P = unitConversion (level,unitType,puMW,systemBasePower);
}

void gridLoad::setLoad (double Plevel, double Qlevel,units_t unitType)
{
  P = unitConversion (Plevel, unitType, puMW, systemBasePower);
  Q = unitConversion (Qlevel, unitType, puMW, systemBasePower);
}

void gridLoad::dynObjectInitializeA (double /*time0*/, unsigned long flags)
{
  double V;
  if ((opFlags[convert_to_constant_impedance])||CHECK_CONTROLFLAG (flags,all_loads_to_constant_impedence))
    {
      V = bus->getVoltage ();

      Yp = Yp + P / (V * V);
      P = 0;
      if (opFlags[use_power_factor_flag])
        {
          Yq = Yq + P * pfq / (V * V);
          Q = 0;
        }
      else
        {
          Yq = Yq + Q / (V * V);
          Q = 0;
        }
      r = (Yq == 0) ? 1 / Yp : Yp / Yq * x;
      x = (Yp == 0) ? 1 / Yq : Yq / Yp * r;
    }

#ifdef SGS_DEBUG
  std::cout << "SGS : " << prevTime << " : " << name << " gridLoad::dynInitializeA realPower = " << getRealPower () << " reactive power = " << getReactivePower () << '\n';
#endif

}

void gridLoad::setState (double ttime, const double /*state*/[], const double /*dstate_dt*/ [], const solverMode &)
{
  if (ttime != prevTime)
    {
      loadUpdateForward (ttime);
    }
}

void gridLoad::timestep (double ttime, const IOdata &args, const solverMode &)
{
  if (!enabled)
    {
      Pout = 0;
      dPdf = 0;
      return;
    }
  if (ttime != prevTime)
    {
      loadUpdateForward (ttime);
    }

  double freq = (args.size () > 2) ? args[frequencyInLocation] : 1.0;
  double V = (args.empty ()) ? (bus->getVoltage ()) : args[voltageInLocation];
  Pout = -getRealPower (V);
  prevTime = ttime;
  //Pout+=Pout*(M*freq)-2*df/60.0*H*Psched;
  Pout -= Pout * (M * (freq - 1.0));
  dPdf = -H / 30 * Pout;
  Qout = -getReactivePower (V);
  Qout -= Qout * (M * (freq - 1.0));

#ifdef SGS_DEBUG
  std::cout << "SGS : " << prevTime << " : " << name << " gridLoad::timestep realPower = " << getRealPower () << " reactive power = " << getReactivePower () << '\n';
#endif
  prevTime = ttime;
}


static const stringVec locNumStrings {
  "p", "q", "yp", "yq", "ip", "iq", "x", "r", "pf", "h", "m", "vpqmin", "vpqmax"
};

static const stringVec locStrStrings {

};

static const stringVec flagStrings {
  "usepowerfactor", "converttoimpedance", "no_pqvoltage_limit"
};

void gridLoad::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
  getParamString<gridLoad, gridObject> (this, pstr, locNumStrings, locStrStrings, flagStrings, pstype);
}

void gridLoad::setFlag (const std::string &flag, bool val)
{
  if (flag == "usepowerfactor")
    {
      if (val)
        {
          if (!(opFlags[use_power_factor_flag]))
            {
              opFlags.set (use_power_factor_flag);
              if (P != 0)
                {
                  pfq = Q / P;
                }
              else
                {
                  pfq = kBigNum;
                }
            }
        }
      else
        {
          opFlags.reset (use_power_factor_flag);
        }

    }
  else if (flag == "converttoimpedance")
    {
      opFlags.set (convert_to_constant_impedance,val);
    }
  else if (flag == "no_pqvoltage_limit")
    {
      opFlags.set (no_pqvoltage_limit,val);
      if (opFlags[no_pqvoltage_limit])
        {
          Vpqmax = 100;
          Vpqmin = -1.0;
        }
    }
  else
    {
      gridSecondary::setFlag (flag, val);
    }
 
}

// set properties
void gridLoad::set (const std::string &param,  const std::string &val)
{

  if (param[0] == '#')
    {
    }
  else
    {
      gridSecondary::set (param, val);
    }

}

double gridLoad::get (const std::string &param, units_t unitType) const
{
  double val = kNullVal;
  if (param.length () == 1)
    {
      switch (param[0])
        {
        case 'p':
          val = unitConversion (P, puMW, unitType, systemBasePower);
          break;
        case 'q':
          val = unitConversion (Q, puMW, unitType, systemBasePower);
          break;
        case 'r':
          val = r;
          break;
        case 'x':
          val = x;
          break;
        case 'z':
          val = 1 / x;
          break;
        case 'y':
        case 'g':
          val = 1 / r;
          break;
		default:
			break;
        }
      return val;
    }
  if (param == "yp")
    {
      val = unitConversion (Yp, puMW, unitType, systemBasePower);
    }
  else if (param == "yq")
    {
      val = unitConversion (Yq, puMW, unitType, systemBasePower);
    }
  else if (param == "ip")
    {
      val = unitConversion (Ip, puMW, unitType, systemBasePower);
    }
  else if (param == "iq")
    {
      val = unitConversion (Iq, puMW, unitType, systemBasePower);
    }
  else if (param == "pf")
    {
      val = pfq;
    }
  else
    {
      val = gridSecondary::get (param, unitType);
    }
  return val;
}

void gridLoad::set (const std::string &param, double val, units_t unitType)
{
  if (param.length () == 1)
    {
      switch (param[0])
        {

        case 'p':
          P = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
          if (opFlags[use_power_factor_flag])
            {
              if (pfq > 1000.0)                          //if the pfq is screwy, recalculate, otherwise leave it the same.
                {
                  if (P != 0)
                    {
                      pfq = Q / P;
                    }

                }
            }
          break;
        case 'q':
          Q = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
          if (opFlags[use_power_factor_flag])
            {
              if (P != 0)
                {
                  pfq = Q / P;

                }
              else
                {
                  pfq = kBigNum;
                }
            }
          break;
        case 'r':
          r = unitConversion (val, unitType, puOhm, systemBasePower, baseVoltage);
          Yp = r / (r * r + x * x);
          Yq = x / (r * r + x * x);
          break;
        case 'x':
          x = unitConversion (val, unitType, puOhm, systemBasePower, baseVoltage);
          Yp = r / (r * r + x * x);
          Yq = x / (r * r + x * x);
          break;
        case 'g':
          if (val != 0.0)
            {
              Yp = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
              r = (Yq == 0) ? 1 / Yp : Yp / Yq * x;
            }
          else
            {
              Yp = 0.0;
              r = kBigNum;
            }
          break;
        case 'b':
          if (val != 0.0)
            {
              Yq = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
              x = (Yp == 0) ? 1 / Yq : Yq / Yp * r;
            }
          else
            {
              Yq = 0.0;
              x = 0.0;
            }
          break;
        case 'h':
          H = val;
          break;
        case 'm':
          M = val;
          break;
        default:
			throw(unrecognizedParameter());
        }
      checkFaultChange ();
	  return;

    }
  if (param.back()=='+') //load increments
  {
	  //load increments  allows a delta on the load through the set functions
	  if (param == "p+")
	  {
		  P += unitConversion(val, unitType, puMW, systemBasePower, baseVoltage);
		  if (opFlags[use_power_factor_flag])
		  {
			  if (pfq > 1000.0)                          //if the pfq is screwy, recalculate, otherwise leave it the same.
			  {
				  if (P != 0)
				  {
					  pfq = Q / P;
				  }

			  }
		  }
	  }
	  else if (param == "q+")
	  {
		  Q += unitConversion(val, unitType, puMW, systemBasePower, baseVoltage);
		  if (opFlags[use_power_factor_flag])
		  {
			  if (P != 0)
			  {
				  pfq = Q / P;

			  }
			  else
			  {
				  pfq = kBigNum;
			  }
		  }
	  }
	  else if ((param == "yp+") || (param == "zr+"))
	  {
		  Yp += unitConversion(val, unitType, puMW, systemBasePower, baseVoltage);
		  if (Yp != 0)
		  {
			  r = (Yq == 0) ? 1 / Yp : Yp / Yq * x;
		  }
		  else
		  {
			  r = kBigNum;
		  }
		  checkFaultChange();
	  }
	  else if ((param == "yq+") || (param == "zq+"))
	  {
		  Yq += unitConversion(val, unitType, puMW, systemBasePower, baseVoltage);
		  if (Yq != 0)
		  {
			  x = (Yp == 0) ? 1 / Yq : Yq / Yp * r;
		  }
		  else
		  {
			  x = 0.0;
		  }
		  checkFaultChange();
	  }
	  else if ((param == "ir+") || (param == "ip+"))
	  {
		  Ip += unitConversion(val, unitType, puA, systemBasePower, baseVoltage);
		  checkFaultChange();
	  }
	  else if (param == "iq+")
	  {
		  Iq += unitConversion(val, unitType, puA, systemBasePower, baseVoltage);
		  checkFaultChange();
	  }
	  else
	  {
		  gridSecondary::set(param, val, unitType);
	  }
  }
  else if (param.back() == '*')
  {
	  //load increments  allows a delta on the load through the set functions
	  if (param == "p*")
	  {
		  P *= val;
		  if (opFlags[use_power_factor_flag])
		  {
			  if (pfq > 1000.0)                          //if the pfq is screwy, recalculate, otherwise leave it the same.
			  {
				  if (P != 0)
				  {
					  pfq = Q / P;
				  }

			  }
		  }
	  }
	  else if (param == "q*")
	  {
		  Q *= val;
		  if (opFlags[use_power_factor_flag])
		  {
			  if (P != 0)
			  {
				  pfq = Q / P;

			  }
			  else
			  {
				  pfq = kBigNum;
			  }
		  }
	  }
	  else if ((param == "yp*") || (param == "zr*"))
	  {
		  Yp *= val;
		  if (Yp != 0)
		  {
			  r = (Yq == 0) ? 1 / Yp : Yp / Yq * x;
		  }
		  else
		  {
			  r = kBigNum;
		  }
		  checkFaultChange();
	  }
	  else if ((param == "yq*") || (param == "zq*"))
	  {
		  Yq *= val;
		  if (Yq != 0)
		  {
			  x = (Yp == 0) ? 1 / Yq : Yq / Yp * r;
		  }
		  else
		  {
			  x = 0.0;
		  }
		  checkFaultChange();
	  }
	  else if ((param == "ir*") || (param == "ip*"))
	  {
		  Ip *= val;
		  checkFaultChange();
	  }
	  else if (param == "iq*")
	  {
		  Iq *= val;
		  checkFaultChange();
	  }
	  else
	  {
		  gridSecondary::set(param, val, unitType);
	  }
  }
  else if (param == "load p")
    {
      P = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      if (opFlags[use_power_factor_flag])
        {
          if (pfq > 1000.0)              //if the pfq is screwy then recalculate otherwise leave it the same.
            {
              if (P != 0)
                {
                  pfq = Q / P;
                }

            }
        }
      checkFaultChange ();
    }
  else if  (param == "load q")
    {
      Q = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      if (opFlags[use_power_factor_flag])
        {
          if (P != 0)
            {
              pfq = Q / P;

            }
          else
            {
              pfq = kBigNum;
            }
        }
      checkFaultChange ();
    }
  else if ((param == "yp") || (param == "shunt g") || (param == "zr"))
    {
      if (val != 0.0)
        {
          Yp = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
          r = (Yq == 0) ? 1 / Yp : Yp / Yq * x;
        }
      else
        {
          Yp = 0.0;
          r = kBigNum;
        }
      checkFaultChange ();
    }
  else if ((param == "yq") || (param == "shunt b") ||(param == "zq"))
    {
      if (val != 0.0)
        {
          Yq = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
          x = (Yp == 0) ? 1 / Yq : Yq / Yp * r;
        }
      else
        {
          Yq = 0.0;
          x = 0.0;
        }
      checkFaultChange ();
    }
  else if ((param == "ir")||(param == "ip"))
    {
      Ip = unitConversion (val, unitType, puA, systemBasePower, baseVoltage);
      checkFaultChange ();
    }
  else if (param == "iq")
    {
      Iq = unitConversion (val, unitType, puA, systemBasePower, baseVoltage);
      checkFaultChange ();
    }
  else if ((param == "pf") || (param == "powerfactor"))
    {
      if (val != 0.0)
        {
          if (std::abs (val) <= 1.0)
            {
              pfq = std::sqrt (1.0 - val * val) / val;
            }
          else
            {
              pfq = 0.0;
            }
        }
      else
        {
          pfq = kBigNum;
        }
      opFlags.set (use_power_factor_flag);
    }
  else if (param == "qratio")
    {
      pfq = val;
      opFlags.set (use_power_factor_flag);
    }
  else if (param == "vpqmin")
    {

      if (!opFlags[no_pqvoltage_limit])
        {
          Vpqmin = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
          trigVVlow = 1.0 / (Vpqmin * Vpqmin);

        }
    }
  else if (param == "vpqmax")
    {
      if (!opFlags[no_pqvoltage_limit])
        {
          Vpqmax = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
          trigVVhigh = 1.0 / (Vpqmax * Vpqmax);

        }
    }
  else if (param == "pqlowvlimit")       //this is mostly a convenience flag for adaptive solving
    {

      if (val > 0.1) //not a flag
        {
          if (Vpqmin < 0.5)
            {
              if (!opFlags[no_pqvoltage_limit])
                {
                  Vpqmin = 0.9;
                  trigVVlow = 1.0 / (Vpqmin * Vpqmin);
                }
            }
        }
    }
  // SGS added to set the base voltage 2015-01-30
  else if ((param == "basevoltage") || (param == "base vol"))
    {
      baseVoltage = val;
    }
  else
    {
      gridSecondary::set (param, val, unitType);
    }
}

void gridLoad::checkFaultChange ()
{
  if (opFlags[pFlow_initialized])
    {
      if (bus->getVoltage () < 0.05)                        //we are in fault condition
        {
          alert (this, POTENTIAL_FAULT_CHANGE);
        }
    }
}


double gridLoad::getRealPower () const
{
  double val = P;
  double V;

  if (isConnected ())
    {
      V = bus->getVoltage ();
      if (V < Vpqmin)
        {
          val = V * V * val * trigVVlow;
        }
      else if (V > Vpqmax)
        {
          val = V * V * val * trigVVhigh;
        }
      val += V * (V * Yp + Ip);
      return val;
    }
  else
    {
      return 0;
    }
}

double gridLoad::getReactivePower () const
{
  if (isConnected ())
    {
      double val = Q;

      if (opFlags[use_power_factor_flag])
        {
          if (pfq < 1000.0)
            {
              val = P * pfq;
            }

        }
      double V;

      V = bus->getVoltage ();
      if (V < Vpqmin)
        {
          val = V * V * val * trigVVlow;
        }
      else if (V > Vpqmax)
        {
          val = V * V * val * trigVVhigh;
        }
      val += V * (V * Yq + Iq);
      return val;
    }
  else
    {
      return 0.0;
    }
}

double gridLoad::getRealPower (const IOdata &args, const stateData *sD, const solverMode &sMode)
{
  if (isConnected ())
    {
      if ((sD) && (sD->time != lastTime))
        {
          loadUpdate (sD->time);
        }
      double V = (args.empty ()) ? (bus->getVoltage ((sD) ? (sD->state) : nullptr, sMode)) : args[voltageInLocation];
      double val = P;

      if (V < Vpqmin)
        {
          val = V * V * val * trigVVlow;
        }
      else if (V > Vpqmax)
        {
          val = V * V * val * trigVVhigh;
        }
      val += V * (V * Yp + Ip);
      return val;
    }
  else
    {
      return 0;
    }
}

double gridLoad::getReactivePower (const IOdata &args, const stateData *sD, const solverMode &sMode)
{

  if ((sD)&&(sD->time != lastTime))
    {
      loadUpdate (sD->time);
    }
  double V = (args.empty ()) ? (bus->getVoltage ((sD) ? (sD->state) : nullptr, sMode)) : args[voltageInLocation];
  double val = Q;
  if (opFlags[use_power_factor_flag])
    {
      if (pfq < 1000.0)
        {
          val = P * pfq;
        }
    }
  if (enabled)
    {
      if (V < Vpqmin)
        {
          val = V * V * val * trigVVlow;
        }
      else if (V > Vpqmax)
        {
          val = V * V * val * trigVVhigh;
        }
      val += V * (V * Yq + Iq);
      return val;
    }
  else
    {
      return 0;
    }
}

double gridLoad::getRealPower (const double V) const
{
  double val = P;
  if (isConnected ())
    {
      if (V < Vpqmin)
        {
          val = V * V * val * trigVVlow;
        }
      else if (V > Vpqmax)
        {
          val = V * V * val * trigVVhigh;
        }
      val += V * (V * Yp + Ip);

      return val;
    }
  else
    {
      return 0;
    }
}

double gridLoad::getReactivePower (double V) const
{
  double val = Q;
  if (opFlags[use_power_factor_flag])
    {
      if (pfq < 1000.0)
        {
          val = P * pfq;
        }
    }
  if (isConnected ())
    {
      if (V < Vpqmin)
        {
          val = V * V * val * trigVVlow;
        }
      else if (V > Vpqmax)
        {
          val = V * V * val * trigVVhigh;
        }
      val += V * (V * Yq + Iq);
      return val;
    }
  else
    {
      return 0;
    }
}

//TODO:: PT remove this function it is duplicative of the one in gridSecondary
IOdata gridLoad::getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode)
{
  IOdata output {
    0.0,0.0
  };
  if (isConnected ())
    {
      output[PoutLocation] = getRealPower (args,sD,sMode);
      output[QoutLocation] = getReactivePower (args, sD, sMode);
    }
  return output;
}

void gridLoad::outputPartialDerivatives (const IOdata &args, const stateData *sD, matrixData<double> *ad, const solverMode &sMode)
{
  if (args.empty ())  //we only have output derivatives if the input arguments are not counted
    {
      auto argsBus = bus->getOutputs (sD,sMode);
      auto argLocs = bus->getOutputLocs (sMode);
      ioPartialDerivatives (argsBus,sD,ad,argLocs,sMode);
    }
}

void gridLoad::ioPartialDerivatives (const IOdata &args, const stateData *sD, matrixData<double> *ad, const IOlocs &argLocs, const solverMode &)
{
  if  ((sD)&&(sD->time != lastTime))
    {
      loadUpdate (sD->time);
    }
  double V = args[voltageInLocation];
  double tv = 0.0;
  if (V < Vpqmin)
    {
      tv = trigVVlow;
    }
  else if (V > Vpqmax)
    {
      tv = trigVVhigh;
    }


  ad->assignCheckCol (PoutLocation, argLocs[voltageInLocation], 2.0 * V * Yp + Ip + 2.0 * V * P * tv);

  if (opFlags[use_power_factor_flag])
    {
      if (pfq < 1000.0)
        {
          ad->assignCheckCol (QoutLocation, argLocs[voltageInLocation], 2.0 * V * Yq + Iq + 2.0 * V * P * pfq * tv);
        }
      else
        {
          ad->assignCheckCol (QoutLocation, argLocs[voltageInLocation], 2.0 * V * Yq + Iq + 2.0 * V * Q * tv);
        }
    }
  else
    {
      ad->assignCheckCol (QoutLocation, argLocs[voltageInLocation], 2.0 * V * Yq + Iq + 2.0 * V * Q * tv);
    }

}

bool compareLoad (gridLoad *ld1, gridLoad *ld2, bool /*printDiff*/)
{
  bool cmp = true;
  if (std::abs (ld1->r - ld2->r) > 0.00001)
    {
      cmp = false;
    }

  if (std::abs (ld1->x - ld2->x) > 0.00001)
    {
      cmp = false;
    }
  if ((ld1->opFlags.to_ullong () & flagMask) != (ld2->opFlags.to_ullong () & flagMask))
    {
      cmp = false;
    }
  if (std::abs (ld1->P - ld2->P) > 0.00001)
    {
      cmp = false;
    }

  if (std::abs (ld1->Q - ld2->Q) > 0.00001)
    {
      cmp = false;
    }
  if (std::abs (ld1->pfq - ld2->pfq) > 0.00001)
    {
      cmp = false;
    }
  if (std::abs (ld1->Ip - ld2->Ip) > 0.00001)
    {
      cmp = false;
    }

  if (std::abs (ld1->Iq - ld2->Iq) > 0.00001)
    {
      cmp = false;
    }
  if (std::abs (ld1->Yp - ld2->Yp) > 0.00001)
    {
      cmp = false;
    }

  if (std::abs (ld1->Yq - ld2->Yq) > 0.00001)
    {
      cmp = false;
    }
  return cmp;
}
