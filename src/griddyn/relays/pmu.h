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

#ifndef GRIDDYN_PMU_H_
#define GRIDDYN_PMU_H_

#include "relays/sensor.h"

namespace griddyn {
namespace relays {
/** @brief class modeling a PMU
*/
class pmu : public sensor
{
public:
	enum pmu_flags
	{
		transmit_active = object_flag11,
	};
protected:
	coreTime transmissionRate = 30.0;  //!< the rate of data transmission
	parameter_t Tv = 0.1;  //!< filter time constant for the voltage measurement
	parameter_t Ttheta = 0.4;		//!< filter time constant for the angle measurement
	coreTime sampleRate = 720.0;  //!< [Hz] the actual sample time 

public:
	pmu(const std::string &objName = "pmu_$");
	coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void setFlag(const std::string &flag, bool val = true) override;
	virtual void set(const std::string &param, const std::string &val) override;

	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual double get(const std::string & param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

	virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;

	virtual void updateA(coreTime time) override;

};

}//namespace relays
}//namespace griddyn
#endif
