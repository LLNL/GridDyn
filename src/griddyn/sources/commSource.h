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

#ifndef COMM_SOURCE_H_
#define COMM_SOURCE_H_

#include "sources/rampSource.h"
#include "comms/commManager.h"

namespace griddyn
{

class Communicator;
class gridSimulation;
namespace sources
{
/** defining a source that can be connected to a communicator*/
class commSource : public rampSource
{
protected:
	std::shared_ptr<Communicator> commLink;       //!<communicator link
	gridSimulation *rootSim = nullptr;		//!< pointer to the root simulation
	comms::commManager cManager;		//!< comm manager object to build and manage the comm link
	double maxRamp = kBigNum;	//!< the maximum rate of change of the source
public:
	enum commSourceFlags
	{
		useRamp = object_flag3,
		no_message_reply = object_flag4,
	};
	commSource(const std::string &objName = "commSource_#");

	coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;

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

}//namespace sources
}//namespace griddyn

#endif

