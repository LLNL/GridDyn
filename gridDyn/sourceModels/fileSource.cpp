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

#include "sourceTypes.h"
#include "core/coreObjectTemplates.h"

fileSource::fileSource (const std::string filename, int column) : rampSource ("filesource_#")
{
  if (!filename.empty ())
    {
      setFile (filename, column);
    }

}

coreObject *fileSource::clone (coreObject *obj) const
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

void fileSource::dynObjectInitializeA (coreTime time0, unsigned long flags)
{
  index_t ii;
  ii = 0;
  prevTime = time0;
  if (opFlags[use_absolute_time_flag])
    {
      double abstime0 = get ("abstime0");

      while (schedLoad.time(ii) < abstime0)
        {
          ++ii;
          if (ii >= schedLoad.size())
            {            //this should never happen
              ii = schedLoad.size();
              break;
            }
        }
      currIndex = ii;
      nextUpdateTime = schedLoad.time(currIndex);
      timestep (time0,noInputs,cLocalSolverMode);

    }
  else
    {
      if (schedLoad.time(currIndex) < time0)
        {
          while (schedLoad.time(currIndex) < time0)
            {
              currIndex++;
            }
          currIndex = currIndex - 1;
          nextUpdateTime = schedLoad.time(currIndex);
          timestep (time0,noInputs,cLocalSolverMode);
        }

    }
  return rampSource::dynObjectInitializeA (time0,flags);

}

void fileSource::updateA (coreTime time)
{
  while (time >= schedLoad.time(currIndex))
    {
      m_output = schedLoad.data(currIndex);
      prevTime = schedLoad.time(currIndex);
      currIndex++;
      if (currIndex >= count)
        {        //this should never happen since the last time should be very large
          currIndex = count;
          mp_dOdt = 0;
          break;
        }

      if (opFlags[use_step_change_flag])
        {
          mp_dOdt = 0;
        }
      else
        {
          double diff = schedLoad.data(currIndex) - m_output;
          double dt = schedLoad.time(currIndex) - schedLoad.time(currIndex - 1);
          mp_dOdt = diff / dt;
        }

      nextUpdateTime = schedLoad.time(currIndex);
    }
}

void fileSource::timestep (coreTime ttime, const IOdata &inputs, const solverMode &sMode)
{
  if (ttime > nextUpdateTime)
    {
      updateA (ttime);
    }

  rampSource::timestep (ttime, inputs, sMode);
}


void fileSource::setFlag(const std::string &flag, bool val)
{
	if (flag == "absolute")
	{
		opFlags.set(use_absolute_time_flag,val);
	}
	else if (flag == "relative")
	{
		opFlags.set(use_absolute_time_flag,!val);
	}
	else if (flag == "step")
	{
		opFlags.set(use_step_change_flag,val);
	}
	else if (flag == "interpolate")
	{
		opFlags.set(use_step_change_flag,!val);
	}
	else
	{
		rampSource::setFlag(flag,val);
	}

}
void fileSource::set (const std::string &param,  const std::string &val)
{
 
  if ((param == "filename") || (param == "file"))
    {
      setFile (val,0);
    }
  else
    {
      gridSource::set (param, val);
    }

}


void fileSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
 
  {
    gridSource::set (param,val,unitType);
  }

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
  if (schedLoad.size() > 0)
    {
      schedLoad.addData (schedLoad.lastTime() + 365.0 * kDayLength,schedLoad.lastData ());
    }
  else
    {
      schedLoad.addData (365.0 * kDayLength,m_output);
    }
  return schedLoad.size();
}
