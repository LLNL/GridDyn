#pragma once

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#ifndef _FMI_OBJECTS_H_
#define _FMI_OBJECTS_H_

#include "fmiImport.h"

#include <type_traits>
#include <exception>


class fmiException : public std::exception
{
	virtual const char *what() const noexcept override
	{
		return "fmi Exception";
	}
};

class fmiDiscardException : public fmiException
{
	virtual const char *what() const noexcept override
	{
		return "return fmiDiscard";
	}
};

class fmiWarningException : public fmiException
{
	virtual const char *what() const noexcept override
	{
		return "return fmiWarning";
	}
};


class fmiErrorException : public fmiException
{
	virtual const char *what() const noexcept override
	{
		return "return fmiError";
	}
};

class fmiFatalException : public fmiException
{
	virtual const char *what() const noexcept override
	{
		return "return fmiFatal";
	}
};

/** base class containing the operation functions for working with an FMU*/
class fmi2Object
{
public:
	bool exceptionOnDiscard = true;
public:
	fmi2Object(fmi2Component cmp, std::shared_ptr<const fmiInfo> info, std::shared_ptr<const fmiCommonFunctions> comFunc);
	virtual ~fmi2Object();
	void setupExperiment(fmi2Boolean toleranceDefined, fmi2Real tolerance, fmi2Real startTime, fmi2Boolean stopTimeDefined, fmi2Real stopTime);
	virtual void setMode(fmuMode newMode);
	fmuMode getCurrentMode() const;
	void reset();
	
	template<typename T>
	T get(const std::string &param) const
	{
		auto ref = info->getVariableInfo(param);
		fmi2Status retval=fmi2Status::fmi2Discard;
		T ret(0);
		switch (ref.type.value())
		{
			case fmi_variable_type_t::real:
			{
				fmi2Real res;
				retval=commonFunctions->fmi2GetReal(comp, &(ref.valueRef), 1, &res);
				ret = T(res);
			}
			break;
			case fmi_variable_type_t::integer:
			{
				fmi2Integer res;
				retval=commonFunctions->fmi2GetInteger(comp, &(ref.valueRef), 1, &res);
				ret = T(res);
			}
			break;
			case fmi_variable_type_t::boolean:
			{
				fmi2Boolean res;
				retval=commonFunctions->fmi2GetBoolean(comp, &(ref.valueRef), 1, &res);
				ret = T(res);
			}
			break;
			case fmi_variable_type_t::enumeration:
			{
				fmi2Integer res;
				retval=commonFunctions->fmi2GetInteger(comp, &(ref.valueRef), 1, &res);
				ret = T(res);
			}
			break;
			default:
				retval = fmi2Status::fmi2Discard;
				break;
		}
			
		if (retval != fmi2Status::fmi2OK)
		{
			handleNonOKReturnValues(retval);
		}
		return ret;
	}
	

	void get(const fmiVariableSet &vrset, fmi2Real[]) const;
	void get(const fmiVariableSet &vrset, fmi2Integer[]) const;
	void get(const fmiVariableSet &vrset, fmi2String[]) const;

	void set(const fmiVariableSet &vrset, fmi2Integer[]);

	void set(const fmiVariableSet &vrset, fmi2Real[]);
	//!< concepts would be really useful here
	void set(const std::string &param, const char *val);
	void set(const std::string &param, const std::string &val);

	template<typename T>
	void set(const std::string &param, T val)
	{

		auto ref = info->getVariableInfo(param);
		fmi2Status ret = fmi2Status::fmi2Discard;
		switch (ref.type.value())
		{
		case fmi_variable_type_t::real:
		{
			fmi2Real val2 = static_cast<fmi2Real>(val);
			ret=commonFunctions->fmi2SetReal(comp, &(ref.valueRef), 1, &val2);
		}
		break;
		case fmi_variable_type_t::integer:
		{
			fmi2Integer val2 = static_cast<fmi2Integer>(val);
			ret=commonFunctions->fmi2SetInteger(comp, &(ref.valueRef), 1, &val2);
		}
		break;
		case fmi_variable_type_t::boolean:
		{
			fmi2Boolean val2 = static_cast<fmi2Boolean>(val);
			ret=commonFunctions->fmi2SetBoolean(comp, &(ref.valueRef), 1, &val2);
		}
		break;
		case fmi_variable_type_t::enumeration:
		{
			fmi2Integer val2 = static_cast<fmi2Integer>(val);
			ret=commonFunctions->fmi2SetInteger(comp, &(ref.valueRef), 1, &val2);
		}
		break;
		default:
			break;
		}
		if (ret != fmi2Status::fmi2OK)
		{
			handleNonOKReturnValues(ret);
		}
	}

	void getFMUState(fmi2FMUstate* FMUState);
	void setFMUState(fmi2FMUstate FMUState);

	size_t serializedStateSize(fmi2FMUstate FMUState);
	void serializeState(fmi2FMUstate FMUState, fmi2Byte[], size_t);

