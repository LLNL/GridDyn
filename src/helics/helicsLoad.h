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

#ifndef HELICS_LOAD_HEADER_
#define HELICS_LOAD_HEADER_

#include "loads/rampLoad.h"
namespace griddyn
{
namespace helicsLib
{
class helicsCoordinator;
enum class helicsValueType :char;

class helicsLoad:public loads::rampLoad
{
public:
	enum helics_load_flags
	{
		use_ramp = object_flag8,
		predictive_ramp = object_flag9,
		initial_query = object_flag10,
	};
protected:
	std::string voltageKey;			//!< the key to send voltage
	std::string loadKey; 	//!< time series containing the load information
	int32_t voltageIndex;  //!< index for sending the voltage data
	int32_t loadIndex;  //!< index for getting the load data
	gridUnits::units_t inputUnits = gridUnits::MW;
	helicsValueType loadType;
	helicsValueType voltageType;
	double scaleFactor = 1.0;			//!< scaling factor on the load
	helicsCoordinator *coord=nullptr;  //!< the coordinator for interaction with the helics interface
private:
	double prevP = 0;
	double prevQ = 0;
public:
	explicit helicsLoad(const std::string &objName = "helicsLoad_$");

	coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void pFlowObjectInitializeA(coreTime time0, uint32_t flags) override;
	virtual void pFlowObjectInitializeB() override;

	virtual void updateA(coreTime time) override;
	virtual coreTime updateB() override;
	virtual void timestep(coreTime ttime, const IOdata &inputs, const solverMode &sMode) override;
	virtual void setFlag(const std::string &param, bool val = true) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
private:
	void setSubscription();
};

}// namespace helicsLib
} // namespace griddyn
#endif