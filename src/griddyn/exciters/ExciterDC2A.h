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

#ifndef EXCITERDC2A_H_
#define EXCITERDC2A_H_

#include "ExciterDC1A.h"
namespace griddyn
{
namespace exciters
{

/** @brief DC2A exciter
*/
class ExciterDC2A : public ExciterDC1A
{
protected:
public:
	explicit ExciterDC2A(const std::string &objName = "exciterDC2A_#");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
	virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
	virtual void rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
	virtual change_code rootCheck(const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;
protected:
	virtual void limitJacobian(double V, int VLoc, int refLoc, double cj, matrixData<double> &md) override;
};

}//namespace exciters
}//namespace griddyn



#endif //EXCITERDC2A_H_
