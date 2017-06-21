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

#ifndef PULSE_SOURCE_H_
#define PULSE_SOURCE_H_

#include "Source.h"

namespace griddyn
{
namespace sources
{
/** @brief describe a pulsing source*/
class pulseSource : public Source
{

public:
	static const char invert_flag = object_flag3;  //
	enum class pulse_type_t
	{
		square, triangle, gaussian, biexponential, exponential, cosine, flattop, monocycle
	};
	pulse_type_t ptype = pulse_type_t::square;  //!< the type of the pulse
protected:
	coreTime period = maxTime;         //!<[s] pulse period
	double dutyCycle = 0.5;           //!<[%] pulse duty cycle
	double A = 0.0;                    //!< pulse amplitude
	coreTime cycleTime = maxTime;           //!<[s] the start time of the last cycle
	double baseValue;                  //!< the base level of the output
	double shift = 0;                 //!< storage for phase shift fraction (should be between 0 and 1)

public:
	pulseSource(const std::string &objName = "pulseSource_#", double startVal = 0.0);

	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;

	virtual void updateOutput(coreTime time) override;
	virtual double computeOutput(coreTime time) const override;
	virtual double getDoutdt(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

	virtual void setLevel(double val) override;
	/** function to calculate a value of the pulsing output
	@param[in] td the time change from the last update
	@return the output value
	*/
	double pulseCalc(double td) const;

};

}//namespace sources
}//namespace griddyn

#endif

