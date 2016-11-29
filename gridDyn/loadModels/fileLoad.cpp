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

#include "loadModels/otherLoads.h"
#include "gridCoreTemplates.h"
#include "gridBus.h"
#include "stringOps.h"

#include "vectorOps.hpp"


gridFileLoad::gridFileLoad (const std::string &objName) : gridRampLoad (objName)
{

}

gridCoreObject *gridFileLoad::clone (gridCoreObject *obj) const
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


void gridFileLoad::pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  index_t ii = 0;
  count = 0;
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
      for (index_t kk = 0; (kk < schedLoad.cols) && (kk < 6); ++kk)
        {
          columnkey[kk] = kk;
        }
    }
  updateA (time0);
  gridRampLoad::pFlowObjectInitializeA (time0, flags);
  if (opFlags.test (use_absolute_time_flag))
    {
      double abstime0 = get ("abstime0");
      while (schedLoad.time[ii] < abstime0)
        {
          ++ii;
          if (ii >= schedLoad.count)
            {            //this should never happen
              ii = schedLoad.count;
              break;
            }
        }
      currIndex = ii;
      nextUpdateTime = schedLoad.time[currIndex];
      timestep (time0, bus->getOutputs (nullptr, cLocalSolverMode), cLocalSolverMode);

    }
  else
    {
      if (schedLoad.time[currIndex] < time0)
        {
          while (schedLoad.time[currIndex] < time0)
            {
              currIndex++;
            }
          currIndex = currIndex - 1;
          nextUpdateTime = schedLoad.time[currIndex];
          timestep (time0, bus->getOutputs (nullptr, cLocalSolverMode), cLocalSolverMode);
        }

    }


}

void gridFileLoad::updateA (gridDyn_time time)
{
  while (time+kSmallTime >= schedLoad.time[currIndex])
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
  double val;
  prevTime = schedLoad.time[currIndex];
  auto dt = (currIndex < count - 1) ? (schedLoad.time[currIndex + 1] - prevTime) : maxTime;
  for (count_t pp = 0; pp < schedLoad.cols; ++pp)
    {
      if (columnkey[pp] < 0)
        {
          continue;
        }
      val = schedLoad.data[pp][currIndex] * scaleFactor;
      if (currIndex < count - 1)
        {
          diffrate = (opFlags.test (use_step_change_flag)) ? 0 : (schedLoad.data[pp][currIndex + 1] * scaleFactor - val) / dt;
        }
      else
        {
          diffrate = 0;
        }


      switch (columnkey[pp])
        {
        case -1:
          break;
        case 0:
          P = val;
          dPdt = diffrate;
          if (qratio != kNullVal)
            {
              Q = qratio * P;
              dQdt = qratio * diffrate;
            }
          break;
        case 1:
          Q = val;
          dQdt = diffrate;
          break;
        case 2:
          Ip = val;
          dIpdt = diffrate;
          if (qratio != kNullVal)
            {
              Iq = qratio * Ip;
              dIqdt = qratio * diffrate;
            }
          break;
        case 3:
          Iq = val;
          dIqdt = diffrate;
          break;
        case 4:
          Yp = val;
          dYpdt = diffrate;
          if (qratio != kNullVal)
            {
              Yq = qratio * Yp;
              dYqdt = qratio * diffrate;
            }
          break;
        case 5:
          Yq = val;
          dYqdt = diffrate;
          break;
        case 6:
          gridLoad::set ("r", val);              //there is some additional computations that need to happen in the load object
          drdt = diffrate;
          break;
        case 7:
          gridLoad::set ("x", val);               //there is some additional computations that need to happen in the load object
          dxdt = diffrate;
          break;
        default:
          break;

        }
    }
  if (!opFlags.test (use_step_change_flag))
    {
      gridRampLoad::loadUpdateForward (time);
    }
  lastUpdateTime = time;
  nextUpdateTime = (currIndex == count - 1) ? kBigNum : schedLoad.time[currIndex + 1];
}

void gridFileLoad::timestep (gridDyn_time ttime, const IOdata &args,const solverMode &sMode)
{
  if (ttime > nextUpdateTime)
    {
      updateA (ttime);
    }

  gridRampLoad::timestep (ttime, args, sMode);
}


void gridFileLoad::setTime (gridDyn_time time)
{
  if (opFlags.test (use_absolute_time_flag))
    {
      timestep (time, bus->getOutputs (nullptr, cLocalSolverMode), cLocalSolverMode);
    }
  else
    {
      auto dt = time - prevTime;
      for (index_t kk = 0; kk < count; ++kk)
        {
          schedLoad.time[kk] += dt;
        }
    }

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
      gridLoad::setFlag (param, val);
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
      int col = trailingStringInt (param, -1);
      auto sp = splitlineTrim (val);
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
      gridLoad::set (param, val);
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
      gridLoad::set (param,val,unitType);
    }
}

count_t gridFileLoad::loadFile ()
{
  auto stl = fname.length ();
  switch (fname[stl - 3])
  {
  case 'b': case 'd':
	  schedLoad.loadBinaryFile(fname);
	  break;
  case 'c':case 't':
	  schedLoad.loadTextFile(fname);
	  break;
  default:
	  LOG_ERROR("unable to load file " + fname);
	  return 0;
  }
  if (schedLoad.count > 0)
    {
      schedLoad.addData (schedLoad.time.back () + 365.0 * kDayLength,schedLoad.data.back ());
	  if (inputUnits != gridUnits::defUnit)
	  {
		  double scalar = gridUnits::unitConversion(1.0, inputUnits, gridUnits::puMW, systemBasePower, baseVoltage);
		  for (index_t ii = 0; ii < schedLoad.cols; ++ii)
		  {
			  std::transform(schedLoad.data[ii].begin(), schedLoad.data[ii].end(), schedLoad.data[ii].begin(),
				  [=](double val) {return val*scalar; });
		  }
	  }
    }
  else
    {
      schedLoad.addData (365.0 * kDayLength,Psched);
    }
  if (columnkey.size() < schedLoad.cols)
  {
	  columnkey.resize(schedLoad.cols, -1);
  }
  return schedLoad.count;
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