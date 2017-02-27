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

#include "loadModels/otherLoads.h"
#include "gridBus.h"
#include "stringOps.h"
#include "matrixData.h"
#include "core/coreObjectTemplates.h"
#include <cmath>

exponentialLoad::exponentialLoad (const std::string &objName) : gridLoad (objName)
{
}

exponentialLoad::exponentialLoad (double rP, double qP, const std::string &objName) : gridLoad (rP,qP,objName)
{

}

coreObject *exponentialLoad::clone (coreObject *obj) const
{
  exponentialLoad *ld = cloneBase<exponentialLoad, gridLoad> (this, obj);
  if (ld == nullptr)
    {
      return obj;
    }

  ld->alphaP = alphaP;
  ld->alphaQ = alphaQ;
  return ld;
}



// set properties
void exponentialLoad::set (const std::string &param,  const std::string &val)
{
  gridLoad::set (param, val);
}

void exponentialLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if ((param == "alphap")||(param == "ap"))
    {
      alphaP = val;
    }
  else if ((param == "alphaq")||(param == "aq"))
    {
      alphaQ = val;
    }
  else if ((param == "alpha")||(param == "a"))
    {
      alphaP = alphaQ = val;
    }
  else
    {
      gridLoad::set (param, val, unitType);
    }
 
}




void exponentialLoad::ioPartialDerivatives (const IOdata &inputs, const stateData &, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &)
{
  const double V = inputs[voltageInLocation];
  // power vs voltage
  if  (inputLocs[voltageInLocation] != kNullLocation)
    {
      ad.assign (PoutLocation, inputLocs[voltageInLocation], getP() * alphaP * pow (V, alphaP - 1.0));

      // reactive power vs voltage
      ad.assign (QoutLocation, inputLocs[voltageInLocation], getQ() * alphaQ * pow (V, alphaQ - 1.0));
    }
}



double exponentialLoad::getRealPower () const
{
  return getRealPower(bus->getVoltage());
}

double exponentialLoad::getReactivePower () const
{
	return getReactivePower(bus->getVoltage());
}

double exponentialLoad::getRealPower (const IOdata &inputs, const stateData &, const solverMode &) const
{
  return getRealPower(inputs[voltageInLocation]);
}

double exponentialLoad::getReactivePower (const IOdata &inputs, const stateData &, const solverMode &) const
{
  return getReactivePower(inputs[voltageInLocation]);
}

double exponentialLoad::getRealPower (const double V) const
{
  if (isConnected())
    {
	  double val = getP();
      val *= pow (V, alphaP);
      return val;
    }
  else
    {
      return 0.0;
    }
}

double exponentialLoad::getReactivePower (double V) const
{
  if (isConnected())
    {
	  double val = getQ();
      val *= pow (V, alphaQ);
      return val;
    }
  else
    {
      return 0.0;
    }
}

fDepLoad::fDepLoad (const std::string &objName) : exponentialLoad (objName)
{
}

fDepLoad::fDepLoad (double rP, double qP, const std::string &objName) : exponentialLoad (rP,qP,objName)
{

}

void fDepLoad::dynObjectInitializeA (coreTime time0, unsigned long flags)
{
  if ((betaP) || (betaQ))
    {
      opFlags.set (uses_bus_frequency);
    }
  return exponentialLoad::dynObjectInitializeA (time0,flags);
}

coreObject *fDepLoad::clone (coreObject *obj) const
{
  fDepLoad *ld = cloneBase<fDepLoad, exponentialLoad> (this, obj);
  if (ld == nullptr)
    {
      return obj;
    }

  ld->betaP = betaP;
  ld->betaQ = betaQ;
  return ld;
}



