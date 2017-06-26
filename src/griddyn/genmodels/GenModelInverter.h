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

#ifndef GENMODELINVERTER_H_
#define GENMODELINVERTER_H_

#include "GenModel.h"

namespace griddyn
{
namespace genmodels
{
/** @brief model simulation implementing a simple inverter model
 the GenModel implements a very basic inverter model modeling the generator as a transmission line
with very fast angle adjustments to keep the mechanical input power balanced
*/
class GenModelInverter : public GenModel
{
public:
protected:
	double maxAngle = 89.0 * kPI / 180.0;      //!< maximum firing angle
	double minAngle = -89.0 * kPI / 180.0;      //!< minimum firing angle
public:
	//!< @brief default constructor
	explicit GenModelInverter(const std::string &objName = "genModel_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;
	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual stringVec localStateNames() const override;
	// dynamics

	virtual void residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

	
	virtual IOdata getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;

	using GenModel::getOutput;
	virtual double getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t outNum = 0) const override;

	virtual void jacobianElements(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;
	virtual void outputPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const solverMode &sMode) override;
	virtual count_t outputDependencyCount(index_t num, const solverMode &sMode) const override;
	virtual void ioPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;

	virtual void algebraicUpdate(const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha) override;
	/** helper function to get omega and its state location
	*/
	virtual double getFreq(const stateData &sD, const solverMode &sMode, index_t *freqOffset = nullptr) const override;
	virtual double getAngle(const stateData &sD, const solverMode &sMode, index_t *angleOffset = nullptr) const override;
	virtual void rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
	virtual void rootTrigger(coreTime time, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;
	virtual change_code rootCheck(const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;
private:
	void reCalcImpedences();
	/** @brief compute the real power output
	@param V voltage
	@param Ef Exciter field
	@param cosA  the cosine of the power angle
	@param sinA  the sine of the power angle
	@return the real power output;
	*/
	double realPowerCompute(double V, double Ef, double cosA, double sinA) const;
	/** @brief compute the reactive power output
	@param V voltage
	@param Ef Exciter field
	@param cosA  the cosine of the power angle
	@param sinA  the sine of the power angle
	@return the real power output;
	*/
	double reactivePowerCompute(double V, double Ef, double cosA, double sinA) const;
	double g = 0;
	double b = (1.0 / 1.05);
};

}//namespace genmodels
}//namespace griddyn
#endif //GRIDDYNGENMODEL_H_
