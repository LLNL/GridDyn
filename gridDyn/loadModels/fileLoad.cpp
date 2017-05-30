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
#include "core/coreObjectTemplates.h"
#include "gridBus.h"
#include "utilities/stringOps.h"

#include "utilities/vectorOps.hpp"


gridFileLoad::gridFileLoad (const std::string &objName) : gridRampLoad (objName)
{

}

coreObject *gridFileLoad::clone (coreObject *obj) const
{
  gridFileLoad *nobj = cloneBase<gridFileLoad, gridRampLoad> (this, obj);
  if (!(nobj ))
    {
      return obj;
    }
  nobj->inputUnits = inputUnits;
  nobj->scaleFactor = scaleFactor;
  nobj->fname = fname;

  return nobj;
}


void gridFileLoad::pFlowObjectInitializeA (coreTime time0, unsigned long flags)
{

  currIndex = 0;
  count = loadFile ();
  bool found = false;
  for (auto cc:columnkey)
    {
      if (cc >= 0)
        {
          found = true;
          break;
        }
    }
  if (!found)
    {
      for (index_t kk = 0; (kk < schedLoad.columns()) && (kk < 8); ++kk)
        {
          columnkey[kk] = kk;
        }
    }
  gridRampLoad::pFlowObjectInitializeA(time0, flags);
  updateA(time0);
  


}

void gridFileLoad::updateA (coreTime time)
{
  while (time>= schedLoad.time(currIndex))
    {
      ++currIndex;
      if (currIndex >= count)
        {                //this should never happen since the last time should be very large
          currIndex = count;
          break;
        }
    }
  if (currIndex > 0)
    {
      --currIndex;            //back it off by 1
    }

  double diffrate = 0;

  prevTime = schedLoad.time(currIndex);
  auto dt = (currIndex < count - 1) ? (schedLoad.time(currIndex + 1) - prevTime) : maxTime;
  for (count_t pp = 0; pp < schedLoad.columns(); ++pp)
    {
      if (columnkey[pp] < 0)
        {
          continue;
        }
      double val = schedLoad.data(pp,currIndex) * scaleFactor;
      if (currIndex < count - 1)
        {
          diffrate = (opFlags[use_step_change_flag]) ? 0.0 : (schedLoad.data(pp,currIndex + 1) * scaleFactor - val) / dt;
        }
      else
        {
          diffrate = 0.0;
        }


      switch (columnkey[pp])
        {
        case -1:
          break;
        case 0:
          setP(val);
          dPdt = diffrate;
          if (qratio != kNullVal)
            {
              setQ(qratio * val);
              dQdt = qratio * diffrate;
            }
          break;
        case 1:
          setQ(val);
          dQdt = diffrate;
          break;
        case 2:
          setIp(val);
          dIpdt = diffrate;
          if (qratio != kNullVal)
            {
              setIq(qratio * val);
              dIqdt = qratio * diffrate;
            }
          break;
        case 3:
          setIq(val);
          dIqdt = diffrate;
          break;
        case 4:
          setYp(val);
          dYpdt = diffrate;
          if (qratio != kNullVal)
            {
              setYq(qratio * val);
              dYqdt = qratio * diffrate;
            }
          break;
        case 5:
          setYq(val);
          dYqdt = diffrate;
          break;
        case 6:
          setr(val);
          drdt = diffrate;
          break;
        case 7:
          setx(val);
          dxdt = diffrate;
          break;
        default:
          break;

        }
    }
  lastTime = prevTime;
  if (!opFlags[use_step_change_flag])
    {
	  gridRampLoad::updateLocalCache(noInputs, stateData(time), cLocalSolverMode);
    }
  lastUpdateTime = time;
  nextUpdateTime = (currIndex == count - 1) ? maxTime : schedLoad.time(currIndex + 1);
}

