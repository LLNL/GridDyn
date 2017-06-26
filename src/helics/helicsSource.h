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

#ifndef HELICS_SOURCE_HEADER_
#define HELICS_SOURCE_HEADER_

#include "sources/rampSource.h"

namespace griddyn
{
namespace helicsLib
{
class helicsCoordinator;

enum class helicsValueType:char;

class helicsSource :public sources::rampSource
{
public:
	enum helics_source_flags
	{
		use_ramp = object_flag8,
		predictive_ramp = object_flag9,
		initial_query = object_flag10,
	};
protected:
	std::string valKey; 	//!< time series containing the load information
	parameter_t scaleFactor = 1.0;			//!< scaling factor on the load
	gridUnits::units_t inputUnits = gridUnits::defUnit;  //!< units of the incoming data
	gridUnits::units_t outputUnits = gridUnits::defUnit; //!< units of the outgoing data
	helicsValueType valueType;
	int32_t valueIndex=-1;
	helicsCoordinator *coord_=nullptr;  //!< pointer to the helics coordinator
private:
	double prevVal = 0;
public:
	explicit helicsSource(const std::string &objName = "helicsSource_$");

	coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void pFlowObjectInitializeA(coreTime time0, uint32_t flags) override;
	virtual void pFlowObjectInitializeB() override;
	virtual void dynObjectInitializeA(coreTime time0, uint32_t flags) override;

	virtual void updateA(coreTime time) override;
	virtual void timestep(coreTime ttime, const IOdata &inputs, const solverMode &sMode) override;
	virtual void setFlag(const std::string &param, bool val = true) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
private:
	void updateSubscription();
};

}// namespace helicsLib
} // namespace griddyn
#endif