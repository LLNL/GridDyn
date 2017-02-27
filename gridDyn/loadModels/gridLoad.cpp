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

#include "gridLoad.h"
#include "core/coreExceptions.h"
#include "gridBus.h"
#include "core/coreObjectTemplates.h"
#include "matrixData.h"

#include <iostream>
#include <cmath>
#include <complex>



using namespace gridUnits;

std::atomic<count_t> gridLoad::loadCount(0);
gridLoad::gridLoad (const std::string &objName) : gridSecondary (objName)
{
  constructionHelper ();
}

gridLoad::gridLoad (double rP, double rQ, const std::string &objName) : gridSecondary (objName),P (rP),Q (rQ)
{
  constructionHelper ();

}

void gridLoad::constructionHelper ()
{
  // default values
  setUserID(++loadCount);
  updateName ();
}

coreObject *gridLoad::clone (coreObject *obj) const
{
  gridLoad *nobj = cloneBase<gridLoad, gridSecondary> (this, obj);
  if (!(nobj) )
    {
      return obj;
    }
  nobj->P = P;
  nobj->Q = Q;
  nobj->pfq = pfq;
  return nobj;
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


static const stringVec locNumStrings {
  "p", "q", "pf"
};

static const stringVec locStrStrings {

};

static const stringVec flagStrings {
  "usepowerfactor"
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
		default:
			break;
        }
      return val;
    }
  
  if (param == "pf")
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

void gridLoad::checkFaultChange()
{
	if ((opFlags[pFlow_initialized]) && (bus->getVoltage() < 0.05))
	{
		alert(this, POTENTIAL_FAULT_CHANGE);
	}
}

double gridLoad::getRealPower () const
{
	return P;

}

double gridLoad::getReactivePower () const
{
	return Q;
}

double gridLoad::getRealPower (const IOdata & /*inputs*/, const stateData &, const solverMode &) const
{
	return getRealPower();
}

double gridLoad::getReactivePower (const IOdata & /*inputs*/, const stateData &, const solverMode &) const
{
	return getReactivePower();
 
}

double gridLoad::getRealPower (const double /*V*/) const
{
	return getRealPower();
 
}

double gridLoad::getReactivePower (double /*V*/) const
{
	return getReactivePower();

}

count_t gridLoad::outputDependencyCount(index_t /*num*/, const solverMode & /*sMode*/) const
{
	return 0;
}



