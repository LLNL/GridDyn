
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
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

	virtual IOdata getOutputs(const IOdata &args, const stateData &sD, const solverMode &sMode) const override;
	virtual double getOutput(const IOdata &args, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

	virtual double getOutput(index_t num = 0) const override;

	virtual double getDoutdt(const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

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
	virtual void objectInitializeA(gridDyn_time time0, unsigned long flags) override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual void setFlag(const std::string &flag, bool val) override;
	
	virtual void setLevel(double val) override;
	virtual void updateA(gridDyn_time time) override;

	/** message processing function for use with communicators
	@param[in] sourceID  the source of the comm message
	@param[in] message the actual message to process
	*/
	virtual void receiveMessage(std::uint64_t sourceID, std::shared_ptr<commMessage> message);
};

#endif
