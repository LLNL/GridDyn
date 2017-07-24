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

#ifndef FMI_COSIMLOADMODEL_H_
#define FMI_COSIMLOADMODEL_H_

#include "Load.h"
#include "fmiCoSimWrapper.hpp"


namespace griddyn
{
namespace fmi
{
class fmiCoSimLoad : public fmiCoSimWrapper<Load>
{
public:
	enum threephasefmi_load_flags
	{
		ignore_voltage_angle=object_flag8,
		complex_voltage=object_flag9,
		current_output=object_flag10,
		complex_output=object_flag11,
	};
public:
	
	fmiCoSimLoad(const std::string &objName="fmiLoad_$");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)override;
	virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags)override;
	virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet)override;

	virtual void setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)override;

};

}//namespace fmi
}//namespace griddyn
#endif 
