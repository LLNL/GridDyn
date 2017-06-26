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

#ifndef RAMP_SOURCE_H_
#define RAMP_SOURCE_H_

#include "Source.h"


namespace griddyn
{
namespace sources
{
/**@brief defines a ramping source
*/
class rampSource : public Source
{
protected:
	double mp_dOdt = 0.0;  //!< [1/s] the ramp rate of the output
public:
	rampSource(const std::string &objName = "rampSource_#", double startVal = 0.0);
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual double computeOutput(coreTime time) const override;
	virtual double getDoutdt(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;
	/** @brief clear the ramp (set it to 0)*/
	void clearRamp()
	{
		mp_dOdt = 0.0;
	}
};
}//namespace sources
}//namespace griddyn

#endif

