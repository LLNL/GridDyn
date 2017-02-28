
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

#ifndef OTHERSOURCES_H_
#define OTHERSOURCES_H_


#include "sourceTypes.h"
#include "comms/commManager.h"
#include <functional>

/** class allowing the specification of an arbitrary function as the source generator*/
class functionSource : public gridSource
{
private:
	std::function<double(double)> sourceFunc;

public:
	functionSource(const std::string &objName = "functionsource_#");

	coreObject * clone(coreObject *obj = nullptr) const override;

	virtual IOdata getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
	virtual double getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

	virtual double getOutput(index_t num = 0) const override;

	virtual double getDoutdt(const IOdata & inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

	void setFunction(std::function<double(double)> calcFunc);


};

class gridCommunicator;
class gridSimulation;
/** defining a source that can be connected to a communicator*/
class commSource : public rampSource
{
protected:
	std::shared_ptr<gridCommunicator> commLink;       //!<communicator link
	gridSimulation *rootSim = nullptr;		//!< pointer to the root simulation
	commManager cManager;		//!< comm manager object to build and manage the comm link
	double maxRamp = kBigNum;	//!< the maximum rate of change of the source
public:
	enum commSourceFlags
	{
		useRamp = object_flag3,
		no_message_reply=object_flag4,
	};
	commSource(const std::string &objName = "commSource_#");

	coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void dynObjectInitializeA(coreTime time0, unsigned long flags) override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual void setFlag(const std::string &flag, bool val) override;
	
	virtual void setLevel(double val) override;
	virtual void updateA(coreTime time) override;

	/** message processing function for use with communicators
	@param[in] sourceID  the source of the comm message
	@param[in] message the actual message to process
	*/
	virtual void receiveMessage(std::uint64_t sourceID, std::shared_ptr<commMessage> message);
};


class basicBlock;

class blockSource : public gridSource
{
private:
	gridSource *src=nullptr;
	basicBlock *blk=nullptr;
	double maxStepSize = kBigNum;
public:
	blockSource(const std::string &objName = "functionsource_#");

	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void add(coreObject *obj) override;
	virtual void remove(coreObject *obj) override;
protected:
	 virtual void dynObjectInitializeA (coreTime time0, unsigned long flags) override;

  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet) override;
public:
  virtual void setFlag (const std::string &flag, bool val) override;
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual double get(const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

  //virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode);

virtual void residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;

virtual void algebraicUpdate(const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha) override;

virtual void jacobianElements(const IOdata &inputs, const stateData &sD,
	matrixData<double> &ad,
	const IOlocs &inputLocs, const solverMode &sMode) override;

virtual void timestep(coreTime ttime, const IOdata &inputs, const solverMode &sMode) override;

virtual void rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
virtual void rootTrigger(coreTime ttime, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;
virtual change_code rootCheck(const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

virtual void updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;

virtual IOdata getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
virtual double getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

virtual double getOutput(index_t num = 0) const override;

virtual double getDoutdt(const IOdata & inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;


virtual void setLevel(double newLevel) override;

virtual coreObject* find(const std::string &object) const override;
virtual coreObject* getSubObject(const std::string & typeName, index_t num) const override;

};


class grabberSet;
/** source to grab data from another location and use it in another context*/
class grabberSource: public rampSource
{
private:
std::unique_ptr<grabberSet> gset; //!< the grabberSet to get the data
std::string field;  //!< the field to grab
std::string target; //!< the name of the target

public:
grabberSource(const std::string &objName = "grabbersource_#");
~grabberSource();
protected:
	virtual void dynObjectInitializeA(coreTime time0, unsigned long flags) override;

	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet) override;

virtual coreObject * clone(coreObject *obj = nullptr) const override;

void updateField(const std::string &field);

void updateTarget(const std::string &field);

void updateTarget(coreObject *obj);

virtual void setFlag(const std::string &flag, bool val) override;
virtual void set(const std::string &param, const std::string &val) override;
virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
virtual double get(const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

virtual IOdata getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
virtual double getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

virtual double getOutput(index_t num = 0) const override;

virtual double getDoutdt(const IOdata &inputs,const stateData &sD, const solverMode &sMode, index_t num = 0) const override;
};

#endif
