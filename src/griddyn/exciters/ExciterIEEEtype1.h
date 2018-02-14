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

#ifndef EXCITER_IEEE_TYPE1_H_
#define EXCITER_IEEE_TYPE1_H_

#include "Exciter.h"
namespace griddyn
{
 namespace exciters
 {

/** @brief IEEE Type 1 exciter
*/
class ExciterIEEEtype1 : public Exciter
{
protected:
	parameter_t Ke = 1.0;            // [pu] self-excited field
	parameter_t Te = 1.0;            // [s]    exciter time constant
	parameter_t Kf = 0.03;            // [pu] stabilizer gain
	parameter_t Tf = 1.0;            // [s]    stabilizer time constant
	parameter_t Aex = 0.0;           // [pu] parameter saturation function
	parameter_t Bex = 0.0;           // [pu] parameter saturation function
public:
	explicit ExciterIEEEtype1(const std::string &objName = "exciterIEEEtype1_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;
	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual stringVec localStateNames() const override;

	virtual void timestep(coreTime time, const IOdata &inputs, const solverMode &sMode) override;
	virtual void residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
	virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
	//only called if the genModel is not present
	virtual void jacobianElements(const IOdata &inputs, const stateData &sD,
		matrixData<double> &md,
		const IOlocs &inputLocs, const solverMode &sMode) override;

	virtual void rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
	virtual change_code rootCheck(const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

};

}//namespace exciters

}//namespace griddyn



#endif //GRIDDYNEXCITER_H_
