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

#ifndef FMI_MELOADMODEL_H_
#define FMI_MELOADMODEL_H_

#include "griddyn/Load.h"
#include "fmiMEWrapper.hpp"


namespace griddyn
{
namespace fmi
{
class fmiMESubModel;

class fmiMELoad : public fmiMEWrapper<Load>
{
public:
	enum threephasefmi_load_flags
	{
		ignore_voltage_angle = object_flag8,
		complex_voltage = object_flag9,
		current_output = object_flag10,
		complex_output = object_flag11,
	};
public:
	fmiMELoad(const std::string &objName="fmiLoad_$");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	
	virtual void set (const std::string &param, const std::string &val)override;
	virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit)override;
	
    virtual void updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
	virtual void setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)override;
protected:
    IOdata outputTranslation(const IOdata &fmiOutput, const IOdata &busV);
};

}//namespace fmi
}//namespace griddyn
#endif 
