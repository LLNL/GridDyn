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

#ifndef GENMODEL4_H_
#define GENMODEL4_H_

#include "GenModel3.h"
#include "utilities/saturation.h"
namespace griddyn
{
namespace genmodels
{

class GenModel4 : public GenModel3
{

protected:
	double Xqp = 0.35;               //!< [pu] q-axis transient reactance
	double Tqop = 1.0;                //!< [s]    q-axis time constant
	double S10 = 1.0;           //!< the saturation S (1.0) const
	double S12 = 1.0;                    //!< the saturation S(1.2)
	utilities::saturation sat;                     //!< saturation function object
public:
	explicit GenModel4(const std::string &objName = "genModel4_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;
	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual stringVec localStateNames() const override;
	// dynamics
	virtual void timestep(coreTime time, const IOdata &inputs, const solverMode &sMode) override;
	virtual void residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
	virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
	virtual void jacobianElements(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;

	virtual void algebraicUpdate(const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha) override;
};


}//namespace genmodels
}//namespace griddyn
#endif //GENMODEL4_H_
