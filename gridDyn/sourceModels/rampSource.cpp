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
#include "vectorOps.hpp"

#include <ctime>

rampSource::rampSource (const std::string &objName, double startVal) : gridSource (objName,startVal)
{

}

gridCoreObject *rampSource::clone (gridCoreObject *obj) const
{
  rampSource *ld;
  if (obj == nullptr)
    {
      ld = new rampSource ();
    }
  else
    {
      ld = dynamic_cast<rampSource *> (obj);
      if (ld == nullptr)
        {
          gridSource::clone (obj);
          return obj;
        }
    }
  gridSource::clone (ld);
  ld->mp_dOdt = mp_dOdt;
  return ld;
}



// set properties
int rampSource::set (const std::string &param,  const std::string &val)
{
  return gridSource::set (param, val);
}

int rampSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;

  if ((param == "dodt") || (param == "ramp")||(param == "rate"))
    {
      mp_dOdt = val;
    }
  else
    {
      out = gridSource::set (param, val, unitType);
    }
  return out;
}


void rampSource::sourceUpdate (const double ttime)
{
  double tdiff = ttime - prevTime;
  if (tdiff == 0.0)
    {
      return;
    }
  m_tempOut = m_output + mp_dOdt * tdiff;
}


double rampSource::getDoutdt(const stateData *, const solverMode &, index_t /*num*/)
{
	return mp_dOdt;
}