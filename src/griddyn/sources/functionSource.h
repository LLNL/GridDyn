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

#ifndef FUNCTION_SOURCE_H_
#define FUNCTION_SOURCE_H_

#include "Source.h"
#include <functional>

namespace griddyn
{
namespace sources
{
/** class allowing the specification of an arbitrary function as the source generator*/
class functionSource : public Source
{
private:
	std::function<double(double)> sourceFunc;

public:
	functionSource(const std::string &objName = "functionsource_#");

	coreObject * clone(coreObject *obj = nullptr) const override;

	virtual IOdata getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
	virtual double getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

	virtual double getOutput(index_t num = 0) const override;

	virtual double getDoutdt(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

	void setFunction(std::function<double(double)> calcFunc);


};
}//namespace sources
}//namespace griddyn

#endif

