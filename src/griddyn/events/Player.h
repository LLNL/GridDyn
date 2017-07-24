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

#ifndef GRIDDYN_PLAYER_H_
#define GRIDDYN_PLAYER_H_
#pragma once


#include "Event.h"
#include "utilities/timeSeriesMulti.hpp"

#include <future>
namespace griddyn
{
/** namespace to contain different event types that can be used in the griddyn system*/
namespace events
{
/** event player allowing a timeSeries of events to occur over numerous time points on a single object and field*/
class Player : public Event
{
protected:
	coreTime period = maxTime;  //!< period of the player
	timeSeries<double, coreTime> ts;	//!< the time series containing the data for the player 
	index_t currIndex = kNullLocation;	//!< the current index of the player
	index_t column = 0;	//!< the column in the file to use as the value set
	coreTime timeOffset = timeZero; //!< an offset to the time series time
	std::string eFile;		//!< the file name
	std::future<void> fileLoaded; //!< future indicating that the file operations have completed
	bool loadFileProcess = false;	//!< flag indicating that the files need to be loaded yet
public:
	explicit Player(const std::string &eventName);
	Player(coreTime time0 = 0.0, double loopPeriod = 0.0);
	Player(EventInfo &gdEI, coreObject *rootObject);
	virtual std::shared_ptr<Event> clone(std::shared_ptr<Event> gE = nullptr) const override;

	virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject) override;
	virtual change_code trigger() override;
	virtual change_code trigger(coreTime time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	void setTime(coreTime time) override;
	/** set a time and value pair as part of the player
	@param[in] time the time associated with the value
	@param[in] val the value to set at the given time
	*/
	void setTimeValue(coreTime time, double val);
	/** set a time and value set as part of the player
	@param[in] time the times associated with the value
	@param[in] val the values to set at the given times
	*/
	void setTimeValue(const std::vector<coreTime> &time, const std::vector<double> &val);
	/** load a file containing the time/value data for the player
	@param[in] filename the file to load
	*/
	void loadEventFile(const std::string &fileName);
	virtual std::string to_string() override;

	virtual bool setTarget(coreObject *gdo, const std::string &var = "") override;

	virtual void initialize() override;
	//friendly helper functions for sorting
protected:
	/** update the trigger time*/
	virtual void updateTrigger(coreTime time);
	/** queue up the next value to set when the event is triggered*/
	virtual void setNextValue();
};
}//namespace events
}//namespace griddyn
#endif
