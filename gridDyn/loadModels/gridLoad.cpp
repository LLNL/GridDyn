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
#include <complex>

static gridBus defBus (1.0,0);

using namespace gridUnits;

//setup the load object factories
static typeFactory<gridLoad> glf ("load", stringVec { "basic", "zip" }, "basic"); //set basic to the default
static typeFactoryArg<sourceLoad, sourceLoad::sourceType> glfp ("load", "pulse", sourceLoad::sourceType::pulse);
static typeFactoryArg<sourceLoad, sourceLoad::sourceType> cfgsl ("load", stringVec {"sine","sin","sinusoidal"}, sourceLoad::sourceType::sine);
static childTypeFactory<gridRampLoad, gridLoad> glfr ("load", "ramp");
static typeFactoryArg<sourceLoad, sourceLoad::sourceType> glfrand ("load", stringVec {"random","rand"}, sourceLoad::sourceType::random);
static childTypeFactory<gridFileLoad, gridLoad> glfld ("load", "file");
static childTypeFactory<sourceLoad, gridLoad> srcld("load", stringVec{ "src","source" });
static childTypeFactory<exponentialLoad, gridLoad> glexp("load", stringVec{ "exponential","exp" });
static childTypeFactory<fDepLoad,gridLoad> glfd ("load", "fdep");


std::atomic<count_t> gridLoad::loadCount(0);
gridLoad::gridLoad (const std::string &objName) : gridSecondary (objName),bus (&defBus)
{
  constructionHelper ();
}

gridLoad::gridLoad (double rP, double rQ, const std::string &objName) : gridSecondary (objName),P (rP),Q (rQ), bus(&defBus)
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


coreObject *gridLoad::clone (coreObject *obj) const
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
  nobj->pfq = pfq;
  nobj->Pout = Pout;
  nobj->dPdf = dPdf;
  nobj->M = M;
  nobj->H = H;
  nobj->Vpqmax = Vpqmax;
  nobj->Vpqmin = Vpqmin;
  return nobj;
}

void gridLoad::pFlowObjectInitializeA (gridDyn_time time0, unsigned long /*flags*/)
{
  bus = static_cast<gridBus *> (find ("bus"));
  if (!bus)
    {
      bus = &defBus;
    }
  Psched = getRealPower ();
  dPdf = -H / 30 * Psched;
  lastTime = time0;
#ifdef SGS_DEBUG
  std::cout << "SGS : " << prevTime << " : " << name << " gridLoad::pFlowInitializeA realPower = " << getRealPower () << " reactive power = " << getReactivePower () << '\n';
#endif

}


void gridLoad::setLoad (double level, units_t unitType )
{
  setP(unitConversion (level,unitType,puMW,systemBasePower));
}

void gridLoad::setLoad (double Plevel, double Qlevel,units_t unitType)
{
  setP(unitConversion (Plevel, unitType, puMW, systemBasePower));
  setQ(unitConversion (Qlevel, unitType, puMW, systemBasePower));
}

void gridLoad::dynObjectInitializeA (gridDyn_time /*time0*/, unsigned long flags)
{
  if ((opFlags[convert_to_constant_impedance])||CHECK_CONTROLFLAG (flags,all_loads_to_constant_impedence))
    {
      double V = bus->getVoltage ();

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
    }

#ifdef SGS_DEBUG
  std::cout << "SGS : " << prevTime << " : " << name << " gridLoad::dynInitializeA realPower = " << getRealPower () << " reactive power = " << getReactivePower () << '\n';
#endif

}


