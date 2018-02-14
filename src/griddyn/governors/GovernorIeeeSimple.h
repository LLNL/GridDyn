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

#ifndef GOVERNORIEEE_SIMPLE_H_
#define GOVERNORIEEE_SIMPLE_H_
#pragma once

#include "Governor.h"

#include "blocks/delayBlock.h"
#include "blocks/deadbandBlock.h"
#include "blocks/controlBlock.h"

namespace griddyn
{
namespace governors
{
class GovernorIeeeSimple : public Governor
{
public:
protected:
	double T3;                  //!< [s]    servo motor time constant
	double Pup;                 //!< [pu] upper ramp limit
	double Pdown;               //!< [pu] lower ramp limit
public:
	explicit GovernorIeeeSimple(const std::string &objName = "govIeeeSimple_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual ~GovernorIeeeSimple();
	virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;
	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual index_t findIndex(const std::string &field, const solverMode &sMode) const override;
	virtual void residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
	virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
	virtual void jacobianElements(const IOdata &inputs, const stateData &sD,
		matrixData<double> &md,
		const IOlocs &inputLocs, const solverMode &sMode) override;
	virtual void timestep(coreTime time, const IOdata &inputs, const solverMode &sMode) override;
	virtual void rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
	virtual void rootTrigger(coreTime time, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;
	//virtual void setTime(coreTime time){prevTime=time;};
};

}//namespace governors
}//namespace griddyn

#endif //GRIDGOVERNOR_H_
