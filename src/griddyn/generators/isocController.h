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

#ifndef ISOCCONTROLLER_H_
#define ISOCCONTROLLER_H_


#include "gridSubModel.h"
namespace griddyn
{
class Generator;

class isocController : public gridSubModel
{
protected:
	double db = 0.005;
	double upStep = -0.01;
	double downStep = 0.02*0.25;
	double upPeriod = 1.0;
	double downPeriod = 0.25;
	double maxLevel = 1.0;
	double minLevel = -1.0;
	double lastFreq = 0;
	double integralTrigger = 0.02;
	double integratorLevel = 0;
	Generator *gen=nullptr;

public:
	explicit isocController(const std::string &objName = "ISOC_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;

	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

	virtual void updateA(coreTime time) override;

	virtual void timestep(coreTime time, const IOdata &inputs, const solverMode &sMode) override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual void setLimits(double max, double min);
	virtual void setLevel(double level);

	void setFreq(double freq);
	void deactivate();
	void activate(coreTime time);

};


}//namespace griddyn
#endif

