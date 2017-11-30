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

#ifndef FMI_GOVERNOR_H_
#define FMI_GOVERNOR_H_


#include "Governor.h"
#include "fmiMEWrapper.hpp"


namespace griddyn
{
namespace fmi
{

class fmiMESubModel;
/** class defining a governor with the dynamics through an FMI object*/
class fmiGovernor : public fmiMEWrapper<Governor>
{

public:
	fmiGovernor(const std::string &objName = "fmiExciter_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void set (const std::string &param, const std::string &val) override;
	virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

};

}//namespace fmi
}//namespace griddyn
#endif 