/*
* LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "core/objectFactoryTemplates.hpp"
#include "../Generator.h"
#include "../gridBus.h"
#include "otherGenModels.h"
#include "utilities/matrixData.hpp"
#include "utilities/vectorOps.hpp"

namespace griddyn
{
static typeFactory<GenModel> gdm ("genmodel", stringVec{"trivial"});

namespace genmodels
{
static childTypeFactory<GenModelInverter, GenModel> gfgm ("genmodel", stringVec{"inverter"});

static childTypeFactory<GenModelClassical, GenModel>
  gfgm2 ("genmodel", stringVec{"basic", "2", "second", "secondorder", "classic", "classical", "II"});

static childTypeFactory<GenModel3, GenModel> gfgm3 ("genmodel", stringVec{"3", "third", "thirdorder", "III"});
static childTypeFactory<GenModel4, GenModel> gfgm4 ("genmodel",
                                                    stringVec{"4", "fourth", "fourthorder", "IV", "grdc"},
                                                    "4");  // set 4th order as the default

static childTypeFactory<GenModel5, GenModel>
  gfgm5 ("genmodel", stringVec{"5", "fifth", "fifthorder", "5.1", "Vtype1", "V"});

static childTypeFactory<GenModel5type2, GenModel>
  gfgm5p2 ("genmodel", stringVec{"5.2", "fifthtype2", "fifthordertype2", "Vtype2"});

// static typeFactory<GenModel5type3> gfgm5p3 ("genmodel", stringVec{"5.3",
// "fifthtype3", "fifthordertype3", "Vtype3"});

static childTypeFactory<GenModel6, GenModel> gfgm6 ("genmodel", stringVec{"6", "six", "sixthorder", "VI"});

static childTypeFactory<GenModel6type2, GenModel>
  gfgm6p2 ("genmodel", stringVec{"6.2", "sixtype2", "sixthordertype2", "VItype2", "VI.2"});
/*
static typeFactory<GenModelGENROU> gfgm6p3("genmodel", stringVec{"6.3",
"sixtype3", "sixthordertype3", "VItype3", "VI.3","genrou"});
*/

static childTypeFactory<GenModel8, GenModel> gfgm8 ("genmodel", stringVec{"8", "eight", "eighthorder", "VIII"});

}//namespace genmodels

GenModel::GenModel(const std::string &objName) : gridSubModel(objName) { m_inputSize = 4; m_outputSize = 2; }
coreObject *GenModel::clone (coreObject *obj) const
{
    auto *gd = cloneBase<GenModel, gridSubModel> (this, obj);
    if (gd==nullptr)
    {
        return obj;
    }

    gd->Rs = Rs;
    gd->Xd = Xd;
    gd->machineBasePower = machineBasePower;
    return gd;
}

// initial conditions

void GenModel::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
    if (inputs[voltageInLocation] > 0.85)
    {
        fieldSet[genModelPmechInLocation] = desiredOutput[PoutLocation];  // Pmt
        fieldSet[genModelEftInLocation] = desiredOutput[QoutLocation] / Xd;
    }
    else
    {
        fieldSet[genModelPmechInLocation] = desiredOutput[PoutLocation] / inputs[voltageInLocation] * 0.85;  // Pmt
        fieldSet[genModelEftInLocation] = desiredOutput[QoutLocation] / Xd / inputs[voltageInLocation] * 0.85;
    }

    bus = static_cast<gridBus *> (find ("bus"));
}

// residual

double GenModel::getFreq (const stateData &sD, const solverMode &sMode, index_t *freqOffset) const
{
    // there is no inertia in this gen model so it can't compute a frequency and
    // must use the bus frequency
    if (freqOffset!=nullptr)
    {
        *freqOffset = bus->getOutputLoc (sMode, frequencyInLocation);
    }
    return bus->getFreq (sD, sMode);
}

