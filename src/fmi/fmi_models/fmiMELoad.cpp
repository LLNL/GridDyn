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


#include "fmiMELoad.h"
#include "fmi_import/fmiObjects.h"
#include "fmiMESubModel.h"
#include "core/coreObjectTemplates.hpp"
#include "gridBus.h"
#include "utilities/stringOps.h"
#include "core/coreExceptions.h"
#include <complex>

namespace griddyn
{
namespace fmi
{
fmiMELoad::fmiMELoad(const std::string &objName):fmiMEWrapper<Load>(objName)
{
	
}


coreObject * fmiMELoad::clone(coreObject *obj) const
{
	auto nobj = cloneBase<fmiMELoad, fmiMEWrapper<Load>>(this, obj);
	if (nobj==nullptr)
	{
		return obj;
	}

	return nobj;
	
}


void fmiMELoad::updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
    auto V = inputs;
    auto vc = std::polar(inputs[voltageInLocation], inputs[angleInLocation]);
    if (opFlags[complex_voltage])
    {
        V[0] = vc.real();
        V[1] = vc.imag();
    }
    V[1] *= 180.0 / kPI;
    fmisub->updateLocalCache(V, sD, sMode);
    auto res = fmisub->getOutputs(V, sD, sMode);
    //printf("V[%f,%f,%f,%f,%f,%f], I[%f,%f,%f,%f,%f,%f]\n", V[0], V[1], V[2], V[3], V[4], V[5], I[0], I[1], I[2], I[3], I[4], I[5]);
    
    auto act = outputTranslation(res, inputs);
    setP(act[PoutLocation]);
    setQ(act[QoutLocation]);

}

void fmiMELoad::set (const std::string &param, const std::string &val)
{
	if (param.empty())
	{

	}
	else
	{
		fmiMEWrapper<Load>::set(param, val);
	}
}
void fmiMELoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{
	if (param.empty())
	{

	}
	else
	{
		fmiMEWrapper<Load>::set(param, val,unitType);
	}
}


void fmiMELoad::setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
	fmiMEWrapper<Load>::setState(time, state, dstate_dt, sMode);
	auto out = fmisub->getOutputs(noInputs, emptyStateData, cLocalSolverMode);
    
    IOdata V = { bus->getVoltage(state, sMode), bus->getAngle(state, sMode) };
    auto act = outputTranslation(out, V);
	setP(act[PoutLocation]);
	setQ(act[QoutLocation]);
}


IOdata fmiMELoad::outputTranslation(const IOdata &fmiOutput, const IOdata &busV)
{
    auto busVoltage = std::complex<double>(busV[voltageInLocation], busV[angleInLocation]);
    IOdata powers(2);
    if (opFlags[current_output])
    {
        if (opFlags[complex_output])
        {
            auto I = std::complex<double>(fmiOutput[0], fmiOutput[1]);
            auto Pact = busVoltage*std::conj(I);
            powers[PoutLocation]=Pact.real();
            powers[QoutLocation] = Pact.imag();
        }
        else
        {
            auto I = std::polar(fmiOutput[0], fmiOutput[1] * kPI / 180.0);
            auto Pact = busVoltage*std::conj(I);
            powers[PoutLocation]=Pact.real();
            powers[QoutLocation] = Pact.imag();
        }
    }
    else
    {
        if (opFlags[complex_output])
        {
            powers[PoutLocation]= fmiOutput[PoutLocation];
            powers[QoutLocation] = fmiOutput[QoutLocation];
        }
        else
        {
            auto power = std::polar(fmiOutput[0], fmiOutput[1]);
            powers[PoutLocation]=power.real();
            powers[QoutLocation] = power.imag();
        }
    }
    return powers;
}
}//namespace fmi
}//namespace griddyn