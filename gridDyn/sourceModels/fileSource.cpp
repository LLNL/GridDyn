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

#include "gridSource.h"
#include "stringOps.h"
#include "gridCoreTemplates.h"

fileSource::fileSource (const std::string filename, int column) : rampSource ("filesource_#")
{
  if (!filename.empty ())
    {
      setFile (filename, column);
    }

}

gridCoreObject *fileSource::clone (gridCoreObject *obj) const
{
  fileSource *nobj = cloneBase<fileSource, rampSource> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }

  nobj->setFile (fname,m_column);
  return nobj;
}


int fileSource::setFile (const std::string filename, index_t column)
{
  fname = filename;
  m_column = column;
  count = loadFile ();
  currIndex = 0;
  return count;
}

void fileSource::objectInitializeA (double time0, unsigned long flags)
{
  index_t ii;
  ii = 0;
  prevTime = time0;
  if (opFlags[use_absolute_time_flag])
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
      timestep (time0,{},cLocalSolverMode);

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
          timestep (time0,{},cLocalSolverMode);
        }

    }
  return rampSource::objectInitializeA (time0,flags);

}

void fileSource::updateA (double time)
{
  while (time >= schedLoad.time[currIndex])
    {
      m_output = schedLoad.data[currIndex];
      prevTime = schedLoad.time[currIndex];
      currIndex++;
      if (currIndex >= count)
        {        //this should never happen since the last time should be very large
          currIndex = count;
          mp_dOdt = 0;
          break;
        }

      if (opFlags.test (use_step_change_flag))
        {
          mp_dOdt = 0;
        }
      else
        {
          double diff = schedLoad.data[currIndex] - m_output;
          double dt = schedLoad.time[currIndex] - schedLoad.time[currIndex - 1];
          mp_dOdt = diff / dt;
        }

      nextUpdateTime = schedLoad.time[currIndex];
    }
}

double fileSource::timestep (double ttime, const IOdata &args, const solverMode &sMode)
{
  if (ttime > nextUpdateTime)
    {
      updateA (ttime);
    }

  rampSource::timestep (ttime, args, sMode);
  return m_output;
}


void fileSource::setTime (double time)
{
  if (opFlags[use_absolute_time_flag])
    {
      timestep (time, {},cLocalSolverMode);
    }
  else
    {
      double dt = time - prevTime;
      for (index_t kk = 0; kk < count; ++kk)
        {
          schedLoad.time[kk] += dt;
        }
    }

}

int fileSource::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;
  if ((param == "filename") || (param == "file"))
    {
      setFile (val,0);
    }
  if ((param == "flags") || (param == "mode"))
    {

      stringVec flgs = splitlineTrim (val);
      for (auto &str : flgs)
        {
          //TODO:: PT convert to setFlags function
          makeLowerCase (str);
          if (str == "absolute")
            {
              opFlags.set (use_absolute_time_flag);
            }
          else if (str == "relative")
            {
              opFlags.reset (use_absolute_time_flag);
            }
          else if (str == "step")
            {
              opFlags.set (use_step_change_flag);
            }
          else if (str == "interpolate")
            {
              opFlags.reset (use_step_change_flag);
            }
          else
            {
              rampSource::set ("flags", str);
            }
        }
    }

  else
    {
      out = gridSource::set (param, val);
    }
  return out;
}


int fileSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;
  {
    out = gridSource::set (param,val,unitType);
  }
  return out;
}

int fileSource::loadFile ()
{
  auto stl = fname.length ();

  if ((fname[stl - 3] == 'c')||(fname[stl - 3] == 't'))
    {
      schedLoad.loadTextFile (fname,m_column);
    }
  else
    {
      schedLoad.loadBinaryFile (fname,m_column);
    }
  if (schedLoad.count > 0)
    {
      schedLoad.addData (schedLoad.time.back () + 365.0 * kDayLength,schedLoad.data.back ());
    }
  else
    {
      schedLoad.addData (365.0 * kDayLength,m_output);
    }
  return schedLoad.count;
}
