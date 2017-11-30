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

#ifndef _FMI_RUNNER_H_
#define _FMI_RUNNER_H_

#include "fileInput/gridDynRunner.h"
#include "fmi/FMI2/fmi2FunctionTypes.h"

#include "core/coreOwningPtr.hpp"
#include <bitset>
#include <future>

namespace griddyn
{
class readerInfo;
namespace fmi
{
class fmiCoordinator;

/** fmiRunner is the execution object for executing under an fmi context
it inherits from gridDynRunner and adds some extra features necessary for executing under an fMI
*/
class fmiRunner : public GriddynRunner
{
	private:
		coreOwningPtr<fmiCoordinator> coord; //!< the coordinator object for managing object that manage the fmi inputs and outputs
		std::bitset<6> loggingCategories;  //!< indicators of which logging categories to use
		bool runAsync_ = false;	//!< indicator that we should run asynchronously
		std::future<void> async_retFMI;	//!< the future object corresponding to the asyncrhonous operation
public:
	fmiRunner(const std::string &name, const std::string &resourceLocations, const fmi2CallbackFunctions* functions);
	~fmiRunner();
	//most of the stuff that would be in here is dealt with in the constructor and has to be handled differently in the FMU
private:
	using GriddynRunner::Initialize;
public:
	virtual int Initialize(int argc, char *argv[]) override final;
	

	virtual coreTime Run() override;
	 
	/** update the FMI outputs*/
	void UpdateOutputs();

	virtual coreTime Step(coreTime time) override;
	virtual void StepAsync(coreTime time) override;
	virtual void Finalize() override;
private:
	using GriddynRunner::Reset;
public:
	virtual int Reset() override;

	id_type_t GetID() const;

	virtual bool Set(index_t vr, double val);
	virtual bool SetString(index_t vr, const char *s);
	virtual double Get(index_t vr);

	void setLoggingCategories(std::bitset<6> logCat)
	{
		loggingCategories = logCat;
	}
	/** check whether the runner is set to run asynchronously*/
	bool runAsynchronously() const
	{
		return runAsync_;
	}
	
	void setAsynchronousMode(bool async)
	{
		runAsync_ = (stepFinished != nullptr) ? async : false;	
	}
	/** check whether an asynchronous step call is finished*/
	bool isFinished() const;
	/** locate a value reference from a name*/
	index_t findVR(const std::string &varName) const;

	void logger(int level, const std::string &logMessage);

private:
	//these are used for logging
	fmi2CallbackLogger   loggerFunc=nullptr;  //reference to the logger function
	fmi2StepFinished     stepFinished = nullptr; //!< reference to a step finished function
public:
	fmi2Component fmiComp;
	std::string identifier;
	std::string recordDirectory;
	std::string resource_loc;
};

}//namespace fmi
}//namespace griddyn
#endif
