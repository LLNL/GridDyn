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

#include "submodels/gridDynGenModel.h"
#include "submodels/otherGenModels.h"
#include "generators/gridDynGenerator.h"
#include "core/coreExceptions.h"
#include "core/objectFactoryTemplates.h"
#include "gridBus.h"
#include  "utilities/matrixData.h"
#include "core/coreObjectTemplates.h"
#include "utilities/vectorOps.hpp"


static typeFactory<gridDynGenModel> gdm ("genmodel", stringVec { "trivial" });

static childTypeFactory<gridDynGenModelInverter, gridDynGenModel> gfgm ("genmodel", stringVec { "inverter"});

static childTypeFactory<gridDynGenModelClassical,gridDynGenModel> gfgm2 ("genmodel", stringVec {"basic", "2", "second", "secondorder", "classic", "classical", "II"});

static childTypeFactory<gridDynGenModel3, gridDynGenModel> gfgm3 ("genmodel", stringVec {"3", "third", "thirdorder", "III"});
static childTypeFactory<gridDynGenModel4, gridDynGenModel> gfgm4 ("genmodel", stringVec {"4", "fourth", "fourthorder", "IV", "grdc"}, "4"); //set 4th order as the default

static childTypeFactory<gridDynGenModel5, gridDynGenModel> gfgm5 ("genmodel", stringVec {"5", "fifth", "fifthorder", "5.1", "Vtype1", "V"});

static childTypeFactory<gridDynGenModel5type2, gridDynGenModel> gfgm5p2 ("genmodel", stringVec {"5.2", "fifthtype2", "fifthordertype2", "Vtype2"});

//static typeFactory<gridDynGenModel5type3> gfgm5p3 ("genmodel", stringVec{"5.3", "fifthtype3", "fifthordertype3", "Vtype3"});


static childTypeFactory<gridDynGenModel6, gridDynGenModel> gfgm6 ("genmodel", stringVec {"6", "six", "sixthorder", "VI"});

static childTypeFactory<gridDynGenModel6type2, gridDynGenModel> gfgm6p2 ("genmodel", stringVec {"6.2", "sixtype2", "sixthordertype2", "VItype2", "VI.2"});
/*
static typeFactory<gridDynGenModelGENROU> gfgm6p3("genmodel", stringVec{"6.3", "sixtype3", "sixthordertype3", "VItype3", "VI.3","genrou"});
*/

static childTypeFactory<gridDynGenModel8, gridDynGenModel> gfgm8 ("genmodel", stringVec {"8", "eight", "eighthorder", "VIII"});


gridDynGenModel::gridDynGenModel (const std::string &objName) : gridSubModel (objName)
{

}

coreObject *gridDynGenModel::clone (coreObject *obj) const
{
  gridDynGenModel *gd = cloneBase<gridDynGenModel, gridSubModel> (this,obj);
  if (!(gd))
    {
      return obj;
    }

  gd->Rs   = Rs;
  gd->Xd = Xd;
  gd->machineBasePower = machineBasePower;
  return gd;
}



// initial conditions

void gridDynGenModel::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet)
{

  if (inputs[voltageInLocation] > 0.85)
    {
      inputSet[genModelPmechInLocation] = desiredOutput[PoutLocation];                 //Pmt
      inputSet[genModelEftInLocation] = desiredOutput[QoutLocation] / Xd;
    }
  else
    {
      inputSet[genModelPmechInLocation] = desiredOutput[PoutLocation] / inputs[voltageInLocation] * 0.85;               //Pmt
      inputSet[genModelEftInLocation] = desiredOutput[QoutLocation] / Xd / inputs[voltageInLocation] * 0.85;
    }


  bus = static_cast<gridBus *> (find ("bus"));
}

// residual

double gridDynGenModel::getFreq (const stateData &sD, const solverMode &sMode, index_t *FreqOffset) const
{
  //there is no inertia in this gen model so it can't compute a frequency and must use the bus frequency
  if (FreqOffset)
    {
	  *FreqOffset = bus->getOutputLoc(sMode, frequencyInLocation);
    }
    return bus->getFreq (sD, sMode);

}

