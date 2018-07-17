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
#pragma once

#include "../griddyn/gridBus.h"
namespace griddyn
{
namespace helicsLib
{
class helicsCoordinator;
/** class meant to implement a Ghost Bus 
@details the bus gets its voltage from another simulation, otherwise it acts pretty much like an 
infinite bus*/
class helicsGhostBus :public gridBus
{
protected:
	std::string voltageKey;			//!< the key to send voltage
	std::string loadKey; 	//!< time series containing the load information
	int32_t voltageIndex; //!< reference indices for the voltage
	int32_t loadIndex; //!< reference indices for the load
	gridUnits::units_t outUnits = gridUnits::defUnit;
	helicsCoordinator *coord_ = nullptr;
public:
	explicit helicsGhostBus(const std::string &objName = "helicsGhostbus_$");

	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void pFlowObjectInitializeA(coreTime time0, uint32_t flags) override;
	virtual void pFlowObjectInitializeB() override;

	virtual void updateA(coreTime time) override;
	virtual coreTime updateB() override;
	virtual void timestep (coreTime ttime, const IOdata &inputs, const solverMode &sMode) override;
	virtual void setFlag(const std::string &param, bool val = true) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

private:
	void updateSubscription();

};

}// namespace helicsLib
} // namespace griddyn