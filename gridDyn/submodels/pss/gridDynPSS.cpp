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

#include "submodels/gridDynPSS.h"
#include "generators/gridDynGenerator.h"
#include "gridBus.h"
#include "objectFactoryTemplates.h"
#include <cmath>


static const typeFactory<gridDynPSS> gf ("pss", stringVec { "basic" });

gridDynPSS::gridDynPSS (const std::string &objName) : gridSubModel (objName)
{

}

gridCoreObject *gridDynPSS::clone (gridCoreObject *obj) const
{
  gridDynPSS *pss;
  if (obj == nullptr)
    {
      pss = new gridDynPSS ();
    }
  else
    {
      pss = dynamic_cast<gridDynPSS *> (obj);
      if (pss == nullptr)
        {
          gridSubModel::clone (obj);
          return obj;
        }
    }
  gridSubModel::clone (pss);
  return pss;
}

// destructor
gridDynPSS::~gridDynPSS ()
{

}

// initial conditions
void gridDynPSS::objectInitializeB (const IOdata & /*args*/, const IOdata & /*outputSet*/, IOdata & /*fieldSet*/)
{

}


// residual
void gridDynPSS::residual (const IOdata & /*args*/, const stateData *, double /*resid*/ [],  const solverMode &)
{

}

index_t gridDynPSS::findIndex (const std::string & /*field*/, const solverMode &) const
{
  return kInvalidLocation;
}

void gridDynPSS::set (const std::string &param,  const std::string &val)
{
  return gridCoreObject::set (param, val);
}

// set parameters
void gridDynPSS::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  {
    gridCoreObject::set (param,val,unitType);
  }

}

void gridDynPSS::jacobianElements (const IOdata & /*args*/, const stateData *,
                                   matrixData<double> &,
                                   const IOlocs & /*argLocs*/, const solverMode &sMode)
{
  if  (isAlgebraicOnly (sMode))
    {
      return;
    }
}


void gridDynPSS::derivative (const IOdata & /*args*/, const stateData *, double /*deriv*/[], const solverMode &)
{

}