// set properties
void fDepLoad::set (const std::string &param,  const std::string &val)
{

  if (param == "loadtype")
    {
      auto vtype = convertToLowerCase (val);
      if (vtype == "fluorescent")
        {
          alphaP = 1.2;
          alphaQ = 3.0;
          betaP = -0.1;
          betaQ = 2.8;
        }
      else if (vtype == "incandescent")
        {
          alphaP = 1.6;
          alphaQ = 0.0;
          betaP = 0.0;
          betaQ = 0.0;
        }
      else if (vtype == "heater")
        {
          alphaP = 2.0;
          alphaQ = 0.0;
          betaP = 0.0;
          betaQ = 0.0;
        }
      else if (vtype == "motor-full")
        {
          alphaP = 0.1;
          alphaQ = 0.6;
          betaP = 2.8;
          betaQ = 1.8;
        }
      else if (vtype == "motor-half")
        {
          alphaP = 0.2;
          alphaQ = 1.6;
          betaP = 1.5;
          betaQ = -0.3;
        }
      else if (vtype == "Reduction_furnace")
        {
          alphaP = 1.9;
          alphaQ = 2.1;
          betaP = -0.5;
          betaQ = 0.0;
        }
      else if (vtype == "aluminum_plant")
        {
          alphaP = 1.8;
          alphaQ = 2.2;
          betaP = -0.3;
          betaQ = 0.6;
        }

    }
  else
    {
      exponentialLoad::set (param, val);
    }

}

void fDepLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "betap")
    {
      betaP = val;
    }
  else if (param == "betaq")
    {
      betaQ = val;
    }
  else if (param == "beta")
    {
      betaP = betaQ = val;
    }
  else
    {
      exponentialLoad::set (param, val, unitType);
    }
  if ((betaP) || (betaQ))
    {
      opFlags.set (uses_bus_frequency);
    }
 
}


void fDepLoad::ioPartialDerivatives (const IOdata &inputs, const stateData &, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &)
{
  const double V = inputs[voltageInLocation];
  double freq = inputs[frequencyInLocation];
  // power vs voltage
  if  (inputLocs[voltageInLocation] != kNullLocation)
    {
      ad.assign (PoutLocation,inputLocs[voltageInLocation], getP() * alphaP * pow (V, alphaP - 1.0) * pow (freq, betaP));

      // reactive power vs voltage
      ad.assign (QoutLocation, inputLocs[voltageInLocation], getQ() * alphaQ * pow (V, alphaQ - 1.0) * pow (freq, betaQ));
    }
  if (inputLocs[frequencyInLocation] != kNullLocation)
    {
      ad.assign (PoutLocation, inputLocs[frequencyInLocation], getP() * pow (V, alphaP) * betaP * pow (freq, betaP - 1.0));
      ad.assign (QoutLocation, inputLocs[frequencyInLocation], getQ() * pow (V, alphaQ) * betaQ * pow (freq, betaQ - 1.0));
    }
}



double fDepLoad::getRealPower () const
{
	return getRealPower(bus->getVoltage(), bus->getFreq());
}

double fDepLoad::getReactivePower () const
{
	return getReactivePower(bus->getVoltage(), bus->getFreq());
}

double fDepLoad::getRealPower (const IOdata &inputs, const stateData &, const solverMode &) const
{
	return getRealPower(inputs[voltageInLocation], inputs[frequencyInLocation]);
}

double fDepLoad::getReactivePower (const IOdata &inputs, const stateData &, const solverMode &) const
{

  return getReactivePower(inputs[voltageInLocation], inputs[frequencyInLocation]);
}

double fDepLoad::getRealPower (const double V) const
{
	return getRealPower(V, bus->getFreq());
}

double fDepLoad::getReactivePower (double V) const
{
	return getReactivePower(V, bus->getFreq());

}


double fDepLoad::getRealPower(double V, double f) const
{
	if (isConnected())
	{
		double val = getP();
		val *= pow(V, alphaP) * pow(f, betaP);
		return val;
	}
	else
	{
		return 0.0;
	}
}

double fDepLoad::getReactivePower(double V, double f) const
{
	
	if (isConnected())
	{
		double val = getQ();
		val *= pow(V, alphaQ) * pow(f, betaQ);
		return val;
	}
	else
	{
		return 0.0;
	}
}