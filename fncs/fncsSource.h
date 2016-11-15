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

#ifndef FNCS_SOURCE_HEADER_
#define FNCS_SOURCE_HEADER_

#include "sourceModels/otherSources.h"

class fncsSource :public rampSource
{
public:
	enum fncs_source_flags
	{
		use_ramp = object_flag8,
		predictive_ramp = object_flag9,
		initial_query = object_flag10,
	};
protected:
	std::string valKey; 	//!< time series containing the load information
	gridUnits::units_t inputUnits = gridUnits::defUnit;  //!< units of the incoming data
	gridUnits::units_t outputUnits = gridUnits::defUnit; //!< units of the outgoing data
	double scaleFactor = 1.0;			//!< scaling factor on the load
private:
	double prevVal = 0;
public:
	explicit fncsSource(const std::string &objName = "fncsSource_$");

	~fncsSource()
	{
	}
	gridCoreObject * clone(gridCoreObject *obj = nullptr) const override;
	virtual void objectInitializeA(gridDyn_time time0, unsigned long flags) override;

	virtual void updateA(gridDyn_time time) override;
	virtual void timestep(gridDyn_time ttime, const IOdata &args, const solverMode &sMode) override;
	virtual void setFlag(const std::string &param, bool val = true) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

};
#endif