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

#ifndef FUNCTION_BLOCK_H_
#define FUNCTION_BLOCK_H_
#pragma once

#include "Block.h"
#include <functional>

namespace griddyn
{
namespace blocks
{
/** @brief class implementing a function operation on the input
a wide assortment of functions are available including trig, logs, and other common math operations*/
class functionBlock : public Block
{

public:
	//!< flags for function block
	enum functionblock_flags
	{
		uses_constantarg = object_flag10,              //!< flag indicating that the function should use a constant argument for the second argument of functions
	};
protected:
	std::function<double(double)> fptr;        //!< function object for single argument functions
	std::function<double(double, double)> fptr2;        //!< function object for multiple argument functions
	double gain = 1.0;         //!< extra gain factor
	double arg2 = 0.0;         //!< second argument for 2 argument functions
public:
	/** @brief default constructor*/
	functionBlock();
	/** @brief alternate constructor
	@param[in] function the name of the function as a string
	*/
	explicit functionBlock(const std::string &functionName);
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	//virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

	//virtual void derivElements(double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
	virtual void algElements(double input, const stateData &sD, double update[], const solverMode &sMode) override;


	//only called if the genModel is not present
	virtual void jacElements(double input, double didt, const stateData &sD, matrixData<double> &md, index_t argLoc, const solverMode &sMode) override;
	virtual double step(coreTime time, double input) override;
	//virtual void setTime(coreTime time){prevTime=time;};
protected:
	/** @brief load the function objects from a string input
	@param[in] functionName the name of the function as a string*/
	void setFunction(const std::string &functionName);
};
}//namespace blocks
}//namespace griddyn
#endif //FUNCTION_BLOCK_H_

