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

#ifndef FMI_LOADMODEL_H_
#define FMI_LOADMODEL_H_

#include "loadModels/zipLoad.h"

class fmiMESubModel;

class fmiLoad : public zipLoad
{
protected:
	std::string a_in;
	std::string v_in;
	std::string f_in;
	std::string P_out;
	std::string Q_out;

private:
	fmiMESubModel *fmisub = nullptr;
public:
	fmiLoad(std::string fmd="");
	~fmiLoad();
	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void pFlowObjectInitializeA (coreTime time0, unsigned long flags)override;
	virtual void dynObjectInitializeA (coreTime time, unsigned long flags)override;
	virtual void dynObjectInitializeB (const IOdata & inputs, const IOdata & desiredOutput, IOdata &fieldSet)override;

	virtual void set (const std::string &param, const std::string &val)override;
	virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit)override;
	virtual void getParameterStrings(stringVec &pstr, paramStringType pstype) const override;
	
	virtual void residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)override;

	virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)override;    //return D[0]=dP/dV D[1]=dP/dtheta,D[2]=dQ/dV,D[3]=dQ/dtheta

	virtual void setState(coreTime ttime, const double state[], const double dstate_dt[], const solverMode &sMode)override;
	virtual void outputPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const solverMode &sMode)override;
	virtual void ioPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)override;
	virtual void jacobianElements (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode) override;

	virtual void rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)override;
	virtual void rootTrigger(coreTime ttime, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode)override;

	virtual index_t findIndex(const std::string &field, const solverMode &sMode) const override;
	virtual void timestep(coreTime ttime, const IOdata &inputs, const solverMode &sMode)override;

	virtual IOdata getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
	virtual double getRealPower(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
	virtual double getReactivePower(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
	virtual double getRealPower(double V) const override;
	virtual double getReactivePower(double V) const override;
	virtual double getRealPower() const override;
	virtual double getReactivePower() const override;
protected:
	void setupFmiIo();
};
#endif 
