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

#ifndef GRIDDYN_COMPOUND_EVENT_PLAYER_H_
#define GRIDDYN_COMPOUND_EVENT_PLAYER_H_
#pragma once
// headers
//#include "griddyn.h"

#include "compoundEvent.h"
#include "utilities/timeSeriesMulti.hpp"


namespace griddyn
{
namespace events
{
/** event type allowing multiple changes on multiple object at a set of given time points*/
class compoundEventPlayer : public compoundEvent
{
protected:
	coreTime period = maxTime;  //!< period of the player
	timeSeriesMulti<double,coreTime> ts;	//!< the time series containing the data for the player 
	index_t currIndex = kNullLocation;	//!< the current index of the player
	std::string eFile;		//!< the file name
	std::vector<index_t> columns;
public:
	explicit compoundEventPlayer(const std::string &eventName);
	compoundEventPlayer();
	compoundEventPlayer(EventInfo &gdEI, coreObject *rootObject);
	virtual std::shared_ptr<Event> clone(std::shared_ptr<Event> gE = nullptr) const override;

	//virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject) override;

	virtual change_code trigger() override;
	virtual change_code trigger(coreTime time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	void setTime(coreTime time) override;
	void setTimeValue(coreTime time, double val);
	void setTimeValue(const std::vector<coreTime> &time, const std::vector<double> &val);
	void loadEventFile(const std::string &fileName);
	virtual std::string to_string() override;

	virtual bool setTarget(coreObject *gdo, const std::string &var = "") override;
	virtual void initialize() override;
protected:
	virtual void updateTrigger(coreTime time);

};
}//namespace events
}//namespace griddyn
#endif
