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

#ifndef FMI_GENMODEL_H_
#define FMI_GENMODEL_H_


#include "griddyn/GenModel.h"
#include "fmiMEWrapper.hpp"


namespace griddyn
{
namespace fmi
{

class fmiMESubModel;

class fmiGenModel : public fmiMEWrapper<GenModel>
{

public:
	fmiGenModel(const std::string &objName = "fmiGenModel_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	//virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
	//virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

	virtual void set (const std::string &param, const std::string &val) override;
	virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

};

}//namespace fmi
}//namespace griddyn
#endif 