	void setInputs(const fmi2Real inputs[]);
	void getCurrentInputs(fmi2Real inputs[]);
	void getOutputs(fmi2Real outputs[]) const;
	fmi2Real getOutput(size_t outNum) const;
	void deSerializeState(const fmi2Byte[], size_t, fmi2FMUstate* FMUState);
	void getDirectionalDerivative(const fmi2ValueReference[], size_t, const fmi2ValueReference[], size_t, const fmi2Real[], fmi2Real[]);
	
	fmi2Real getPartialDerivative(int index_x, int index_y, double dx);
	void setOutputVariables(const std::vector<std::string> &outNames);
	void setOutputVariables(const std::vector<int> &outIndices);
	void setInputVariables(const std::vector<std::string> &inNames);
	void setInputVariables(const std::vector<int> &inIndices);

	fmiVariableSet getVariableSet(const std::string &variable) const;
	fmiVariableSet getVariableSet(int index) const;
	const fmiInfo *fmuInformation() const
	{
		return info.get();
	}
	int inputSize() const
	{
		return static_cast<int>(activeInputs.getVRcount());
	}
	int outputSize() const
	{
		return static_cast<int>(activeOutputs.getVRcount());
	}

	std::vector<std::string> getOutputNames() const;
	std::vector<std::string> getInputNames() const;

	bool isParameter(const std::string &param, fmi_variable_type_t type = fmi_variable_type_t::numeric);
	bool isVariable(const std::string &var, fmi_variable_type_t type = fmi_variable_type_t::numeric);
	std::shared_ptr<const fmiCommonFunctions> getFmiCommonFunctions() const
	{
		return commonFunctions;
	}

	fmi2Component getFmiComponent() const
	{
		return comp;
	}
protected:
	fmi2Component comp;
	fmuMode currentMode = fmuMode::instantiatedMode;
	std::shared_ptr<const fmiInfo> info;
	//structures for maintaining the inputs and outputs
	fmiVariableSet activeInputs;
	std::vector<int> activeInputIndices;
	fmiVariableSet activeOutputs;
	std::vector<int> activeOutputIndices;
	
	void handleNonOKReturnValues(fmi2Status retval) const;
	/** set the inputs to all the defined inputs*/
	void setDefaultInputs();
	/** set the outputs to be all defined outputs*/
	void setDefaultOutputs();
private:
	std::shared_ptr<const fmiCommonFunctions> commonFunctions;

};


template<>
std::string fmi2Object::get<std::string>(const std::string &param) const;

/** class containing the information for working with a model exchange object*/
class fmi2ModelExchangeObject : public fmi2Object
{
public:
	fmi2ModelExchangeObject(fmi2Component cmp, std::shared_ptr<const fmiInfo> info, std::shared_ptr<const fmiCommonFunctions> comFunc, std::shared_ptr<const fmiModelExchangeFunctions> MEFunc);
	void newDiscreteStates(fmi2EventInfo*);
	void completedIntegratorStep(fmi2Boolean, fmi2Boolean*, fmi2Boolean*);
	void setTime(fmi2Real time);
	void setStates(const fmi2Real states[]);
	void getDerivatives(fmi2Real deriv[]) const;
	void getEventIndicators(fmi2Real eventIndicators[]) const;
	void getStates(fmi2Real states[]) const;
	void getNominalsOfContinuousStates(fmi2Real nominalValues[]) const;

	virtual void setMode(fmuMode mode) override;
	size_t getNumberOfStates() const
	{
		return numStates;
	}
	size_t getNumberOfIndicators() const
	{
		return numIndicators;
	}

	std::shared_ptr<const fmiModelExchangeFunctions> getModelExchangeFunctions() const
	{
		return ModelExchangeFunctions;
	}
	std::vector<std::string> getStateNames() const;
private:
	size_t numStates=0;
	size_t numIndicators;
	std::shared_ptr<const fmiModelExchangeFunctions> ModelExchangeFunctions;
};

/** class containing the Information for working with a FMI coSimulation object*/
class fmi2CoSimObject : public fmi2Object
{
public:
	/**constructor*/
	fmi2CoSimObject(fmi2Component cmp, std::shared_ptr<const fmiInfo> info, std::shared_ptr<const fmiCommonFunctions> comFunc, std::shared_ptr<const fmiCoSimFunctions> csFunc);
	/** set the input derivatives of particular order
	@param[in] order
	@param[in] dIdt the input derivatives must be of the size as the number of inputs
	*/
	void setInputDerivatives(int order, const fmi2Real dIdt[]);
	/** get the output derivatives of a particular order
	@param[in] order the order of the derivatives to retrieve
	@param[out] dOdt the output derivatives
	*/
	void getOutputDerivatives(int order, fmi2Real dOdt[]) const;
	/** advance a time step 
	@param[in]
	@param[in] 
	@param[in] 
	*/
	void doStep(fmi2Real, fmi2Real, fmi2Boolean);
	/** cancel a pending time step*/
	void cancelStep();
	fmi2Real getLastStepTime() const;
	bool isPending();
	std::string getStatus() const;

	std::shared_ptr<const fmiCoSimFunctions> getCoSimulationFunctions() const
	{
		return CoSimFunctions;
	}
private:
	std::shared_ptr<const fmiCoSimFunctions> CoSimFunctions;
	bool stepPending;
};
#endif
