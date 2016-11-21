/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
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


#include "gridObjects.h"

class gridDynGenerator;

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
	gridDynGenerator *gen;

public:
	isocController(const std::string &objName = "ISOC_#");
	virtual gridCoreObject * clone(gridCoreObject *obj = nullptr) const override;
	virtual void objectInitializeA(gridDyn_time time0, unsigned long flags) override;

	virtual void objectInitializeB(const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

	virtual void updateA(double time) override;

	virtual void timestep(double ttime, const IOdata &args, const solverMode &sMode) override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual void setLimits(double max, double min);
	virtual void setLevel(double level);

	void setFreq(double freq);
	void deactivate();
	void activate(double time);

};

#endif

