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

#ifndef FMI_COSIMLOADMODEL_3PHASE_H_
#define FMI_COSIMLOADMODEL_3PHASE_H_

#include "loads/ThreePhaseLoad.h"
#include "fmiCoSimWrapper.hpp"


namespace griddyn
{
namespace fmi
{
class fmiCoSimSubModel;

class fmiCoSimLoad3phase : public fmiCoSimWrapper<loads::ThreePhaseLoad>
{
public:
	enum threephasefmi_load_flags
	{
		ignore_voltage_angle = object_flag8,
		complex_voltage = object_flag9,
		current_output = object_flag10,
		complex_current_output = object_flag11,
	};
public:
	fmiCoSimLoad3phase(const std::string &objName="fmi3phase_$");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void set (const std::string &param, const std::string &val)override;
	virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit)override;
	virtual void setFlag(const std::string &flag, bool val) override;

	virtual void setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)override;

	virtual const std::vector<stringVec> &fmiInputNames() const override;

	virtual const std::vector<stringVec> &fmiOutputNames() const override;


};

}//namespace fmi
}//namespace griddyn
#endif 