void gridLoad::timestep (gridDyn_time ttime, const IOdata &args, const solverMode &)
{
  if (!isConnected())
    {
      Pout = 0;
      dPdf = 0;
      return;
    }
  if (ttime != prevTime)
    {
	  updateLocalCache(args, stateData(ttime), cLocalSolverMode);
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
			  updatepfq();
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
          val = getr();
          break;
        case 'x':
          val = getx();
          break;
        case 'z':
          val = std::abs(std::complex<double>(getr(),getx()));
          break;
        case 'y':
			val = std::abs(1.0/std::complex<double>(getr(), getx()));
			break;
        case 'g':
          val = 1 / getr();
		  break;
		case 'b':
			val = 1.0 / getx();
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
          setP(unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
          break;
        case 'q':
          setQ(unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
          break;
        case 'r':
          setr(unitConversion (val, unitType, puOhm, systemBasePower, baseVoltage));
          break;
        case 'x':
          setx(unitConversion (val, unitType, puOhm, systemBasePower, baseVoltage));
          break;
        case 'g':
              setYp(unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
          break;
        case 'b':
              setYq(unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
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
		  checkpfq();
	  }
	  else if (param == "q+")
	  {
		  Q += unitConversion(val, unitType, puMW, systemBasePower, baseVoltage);
		  updatepfq();
	  }
	  else if ((param == "yp+") || (param == "zr+"))
	  {
		  Yp += unitConversion(val, unitType, puMW, systemBasePower, baseVoltage);
		  checkFaultChange();
	  }
	  else if ((param == "yq+") || (param == "zq+"))
	  {
		  Yq += unitConversion(val, unitType, puMW, systemBasePower, baseVoltage);
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
		  checkpfq();
	  }
	  else if (param == "q*")
	  {
		  Q *= val;
		  updatepfq();
	  }
	  else if ((param == "yp*") || (param == "zr*"))
	  {
		  Yp *= val;
		  checkFaultChange();
	  }
	  else if ((param == "yq*") || (param == "zq*"))
	  {
		  Yq *= val;
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
      setP(unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
    }
  else if  (param == "load q")
    {
      setQ(unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
    }
  else if ((param == "yp") || (param == "shunt g") || (param == "zr"))
    {
       setYp(unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
    }
  else if ((param == "yq") || (param == "shunt b") ||(param == "zq"))
    {

       setYq(unitConversion (val, unitType, puMW, systemBasePower, baseVoltage));
    }
  else if ((param == "ir")||(param == "ip"))
    {
      setIp(unitConversion (val, unitType, puA, systemBasePower, baseVoltage));
    }
  else if (param == "iq")
    {
      setIq(unitConversion (val, unitType, puA, systemBasePower, baseVoltage));
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


void gridLoad::setP(double newP)
{
	P = newP;
	checkpfq();
	checkFaultChange();
}

void gridLoad::setQ(double newQ)
{
	Q = newQ;
	updatepfq();
	checkFaultChange();
}

void gridLoad::setYp(double newYp)
{
	Yp = newYp;
	checkFaultChange();
}

void gridLoad::setYq(double newYq)
{
	Yq = newYq;
	checkFaultChange();
}

void gridLoad::setIp(double newIp)
{
	Ip = newIp;
	checkFaultChange();
}

void gridLoad::setIq(double newIq)
{
	Iq = newIq;
	checkFaultChange();
}

void gridLoad::setr(double newr)
{
	if (newr == 0.0)
	{
		Yp = 0.0;
		return;
	}
	std::complex<double> z(newr, getx());
	auto y = std::conj(1.0 / z);
	Yp = y.real();
	Yq = y.imag();
	checkFaultChange();
}
void gridLoad::setx(double newx)
{
	if (newx == 0.0)
	{
		Yq = 0.0;
		return;
	}
	std::complex<double> z(getr(), -newx);
	auto y = 1.0 / z;
	Yp = y.real();
	Yq = y.imag();
	checkFaultChange();
}

double gridLoad::getr() const
{
	if (Yp == 0.0)
	{
		return 0.0;
	}
	std::complex<double> y(Yp, Yq);
	auto z = 1.0 / y; //I would take a conjugate but it doesn't matter since we are only returning the real part
	return z.real();
}

double gridLoad::getx() const
{
	if (Yq == 0.0)
	{
		return 0.0;
	}
	std::complex<double> y(Yp, Yq);
	auto z = std::conj(1.0 / y);
	return z.imag();
}


void gridLoad::updatepfq()
{
	if (opFlags[use_power_factor_flag])
	{
		pfq = (P == 0.0) ? kBigNum : Q / P;
	}
}

void gridLoad::checkpfq()
{
	if (opFlags[use_power_factor_flag])
	{
		if (pfq > 1000.0)                          //if the pfq is screwy, recalculate, otherwise leave it the same.
		{
			if (P != 0.0)
			{
				pfq = Q / P;
			}

		}
	}
}

void gridLoad::checkFaultChange ()
{
  if ((opFlags[pFlow_initialized])&& (bus->getVoltage() < 0.05))
    {
          alert (this, POTENTIAL_FAULT_CHANGE);
    }
}

void gridLoad::updateLocalCache(const IOdata & /*args*/, const stateData &sD, const solverMode &)
{
	lastTime = sD.time;
}

void gridLoad::setState(gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	stateData sD(ttime, state, dstate_dt);
	updateLocalCache(emptyArguments, sD, sMode);
	prevTime = ttime;
}

double gridLoad::voltageAdjustment(double val, double V) const
{
	if (V < Vpqmin)
	{
		val = V * V * val * trigVVlow;
	}
	else if (V > Vpqmax)
	{
		val = V * V * val * trigVVhigh;
	}
	return val;
}

double gridLoad::getQval() const
{
	double val = Q;

	if (opFlags[use_power_factor_flag])
	{
		if (pfq < 1000.0)
		{
			val = P * pfq;
		}
	}
	return val;
}

double gridLoad::getRealPower () const
{
	double V = bus->getVoltage();
	return getRealPower(V);

}

double gridLoad::getReactivePower () const
{
      double V = bus->getVoltage ();
	  return getReactivePower(V);
}

double gridLoad::getRealPower (const IOdata &args, const stateData &sD, const solverMode &sMode) const
{
      double V = (args.empty ()) ? (bus->getVoltage (sD, sMode)) : args[voltageInLocation];
	  return getRealPower(V);
}

double gridLoad::getReactivePower (const IOdata &args, const stateData &sD, const solverMode &sMode) const
{
  double V = (args.empty ()) ? (bus->getVoltage (sD, sMode)) : args[voltageInLocation];
  return getReactivePower(V);
 
}

double gridLoad::getRealPower (const double V) const
{
	if (!isConnected())
	{
		return 0.0;
	}
	double val = voltageAdjustment(P, V);
      val += V * (V * Yp + Ip);

      return val;
 
}

double gridLoad::getReactivePower (double V) const
{
	if (!isConnected())
	{
		return 0.0;
	}
	double val = voltageAdjustment(getQval(), V);
     
      val += V * (V * Yq + Iq);
      return val;

}

void gridLoad::outputPartialDerivatives (const IOdata &args, const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
{
  if (args.empty ())  //we only have output derivatives if the input arguments are not counted
    {
      auto argsBus = bus->getOutputs (sD,sMode);
      auto argLocs = bus->getOutputLocs (sMode);
      ioPartialDerivatives (argsBus,sD,ad,argLocs,sMode);
    }
}

void gridLoad::ioPartialDerivatives (const IOdata &args, const stateData &sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode)
{
  if  (sD.time != lastTime)
    {
      updateLocalCache (args,sD,sMode);
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


  ad.assignCheckCol (PoutLocation, argLocs[voltageInLocation], 2.0 * V * Yp + Ip + 2.0 * V * P * tv);

  if (opFlags[use_power_factor_flag])
    {
      if (pfq < 1000.0)
        {
          ad.assignCheckCol (QoutLocation, argLocs[voltageInLocation], 2.0 * V * Yq + Iq + 2.0 * V * P * pfq * tv);
        }
      else
        {
          ad.assignCheckCol (QoutLocation, argLocs[voltageInLocation], 2.0 * V * Yq + Iq + 2.0 * V * Q * tv);
        }
    }
  else
    {
      ad.assignCheckCol (QoutLocation, argLocs[voltageInLocation], 2.0 * V * Yq + Iq + 2.0 * V * Q * tv);
    }

}

bool compareLoad (gridLoad *ld1, gridLoad *ld2, bool /*printDiff*/)
{
  bool cmp = true;
 
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