void gridFileLoad::timestep (coreTime ttime, const IOdata &inputs,const solverMode &sMode)
{
  if (ttime >= nextUpdateTime)
    {
      updateA (ttime);
    }

  gridRampLoad::timestep (ttime, inputs, sMode);
}


void gridFileLoad::setFlag (const std::string &param, bool val)
{

  if (param == "absolute")
    {
      opFlags.set (use_absolute_time_flag, val);
    }
  else if (param == "relative")
    {
      opFlags.set (use_absolute_time_flag, !val);
    }
  else if (param == "step")
    {
      opFlags.set (use_step_change_flag, val);
    }
  else if (param == "interpolate")
    {
      opFlags.set (use_step_change_flag, !val);
    }
  else
    {
      zipLoad::setFlag (param, val);
    }

}

void gridFileLoad::set (const std::string &param,  const std::string &val)
{

  if ((param == "filename") || (param == "file"))
    {
      fname = val;
      if (opFlags[pFlow_initialized])
        {
          count = 0;
          currIndex = 0;
          count = loadFile ();
        }
    }
  else if (param.compare (0, 6, "column") == 0)
    {
      int col = stringOps::trailingStringInt (param, -1);
      auto sp = stringOps::splitline(val);
	  stringOps::trim(sp);
      if (col >= 0)
        {
          if (columnkey.size () < col + sp.size ())
            {
              columnkey.resize (col + sp.size (),-1);
            }
        }
      else
        {
          if (columnkey.size () < sp.size ())
            {
              columnkey.resize (sp.size (),-1);
            }
        }
      for (auto &str : sp)
        {
          int code = columnCode (str);
          if (col >= 0)
            {
              columnkey[col] = code;
              ++col;
            }
          else
            {
              int ncol = 0;
              while (columnkey[ncol] >= 0)
                {
                  ++ncol;
                  if (ncol == static_cast<int> (columnkey.size ()))
                    {
                      columnkey.push_back (-1);
                    }
                }
              columnkey[ncol] = code;
            }
        }
    }
  else if (param == "units")
    {
      inputUnits = gridUnits::getUnits (val);
    }
  else if ((param == "mode")||(param == "timemode"))
    {
      setFlag (val, true);
    }

  else
    {
      zipLoad::set (param, val);
    }

}


void gridFileLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{
 
  if ((param == "scalefactor") || (param == "scaling"))
    {
      scaleFactor = val;
    }
  else if (param == "qratio")
    {
      qratio = val;
    }
  else
    {
      zipLoad::set (param,val,unitType);
    }
}

count_t gridFileLoad::loadFile ()
{
  schedLoad.loadFile(fname);
  if (schedLoad.size() > 0)
    {
      schedLoad.addData (schedLoad.lastTime () + 365.0 * kDayLength,schedLoad.lastData());
	  if (inputUnits != gridUnits::defUnit)
	  {
		  double scalar = gridUnits::unitConversion(1.0, inputUnits, gridUnits::puMW, systemBasePower, baseVoltage);
		  schedLoad.scaleData(scalar);
	  }
    }
  else
    {
      schedLoad.addData (365.0 * kDayLength,getP());
    }
  if (columnkey.size() < schedLoad.columns())
  {
	  columnkey.resize(schedLoad.columns(), -1);
  }
  return schedLoad.size();
}


int gridFileLoad::columnCode(const std::string &ldc)
{
	auto lc = convertToLowerCase(ldc);
	int code = -1;
	if (lc == "p")
	{
		code = 0;
	}
	else if (lc == "q")
	{
		code = 1;
	}
	else if (lc == "ip")
	{
		code = 2;
	}
	else if (lc == "iq")
	{
		code = 3;
	}
	else if ((lc == "zr") || (lc == "yp") | (lc == "zp") || (lc == "yr"))
	{
		code = 4;
	}
	else if ((lc == "zq") || (lc == "yq"))
	{
		code = 5;
	}
	else if (lc == "r")
	{
		code = 6;
	}
	else if (lc == "x")
	{
		code = 7;
	}
	return code;
}