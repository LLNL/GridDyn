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

#ifndef FMI_EXCITER_H_
#define FMI_EXCITER_H_


#include "Exciter.h"
#include "fmiMEWrapper.hpp"


namespace griddyn
{
namespace fmi
{

class fmiMESubModel;

class fmiExciter : public fmiMEWrapper<Exciter>
{
public:
	fmiExciter(const std::string &objName = "fmiExciter_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void set (const std::string &param, const std::string &val) override;
	virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

};

}//namespace fmi
}//namespace griddyn
#endif 