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

#ifndef SINE_SOURCE_H_
#define SINE_SOURCE_H_

#include "sources/pulseSource.h"

namespace griddyn
{
namespace sources
{
/** A source generating a sinusoidal output
*/
class sineSource : public pulseSource
{
public:
	static const char pulsed_flag = object_flag4; //!< indicator that the source should be pulsed
protected:
	parameter_t frequency = 0.0;			//!<[Hz] frequency of an oscillation
	parameter_t phase = 0.0;				//!<[rad]  the offset angle
	coreTime lastCycle = negTime;		///!< time of the last cycle completion
	parameter_t Amp = 0.0;					//!< the amplitude of the pulse
	coreTime sinePeriod = maxTime;		//!< the period of the sinusoid
	parameter_t dfdt = 0.0;				///!<[Hz/s] the rate of change of frequency
	parameter_t dAdt = 0.0;				//!< [1/s] the rate of change of amplitude

public:
	sineSource(const std::string &objName = "sineSource_#", double startVal = 0.0);

	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;

	virtual void updateOutput(coreTime time) override;
	virtual double computeOutput(coreTime time) const override;
};
}//namespace sources
}//namespace griddyn

#endif