double gridDynGenModel::getAngle (const stateData &, const solverMode &, index_t *AngleOffset) const
{
  //there is no inertia in this gen model so it can't compute a frequency and must use the bus frequency
  if (AngleOffset)
    {
      *AngleOffset = kNullLocation;
    }
  return kNullVal;

}


count_t gridDynGenModel::outputDependencyCount(index_t /*num*/, const solverMode & /*sMode*/) const
{
	return 0;
}

IOdata gridDynGenModel::getOutputs (const IOdata &inputs, const stateData &, const solverMode &) const
{

  IOdata out (2);
  double V = inputs[voltageInLocation];
  double Eft = inputs[genModelEftInLocation];
  if (V > 0.85)
    {
      out[PoutLocation] = -inputs[genModelPmechInLocation];
      out[QoutLocation] = -Eft * Xd;

    }
  else
    {
      out[PoutLocation] = -inputs[genModelPmechInLocation] * V / 0.85;
      out[QoutLocation] = -Eft * Xd * V / 0.85;
    }

  return out;

}



double gridDynGenModel::getOutput (const IOdata &inputs, const stateData &, const solverMode &, index_t numOut) const
{
  double V = inputs[voltageInLocation];
  double Eft = inputs[genModelEftInLocation];
  if (V > 0.85)
    {
      if (numOut == PoutLocation)
        {
          return -inputs[genModelPmechInLocation];
        }
      else if (numOut == QoutLocation)
        {
          return -Eft * Xd;

        }
    }
  else
    {
      if (numOut == PoutLocation)
        {
          return -inputs[genModelPmechInLocation] * V / 0.85;
        }
      else if (numOut == QoutLocation)
        {
          return -Eft * Xd * V / 0.85;

        }

    }
  return kNullVal;

}


void gridDynGenModel::ioPartialDerivatives (const IOdata &inputs, const stateData &, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &)
{

  double V = inputs[voltageInLocation];

  if (V > 0.85)
    {
      ad.assignCheckCol (QoutLocation, inputLocs[genModelEftInLocation], -Xd);

      if (inputLocs[voltageInLocation] != kNullLocation)
        {
          ad.assign (PoutLocation, inputLocs[voltageInLocation], 0);
          ad.assign (QoutLocation, inputLocs[voltageInLocation], 0);
        }
      ad.assignCheckCol (PoutLocation, inputLocs[genModelPmechInLocation], -1.0);
    }
  else
    {
      double factor = V / 0.85;
      ad.assignCheckCol (QoutLocation, inputLocs[genModelEftInLocation], -Xd * factor);

      if (inputLocs[voltageInLocation] != kNullLocation)
        {
          double Eft = inputs[genModelEftInLocation];
          ad.assign (PoutLocation, inputLocs[voltageInLocation],-inputs[genModelPmechInLocation] / 0.85);
          ad.assign (QoutLocation, inputLocs[voltageInLocation], -Eft * Xd / 0.85);
        }
      ad.assignCheckCol (PoutLocation, inputLocs[genModelPmechInLocation], -factor);
    }


}


// set parameters
void gridDynGenModel::set (const std::string &param,  const std::string &val)
{
  return gridSubModel::set (param, val);
}

void gridDynGenModel::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param.length () == 1)
    {
      switch (param[0])
        {
        case 'x':
          Xd = val;
          break;
        case 'r':
          Rs = val;
		  break;
        default:
			throw(unrecognizedParameter());

        }
	  return;
    }

  if ((param == "xd")||(param == "xs"))
    {
      Xd = val;
    }
  else if (param == "rs")
    {
      Rs = val;

    }
  else if ((param == "base")||(param == "mbase"))
    {
      machineBasePower = val;
    }
  else
    {
      gridSubModel::set (param,val,unitType);
    }


}