double GenModel::getAngle (const stateData & /*sD*/, const solverMode & /*sMode*/, index_t *angleOffset) const
{
    // there is no inertia in this gen model so it can't compute a frequency and
    // must use the bus frequency
    if (angleOffset!=nullptr)
    {
        *angleOffset = kNullLocation;
    }
    return kNullVal;
}

count_t GenModel::outputDependencyCount (index_t /*num*/, const solverMode & /*sMode*/) const { return 0; }
IOdata GenModel::getOutputs (const IOdata &inputs, const stateData & /*sD*/, const solverMode & /*sMode*/) const
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

double GenModel::getOutput (const IOdata &inputs,
                            const stateData & /*sD*/,
                            const solverMode & /*sMode*/,
                            index_t outNum) const
{
    double V = inputs[voltageInLocation];
    double Eft = inputs[genModelEftInLocation];
    if (V > 0.85)
    {
        if (outNum == PoutLocation)
        {
            return -inputs[genModelPmechInLocation];
        }
        if (outNum == QoutLocation)
        {
            return -Eft * Xd;
        }
    }
    else
    {
        if (outNum == PoutLocation)
        {
            return -inputs[genModelPmechInLocation] * V / 0.85;
        }
        if (outNum == QoutLocation)
        {
            return -Eft * Xd * V / 0.85;
        }
    }
    return kNullVal;
}

double GenModel::getOutput(
	index_t /*numOut*/) const
{	
	return kNullVal;
}

void GenModel::ioPartialDerivatives (const IOdata &inputs,
                                     const stateData & /*sD*/,
                                     matrixData<double> &md,
                                     const IOlocs &inputLocs,
                                     const solverMode & /*sMode*/)
{
    double V = inputs[voltageInLocation];

    if (V > 0.85)
    {
        md.assignCheckCol (QoutLocation, inputLocs[genModelEftInLocation], -Xd);

        if (inputLocs[voltageInLocation] != kNullLocation)
        {
            md.assign (PoutLocation, inputLocs[voltageInLocation], 0);
            md.assign (QoutLocation, inputLocs[voltageInLocation], 0);
        }
        md.assignCheckCol (PoutLocation, inputLocs[genModelPmechInLocation], -1.0);
    }
    else
    {
        double factor = V / 0.85;
        md.assignCheckCol (QoutLocation, inputLocs[genModelEftInLocation], -Xd * factor);

        if (inputLocs[voltageInLocation] != kNullLocation)
        {
            double Eft = inputs[genModelEftInLocation];
            md.assign (PoutLocation, inputLocs[voltageInLocation], -inputs[genModelPmechInLocation] / 0.85);
            md.assign (QoutLocation, inputLocs[voltageInLocation], -Eft * Xd / 0.85);
        }
        md.assignCheckCol (PoutLocation, inputLocs[genModelPmechInLocation], -factor);
    }
}

// set parameters
void GenModel::set (const std::string &param, const std::string &val) { return gridSubModel::set (param, val); }
void GenModel::set (const std::string &param, double val, gridUnits::units_t unitType)
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
            throw (unrecognizedParameter (param));
        }
        return;
    }

    if ((param == "xd") || (param == "xs"))
    {
        Xd = val;
    }
    else if (param == "rs")
    {
        Rs = val;
    }
    else if ((param == "base") || (param == "mbase"))
    {
        machineBasePower = val;
    }
    else
    {
        gridSubModel::set (param, val, unitType);
    }
}


static const std::vector<stringVec> inputNamesStr
{
	{ "voltage","v","volt" },
	{ "angle","ang","a" },
	{ "eft","e","field","exciter"},
	{ "pmech","power","p","mech" },
};

const std::vector<stringVec> &GenModel::inputNames() const
{
	return inputNamesStr;
}

static const std::vector<stringVec> outputNamesStr
{
	{ "e","field","exciter" },
};

const std::vector<stringVec> &GenModel::outputNames() const
{
	return outputNamesStr;
}


}//namespace griddyn
