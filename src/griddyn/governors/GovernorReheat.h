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

#ifndef GOVERNOR_REHEAT_H_
#define GOVERNOR_REHEAT_H_
#pragma once

#include "Governor.h"


namespace griddyn
{
namespace governors
{
class GovernorReheat : public Governor
{

public:
protected:
	double T3;                  //!< [s]    Transient gain time constant
	double T4;                          //!< [s]    Power fraction time constant
	double T5;                          //!< [s]    Reheat time constant
public:
	explicit GovernorReheat(const std::string &objName = "govReheat_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual ~GovernorReheat();
	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual index_t findIndex(const std::string &field, const solverMode &sMode) const override;
	virtual void residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
	virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
	virtual void jacobianElements(const IOdata &inputs, const stateData &sD,
		matrixData<double> &md,
		const IOlocs &inputLocs, const solverMode &sMode) override;

	//virtual void setTime (coreTime time) const{prevTime=time;};
};

}//namespace governors
}//namespace griddyn

#endif //GOVERNOR_REHEAT_H_
