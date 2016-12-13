
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

#ifndef FNCS_GHOSTBUS_HEADER_
#define FNCS_GHSOSTBUS_HEADER_
#pragma once
#include "gridBus.h"

/** class meant to implement a Ghost Bus 
@details the bus gets its voltage from another simulation, otherwise it acts pretty much like an 
infinite bus*/
class fncsGhostBus :public gridBus
{
protected:
	std::string voltageKey;			//!< the key to send voltage
	std::string loadKey; 	//!< time series containing the load information
	gridUnits::units_t outUnits = gridUnits::defUnit;
public:
	explicit fncsGhostBus(const std::string &objName = "fncsGhostbus_$");

	~fncsGhostBus()
	{
	}
	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void pFlowObjectInitializeA(gridDyn_time time0, unsigned long flags) override;
	virtual void pFlowObjectInitializeB() override;

	virtual void updateA(gridDyn_time time) override;
	virtual gridDyn_time updateB() override;
	virtual void timestep(gridDyn_time ttime, const solverMode &sMode) override;
	virtual void setFlag(const std::string &param, bool val = true) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

private:
	void updateSubscription();

};

#endif