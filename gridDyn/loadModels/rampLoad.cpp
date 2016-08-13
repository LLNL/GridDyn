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
#include "gridBus.h"
#include "vectorOps.hpp"
#include "gridCoreTemplates.h"


#include <ctime>

using namespace gridUnits;
gridRampLoad::gridRampLoad (const std::string &objName) : gridLoad (objName)
{

}

gridRampLoad::gridRampLoad (double rP, double qP, const std::string &objName) : gridLoad (rP,qP,objName)
{
}

gridCoreObject *gridRampLoad::clone (gridCoreObject *obj) const
{
  gridRampLoad *ld = cloneBase<gridRampLoad, gridLoad> (this, obj);
  if (ld == nullptr)
    {
      return obj;
    }

  ld->dPdt = dPdt;
  ld->dQdt = dQdt;
  ld->drdt = drdt;
  ld->dxdt = dxdt;
  ld->dIpdt = dIpdt;
  ld->dIqdt = dIqdt;
  ld->dYqdt = dYqdt;
  ld->dYpdt = dYpdt;
  return ld;
}

// destructor
gridRampLoad::~gridRampLoad ()
{
}


// set properties
int gridRampLoad::set (const std::string &param,  const std::string &val)
{
  return gridLoad::set (param, val);
}

int gridRampLoad::set (const std::string &param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;
  if (param.length () == 4)
    {
      if ((param[0] == 'd') && (param[2] == 'd') && (param[3] == 't'))
        {
          switch (param[1])
            {
            case 'p':        //dpdt
              dPdt = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
              break;
            case 'r':      //drdt
              drdt = unitConversion (val, unitType, puA, systemBasePower, baseVoltage);
              break;
            case 'x':       //dxdt
              dxdt = unitConversion (val, unitType, puOhm, systemBasePower, baseVoltage);
              break;
            case 'q':       //dqdt
              dQdt = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
              break;
            case 'i':       //didt
              dIpdt = unitConversion (val, unitType, puOhm, systemBasePower, baseVoltage);
              break;
            case 'z':
            case 'y':                //dzdt dydt
              dYpdt = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
              break;
            default:
              out = PARAMETER_NOT_FOUND;

            }
        }
      else
        {
          out = gridLoad::set (param, val, unitType);
        }
    }
  else if (param.length () == 5)
    {
      if ((param[0] == 'd') && (param[3] == 'd') && (param[4] == 't'))
        {
          switch (param[2])
            {
            case 'r':
            case 'p':
              switch (param[1])
                {
                case 'i':            //dirdt dipdt
                  dIpdt = unitConversion (val, unitType, puOhm, systemBasePower, baseVoltage);
                  break;
                case 'z':
                case 'y':                     //dzrdt dyrdt dzpdt dzrdt
                  dYpdt = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
                  break;
                default:
                  out = PARAMETER_NOT_FOUND;
                }
              break;
            case 'q':
              switch (param[1])
                {
                case 'i':          //diqdt
                  dIqdt = unitConversion (val, unitType, puOhm, systemBasePower, baseVoltage);
                  break;
                case 'z':
                case 'y':                    //dzqdt dyqdt
                  dYqdt = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
                  break;
                default:
                  out = PARAMETER_NOT_FOUND;
                }
              break;
            default:
              out = PARAMETER_NOT_FOUND;
            }
        }
      else
        {
          out = gridLoad::set (param, val, unitType);
        }
    }

  else
    {
      out = gridLoad::set (param, val, unitType);
    }
  return out;
}


void gridRampLoad::loadUpdate (double ttime)
{
  double tdiff = ttime - lastTime;
  if (tdiff == 0.0)
    {
      return;
    }
  P = P + dPdt * tdiff;
  Q = Q + dQdt * tdiff;
  if ((drdt != 0) || (dxdt != 0))
    {
      r = r + drdt * tdiff;
      x = x + dxdt * tdiff;
      Yp = r / (r * r + x * x);
      Yq = x / (r * r + x * x);
    }
  else if ((dYpdt != 0.0) || (dYqdt != 0.0))
    {
      Yp = Yp + dYpdt * tdiff;
      Yq = Yq + dYqdt * tdiff;
      if (Yq == 0)
        {
          r = 1 / Yp;
          x = 0;
        }
      else
        {
          double rat = (Yp / Yq);
          x = rat / Yp / (1 + rat);
          r = x * rat;
        }
    }

  Ip = Ip + dIpdt * tdiff;
  Iq = Iq + dIqdt * tdiff;

  lastTime = ttime;
}

void gridRampLoad::clearRamp ()
{
  dPdt = 0.0;
  dQdt = 0.0;
  drdt = 0.0;
  dxdt = 0.0;
  dIpdt = 0.0;
  dIqdt = 0.0;
  dYqdt = 0.0;
  dYpdt = 0.0;
}
